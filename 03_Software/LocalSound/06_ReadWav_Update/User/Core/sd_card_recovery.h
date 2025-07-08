/**
 * @file sd_card_recovery.h
 * @brief SD card error recovery and stability improvements header
 * @description Addresses FR_DISK_ERR and multi-block read issues identified in beta testing
 * @author Beta Test Team
 * @date 2024-12-20
 */

#ifndef SD_CARD_RECOVERY_H
#define SD_CARD_RECOVERY_H

#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include "ff.h"
#include "bsp_spi_sdcard.h"

/* SD Card Statistics Structure */
typedef struct {
    uint32_t init_time;
    uint32_t successful_reads;
    uint32_t failed_reads;
    uint32_t disk_errors;
    uint32_t multi_block_errors;
    uint32_t successful_multi_block_reads;
    uint32_t successful_recoveries;
    uint32_t failed_recoveries;
    uint32_t interface_resets;
} SDCardStats_t;

/* Function prototypes */
FRESULT SDCard_RecoveryInit(void);
FRESULT SDCard_ReadWithRecovery(FIL* file, void* buffer, UINT size, UINT* bytes_read);
SD_Error SDCard_ReadMultiBlocksImproved(uint8_t *pBuffer, uint64_t ReadAddr, uint16_t BlockSize, uint32_t NumberOfBlocks);
SDCardStats_t* SDCard_GetStats(void);
void SDCard_GenerateHealthReport(void);
void SDCard_ResetStats(void);

/* Convenience macros */
#define SD_READ_WITH_RECOVERY(file, buffer, size, bytes_read) \
    SDCard_ReadWithRecovery(file, buffer, size, bytes_read)

#define SD_READ_MULTI_BLOCKS_SAFE(buffer, addr, block_size, num_blocks) \
    SDCard_ReadMultiBlocksImproved(buffer, addr, block_size, num_blocks)

#endif /* SD_CARD_RECOVERY_H */