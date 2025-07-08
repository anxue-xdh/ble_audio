/**
 * @file audio_buffer_manager.h
 * @brief Audio buffer management system header with overflow protection
 * @description Addresses "Core obliterated Data" errors and audio stability issues
 * @author Beta Test Team
 * @date 2024-12-20
 */

#ifndef AUDIO_BUFFER_MANAGER_H
#define AUDIO_BUFFER_MANAGER_H

#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "queue.h"

/* Audio result enumeration */
typedef enum {
    AUDIO_OK = 0,
    AUDIO_ERROR_INVALID_PARAM = 1,
    AUDIO_ERROR_NO_MEMORY = 2,
    AUDIO_ERROR_BUFFER_FULL = 3,
    AUDIO_ERROR_BUFFER_EMPTY = 4,
    AUDIO_ERROR_TIMEOUT = 5,
    AUDIO_ERROR_OVERRUN = 6,
    AUDIO_ERROR_OS = 7
} AudioResult_t;

/* Audio event types */
typedef enum {
    AUDIO_EVENT_BUFFER_OVERFLOW = 0,
    AUDIO_EVENT_BUFFER_UNDERFLOW,
    AUDIO_EVENT_NOTIFICATION_OVERRUN,
    AUDIO_EVENT_BUFFER_LEVEL_HIGH,
    AUDIO_EVENT_BUFFER_LEVEL_LOW
} AudioEventType_t;

/* Audio buffer configuration */
typedef struct {
    uint32_t buffer_size;           // Buffer size in samples
    uint32_t sample_rate;           // Audio sample rate
    uint8_t channels;               // Number of audio channels
    uint8_t bits_per_sample;        // Bits per sample
    uint8_t enable_double_buffering; // Enable double buffering
} AudioBufferConfig_t;

/* Audio buffer statistics */
typedef struct {
    uint32_t init_time;
    uint32_t successful_writes;
    uint32_t successful_reads;
    uint32_t successful_notifications;
    uint32_t buffer_switches;
    uint32_t total_samples_written;
    uint32_t total_samples_read;
    uint32_t buffer_overflows;
    uint32_t buffer_underflows;
    uint32_t notification_overruns;
    uint32_t notification_timeouts;
    uint32_t mutex_timeouts;
} AudioBufferStats_t;

/* Audio event structure */
typedef struct {
    AudioEventType_t type;
    uint32_t timestamp;
    uint32_t data;
} AudioEvent_t;

/* Audio buffer manager structure */
typedef struct {
    AudioBufferConfig_t config;
    AudioBufferStats_t stats;
    
    int16_t* primary_buffer;
    int16_t* secondary_buffer;
    int16_t* temp_buffer;
    
    uint32_t write_index;
    uint32_t read_index;
    uint32_t buffer_count;
    uint8_t active_buffer;
} AudioBufferManager_t;

/* Function prototypes */
AudioResult_t AudioBuffer_Init(AudioBufferConfig_t *config);
AudioResult_t AudioBuffer_Write(int16_t *data, uint32_t samples);
AudioResult_t AudioBuffer_Read(int16_t *data, uint32_t samples, uint32_t *samples_read);
AudioResult_t AudioBuffer_HandleNotification(TaskHandle_t notification_handle, uint32_t notification_value);
AudioResult_t AudioBuffer_SwitchBuffer(void);
AudioBufferStats_t* AudioBuffer_GetStats(void);
void AudioBuffer_GenerateHealthReport(void);
void AudioBuffer_Cleanup(void);

/* Convenience macros for safer task notifications */
#define AUDIO_SAFE_NOTIFY_TAKE(timeout_ms) \
    ({ \
        uint32_t count = ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(timeout_ms)); \
        if (count > 1) { \
            BetaTest_IncrementCoreObliteratedErrors(); \
            Uart1_SendData("[ERROR] Core obliterated Data!! Count: %lu\r\n", count); \
        } \
        count; \
    })

#define AUDIO_SAFE_NOTIFY_WAIT(clear_on_entry, clear_on_exit, value, timeout_ms) \
    ({ \
        uint32_t notification_value = 0; \
        BaseType_t result = xTaskNotifyWait(clear_on_entry, clear_on_exit, &notification_value, pdMS_TO_TICKS(timeout_ms)); \
        if (result == pdTRUE) { \
            *(value) = notification_value; \
        } \
        result; \
    })

#endif /* AUDIO_BUFFER_MANAGER_H */