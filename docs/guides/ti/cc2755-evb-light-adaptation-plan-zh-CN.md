# CC2755R10 EVB Matter Light 固件完整适配方案

## 1. 文档目的

本文给出将本仓库 TI CC27xx Matter Lighting 示例适配到 CC2755R10
定制 EVB 的完整实施方案。目标硬件带有 240 x 240 LCD、共阳 RGB LED、
四个按键和调试 UART；量产固件还需要兼容 Factory Data、PSA/HSM、Secure
Boot 和 Matter OTA 流程。

本文是实施设计和验收基线，不表示相关代码已经完成。所有地址、引脚和构建
结果在进入量产前都必须通过 CC2755R10 真机及最终量产烧录链路验证。

## 2. 适配目标

完成适配后，固件应满足以下要求：

1. 在 CC2755R10 EVB 上正常启动 Matter Lighting 应用。
2. 支持 BLE 配网和 Thread 入网，能够被 Matter Controller 控制。
3. 使用板载共阳 RGB LED 表示灯光状态和 Identify 状态。
4. 支持四个板载按键，保留配网、开关和恢复出厂等关键操作。
5. 通过 SPI 驱动板载 240 x 240 LCD，并显示启动、配网、在线和灯光状态。
6. LCD 刷新不得长期阻塞 Matter、BLE 或 Thread 事件处理。
7. Factory Data、Matter NVS、PSA ITS、HSM 固件和应用程序之间不得发生
   Flash 区域重叠。
8. 支持现有生产流程使用的外部签名、Secure Boot 和固件发布流程。
9. 在没有外部 SPI Flash 的情况下明确限制 OTA，或者为 OTA 提供经验证的
   替代存储方案。

## 3. 输入资料与代码基线

### 3.1 当前 Matter 工程

主要代码位于：

- `examples/lighting-app/ti/cc27xx`
- `examples/platform/ti/sysconfig/chip_cc27xx.syscfg`
- `src/platform/ti/cc27xx`
- `src/platform/ti/FactoryDataProvider.cpp`
- `third_party/ti_simplelink_sdk`

当前默认板型仍为：

```gn
ti_simplelink_board = "LP_EM_CC2745R10_Q1"
```

仓库已经包含 CC27xx x10 系列的 GN、链接脚本和 SysConfig 基础配置，
因此不需要新建完整的平台端口，但需要增加 `LP_EM_CC2755R10_BG` 板型映射
和定制 EVB 的板级资源配置。

### 3.2 LCD 参考工程

参考工程位于：

```text
C:\Users\DELL\Desktop\matter\TI\2755_Lcd_TI_Demo_board\
  src\Lcd_LP_EM_CC2755P10_nortos_ticlang
```

参考工程环境：

| 项目 | 参考值 |
| --- | --- |
| 目标芯片 | CC2755P10 |
| RTOS | NoRTOS |
| 编译器 | TI Arm Clang 5.1.1.LTS |
| SimpleLink SDK | 9.20.01.21 |
| SysConfig | 1.28.0 |
| LCD | ST7789 兼容，240 x 240 |
| 像素格式 | RGB565，16 bit/pixel |
| SPI | 8 MHz，Mode 0，8 bit，阻塞传输 |

参考工程可作为 LCD 初始化序列、SPI 参数和 EVB 引脚分配的来源，但不能直接
作为 Matter 工程导入，因为目标工程使用 CC2755R10、FreeRTOS、GN/GCC 和
Matter 自己的任务模型。

### 3.3 版本要求

首版适配建议固定使用以下工具版本，以减少 SysConfig 自动求解差异：

- TI SimpleLink Low Power F3 SDK 9.20.01.21
- SysConfig 1.28.0
- 仓库当前支持的 Arm GCC 工具链

如 CI 使用其他 SDK 或 SysConfig 版本，必须检查生成的引脚、DMA、驱动库和
链接布局是否与本地一致。

## 4. 已确认的硬件资源

### 4.1 LCD

