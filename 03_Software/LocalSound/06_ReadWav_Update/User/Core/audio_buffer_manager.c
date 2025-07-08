/**
 * @file audio_buffer_manager.c
 * @brief Audio buffer management system with overflow protection
 * @description Addresses "Core obliterated Data" errors and audio stability issues
 * @author Beta Test Team
 * @date 2024-12-20
 */

#include "audio_buffer_manager.h"
#include "beta_test_framework.h"
#include "user_bsp.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "queue.h"

/* Private defines */
#define AUDIO_BUFFER_WATERMARK_LOW  25   // 25% of buffer size
#define AUDIO_BUFFER_WATERMARK_HIGH 75   // 75% of buffer size
#define AUDIO_NOTIFICATION_TIMEOUT_MS 100
#define AUDIO_BUFFER_MONITOR_INTERVAL_MS 50

/* Private variables */
static AudioBufferManager_t audio_manager;
static StaticSemaphore_t audio_mutex_buffer;
static SemaphoreHandle_t audio_mutex;
static TaskHandle_t audio_monitor_task_handle;
static QueueHandle_t audio_event_queue;

/* Private function prototypes */
static void audio_buffer_monitor_task(void *pvParameters);
static void audio_check_buffer_levels(void);
static void audio_handle_buffer_overflow(void);
static void audio_handle_buffer_underflow(void);
static uint32_t audio_calculate_buffer_usage(void);

/**
 * @brief Initialize audio buffer management system
 * @param config Configuration parameters
 * @retval AUDIO_OK if successful
 */
AudioResult_t AudioBuffer_Init(AudioBufferConfig_t *config)
{
    if (config == NULL) {
        return AUDIO_ERROR_INVALID_PARAM;
    }
    
    // Initialize audio manager structure
    memset(&audio_manager, 0, sizeof(AudioBufferManager_t));
    
    // Copy configuration
    audio_manager.config = *config;
    
    // Allocate audio buffers
    audio_manager.primary_buffer = (int16_t*)malloc(config->buffer_size * sizeof(int16_t));
    audio_manager.secondary_buffer = (int16_t*)malloc(config->buffer_size * sizeof(int16_t));
    audio_manager.temp_buffer = (int16_t*)malloc(config->buffer_size * sizeof(int16_t));
    
    if (audio_manager.primary_buffer == NULL || 
        audio_manager.secondary_buffer == NULL || 
        audio_manager.temp_buffer == NULL) {
        Uart1_SendData("[AUDIO_MGR] Failed to allocate audio buffers\r\n");
        return AUDIO_ERROR_NO_MEMORY;
    }
    
    // Initialize buffer pointers and counters
    audio_manager.write_index = 0;
    audio_manager.read_index = 0;
    audio_manager.buffer_count = 0;
    audio_manager.active_buffer = 0;  // Start with primary buffer
    
    // Create mutex for thread-safe buffer access
    audio_mutex = xSemaphoreCreateMutexStatic(&audio_mutex_buffer);
    if (audio_mutex == NULL) {
        Uart1_SendData("[AUDIO_MGR] Failed to create audio mutex\r\n");
        return AUDIO_ERROR_OS;
    }
    
    // Create event queue for audio events
    audio_event_queue = xQueueCreate(10, sizeof(AudioEvent_t));
    if (audio_event_queue == NULL) {
        Uart1_SendData("[AUDIO_MGR] Failed to create audio event queue\r\n");
        return AUDIO_ERROR_OS;
    }
    
    // Create buffer monitor task
    BaseType_t task_result = xTaskCreate(
        audio_buffer_monitor_task,
        "AudioMonitor",
        configMINIMAL_STACK_SIZE * 2,
        NULL,
        tskIDLE_PRIORITY + 2,
        &audio_monitor_task_handle
    );
    
    if (task_result != pdPASS) {
        Uart1_SendData("[AUDIO_MGR] Failed to create audio monitor task\r\n");
        return AUDIO_ERROR_OS;
    }
    
    // Initialize statistics
    audio_manager.stats.init_time = xTaskGetTickCount();
    
    Uart1_SendData("[AUDIO_MGR] Audio buffer manager initialized successfully\r\n");
    Uart1_SendData("[AUDIO_MGR] Buffer size: %lu samples\r\n", config->buffer_size);
    Uart1_SendData("[AUDIO_MGR] Low watermark: %lu%%, High watermark: %lu%%\r\n", 
                  AUDIO_BUFFER_WATERMARK_LOW, AUDIO_BUFFER_WATERMARK_HIGH);
    
    return AUDIO_OK;
}

