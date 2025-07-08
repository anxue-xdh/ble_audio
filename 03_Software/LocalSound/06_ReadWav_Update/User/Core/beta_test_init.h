/**
 * @file beta_test_init.h
 * @brief Beta testing framework initialization header
 * @description Initialize all beta testing components
 * @author Beta Test Team
 * @date 2024-12-20
 */

#ifndef BETA_TEST_INIT_H
#define BETA_TEST_INIT_H

#include <stdint.h>

/* Function prototypes */
int BetaTest_SystemInit(void);
uint8_t BetaTest_IsInitialized(void);
void BetaTest_PrintHelp(void);
void BetaTest_PeriodicMonitor(void);

/* Convenience macro for system initialization */
#define BETA_TEST_SYSTEM_INIT() BetaTest_SystemInit()

/* Macro for adding to UART command handler */
#define BETA_TEST_ADD_HELP_COMMAND() \
    else if (Uart1_strcmp("help")) { \
        BetaTest_PrintHelp(); \
    }

#endif /* BETA_TEST_INIT_H */