| 信号 | CC2755 DIO | 方向 | 有效电平/说明 |
| --- | ---: | --- | --- |
| LCD_BLK | DIO0 | 输出 | 高电平打开背光 |
| LCD_RES | DIO7 | 输出 | 低电平复位 |
| LCD_MOSI/SDA | DIO11 | 输出 | SPI PICO/MOSI |
| LCD_DC | DIO12 | 输出 | 低为命令，高为数据 |
| LCD_SCLK | DIO15 | 输出 | SPI 时钟 |
| LCD_CS | DIO16 | 输出 | 低电平选中 |

LCD 为写入型设备，应用没有读取 LCD 寄存器的需求。若 TI SPI 驱动必须分配
POCI/MISO，应选择未使用且不会影响量产硬件的引脚；若 SysConfig 支持不分配
POCI，则优先不分配，避免无意义地占用 DIO28A0。

### 4.2 RGB LED

| 通道 | CC2755 DIO | 说明 |
| --- | ---: | --- |
| Red | DIO3 | 共阳结构，低电平点亮 |
| Green | DIO4 | 共阳结构，低电平点亮 |
| Blue | DIO5 | 共阳结构，低电平点亮 |

所有通道上电初始值必须为高，避免启动阶段闪烁。TI LED API 的 On/Off 语义
不等同于该硬件的发光语义，建议在板级驱动中封装颜色接口，不允许业务代码
直接依赖 `LED_setOn()` 和 `LED_setOff()` 的字面意义。

### 4.3 按键

| 按键 | CC2755 DIO | 输入特性 |
| --- | ---: | --- |
| SW1 | DIO19 | 外部上拉，按下为低 |
| SW2 | DIO20 | 外部上拉，按下为低 |
| SW3 | DIO18 | 外部上拉，按下为低 |
| SW4 | DIO17 | 外部上拉，按下为低 |

按键应使用 TI Button 驱动完成去抖，回调只投递 AppEvent，不在中断回调里执行
Matter 操作、LCD 刷新或长时间日志输出。

### 4.4 调试串口

| 信号 | CC2755 DIO |
| --- | ---: |
| UART TX | DIO1 |
| UART RX | DIO2 |

## 5. 参考 Demo 可复用范围

### 5.1 可以移植

- ST7789 兼容初始化命令序列。
- LCD 硬件复位和退出睡眠时序。
- SPI Mode 0、8 MHz、8 bit 参数。
- `CASET`、`RASET`、`RAMWR` 窗口写入方式。
- RGB565 大端字节顺序。
- 256 字节分块清屏方法。
- 首帧写完后再打开背光的策略。
- GPIO、SPI、RGB LED 和四个按键的引脚映射。

### 5.2 不能直接移植

- `main_nortos.c` 和 `NoRTOS_start()`。
- Demo 的无限轮询主循环。
- CCS `.project`、`.cproject`、`.out` 和 `.hex`。
- `Debug/syscfg` 下的自动生成文件。
- P10 专用链接文件和目标配置。
- TI Clang 专用构建参数。
- 在按键回调后直接由主循环执行整屏刷新的任务模型。

### 5.3 许可和资源注意事项

`lcdDisplay.c` 带有 TI BSD 风格许可，移植时必须保留原始版权和许可头。
`lcd_logo.h` 没有明确的软件许可头，并包含公司 Logo 图像。没有确认素材授权前，
不要把该文件直接提交到仓库；可以先使用纯色、文本或项目已授权的图片验证驱动。

## 6. 当前工程差距和阻断项

### 6.1 默认板型不正确

`examples/lighting-app/ti/cc27xx/args.gni` 默认选择
`LP_EM_CC2745R10_Q1`。EVB 构建必须显式使用：

```gn
ti_simplelink_board = "LP_EM_CC2755R10_BG"
```

不建议为了 EVB 直接覆盖 TI 参考板配置。定制板配置应有独立开关或
独立 SysConfig 文件，保证上游标准开发板仍能构建。

### 6.2 默认 LED 和按键冲突