/**
 * @brief Write audio data to buffer with overflow protection
 * @param data Audio data to write
 * @param samples Number of samples to write
 * @retval AUDIO_OK if successful
 */
AudioResult_t AudioBuffer_Write(int16_t *data, uint32_t samples)
{
    if (data == NULL || samples == 0) {
        return AUDIO_ERROR_INVALID_PARAM;
    }
    
    // Take mutex for thread-safe access
    if (xSemaphoreTake(audio_mutex, pdMS_TO_TICKS(AUDIO_NOTIFICATION_TIMEOUT_MS)) != pdTRUE) {
        audio_manager.stats.mutex_timeouts++;
        return AUDIO_ERROR_TIMEOUT;
    }
    
    // Check if buffer has enough space
    uint32_t free_space = audio_manager.config.buffer_size - audio_manager.buffer_count;
    if (samples > free_space) {
        audio_manager.stats.buffer_overflows++;
        audio_handle_buffer_overflow();
        
        // Truncate data to fit available space
        samples = free_space;
        if (samples == 0) {
            xSemaphoreGive(audio_mutex);
            return AUDIO_ERROR_BUFFER_FULL;
        }
    }
    
    // Get current active buffer
    int16_t *current_buffer = (audio_manager.active_buffer == 0) ? 
                             audio_manager.primary_buffer : 
                             audio_manager.secondary_buffer;
    
    // Copy data to buffer with wrap-around handling
    for (uint32_t i = 0; i < samples; i++) {
        current_buffer[audio_manager.write_index] = data[i];
        audio_manager.write_index = (audio_manager.write_index + 1) % audio_manager.config.buffer_size;
        audio_manager.buffer_count++;
    }
    
    audio_manager.stats.total_samples_written += samples;
    audio_manager.stats.successful_writes++;
    
    xSemaphoreGive(audio_mutex);
    
    // Check buffer levels and trigger events if needed
    audio_check_buffer_levels();
    
    return AUDIO_OK;
}

/**
 * @brief Read audio data from buffer with underflow protection
 * @param data Buffer to read into
 * @param samples Number of samples to read
 * @param samples_read Actual number of samples read
 * @retval AUDIO_OK if successful
 */
AudioResult_t AudioBuffer_Read(int16_t *data, uint32_t samples, uint32_t *samples_read)
{
    if (data == NULL || samples == 0 || samples_read == NULL) {
        return AUDIO_ERROR_INVALID_PARAM;
    }
    
    *samples_read = 0;
    
    // Take mutex for thread-safe access
    if (xSemaphoreTake(audio_mutex, pdMS_TO_TICKS(AUDIO_NOTIFICATION_TIMEOUT_MS)) != pdTRUE) {
        audio_manager.stats.mutex_timeouts++;
        return AUDIO_ERROR_TIMEOUT;
    }
    
    // Check available data
    uint32_t available_samples = audio_manager.buffer_count;
    if (available_samples == 0) {
        audio_manager.stats.buffer_underflows++;
        audio_handle_buffer_underflow();
        xSemaphoreGive(audio_mutex);
        return AUDIO_ERROR_BUFFER_EMPTY;
    }
    
    // Read only available samples
    uint32_t samples_to_read = (samples > available_samples) ? available_samples : samples;
    
    // Get current active buffer
    int16_t *current_buffer = (audio_manager.active_buffer == 0) ? 
                             audio_manager.primary_buffer : 
                             audio_manager.secondary_buffer;
    
    // Copy data from buffer with wrap-around handling
    for (uint32_t i = 0; i < samples_to_read; i++) {
        data[i] = current_buffer[audio_manager.read_index];
        audio_manager.read_index = (audio_manager.read_index + 1) % audio_manager.config.buffer_size;
        audio_manager.buffer_count--;
    }
    
    *samples_read = samples_to_read;
    audio_manager.stats.total_samples_read += samples_to_read;
    audio_manager.stats.successful_reads++;
    
    xSemaphoreGive(audio_mutex);
    
    // Check buffer levels and trigger events if needed
    audio_check_buffer_levels();
    
    return AUDIO_OK;
}

