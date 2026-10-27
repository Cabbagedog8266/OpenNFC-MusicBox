# OpenNFC Music Box

**无源 NFC 音乐盒** · STM32G031 + NTAG I²C plus

**简体中文** | [English](README.en.md)

手机靠近板载 NFC 天线，芯片从射频场取电，STM32G031 通过 PWM 驱动无源蜂鸣器播放旋律；板载 NTAG I²C plus 同时提供手机可读写的 NDEF 区。整块板不需要电池。

> 本项目是硬件 + 固件的完整参考设计：PCB 天线、Gerber、EasyEDA 工程、KiCad 线圈封装、STM32 固件和配套的旋律提取流程都在仓库里。

## 特性

- **无源工作**：能量来自手机 NFC 射频场，无需电池和外部供电
- **欠压保护**：PVD 监测到电压跌落时停止播放并进入 STOP 模式，电压恢复后自动继续
- **差分 PWM**：TIM1 CH1/CH1N 驱动无源蜂鸣器，动态改频、50% 占空比
- **手机可写标签**：NTAG I²C plus（1K，I²C 地址 `0x55`）提供 NDEF 存储
- **防锁死**：固件每次上电都会重置 NTAG 的 CC、空 NDEF TLV 和 I²C 锁状态
- **天线可替换**：板上预留 `R0` 位置，可换成自定义线圈
- **旋律可换**：旋律以 `{频率, 时长}` 数组编译进固件，改完重新烧录即可

## 目录结构

| 路径 | 说明 |
| --- | --- |
| `firmware/` | STM32G031 固件（STM32CubeMX 工程 + CMake） |
| `hardware/gerber.zip` | Gerber 与钢网下单文件（嘉立创可直接下单） |
| `hardware/nfc-music-box_v2026-10-19.epro2` | 立创 EDA 专业版工程 |
| `hardware/kicad-coil/` | KiCad 线圈符号 / 封装 / BOM（48×75 mm，N3） |
| `demo/` | 示例 MIDI 与旋律 TXT |
| `docs/README.html` | 排版版中文说明（离线打开） |

## 硬件

| 项目 | 参数 |
| --- | --- |
| 主控 | STM32G031G8Ux，Cortex-M0+，UFQFPN28，64 KB Flash |
| NFC 标签 | NTAG I²C plus，1K，I²C 地址 `0x55` |
| 蜂鸣器 | 无源蜂鸣器，TIM1 CH1/CH1N 差分 PWM，1 MHz 计数时钟 |
| 天线 | 48 × 75 mm，3 匝（N3） |
| 取电 | NFC 射频场取电，PVD 阈值 2.2 V |

### 天线设计要点

- 板上 `R0` 是预留位，把封装替换成自己的天线即可。替换后出现的 **DRC 报错可以忽略**。
- 用 Pro Antenna Designer 生成的线圈默认使用插件焊盘，同样会带来可忽略的 DRC 报错。
- 建议 **Q 值 ≈ 20，且不要超过 20**，以保证带宽和能量传输。
- 线圈外围 **5 mm** 范围内（含正反两面）不要放焊盘、走线或其他金属。
- 谐振的微小偏差对性能影响约 0.05 dB，没有专业仪器时直接用 Designer 给出的 LC 匹配参数即可。

> **制板后必做**：KiCad 封装的内圈端子（Pad 2）是镀通孔，需要在 **B.Cu 上把它走线引出**。这一步不属于 Gerber 图形，板厂不会代做；不做的话线圈是开路的。详见 [`hardware/kicad-coil/README.md`](hardware/kicad-coil/README.md)。

## 固件

### 环境

- CMake ≥ 3.22、Ninja
- `arm-none-eabi-gcc`（Arm GNU Toolchain）
- STM32CubeProgrammer + ST-Link（烧录）
- 可选：STM32CubeMX（修改外设配置时用）

### 编译与烧录

```bash
cd firmware
cmake --preset Release
cmake --build --preset Release
```

产物为 `firmware/build/Release/project.elf`。用 ST-Link 烧录：

```bash
STM32_Programmer_CLI -c port=SWD -w firmware/build/Release/project.elf -v -rst
```

也可以直接用 STM32CubeProgrammer 图形界面打开 ELF 烧录。`firmware/project.ioc` 可以在 STM32CubeMX 中打开并重新生成代码；自定义逻辑都放在 `USER CODE` 区块内，重新生成后请检查是否保留。

### 换一首曲子

旋律在 [`firmware/Core/Src/music_data.c`](firmware/Core/Src/music_data.c)，每一项是：

```c
{频率(Hz), 时长(ms)}
```

频率填 `0` 表示休止符。改完数组后，把 [`firmware/Core/Inc/music_data.h`](firmware/Core/Inc/music_data.h) 里的 `MUSIC_LEN` 改成数组实际长度，再重新编译烧录。

### 主要文件