当前 `chip_cc27xx.syscfg` 把 LED 和按钮绑定到 TI LaunchPad 自带硬件。这些引脚
与 EVB LCD、RGB 和按键并不一致，必须由 EVB 配置替换。

### 6.3 外部 SPI Flash 与 EVB 冲突

当前 SysConfig 创建了 `CONFIG_NVSEXTERNAL` 并绑定 LaunchPad 的
MX25R8035F。该外部 Flash 使用的部分引脚与 EVB 的 DIO3、DIO4、DIO5、DIO19
冲突，也就是与 RGB LED 和 SW1 冲突。

EVB 构建必须解除该硬件绑定。由于 `OTAImageProcessorImpl` 当前通过
`CONFIG_NVSEXTERNAL` 下载 OTA 镜像，因此需要二选一：

1. 第一阶段保持 `chip_enable_ota_requestor=false`，明确不支持设备端 Matter OTA。
2. 增加独立外部 Flash 或设计内部 Flash OTA 方案，并重新实现和验证 OTA
   Image Processor。

在没有确定 OTA 存储介质前，不允许仅为了通过 SysConfig 而把外部 NVS 随意
映射到未连接引脚。

### 6.4 R10 PSA/HSM 地址布局

CC2755R10 使用 1 MiB x10 Flash 布局：

- PSA ITS：`0x000DF000`，长度 `0x2000`
- Matter NVS：`0x000E1000`，长度 `0x6000`
- Factory Data：`0x000E7000`，长度 `0x1000`
- HSM 保留区从 `0x000E8000` 起

应用必须在 `0x000DC000` 前结束，并通过 CCFG 的
`chipEraseRetain_mainSectors256_511` 保留 Flash 尾部。该字段位于
`0x4E02003C`，期望值为 `0xFF800000`。构建流程必须验证：

```text
R10 PSA ITS end == R10 Matter NVS start
R10 Matter NVS end == R10 Factory Data start
R10 Factory Data end <= R10 HSM reserved start
```

### 6.5 Matter Lighting 行为不完整

当前物理输出主要跟随 OnOff 和 Identify。虽然 Lighting Common 数据模型可能
包含 Level Control 或 Color Control，当前 `ZclCallbacks.cpp` 并没有完整地将
这些属性映射到 RGB 输出。首版应明确只实现 OnOff，后续再增加亮度和颜色，
避免 UI 表现与 Matter 属性不一致。

## 7. 推荐软件架构

```text
Matter Controller / Local Buttons
               |
               v
          AppTask event queue
               |
       +-------+--------+
       |                |
       v                v
 LightingManager   DisplayManager
       |                |
       v                v
  BoardRgbLed       LCD worker task
                         |
                         v
                 St7789Display driver
                         |
                         v
                 TI SPI + GPIO driver
```

架构原则：

1. Matter 属性、按键和 Identify 最终都通过 AppTask 串行协调。
2. RGB 驱动只负责硬件极性和颜色输出，不维护 Matter 属性。
3. DisplayManager 把业务状态转换为显示命令。
4. LCD Worker 是低优先级 FreeRTOS 任务，独占 SPI 句柄。
5. 中断回调和 Matter 平台事件线程都不直接刷新 LCD。
6. 队列只保留最新状态，避免网络状态快速变化时积压多次整屏刷新。

## 8. 文件级实施方案

### 8.1 构建参数

修改 `examples/lighting-app/ti/cc27xx/args.gni`：

- 保留现有默认板型，避免破坏 CC2745 构建。
- 增加类似 `ti_cc2755_evb = false` 的定制板开关。
- EVB 构建命令同时指定：

```gn
ti_simplelink_board = "LP_EM_CC2755R10_BG"
ti_cc2755_evb = true
```

### 8.2 SysConfig