/**
 * @brief Handle task notifications with overflow detection
 * @param notification_handle Task handle to notify
 * @param notification_value Value to send
 * @retval AUDIO_OK if successful
 */
AudioResult_t AudioBuffer_HandleNotification(TaskHandle_t notification_handle, uint32_t notification_value)
{
    if (notification_handle == NULL) {
        return AUDIO_ERROR_INVALID_PARAM;
    }
    
    // Use task notification with timeout to detect overruns
    uint32_t notification_count = ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(AUDIO_NOTIFICATION_TIMEOUT_MS));
    
    if (notification_count > 1) {
        // Multiple notifications received - potential "Core obliterated Data" condition
        audio_manager.stats.notification_overruns++;
        BetaTest_IncrementNotificationOverruns();
        
        Uart1_SendData("[AUDIO_MGR] Notification overrun detected: %lu notifications\r\n", notification_count);
        
        // Send audio event
        AudioEvent_t event = {
            .type = AUDIO_EVENT_NOTIFICATION_OVERRUN,
            .timestamp = xTaskGetTickCount(),
            .data = notification_count
        };
        xQueueSend(audio_event_queue, &event, 0);
        
        return AUDIO_ERROR_OVERRUN;
    } else if (notification_count == 1) {
        // Normal single notification
        audio_manager.stats.successful_notifications++;
        return AUDIO_OK;
    } else {
        // Timeout - no notification received
        audio_manager.stats.notification_timeouts++;
        return AUDIO_ERROR_TIMEOUT;
    }
}

/**
 * @brief Switch to alternate buffer for double buffering
 * @retval AUDIO_OK if successful
 */
AudioResult_t AudioBuffer_SwitchBuffer(void)
{
    if (xSemaphoreTake(audio_mutex, pdMS_TO_TICKS(AUDIO_NOTIFICATION_TIMEOUT_MS)) != pdTRUE) {
        return AUDIO_ERROR_TIMEOUT;
    }
    
    // Switch to alternate buffer
    audio_manager.active_buffer = (audio_manager.active_buffer == 0) ? 1 : 0;
    
    // Reset buffer indices for new buffer
    audio_manager.write_index = 0;
    audio_manager.read_index = 0;
    audio_manager.buffer_count = 0;
    
    audio_manager.stats.buffer_switches++;
    
    xSemaphoreGive(audio_mutex);
    
    Uart1_SendData("[AUDIO_MGR] Switched to buffer %d\r\n", audio_manager.active_buffer);
    
    return AUDIO_OK;
}

/**
 * @brief Audio buffer monitor task
 * @param pvParameters Task parameters (unused)
 */
