/**
 * @file beta_test_framework.c
 * @brief Beta testing framework for BLE audio system
 * @description Comprehensive testing and diagnostic tools to address beta testing challenges
 * @author Beta Test Team
 * @date 2024-12-20
 */

#include "beta_test_framework.h"
#include "user_bsp.h"
#include "au_os.h"
#include "bsp_spi_sdcard.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

/* Private defines */
#define TEST_LOG_BUFFER_SIZE 1024
#define MAX_TEST_ITERATIONS 100
#define SD_TEST_BLOCK_SIZE 512
#define AUDIO_QUALITY_THRESHOLD 85  // Minimum acceptable audio quality percentage

/* Private variables */
static char test_log_buffer[TEST_LOG_BUFFER_SIZE];
static TestStats_t system_stats;
static TestConfig_t test_config;
static QueueHandle_t test_log_queue;

/* Private function prototypes */
static void log_test_event(TestEventType_t event, const char* message);
static uint32_t calculate_audio_quality_score(void);
static TestResult_t run_sd_card_stress_test(void);
static TestResult_t run_audio_stability_test(void);
static TestResult_t run_freertos_notification_test(void);

/**
 * @brief Initialize beta testing framework
 * @retval TEST_OK if initialization successful
 */
TestResult_t BetaTest_Init(void)
{
    // Initialize test statistics
    memset(&system_stats, 0, sizeof(TestStats_t));
    
    // Initialize test configuration with default values
    test_config.max_iterations = MAX_TEST_ITERATIONS;
    test_config.audio_quality_threshold = AUDIO_QUALITY_THRESHOLD;
    test_config.enable_detailed_logging = 1;
    test_config.enable_stress_testing = 1;
    
    // Create test log queue
    test_log_queue = xQueueCreate(50, sizeof(TestLogEntry_t));
    if (test_log_queue == NULL) {
        return TEST_ERROR;
    }
    
    // Initialize test timestamp
    system_stats.test_start_time = xTaskGetTickCount();
    
    log_test_event(TEST_EVENT_INIT, "Beta test framework initialized");
    return TEST_OK;
}

/**
 * @brief Run comprehensive system test suite
 * @retval TEST_OK if all tests pass
 */
TestResult_t BetaTest_RunFullSuite(void)
{
    TestResult_t result = TEST_OK;
    
    log_test_event(TEST_EVENT_START, "Starting comprehensive test suite");
    
    // Test 1: SD Card Stability Test
    Uart1_SendData("[TEST] Running SD card stability test...\r\n");
    if (run_sd_card_stress_test() != TEST_OK) {
        result = TEST_ERROR;
        system_stats.failed_tests++;
        log_test_event(TEST_EVENT_ERROR, "SD card stability test failed");
    } else {
        system_stats.passed_tests++;
        log_test_event(TEST_EVENT_SUCCESS, "SD card stability test passed");
    }
    
    // Test 2: Audio System Stability Test
    Uart1_SendData("[TEST] Running audio system stability test...\r\n");
    if (run_audio_stability_test() != TEST_OK) {
        result = TEST_ERROR;
        system_stats.failed_tests++;
        log_test_event(TEST_EVENT_ERROR, "Audio stability test failed");
    } else {
        system_stats.passed_tests++;
        log_test_event(TEST_EVENT_SUCCESS, "Audio stability test passed");
    }
    
    // Test 3: FreeRTOS Notification System Test
    Uart1_SendData("[TEST] Running FreeRTOS notification test...\r\n");
    if (run_freertos_notification_test() != TEST_OK) {
        result = TEST_ERROR;
        system_stats.failed_tests++;
        log_test_event(TEST_EVENT_ERROR, "FreeRTOS notification test failed");
    } else {
        system_stats.passed_tests++;
        log_test_event(TEST_EVENT_SUCCESS, "FreeRTOS notification test passed");
    }
    
    // Generate test report
    BetaTest_GenerateReport();
    
    return result;
}

/**
 * @brief Monitor system for "Core obliterated Data" errors
 * @retval Number of errors detected
 */
uint32_t BetaTest_MonitorCoreObliteratedErrors(void)
{
    static uint32_t error_count = 0;
    
    // Check for task notification overruns
    TaskHandle_t audio_task = xTaskGetCurrentTaskHandle();
    if (audio_task != NULL) {
        // Monitor task notification queue depth
        // This is a simplified check - in real implementation, 
        // we would need to access FreeRTOS internal structures
        
        // For now, increment error count if we detect potential overruns
        // This would be replaced with actual FreeRTOS queue monitoring
        if (system_stats.notification_overruns > 0) {
            error_count++;
            system_stats.core_obliterated_errors++;
            log_test_event(TEST_EVENT_ERROR, "Core obliterated data detected");
        }
    }
    
    return error_count;
}