复用 `examples/platform/ti/sysconfig/chip_cc27xx.syscfg`，由
`examples/lighting-app/ti/cc27xx/BUILD.gn` 在 `ti_cc2755_evb=true` 时
传入专用脚本参数并启用 EVB 外设和引脚。标准构建传入 `false`，不会让
`LP_EM_CC2755R10_BG` 参考板自动变成定制 EVB 引脚。

EVB SysConfig 必须包含：

- Matter 原有 AESCCM、AESECB、ECDH、RNG、SHA2、BLE、RCL 和内部 NVS。
- LCD SPI 实例和四个控制 GPIO。
- 三路共阳 RGB 输出，初始电平为高。
- 四个外部上拉、低电平有效按键。
- 调试 UART。
- R10 正确的 PSA KeyStore/ITS 地址。
- 不绑定 LaunchPad MX25R8035F。

若直接从标准 `chip_cc27xx.syscfg` 派生文件，应定期对照上游安全、BLE 和射频
配置变更，避免两个 SysConfig 文件长期漂移。后续可把公共逻辑抽到 SysConfig
辅助脚本，但不建议在首版适配中同时进行大范围重构。

### 8.3 LCD 驱动

建议新增：

```text
examples/lighting-app/ti/cc27xx/src/St7789Display.h
examples/lighting-app/ti/cc27xx/src/St7789Display.cpp
```

驱动接口建议控制在以下范围：

```cpp
class St7789Display
{
public:
    CHIP_ERROR Init();
    CHIP_ERROR SetBacklight(bool enabled);
    CHIP_ERROR Fill(uint16_t rgb565);
    CHIP_ERROR DrawImage(uint16_t x, uint16_t y, uint16_t width,
                         uint16_t height, const uint8_t * data);
};
```

实现要求：

- SPI 参数固定为 8 MHz、Mode 0、8 bit。
- CS、DC、RES 和 BLK 由 SysConfig 名称引用，不在驱动中写数字引脚。
- 保留 256 字节静态传输缓冲，不分配完整帧缓冲。
- 所有坐标和长度执行溢出、空指针及边界检查。
- SPI 传输失败必须返回错误并记录有限日志，不进入无日志死循环。
- 初始化失败时保持背光关闭，Matter 核心功能仍应继续运行。
- 仅 LCD Worker 调用驱动，避免增加无必要的全局锁。

整屏 RGB565 数据量为：

```text
240 x 240 x 2 = 115200 bytes
```

在 8 MHz 下纯传输理论下限约 115 ms，实际还包含 SPI 调用、命令和任务切换
开销。因此不能在 AppTask 或 Matter Platform 事件线程中执行整屏刷新。

### 8.4 DisplayManager 和 LCD Task

建议新增：

```text
examples/lighting-app/ti/cc27xx/src/DisplayManager.h
examples/lighting-app/ti/cc27xx/src/DisplayManager.cpp
```

DisplayManager 至少支持以下状态：

- Booting
- Commissionable
- Commissioning
- ThreadAttached
- OnlineLightOn
- OnlineLightOff
- Identify
- Error

任务实现要求：

- 独立低优先级 FreeRTOS Task。
- 使用长度较小的队列或单槽覆盖式通知。
- 启动时先关闭背光、初始化 LCD、绘制首帧，再打开背光。
- 删除参考 demo 中仅用于展示的 1 秒阻塞等待。
- 重复状态不重复刷新。
- Identify 等高频效果优先局部刷新或降低刷新频率。
- LCD 不可用时关闭该功能，不影响 BLE、Thread、Matter 和按键。

### 8.5 RGB 板级驱动

建议新增：

```text
examples/lighting-app/ti/cc27xx/src/BoardRgbLed.h
examples/lighting-app/ti/cc27xx/src/BoardRgbLed.cpp
```

提供明确的硬件语义：

```cpp
void SetRgb(uint8_t red, uint8_t green, uint8_t blue);
void SetOff();
void SetIdentify(bool enabled);
```

首版可以只实现开关值；如果后续实现亮度，确认 TI LED/PWM 资源不会与 SPI DMA、
BLE 或 Thread 使用的定时器发生冲突。共阳极极性必须只在本驱动中处理。