| 文件 | 作用 |
| --- | --- |
| `Core/Src/main.c` | 时钟、初始化、主循环、PVD 欠压处理 |
| `Core/Src/buzzer_driver.c` | PWM 动态改频、播放状态机、欠压暂停/恢复 |
| `Core/Src/music_data.c` | 旋律数组（默认示例曲） |
| `Core/Src/i2c.c` | NTAG I²C 初始化、CC/NDEF 写入、I²C 解锁 |
| `Core/Src/tim.c` | TIM1 PWM 配置 |

## NTAG 读写

使用 **NXP 官方 TagWriter**（[Google Play](https://play.google.com/store/apps/details?id=com.nxp.nfc.tagwriter)）读写标签。

> **首次写入数据集时，需要先擦除标签并格式化为 NDEF**，然后再写入数据。

固件每次上电/烧录后会通过 I²C 做以下初始化，保证手机侧始终可写：

1. 写入 CC：Block 0 的 `byte[12..15] = E1 10 6D 00`
2. 写入空 NDEF TLV：Block 1 的 `byte[0..3] = 03 00 FE 00`
3. 写入配置寄存器 Block `0x3A`（NFC 可写、20 ms WDT、不锁配置）
4. 清除 `NS_REG.I2C_LOCKED`（`0xFE` 寄存器 bit6）

如果手机提示标签只读或写不进去：先让板子重新上电，再靠近手机写入；仍不行时用 TagWriter 的「擦除标签 / 格式化为 NDEF」功能重来一次。

> **不要改** Block 0 的 `byte[0]`：它必须是 `0xAA` 才能保持 I²C 地址 `0x55`。写成 `0x04` 会让从机地址变成 `0x02`，后续 I²C 通信直接失联。

## 音频转换

按手头素材选一种方式：

| 情况 | 做法 |
| --- | --- |
| 已有简谱 / 乐谱 | 直接编辑 [`music_data.c`](firmware/Core/Src/music_data.c)，同步修改 `MUSIC_LEN` |
| 有单音轨 MIDI、没有旋律 TXT | 用 [prts.online](http://prts.online/) 提取旋律，选「保留最高音」 |
| 什么都没有 | 用 [MelodyExtractor 桌面版](https://github.com/Cabbagedog8266/melody-extractor) 从歌曲提取，再手动整理 |

提取工具输出的 TXT 每项是 `{频率, 时长}`（Hz / ms），每行五组；把它整理成 C 数组即可：

```c
const MusicNote_t music_score[MUSIC_LEN] = {
    {1396, 238}, {0, 25}, {932, 475}, {0, 52}, {1864, 476},
    /* ... */
};
```

`MelodyExtractor_Setup_v1.0.0.exe` 安装包放在本仓库的 [Releases](https://github.com/Cabbagedog8266/OpenNFC-MusicBox/releases) 中。

## 演示文件

- [`demo/midi1.mid`](demo/midi1.mid)：示例单音轨 MIDI
- [`demo/midi1_melody.txt`](demo/midi1_melody.txt)：示例旋律 TXT

示例音频 `song1.flac` **没有包含在仓库里**（版权原因），请用自己的音频运行提取工具。

## 常见问题

**编译后没有声音？**
- 确认用的是**无源**蜂鸣器（有源蜂鸣器只会响一个固定音）。
- 手机 NFC 天线位置要和板载线圈中心对齐，不同手机差异较大。
- 检查 TIM1 分频：`tim.c` 的 `Prescaler = 15` 对应 `BUZZER_PWM_CLK = 1 MHz`，两者必须一致。

**高音上不去 / 音准不对？**
- 提高 TIM1 时钟或减小 `Prescaler`，同时同步修改 `buzzer_driver.c` 里的 `BUZZER_PWM_CLK`。

**播放中途断掉？**
- 多半是 NFC 场强不足触发 PVD。贴紧手机、移开金属，或降低音量/音高负载再试。固件在电压恢复后会自动继续。

**手机写不了标签？**
- 先做一次「擦除 + 格式化为 NDEF」；再让板子重新上电清除 I²C 锁。

**DRC 报错要不要管？**
- 天线相关的 DRC 报错是预期内的，可以忽略；其他电气规则错误仍应检查。

## 第三方与版权

- STM32 HAL / CMSIS：ST 的 BSD-3-Clause 许可，版权归 STMicroelectronics。
- NXP TagWriter：第三方软件，**未随本仓库分发**，请从 NXP 官方渠道获取。
- MelodyExtractor：代码 MIT；模型权重 CC BY-NC 4.0（**非商用**），详见其仓库。
- 示例曲目《Storyteller》版权归塞壬唱片-MSR 所有，仅用于学习与演示。

本仓库暂未声明开源许可证（默认保留所有权利）。如需转载、二次分发或商用，请先与作者联系。

## 免责声明

本项目为个人学习与爱好者项目。NFC 取电的可用功率、读写距离和稳定性取决于手机、天线和 PCB 装配，不同设备差异很大；批量生产前请自行完成电气、射频与合规验证。