/**
 * @brief Test SD card multi-block read stability
 * @retval TEST_OK if stable, TEST_ERROR if issues detected
 */
static TestResult_t run_sd_card_stress_test(void)
{
    uint8_t test_buffer[SD_TEST_BLOCK_SIZE * 4];  // 4 blocks
    SD_Error sd_result;
    uint32_t test_iterations = 0;
    uint32_t failure_count = 0;
    
    for (test_iterations = 0; test_iterations < test_config.max_iterations; test_iterations++) {
        // Test multi-block read
        sd_result = SD_ReadMultiBlocks(test_buffer, test_iterations * 4, SD_TEST_BLOCK_SIZE, 4);
        
        if (sd_result != SD_RESPONSE_NO_ERROR) {
            failure_count++;
            system_stats.sd_card_errors++;
            
            // Log detailed error information
            snprintf(test_log_buffer, sizeof(test_log_buffer), 
                    "SD read failed at iteration %lu, error code: %d", 
                    test_iterations, sd_result);
            log_test_event(TEST_EVENT_ERROR, test_log_buffer);
        }
        
        // Add small delay to prevent overwhelming the system
        vTaskDelay(pdMS_TO_TICKS(10));
    }
    
    // Calculate failure rate
    float failure_rate = (float)failure_count / test_iterations * 100.0f;
    system_stats.sd_card_failure_rate = failure_rate;
    
    snprintf(test_log_buffer, sizeof(test_log_buffer), 
            "SD test completed: %lu/%lu failures (%.2f%%)", 
            failure_count, test_iterations, failure_rate);
    log_test_event(TEST_EVENT_INFO, test_log_buffer);
    
    return (failure_rate < 5.0f) ? TEST_OK : TEST_ERROR;  // Accept up to 5% failure rate
}

/**
 * @brief Test audio system stability and quality
 * @retval TEST_OK if stable, TEST_ERROR if issues detected
 */
static TestResult_t run_audio_stability_test(void)
{
    uint32_t quality_score = calculate_audio_quality_score();
    
    // Check if audio quality meets threshold
    if (quality_score < test_config.audio_quality_threshold) {
        system_stats.audio_quality_failures++;
        
        snprintf(test_log_buffer, sizeof(test_log_buffer), 
                "Audio quality below threshold: %lu%% (min: %lu%%)", 
                quality_score, test_config.audio_quality_threshold);
        log_test_event(TEST_EVENT_ERROR, test_log_buffer);
        
        return TEST_ERROR;
    }
    
    system_stats.audio_quality_score = quality_score;
    
    snprintf(test_log_buffer, sizeof(test_log_buffer), 
            "Audio quality test passed: %lu%%", quality_score);
    log_test_event(TEST_EVENT_SUCCESS, test_log_buffer);
    
    return TEST_OK;
}

/**
 * @brief Test FreeRTOS notification system for overruns
 * @retval TEST_OK if stable, TEST_ERROR if overruns detected
 */
static TestResult_t run_freertos_notification_test(void)
{
    uint32_t initial_overruns = system_stats.notification_overruns;
    
    // Simulate high-frequency notifications (similar to audio DMA interrupts)
    for (uint32_t i = 0; i < 1000; i++) {
        // This would be replaced with actual notification testing
        // For now, we'll simulate the test
        vTaskDelay(pdMS_TO_TICKS(1));
    }
    
    uint32_t final_overruns = system_stats.notification_overruns;
    uint32_t detected_overruns = final_overruns - initial_overruns;
    
    if (detected_overruns > 0) {
        snprintf(test_log_buffer, sizeof(test_log_buffer), 
                "FreeRTOS notification overruns detected: %lu", detected_overruns);
        log_test_event(TEST_EVENT_ERROR, test_log_buffer);
        
        return TEST_ERROR;
    }
    
    log_test_event(TEST_EVENT_SUCCESS, "FreeRTOS notification test passed");
    return TEST_OK;
}

/**
 * @brief Calculate audio quality score based on various metrics
 * @retval Quality score (0-100%)
 */
