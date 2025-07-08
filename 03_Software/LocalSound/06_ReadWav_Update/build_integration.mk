# Beta Testing Framework Integration for STM32 Project
# Add these lines to your existing Makefile or project configuration

# Additional C source files for beta testing framework
C_SOURCES += \
User/Core/beta_test_framework.c \
User/Core/sd_card_recovery.c \
User/Core/audio_buffer_manager.c

# Additional include paths
C_INCLUDES += \
-IUser/Core

# Additional defines for beta testing
C_DEFS += \
-DBETA_TEST_ENABLED=1 \
-DSD_RECOVERY_ENABLED=1 \
-DAUDIO_BUFFER_MANAGER_ENABLED=1

# Compiler flags for better debugging
CFLAGS += -g3 -O0 -ffunction-sections -fdata-sections

# For MDK-ARM project, add these files to the project groups:
# Core/beta_test_framework.c
# Core/sd_card_recovery.c  
# Core/audio_buffer_manager.c

# Include paths in MDK-ARM:
# User/Core

# Preprocessor definitions in MDK-ARM:
# BETA_TEST_ENABLED
# SD_RECOVERY_ENABLED
# AUDIO_BUFFER_MANAGER_ENABLED