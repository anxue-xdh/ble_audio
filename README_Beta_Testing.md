# BLE Audio Beta Testing Framework

## 项目概述 (Project Overview)

本项目针对BLE音频系统的公测难点，提供了一套完整的测试框架和诊断工具。主要解决了"Core obliterated Data"错误、SD卡FR_DISK_ERR错误以及音频质量问题。

This project addresses the beta testing challenges in BLE audio systems by providing a comprehensive testing framework and diagnostic tools. It primarily resolves "Core obliterated Data" errors, SD card FR_DISK_ERR errors, and audio quality issues.

## 主要功能 (Main Features)

### 1. 🔍 Beta测试框架 (Beta Testing Framework)
- 自动化错误检测和统计
- 系统健康监控
- 性能评估和报告生成
- 压力测试支持

### 2. 💾 SD卡恢复系统 (SD Card Recovery System)
- FR_DISK_ERR自动恢复
- 多块读取稳定性改进
- SD卡健康监控
- 智能重试机制

### 3. 🎵 音频缓冲区管理器 (Audio Buffer Manager)
- 防溢出缓冲区管理
- 任务通知溢出检测
- 双缓冲机制
- 实时音频质量监控

### 4. 🤖 自动化测试工具 (Automation Tools)
- Python测试脚本
- 串口自动化测试
- 报告自动生成
- 多种测试模式

## 文件结构 (File Structure)

```
03_Software/
├── LocalSound/06_ReadWav_Update/
│   └── User/Core/
│       ├── beta_test_framework.c/h      # 核心测试框架
│       ├── sd_card_recovery.c/h         # SD卡恢复系统
│       ├── audio_buffer_manager.c/h     # 音频缓冲区管理
│       └── beta_test_init.c/h           # 系统初始化
├── beta_test_automation.py              # Python自动化测试
├── Beta_Testing_Guide.md                # 详细测试指南
└── build_integration.mk                 # 构建集成配置
```

## 快速开始 (Quick Start)

### 1. 集成到现有项目

#### 添加源文件到项目
```c
// 在项目中添加以下文件：
User/Core/beta_test_framework.c
User/Core/sd_card_recovery.c
User/Core/audio_buffer_manager.c
User/Core/beta_test_init.c
```

#### 添加头文件路径
```c
// 在项目设置中添加包含路径：
User/Core
```

#### 添加预处理定义
```c
BETA_TEST_ENABLED
SD_RECOVERY_ENABLED
AUDIO_BUFFER_MANAGER_ENABLED
```

### 2. 初始化系统

在main函数中添加：
```c
#include "beta_test_init.h"

int main(void)
{
    // ... 其他初始化代码 ...
    
    // 初始化Beta测试框架
    if (BetaTest_SystemInit() != 0) {
        // 处理初始化失败
        Error_Handler();
    }
    
    // ... 启动FreeRTOS任务 ...
}
```

### 3. 使用改进的错误处理

#### 替换SD卡读取函数
```c
// 原来的代码：
f_read(file, buffer, size, &bytes_read);

// 改进的代码：
SDCard_ReadWithRecovery(file, buffer, size, &bytes_read);
```

#### 替换任务通知处理
```c
// 原来的代码：
if (ulTaskNotifyTake(pdTRUE, timeout) > 1)
    Uart1_SendData("[ERROR] Core obliterated Data!!");

// 改进的代码：
AUDIO_SAFE_NOTIFY_TAKE(timeout);
```

## 测试命令 (Test Commands)

通过串口发送以下命令进行测试：

### 基础命令
- `help` - 显示所有可用命令
- `Query task resource` - 查询任务资源使用情况
- `Query task time` - 查询任务时间统计

### Beta测试命令
- `beta test init` - 初始化测试框架
- `beta test run` - 运行完整测试套件
- `beta test report` - 生成测试报告
- `beta test reset` - 重置测试统计

### SD卡测试命令
- `sd read` - SD卡读取测试
- `sd health` - SD卡健康报告
- `sd stats reset` - 重置SD卡统计

### 音频测试命令
- `music start` - 开始音频播放
- `music stop` - 停止音频播放
- `audio health` - 音频缓冲区健康报告

## 自动化测试 (Automated Testing)

### Python脚本使用方法

```bash
# 基础系统测试
python beta_test_automation.py --port COM3 --test-type basic

# 压力测试（10分钟）
python beta_test_automation.py --port COM3 --test-type stress --duration 10

# 音频质量测试
python beta_test_automation.py --port COM3 --test-type audio

# 完整测试套件
python beta_test_automation.py --port COM3 --test-type full --output report.json
```

### 脚本参数说明
- `--port` : 串口端口 (必需)
- `--baud` : 波特率 (默认115200)
- `--test-type` : 测试类型 (basic/stress/audio/full)
- `--duration` : 压力测试持续时间(分钟)
- `--output` : 输出报告文件
- `--log` : 详细日志文件

## 问题解决 (Problem Resolution)

### "Core obliterated Data" 错误
**症状**: 任务通知队列溢出，音频中断
**解决方案**:
1. 使用 `AUDIO_SAFE_NOTIFY_TAKE()` 宏
2. 检查DMA中断时序配置
3. 调整任务优先级

### FR_DISK_ERR 错误
**症状**: SD卡读取失败，文件系统错误
**解决方案**:
1. 使用 `SDCard_ReadWithRecovery()` 函数
2. 调整SPI时序参数
3. 启用自动重试机制

### 音频质量问题
**症状**: 音频杂音、卡顿、中断
**解决方案**:
1. 初始化音频缓冲区管理器
2. 监控缓冲区使用率
3. 调整缓冲区大小

## 性能优化建议 (Performance Optimization)

### FreeRTOS配置
```c
#define configTIMER_TASK_PRIORITY       (2)
#define configMAX_PRIORITIES            (8)
#define configTOTAL_HEAP_SIZE           (16384)
```

### 任务优先级设置
- 音频任务: 7 (最高)
- SD卡任务: 5 (高)
- UI任务: 3 (中等)
- 监控任务: 2 (低)

### 缓冲区配置
```c
AudioBufferConfig_t config = {
    .buffer_size = 2048,        // 双声道建议2048
    .sample_rate = 44100,
    .channels = 2,
    .bits_per_sample = 16,
    .enable_double_buffering = 1
};
```

## 贡献指南 (Contributing)

1. Fork 项目
2. 创建功能分支 (`git checkout -b feature/AmazingFeature`)
3. 提交更改 (`git commit -m 'Add some AmazingFeature'`)
4. 推送到分支 (`git push origin feature/AmazingFeature`)
5. 开启 Pull Request

## 许可证 (License)

本项目采用 MIT 许可证 - 详见 [LICENSE](LICENSE) 文件

## 联系方式 (Contact)

- 项目链接: [https://github.com/anxue-xdh/ble_audio](https://github.com/anxue-xdh/ble_audio)
- 问题反馈: [GitHub Issues](https://github.com/anxue-xdh/ble_audio/issues)

## 致谢 (Acknowledgments)

- 感谢所有Beta测试用户的反馈
- 感谢STM32和FreeRTOS社区的支持
- 特别感谢在调试过程中提供帮助的开发者们

---

**注意**: 使用本框架前请仔细阅读 [Beta_Testing_Guide.md](Beta_Testing_Guide.md) 获取详细的测试指导。