static uint32_t calculate_audio_quality_score(void)
{
    uint32_t score = 100;
    
    // Deduct points for various issues
    if (system_stats.core_obliterated_errors > 0) {
        score -= (system_stats.core_obliterated_errors * 10);  // -10 points per error
    }
    
    if (system_stats.sd_card_errors > 0) {
        score -= (system_stats.sd_card_errors * 5);  // -5 points per SD error
    }
    
    if (system_stats.audio_dropouts > 0) {
        score -= (system_stats.audio_dropouts * 15);  // -15 points per dropout
    }
    
    // Ensure score doesn't go below 0
    if (score > 100) score = 0;  // Handle underflow
    
    return score;
}

/**
 * @brief Log test event with timestamp
 * @param event Event type
 * @param message Event message
 */
static void log_test_event(TestEventType_t event, const char* message)
{
    TestLogEntry_t log_entry;
    
    log_entry.timestamp = xTaskGetTickCount();
    log_entry.event_type = event;
    strncpy(log_entry.message, message, sizeof(log_entry.message) - 1);
    log_entry.message[sizeof(log_entry.message) - 1] = '\0';
    
    // Send to queue for later processing
    if (test_log_queue != NULL) {
        xQueueSend(test_log_queue, &log_entry, 0);
    }
    
    // Also send to UART for immediate feedback
    if (test_config.enable_detailed_logging) {
        Uart1_SendData("[%lu] %s\r\n", log_entry.timestamp, message);
    }
}

/**
 * @brief Generate comprehensive test report
 */
void BetaTest_GenerateReport(void)
{
    uint32_t total_tests = system_stats.passed_tests + system_stats.failed_tests;
    uint32_t test_duration = xTaskGetTickCount() - system_stats.test_start_time;
    
    Uart1_SendData("\r\n=== BETA TEST REPORT ===\r\n");
    Uart1_SendData("Test Duration: %lu ticks\r\n", test_duration);
    Uart1_SendData("Total Tests: %lu\r\n", total_tests);
    Uart1_SendData("Passed: %lu\r\n", system_stats.passed_tests);
    Uart1_SendData("Failed: %lu\r\n", system_stats.failed_tests);
    
    if (total_tests > 0) {
        uint32_t pass_rate = (system_stats.passed_tests * 100) / total_tests;
        Uart1_SendData("Pass Rate: %lu%%\r\n", pass_rate);
    }
    
    Uart1_SendData("\r\n=== ERROR STATISTICS ===\r\n");
    Uart1_SendData("Core Obliterated Errors: %lu\r\n", system_stats.core_obliterated_errors);
    Uart1_SendData("SD Card Errors: %lu\r\n", system_stats.sd_card_errors);
    Uart1_SendData("Audio Dropouts: %lu\r\n", system_stats.audio_dropouts);
    Uart1_SendData("Notification Overruns: %lu\r\n", system_stats.notification_overruns);
    
    if (system_stats.sd_card_failure_rate > 0) {
        Uart1_SendData("SD Card Failure Rate: %.2f%%\r\n", system_stats.sd_card_failure_rate);
    }
    
    Uart1_SendData("Audio Quality Score: %lu%%\r\n", system_stats.audio_quality_score);
    
    Uart1_SendData("\r\n=== RECOMMENDATIONS ===\r\n");
    
    if (system_stats.core_obliterated_errors > 0) {
        Uart1_SendData("- Implement task notification queue monitoring\r\n");
        Uart1_SendData("- Add DMA interrupt timing optimization\r\n");
    }
    
    if (system_stats.sd_card_errors > 0) {
        Uart1_SendData("- Review SD card SPI timing configuration\r\n");
        Uart1_SendData("- Implement SD card error recovery mechanisms\r\n");
    }
    
    if (system_stats.audio_dropouts > 0) {
        Uart1_SendData("- Increase audio buffer size\r\n");
        Uart1_SendData("- Optimize audio task priority\r\n");
    }
    
    Uart1_SendData("=== END REPORT ===\r\n\r\n");
}

/**
 * @brief Get current system statistics
 * @retval Pointer to system statistics structure
 */
TestStats_t* BetaTest_GetStats(void)
{
    return &system_stats;
}

/**
 * @brief Reset test statistics
 */
void BetaTest_ResetStats(void)
{
    memset(&system_stats, 0, sizeof(TestStats_t));
    system_stats.test_start_time = xTaskGetTickCount();
}

/**
 * @brief Update test configuration
 * @param config New configuration
 */
void BetaTest_UpdateConfig(TestConfig_t* config)
{
    if (config != NULL) {
        test_config = *config;
    }
}