### 8.6 AppTask

修改 `examples/lighting-app/ti/cc27xx/src/AppTask.cpp`：

- EVB 构建下打开四个 Button handle。
- Button 回调只生成 `AppEvent`。
- 初始化 DisplayManager 和 BoardRgbLed。
- DeviceEventCallback 中把配网和 Thread 状态投递给 DisplayManager。
- Identify 开始和结束时同步 RGB/LCD 状态。
- Matter OnOff 完成后更新 RGB 和 LCD。
- 检查所有 `LED_open()`、`Button_open()` 和显示初始化返回值。

推荐按键定义：

| 按键 | 短按 | 长按 |
| --- | --- | --- |
| SW1 | 灯开/关切换 | 保留 |
| SW2 | 打开配网窗口 | 恢复出厂 |
| SW3 | 切换显示页面或调试颜色 | 保留 |
| SW4 | 切换显示页面或背光 | 保留 |

最终按键语义应由产品需求确认。恢复出厂必须要求长按，并保留现有延迟复位、
撤销 DNS-SD/SRP 服务的处理。

### 8.7 LightingManager 和 ZCL 回调

首版范围：

- OnOff Cluster 控制 RGB 开/关。
- Identify Cluster 控制识别效果。
- 保持本地按键动作和 Matter 属性双向一致。

二期范围：

- Level Control 映射到 RGB 亮度。
- Color Control 映射 Hue/Saturation 或 XY 到 RGB。
- 完整处理属性恢复、上电状态和控制器写入。

不要仅改变 RGB 物理颜色而不更新 Matter 属性；也不要只更新 Matter 属性却不
同步物理输出。

### 8.8 BUILD.gn

修改 `examples/lighting-app/ti/cc27xx/BUILD.gn`：

- EVB 模式向共享的 `chip_cc27xx.syscfg` 传入专用脚本参数。
- 加入 `St7789Display.cpp`、`DisplayManager.cpp` 和 `BoardRgbLed.cpp`。
- 图片、字体等资源只在 EVB 模式编译。
- 不加入参考工程的 CCS 生成文件。

## 9. Flash、安全和量产适配

### 9.1 统一地址来源

不应继续在 SysConfig、C++ 和链接脚本中分别维护无法关联的数字地址。建议增加
x10/x20 平台地址定义，并至少让以下组件使用同一组值：

- Linker script
- CryptoKeyKeyStore_PSA SysConfig 配置
- FactoryDataProvider 运行时检查
- 临时烧录/Provisioning 固件
- CI 镜像布局验证脚本

如果 SysConfig 不能直接引用 C/C++ 头文件，则由同一构建参数选择 x10/x20 地址，
并在 CI 中解析 map 文件和生成配置进行交叉检查。

### 9.2 R10 布局验收

构建后必须从 `.map`、`.hex` 和 SysConfig 生成文件确认：

1. 应用正文未进入 NVS、Factory Data 或 HSM 区域。
2. PSA ITS 不与应用或 Matter NVS 重叠。
3. `CONFIG_NVSINTERNAL` 与链接脚本 `.nvs` 地址一致。
4. `.factory_data` 位于 `0xE7000`，长度不超过 `0x1000`。
5. HSM 固件和保留扇区不会被普通应用下载擦除。
6. 恢复出厂只清理 Matter 可变数据，不清除 DAC 私钥和 Factory Data。

### 9.3 Secure Boot

外部签名流程应使用最终 R10 镜像执行，而不是沿用 CC2745 文件名或板型假设。
签名后至少检查：

- 镜像目标芯片和入口地址正确。
- Secure Boot 头和签名覆盖范围正确。
- 签名镜像仍保留 Factory Data、PSA ITS 和 NVS 约定。
- 未签名镜像不能在启用 Secure Boot 的量产设备上启动。
- 使用正确公钥的签名镜像可以冷启动和软件复位。

### 9.4 Factory Data 与 DAC Key