static void audio_buffer_monitor_task(void *pvParameters)
{
    (void)pvParameters;
    
    TickType_t last_wake_time = xTaskGetTickCount();
    AudioEvent_t event;
    
    Uart1_SendData("[AUDIO_MGR] Audio buffer monitor task started\r\n");
    
    while (1) {
        // Check for audio events
        if (xQueueReceive(audio_event_queue, &event, 0) == pdTRUE) {
            switch (event.type) {
                case AUDIO_EVENT_BUFFER_OVERFLOW:
                    Uart1_SendData("[AUDIO_MGR] Buffer overflow at %lu\r\n", event.timestamp);
                    BetaTest_IncrementAudioDropouts();
                    break;
                    
                case AUDIO_EVENT_BUFFER_UNDERFLOW:
                    Uart1_SendData("[AUDIO_MGR] Buffer underflow at %lu\r\n", event.timestamp);
                    BetaTest_IncrementAudioDropouts();
                    break;
                    
                case AUDIO_EVENT_NOTIFICATION_OVERRUN:
                    Uart1_SendData("[AUDIO_MGR] Notification overrun: %lu notifications\r\n", event.data);
                    break;
                    
                case AUDIO_EVENT_BUFFER_LEVEL_HIGH:
                    Uart1_SendData("[AUDIO_MGR] Buffer level high: %lu%%\r\n", event.data);
                    break;
                    
                case AUDIO_EVENT_BUFFER_LEVEL_LOW:
                    Uart1_SendData("[AUDIO_MGR] Buffer level low: %lu%%\r\n", event.data);
                    break;
                    
                default:
                    break;
            }
        }
        
        // Periodic buffer health check
        audio_check_buffer_levels();
        
        // Sleep until next monitoring interval
        vTaskDelayUntil(&last_wake_time, pdMS_TO_TICKS(AUDIO_BUFFER_MONITOR_INTERVAL_MS));
    }
}

/**
 * @brief Check buffer levels and trigger events
 */
static void audio_check_buffer_levels(void)
{
    uint32_t buffer_usage = audio_calculate_buffer_usage();
    
    if (buffer_usage > AUDIO_BUFFER_WATERMARK_HIGH) {
        AudioEvent_t event = {
            .type = AUDIO_EVENT_BUFFER_LEVEL_HIGH,
            .timestamp = xTaskGetTickCount(),
            .data = buffer_usage
        };
        xQueueSend(audio_event_queue, &event, 0);
    } else if (buffer_usage < AUDIO_BUFFER_WATERMARK_LOW) {
        AudioEvent_t event = {
            .type = AUDIO_EVENT_BUFFER_LEVEL_LOW,
            .timestamp = xTaskGetTickCount(),
            .data = buffer_usage
        };
        xQueueSend(audio_event_queue, &event, 0);
    }
}

/**
 * @brief Handle buffer overflow condition
 */
static void audio_handle_buffer_overflow(void)
{
    AudioEvent_t event = {
        .type = AUDIO_EVENT_BUFFER_OVERFLOW,
        .timestamp = xTaskGetTickCount(),
        .data = audio_manager.buffer_count
    };
    xQueueSend(audio_event_queue, &event, 0);
    
    // Implement overflow recovery strategy
    // For now, we'll just log the event
    Uart1_SendData("[AUDIO_MGR] Buffer overflow handled\r\n");
}

/**
 * @brief Handle buffer underflow condition
 */
static void audio_handle_buffer_underflow(void)
{
    AudioEvent_t event = {
        .type = AUDIO_EVENT_BUFFER_UNDERFLOW,
        .timestamp = xTaskGetTickCount(),
        .data = audio_manager.buffer_count
    };
    xQueueSend(audio_event_queue, &event, 0);
    
    // Implement underflow recovery strategy
    // For now, we'll just log the event
    Uart1_SendData("[AUDIO_MGR] Buffer underflow handled\r\n");
}

/**
 * @brief Calculate current buffer usage percentage
 * @retval Buffer usage percentage (0-100)
 */
static uint32_t audio_calculate_buffer_usage(void)
{
    if (audio_manager.config.buffer_size == 0) {
        return 0;
    }
    
    return (audio_manager.buffer_count * 100) / audio_manager.config.buffer_size;
}

/**
 * @brief Get audio buffer statistics
 * @retval Pointer to statistics structure
 */
AudioBufferStats_t* AudioBuffer_GetStats(void)
{
    return &audio_manager.stats;
}

/**
 * @brief Generate audio buffer health report
 */
