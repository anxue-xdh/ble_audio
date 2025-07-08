/**
 * @file beta_test_init.c
 * @brief Beta testing framework initialization
 * @description Initialize all beta testing components
 * @author Beta Test Team
 * @date 2024-12-20
 */

#include "beta_test_init.h"
#include "beta_test_framework.h"
#include "sd_card_recovery.h"
#include "audio_buffer_manager.h"
#include "user_bsp.h"

/* Global variables */
static uint8_t beta_test_initialized = 0;

/**
 * @brief Initialize all beta testing systems
 * @retval 0 if successful, non-zero if error
 */
int BetaTest_SystemInit(void)
{
    if (beta_test_initialized) {
        Uart1_SendData("[BETA_INIT] System already initialized\r\n");
        return 0;
    }
    
    Uart1_SendData("[BETA_INIT] Initializing beta testing framework...\r\n");
    
    // Initialize beta test framework
    TestResult_t result = BetaTest_Init();
    if (result != TEST_OK) {
        Uart1_SendData("[BETA_INIT] Failed to initialize beta test framework: %d\r\n", result);
        return -1;
    }
    
    // Initialize SD card recovery system
    FRESULT sd_result = SDCard_RecoveryInit();
    if (sd_result != FR_OK) {
        Uart1_SendData("[BETA_INIT] Failed to initialize SD card recovery: %d\r\n", sd_result);
        return -2;
    }
    
    // Initialize audio buffer manager
    AudioBufferConfig_t audio_config = {
        .buffer_size = 2048,            // 2048 samples
        .sample_rate = 44100,           // 44.1 kHz
        .channels = 2,                  // Stereo
        .bits_per_sample = 16,          // 16-bit
        .enable_double_buffering = 1    // Enable double buffering
    };
    
    AudioResult_t audio_result = AudioBuffer_Init(&audio_config);
    if (audio_result != AUDIO_OK) {
        Uart1_SendData("[BETA_INIT] Failed to initialize audio buffer manager: %d\r\n", audio_result);
        return -3;
    }
    
    beta_test_initialized = 1;
    Uart1_SendData("[BETA_INIT] Beta testing framework initialized successfully\r\n");
    
    // Print system information
    Uart1_SendData("\r\n=== BETA TESTING FRAMEWORK READY ===\r\n");
    Uart1_SendData("Available Commands:\r\n");
    Uart1_SendData("- beta test init      : Initialize framework\r\n");
    Uart1_SendData("- beta test run       : Run test suite\r\n");
    Uart1_SendData("- beta test report    : Generate report\r\n");
    Uart1_SendData("- beta test reset     : Reset statistics\r\n");
    Uart1_SendData("- sd health           : SD card health report\r\n");
    Uart1_SendData("- audio health        : Audio buffer health report\r\n");
    Uart1_SendData("- help                : Show this help\r\n");
    Uart1_SendData("==========================================\r\n\r\n");
    
    return 0;
}

/**
 * @brief Check if beta testing framework is initialized
 * @retval 1 if initialized, 0 if not
 */
uint8_t BetaTest_IsInitialized(void)
{
    return beta_test_initialized;
}

/**
 * @brief Print help information
 */
void BetaTest_PrintHelp(void)
{
    Uart1_SendData("\r\n=== BLE AUDIO BETA TESTING COMMANDS ===\r\n");
    Uart1_SendData("System Commands:\r\n");
    Uart1_SendData("  Query task resource  - Show task resource usage\r\n");
    Uart1_SendData("  Query task time      - Show task timing info\r\n");
    Uart1_SendData("  soft reset           - Reset system\r\n");
    Uart1_SendData("\r\nBeta Testing Commands:\r\n");
    Uart1_SendData("  beta test init       - Initialize testing framework\r\n");
    Uart1_SendData("  beta test run        - Run comprehensive test suite\r\n");
    Uart1_SendData("  beta test report     - Generate test report\r\n");
    Uart1_SendData("  beta test reset      - Reset test statistics\r\n");
    Uart1_SendData("\r\nSD Card Commands:\r\n");
    Uart1_SendData("  sd read              - Test SD card read\r\n");
    Uart1_SendData("  sd health            - SD card health report\r\n");
    Uart1_SendData("  sd stats reset       - Reset SD statistics\r\n");
    Uart1_SendData("\r\nAudio Commands:\r\n");
    Uart1_SendData("  music start          - Start audio playback\r\n");
    Uart1_SendData("  music stop           - Stop audio playback\r\n");
    Uart1_SendData("  music pause          - Pause audio playback\r\n");
    Uart1_SendData("  music resume         - Resume audio playback\r\n");
    Uart1_SendData("  audio health         - Audio buffer health report\r\n");
    Uart1_SendData("\r\nDebugging Commands:\r\n");
    Uart1_SendData("  help                 - Show this help\r\n");
    Uart1_SendData("  Test                 - Basic test command\r\n");
    Uart1_SendData("========================================\r\n\r\n");
}

/**
 * @brief Periodic monitoring task (call from main loop or timer)
 */
void BetaTest_PeriodicMonitor(void)
{
    if (!beta_test_initialized) {
        return;
    }
    
    // Monitor for core obliterated errors
    BetaTest_MonitorCoreObliteratedErrors();
    
    // Additional periodic monitoring can be added here
    static uint32_t last_report_time = 0;
    uint32_t current_time = HAL_GetTick();
    
    // Generate periodic health report every 5 minutes (300000 ms)
    if (current_time - last_report_time > 300000) {
        Uart1_SendData("[PERIODIC] Generating health status...\r\n");
        
        TestStats_t* stats = BetaTest_GetStats();
        if (stats->core_obliterated_errors > 0 || 
            stats->sd_card_errors > 0 || 
            stats->audio_dropouts > 0) {
            
            Uart1_SendData("[PERIODIC] Issues detected - generating reports\r\n");
            BetaTest_GenerateReport();
            SDCard_GenerateHealthReport();
            AudioBuffer_GenerateHealthReport();
        }
        
        last_report_time = current_time;
    }
}