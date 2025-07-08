/**
 * @file beta_test_framework.h
 * @brief Beta testing framework header for BLE audio system
 * @description Comprehensive testing and diagnostic tools to address beta testing challenges
 * @author Beta Test Team
 * @date 2024-12-20
 */

#ifndef BETA_TEST_FRAMEWORK_H
#define BETA_TEST_FRAMEWORK_H

#include <stdint.h>
#include <string.h>
#include <stdio.h>

/* Test result enumeration */
typedef enum {
    TEST_OK = 0,
    TEST_ERROR = 1,
    TEST_WARNING = 2,
    TEST_TIMEOUT = 3
} TestResult_t;

/* Test event types */
typedef enum {
    TEST_EVENT_INIT = 0,
    TEST_EVENT_START,
    TEST_EVENT_SUCCESS,
    TEST_EVENT_ERROR,
    TEST_EVENT_WARNING,
    TEST_EVENT_INFO,
    TEST_EVENT_TIMEOUT
} TestEventType_t;

/* Test configuration structure */
typedef struct {
    uint32_t max_iterations;
    uint32_t audio_quality_threshold;
    uint8_t enable_detailed_logging;
    uint8_t enable_stress_testing;
    uint32_t test_timeout_ms;
} TestConfig_t;

/* Test statistics structure */
typedef struct {
    uint32_t test_start_time;
    uint32_t passed_tests;
    uint32_t failed_tests;
    uint32_t core_obliterated_errors;
    uint32_t sd_card_errors;
    uint32_t audio_dropouts;
    uint32_t notification_overruns;
    float sd_card_failure_rate;
    uint32_t audio_quality_score;
    uint32_t audio_quality_failures;
} TestStats_t;

/* Test log entry structure */
typedef struct {
    uint32_t timestamp;
    TestEventType_t event_type;
    char message[128];
} TestLogEntry_t;

/* Function prototypes */
TestResult_t BetaTest_Init(void);
TestResult_t BetaTest_RunFullSuite(void);
uint32_t BetaTest_MonitorCoreObliteratedErrors(void);
void BetaTest_GenerateReport(void);
TestStats_t* BetaTest_GetStats(void);
void BetaTest_ResetStats(void);
void BetaTest_UpdateConfig(TestConfig_t* config);

/* Inline utility functions */
static inline void BetaTest_IncrementCoreObliteratedErrors(void)
{
    TestStats_t* stats = BetaTest_GetStats();
    stats->core_obliterated_errors++;
}

static inline void BetaTest_IncrementSdCardErrors(void)
{
    TestStats_t* stats = BetaTest_GetStats();
    stats->sd_card_errors++;
}

static inline void BetaTest_IncrementAudioDropouts(void)
{
    TestStats_t* stats = BetaTest_GetStats();
    stats->audio_dropouts++;
}

static inline void BetaTest_IncrementNotificationOverruns(void)
{
    TestStats_t* stats = BetaTest_GetStats();
    stats->notification_overruns++;
}

/* Test macros for easy integration */
#define BETA_TEST_MONITOR_CORE_OBLITERATED() \
    do { \
        if (ulTaskNotifyTake(pdTRUE, timeout) > 1) { \
            BetaTest_IncrementCoreObliteratedErrors(); \
            Uart1_SendData("[ERROR] Core obliterated Data!! (Monitored by Beta Test)\r\n"); \
        } \
    } while(0)

#define BETA_TEST_MONITOR_SD_ERROR(result) \
    do { \
        if ((result) != FR_OK) { \
            BetaTest_IncrementSdCardErrors(); \
            Uart1_SendData("[ERROR] SD Card Error: %d (Monitored by Beta Test)\r\n", result); \
        } \
    } while(0)

#define BETA_TEST_MONITOR_AUDIO_DROPOUT() \
    do { \
        BetaTest_IncrementAudioDropouts(); \
        Uart1_SendData("[WARNING] Audio Dropout Detected (Monitored by Beta Test)\r\n"); \
    } while(0)

#endif /* BETA_TEST_FRAMEWORK_H */