void AudioBuffer_GenerateHealthReport(void)
{
    AudioBufferStats_t* stats = &audio_manager.stats;
    uint32_t buffer_usage = audio_calculate_buffer_usage();
    
    Uart1_SendData("\r\n=== AUDIO BUFFER HEALTH REPORT ===\r\n");
    Uart1_SendData("Current Buffer Usage: %lu%%\r\n", buffer_usage);
    Uart1_SendData("Active Buffer: %d\r\n", audio_manager.active_buffer);
    Uart1_SendData("Write Index: %lu\r\n", audio_manager.write_index);
    Uart1_SendData("Read Index: %lu\r\n", audio_manager.read_index);
    Uart1_SendData("Buffer Count: %lu/%lu\r\n", audio_manager.buffer_count, audio_manager.config.buffer_size);
    
    Uart1_SendData("\r\n=== STATISTICS ===\r\n");
    Uart1_SendData("Successful Writes: %lu\r\n", stats->successful_writes);
    Uart1_SendData("Successful Reads: %lu\r\n", stats->successful_reads);
    Uart1_SendData("Total Samples Written: %lu\r\n", stats->total_samples_written);
    Uart1_SendData("Total Samples Read: %lu\r\n", stats->total_samples_read);
    
    Uart1_SendData("\r\n=== ERRORS ===\r\n");
    Uart1_SendData("Buffer Overflows: %lu\r\n", stats->buffer_overflows);
    Uart1_SendData("Buffer Underflows: %lu\r\n", stats->buffer_underflows);
    Uart1_SendData("Notification Overruns: %lu\r\n", stats->notification_overruns);
    Uart1_SendData("Notification Timeouts: %lu\r\n", stats->notification_timeouts);
    Uart1_SendData("Mutex Timeouts: %lu\r\n", stats->mutex_timeouts);
    
    Uart1_SendData("\r\n=== HEALTH ASSESSMENT ===\r\n");
    
    if (stats->buffer_overflows == 0 && stats->buffer_underflows == 0) {
        Uart1_SendData("Buffer Health: EXCELLENT\r\n");
    } else if (stats->buffer_overflows + stats->buffer_underflows < 10) {
        Uart1_SendData("Buffer Health: GOOD (Some issues detected)\r\n");
    } else {
        Uart1_SendData("Buffer Health: POOR (Many issues detected)\r\n");
        Uart1_SendData("Recommendation: Increase buffer size or optimize audio processing\r\n");
    }
    
    if (stats->notification_overruns > 0) {
        Uart1_SendData("Notification System: ISSUES DETECTED\r\n");
        Uart1_SendData("Recommendation: Review DMA interrupt timing and task priorities\r\n");
    } else {
        Uart1_SendData("Notification System: HEALTHY\r\n");
    }
    
    Uart1_SendData("=== END AUDIO BUFFER REPORT ===\r\n\r\n");
}

/**
 * @brief Cleanup audio buffer manager
 */
void AudioBuffer_Cleanup(void)
{
    // Delete monitor task
    if (audio_monitor_task_handle != NULL) {
        vTaskDelete(audio_monitor_task_handle);
        audio_monitor_task_handle = NULL;
    }
    
    // Delete queue
    if (audio_event_queue != NULL) {
        vQueueDelete(audio_event_queue);
        audio_event_queue = NULL;
    }
    
    // Delete mutex
    if (audio_mutex != NULL) {
        vSemaphoreDelete(audio_mutex);
        audio_mutex = NULL;
    }
    
    // Free buffers
    if (audio_manager.primary_buffer != NULL) {
        free(audio_manager.primary_buffer);
        audio_manager.primary_buffer = NULL;
    }
    
    if (audio_manager.secondary_buffer != NULL) {
        free(audio_manager.secondary_buffer);
        audio_manager.secondary_buffer = NULL;
    }
    
    if (audio_manager.temp_buffer != NULL) {
        free(audio_manager.temp_buffer);
        audio_manager.temp_buffer = NULL;
    }
    
    Uart1_SendData("[AUDIO_MGR] Audio buffer manager cleaned up\r\n");
}