量产配置应继续采用 DAC 私钥存储在 PSA/HSM、证书存储在 Factory Data 的方案。
验收内容：

- DAC 私钥不能出现在最终 Factory Data HEX 中。
- `psa_get_key_attributes()` 能读取持久化 P-256 密钥。
- Matter DAC challenge 签名成功。
- 恢复出厂后 DAC Key 仍存在。
- 应用升级后 DAC Key ID 和存储地址不变。

## 10. 构建与 CI 方案

### 10.1 本地构建参数示例

具体路径根据构建机安装位置填写：

```bash
gn gen out/cc2755-evb --args='\
  ti_simplelink_board="LP_EM_CC2755R10_BG" \
  ti_cc2755_evb=true \
  custom_factory_data=true \
  ti_dac_key_use_psa_hsm=true \
  chip_enable_ota_requestor=false'
ninja -C out/cc2755-evb
```

首次实现时应同时保留一个标准 CC2745 或 P20 构建，防止公共平台修改
产生回归。

### 10.2 CI 检查项

- SysConfig 命令成功，无 pinmux 和 DMA 冲突。
- EVB Matter light 构建成功。
- 标准 CC27xx 基线构建成功。
- 解析 map 文件检查 `.factory_data`、`.nvs` 和应用 Flash 边界。
- 检查生成文件中的 LCD/RGB/按键 DIO 值。
- 检查最终 HEX 不覆盖禁止区域。
- 对最终发布镜像执行外部签名并验证签名结果。
- 上传 map、elf/out、hex 和签名镜像供追踪。

## 11. 真机验证计划

### 11.1 板级冒烟测试

1. 上电时 RGB 不应闪烁，LCD 背光保持关闭。
2. LCD 复位和初始化成功后显示首帧，再打开背光。
3. 检查 LCD 方向、颜色顺序、边界和坏行。
4. 分别验证 RGB 三色和关闭状态。
5. 分别验证四个按键短按、长按及去抖。
6. 验证 UART TX/RX 不与其他功能冲突。

### 11.2 Matter 功能测试

1. Android/iOS Controller 均能发现和配网。
2. BLE 配网后正常切换到 Thread。
3. OnOff 命令同步更新 Matter 属性、RGB 和 LCD。
4. Identify 命令可见且结束后恢复原状态。
5. 本地按键操作能够向订阅方报告属性变化。
6. 断电重启后保留 Fabric 和期望灯光状态。
7. 删除最后一个 Fabric 后正常恢复到可配网状态。
8. 长按恢复出厂不会破坏 Factory Data 和 DAC Key。

### 11.3 并发和稳定性测试

1. LCD 整屏刷新时持续执行 Matter OnOff 命令。
2. LCD 刷新时进行 Thread attach、BLE 连接和配网。
3. 连续切换显示状态至少 1000 次，检查堆、栈和 SPI 错误。
4. 连续运行 24 小时，检查 Thread 掉线、看门狗和任务阻塞。
5. 强制 SPI 失败或不连接 LCD，确认 Matter 核心仍能运行。
6. 测量 LCD Task、AppTask 和 Matter Event Loop 的栈高水位。

### 11.4 安全和升级测试

1. 未签名、错误签名和正确签名镜像的启动行为符合预期。
2. 工厂烧录后 DAC challenge 签名成功。
3. 恢复出厂、重新配网后 DAC Key 保持不变。
4. 若启用 OTA，验证下载、重启、确认、回滚和断电恢复。
5. 升级前后的 NVS、Factory Data 和 PSA ITS 内容均符合保留策略。

## 12. 分阶段实施与验收标准

### 阶段 A：R10 基线构建

工作内容：

- 切换 `LP_EM_CC2755R10_BG`。
- 验证 R10 PSA/NVS/Factory Data 地址一致性。
- 生成并检查 map/hex。

验收标准：

- R10 Matter light 可构建和启动。
- BLE、Thread 和基本 OnOff 正常。
- Flash 区域检查全部通过。

### 阶段 B：EVB SysConfig 和基础 IO

