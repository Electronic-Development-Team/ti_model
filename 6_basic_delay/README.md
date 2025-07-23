# 6. 基础延时

## 文件结构

```
6_systick_delay/
├── code_library/
│   ├── delay.c   # 延时函数实现
│   └── delay.h   # 延时函数头文件
└── README.md     # 本说明文件
```

## API 说明

### `void delay_ms(uint16_t ms)`

-   **功能**: 毫秒 (ms) 级延时。
-   **参数**: `ms` - 需要延时的毫秒数，最大值为 65535。
-   **返回值**: 无
-   **依赖**: 该函数内部调用了 `delay_cycles`，并依赖于 `CPUCLK_FREQ` 宏定义来计算延时。请确保 `CPUCLK_FREQ` 已在 `ti_msp_dl_config.h` 或相关配置文件中被正确定义为您的 CPU 时钟频率。

## 使用方法

1.  将 `code_library` 文件夹中的 `delay.c` 和 `delay.h` 添加到您的项目中。
2.  在您的代码中包含头文件 `#include "delay.h"`。
3.  确保您的项目中包含了 `ti_msp_dl_config.h` 头文件，并且 `CPUCLK_FREQ` 宏被正确定义。
4.  在需要延时的地方，调用 `delay_ms()` 函数。

**示例:**

```c
#include "delay.h"
#include "ti_msp_dl_config.h" // 确保包含了定义 CPUCLK_FREQ 的头文件

int main(void)
{
    // 系统初始化
    // ...

    while(1)
    {
        // 延时 500ms
        delay_ms(500);
        // 执行其他操作
    }
}
```

## 注意事项

-   本延时函数是一个简单的软件延时，延时期间会占用 CPU。
-   `delay_ms` 的最大延时时间受限于其参数类型 `uint16_t`，最大为 65535 毫秒。
-   延时的精确度依赖于 `CPUCLK_FREQ` 宏的准确性。
-   如果在实时操作系统 (RTOS) 环境下开发，建议使用操作系统提供的延时函数，以避免任务阻塞和影响系统调度。
