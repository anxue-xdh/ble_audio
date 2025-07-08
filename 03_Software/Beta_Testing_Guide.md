# BLE Audio Beta Testing Guide

## 概述 (Overview)

本文档旨在解决BLE音频项目的公测难点，提供完整的测试框架和诊断工具来帮助识别和解决系统中的关键问题。

This document addresses the beta testing challenges in the BLE audio project by providing a comprehensive testing framework and diagnostic tools to identify and resolve critical system issues.

## 主要问题分析 (Main Issues Analysis)

### 1. "Core obliterated Data" 错误
- **现象**: FreeRTOS任务通知队列溢出
- **原因**: DMA中断频率过高，任务处理速度跟不上
- **影响**: 音频播放中断，产生噪音

### 2. FR_DISK_ERR 错误
- **现象**: SD卡读取失败，错误代码(1)
- **原因**: SPI通信时序问题，多块读取命令实现不当
- **影响**: 音频文件读取中断

### 3. 音频缓冲区管理问题
- **现象**: 音频播放时出现杂音、卡顿
- **原因**: 缓冲区上溢/下溢，数据传输速率不匹配
- **影响**: 音频质量下降

## 解决方案框架 (Solution Framework)

### 1. Beta测试框架 (Beta Test Framework)

#### 文件位置
- `User/Core/beta_test_framework.c/h`

#### 主要功能
- 系统健康监控
- 错误统计和分析
- 自动化测试套件
- 性能评估报告

#### 使用方法
```c
// 初始化测试框架
BetaTest_Init();

// 运行完整测试套件
BetaTest_RunFullSuite();

// 生成测试报告
BetaTest_GenerateReport();
```

### 2. SD卡恢复系统 (SD Card Recovery System)

#### 文件位置
- `User/Core/sd_card_recovery.c/h`

#### 主要功能
- FR_DISK_ERR错误恢复
- 多块读取稳定性改进
- SD卡健康监控
- 自动重试机制

#### 使用方法
```c
// 初始化SD卡恢复系统
SDCard_RecoveryInit();

// 使用恢复功能的文件读取
SDCard_ReadWithRecovery(file, buffer, size, &bytes_read);

// 改进的多块读取
SDCard_ReadMultiBlocksImproved(buffer, addr, block_size, num_blocks);
```

### 3. 音频缓冲区管理器 (Audio Buffer Manager)

#### 文件位置
- `User/Core/audio_buffer_manager.c/h`

#### 主要功能
- 防溢出音频缓冲区管理
- 任务通知溢出检测
- 双缓冲机制
- 音频质量监控

#### 使用方法
```c
// 初始化音频缓冲区管理器
AudioBufferConfig_t config = {
    .buffer_size = 2048,
    .sample_rate = 44100,
    .channels = 2,
    .bits_per_sample = 16
};
AudioBuffer_Init(&config);

// 安全的任务通知处理
AUDIO_SAFE_NOTIFY_TAKE(100);  // 带溢出检测的通知

// 缓冲区操作
AudioBuffer_Write(audio_data, sample_count);
AudioBuffer_Read(output_buffer, sample_count, &samples_read);
```

## 自动化测试 (Automated Testing)

### Python测试脚本
- **文件**: `beta_test_automation.py`
- **功能**: 自动化串口测试，错误检测，报告生成

### 使用方法
```bash
# 基础系统测试
python beta_test_automation.py --port COM3 --test-type basic

# 压力测试（10分钟）
python beta_test_automation.py --port COM3 --test-type stress --duration 10

# 音频质量测试
python beta_test_automation.py --port COM3 --test-type audio

# 完整测试套件
python beta_test_automation.py --port COM3 --test-type full --output report.json --log test.log
```

## 测试程序 (Testing Procedures)

### 1. 基础系统测试

#### 测试目标
- 验证系统基本功能
- 检测关键错误模式
- 评估系统稳定性

#### 测试步骤
1. 连接串口调试器
2. 启动测试框架: `BetaTest_Init()`
3. 运行基础测试: `BetaTest_RunFullSuite()`
4. 分析结果: `BetaTest_GenerateReport()`

### 2. SD卡稳定性测试

#### 测试目标
- 验证SD卡读取稳定性
- 测试错误恢复机制
- 评估多块读取性能

#### 测试命令
```
sd read              // 基础读取测试
music start         // 音频文件读取测试
Query task resource // 系统资源监控
```

### 3. 音频质量测试

#### 测试目标
- 评估音频播放质量
- 检测音频中断和杂音
- 验证缓冲区管理效果

#### 测试流程
1. 启动音频播放
2. 监控系统状态
3. 检测音频事件
4. 分析质量指标

### 4. 压力测试

#### 测试目标
- 验证长时间运行稳定性
- 检测内存泄漏
- 评估错误恢复能力

#### 测试配置
- 持续时间: 30-60分钟
- 循环测试各项功能
- 实时监控错误计数

## 问题诊断指南 (Problem Diagnosis Guide)

### 1. "Core obliterated Data" 错误

#### 症状识别
```
[ERROR] Core obliterated Data!!
[ERROR] Uart1 obliterated Data!!
```