工作内容：

- 增加 EVB SysConfig。
- 配置 RGB、四按键、LCD GPIO/SPI 和 UART。
- 解除外部 Flash 引脚冲突。

验收标准：

- SysConfig 无冲突。
- RGB 三色、四按键和 UART 真机测试通过。

### 阶段 C：LCD 驱动和任务化

工作内容：

- 移植 ST7789 初始化及绘图代码。
- 实现 DisplayManager 和 LCD Worker。
- 完成首帧、背光和错误降级处理。

验收标准：

- 冷启动稳定显示。
- LCD 刷新期间 Matter/Thread 无明显阻塞。
- LCD 缺失时 Matter 功能仍可运行。

### 阶段 D：Matter 行为集成

工作内容：

- 接入 OnOff、Identify、设备事件和四按键。
- 同步 RGB、LCD 和 Matter 属性。

验收标准：

- 控制器和本地按键双向状态一致。
- 配网、删除 Fabric 和恢复出厂正常。

### 阶段 E：量产和回归

工作内容：

- Factory Data、PSA/HSM、Secure Boot 和发布流水线验证。
- 稳定性、掉电、重复配网和升级回归。

验收标准：

- 签名镜像可稳定启动。
- DAC Key 生命周期符合量产要求。
- 24 小时稳定性测试通过。
- 发布产物可追溯且不包含明文 DAC 私钥。

## 13. 主要风险与缓解措施

| 风险 | 影响 | 缓解措施 |
| --- | --- | --- |
| P10 Demo 与 R10 目标差异 | 生成配置或驱动资源不一致 | 只复用驱动逻辑，使用 R10 SysConfig 重新生成 |
| LCD 整屏刷新阻塞 | Matter/Thread 超时或掉线 | 独立低优先级任务，合并状态，优先局部刷新 |
| 外部 Flash 引脚冲突 | RGB、按键或 OTA 不可用 | EVB 禁用默认外部 NVS，单独设计 OTA 存储 |
| PSA 地址硬编码 | 覆盖密钥、NVS 或 HSM | 统一地址来源，map/hex 自动检查 |
| 共阳 RGB 极性错误 | 上电常亮或颜色相反 | 板级驱动统一反相，上电默认高 |
| Logo 资源许可不明 | 无法合法提交或发布 | 使用已授权资源，保留代码许可 |
| SysConfig 版本变化 | 引脚/DMA 自动求解改变 | CI 固定 1.28.0，并检查生成结果 |
| LCD 硬件缺失或异常 | 启动卡死 | 显示失败降级，不阻断 Matter 启动 |

## 14. 推荐提交拆分

建议按以下顺序提交，便于审查和回退：

1. `ti: add CC2755R10 persistent storage layout`
2. `ti: add CC2755 EVB SysConfig and board IO`
3. `ti: add ST7789 display driver`
4. `ti: add asynchronous display manager`
5. `ti: integrate CC2755 EVB controls with lighting app`
6. `ci: build and validate CC2755 EVB image`
7. `docs: document CC2755 EVB build and validation`

每个提交都应保持可构建；涉及安全存储和链接布局的提交不得与 LCD UI 修改混在
同一个提交中。

## 15. 完成定义

只有同时满足以下条件，才能认为 CC2755R10 EVB 适配完成：

- EVB 专用构建在本地和 CI 均通过。
- LCD、RGB、四按键和 UART 真机验证通过。
- Android/iOS Matter 配网及 Thread 控制通过。
- OnOff、Identify、本地按键和显示状态一致。
- LCD 刷新不会阻断 Matter、BLE 或 Thread。
- R10 Flash 布局经过 map/hex 自动检查和人工复核。
- Factory Data、DAC Key、PSA/HSM 和恢复出厂流程验证通过。
- Secure Boot 签名镜像验证通过。
- OTA 能力已完成验证，或者在产品范围和构建配置中明确关闭。
- 24 小时稳定性测试通过，并保存测试固件、日志和版本信息。