#### 诊断步骤
1. 检查任务通知计数: `ulTaskNotifyTake()` 返回值 > 1
2. 分析DMA中断频率
3. 检查任务优先级配置
4. 验证缓冲区大小设置

#### 解决方案
- 使用 `AUDIO_SAFE_NOTIFY_TAKE()` 宏
- 调整DMA中断时序
- 增加音频缓冲区大小
- 优化任务优先级

### 2. FR_DISK_ERR 错误

#### 症状识别
```
！！文件读取失败：(1)
FR_DISK_ERR
old pointer:XXXXXX
now pointer:XXXXXX
pointer diff:0
```

#### 诊断步骤
1. 检查SD卡SPI时序配置
2. 验证多块读取实现
3. 监控SD卡错误率
4. 分析错误发生模式

#### 解决方案
- 使用 `SDCard_ReadWithRecovery()`
- 调整SPI分频系数
- 实现自动重试机制
- 改进多块读取逻辑

### 3. 音频质量问题

#### 症状识别
- 音频杂音、卡顿
- 缓冲区上溢/下溢
- 音频播放中断

#### 诊断步骤
1. 监控缓冲区使用率
2. 检查音频数据传输速率
3. 分析任务调度时序
4. 验证DMA配置

#### 解决方案
- 增加缓冲区大小
- 实现双缓冲机制
- 优化数据传输路径
- 调整任务优先级

## 性能优化建议 (Performance Optimization)

### 1. 系统配置优化

#### FreeRTOS配置
```c
#define configTIMER_TASK_PRIORITY       (2)
#define configMAX_PRIORITIES            (8)
#define configMINIMAL_STACK_SIZE        (128)
#define configTOTAL_HEAP_SIZE           (16384)
```

#### 任务优先级
- 音频任务: 最高优先级 (7)
- SD卡任务: 高优先级 (5)
- UI任务: 中等优先级 (3)
- 监控任务: 低优先级 (2)

### 2. 硬件配置优化

#### SPI配置
```c
// SD卡SPI配置
SPI_InitStruct.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_32;  // 32分频
SPI_InitStruct.CLKPolarity = SPI_POLARITY_LOW;
SPI_InitStruct.CLKPhase = SPI_PHASE_1EDGE;
```

#### DMA配置
```c
// 音频DMA配置
hdma_dac1.Init.MemDataAlignment = DMA_MDATAALIGN_HALFWORD;
hdma_dac1.Init.PeriphDataAlignment = DMA_PDATAALIGN_HALFWORD;
hdma_dac1.Init.Mode = DMA_CIRCULAR;
```

### 3. 缓冲区优化

#### 缓冲区大小
- 单声道: 1024 samples
- 双声道: 2048 samples
- 临时缓冲区: 512 samples

#### 内存分配
```c
// 启动文件中堆大小设置
Heap_Size       EQU     0x4000  ; 16KB
```

## 测试报告模板 (Test Report Template)

### 测试信息
- 测试日期: YYYY-MM-DD
- 测试版本: vX.X.X
- 测试人员: [姓名]
- 测试时长: [小时]

### 测试结果统计
- 总测试数: XXX
- 通过测试: XXX
- 失败测试: XXX
- 通过率: XX%

### 错误统计
- Core obliterated错误: XXX次
- SD卡错误: XXX次
- 音频中断: XXX次

### 性能指标
- 音频质量得分: XX%
- SD卡成功率: XX%
- 系统稳定性: XX%

### 问题与建议
1. [具体问题描述]
   - 原因分析
   - 解决建议
   
2. [具体问题描述]
   - 原因分析
   - 解决建议

## 常用调试命令 (Common Debug Commands)

### 系统状态查询
```
Query task resource      // 查询任务资源使用情况
Query task time         // 查询任务运行时间
soft reset             // 软件复位
```

### 音频控制
```
music start            // 开始音频播放
music stop             // 停止音频播放
music pause            // 暂停音频播放
music resume           // 恢复音频播放
```

### SD卡测试
```
sd read                // SD卡读取测试
sd test                // SD卡综合测试
sd health              // SD卡健康检查
```

### 测试框架命令
```
beta test init         // 初始化测试框架
beta test run          // 运行测试套件
beta test report       // 生成测试报告
beta test reset        // 重置测试统计
```

## 故障排除清单 (Troubleshooting Checklist)

### 系统无响应
- [ ] 检查电源供应
- [ ] 验证串口连接
- [ ] 检查复位状态
- [ ] 确认固件版本

### 音频问题
- [ ] 检查DAC配置
- [ ] 验证DMA设置
- [ ] 确认音频文件格式
- [ ] 检查缓冲区大小

### SD卡问题
- [ ] 检查SPI连接
- [ ] 验证SD卡格式
- [ ] 确认文件系统
- [ ] 检查时序配置

### 通信问题
- [ ] 检查串口波特率
- [ ] 验证硬件连接
- [ ] 确认协议设置
- [ ] 检查缓冲区配置

## 联系信息 (Contact Information)

如遇到技术问题，请联系开发团队：
- 邮箱: [开发团队邮箱]
- 微信群: [测试群二维码]
- 论坛: [技术论坛地址]

---

**注意**: 本文档会随着项目进展持续更新，请关注最新版本。