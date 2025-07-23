# 5_gyroscope

## 使用说明

1. syscfg 初始化 iic

![](picture_library\Screenshot_2025-07-23_151643.png)

2. 调用初始化函数

```c
    // 硬件测试
    if (!ICM42688_TestHardware()) {
        ICM42688_RunSimulation();
        return 0;
    }

    // 初始化ICM42688
    if (!ICM42688_Init(&icm42688)) {
        ICM42688_RunSimulation();
        return 0;
    }

    // 校准
    if (!ICM42688_Calibrate(&icm42688, progress_callback)) {
        ICM42688_RunSimulation();
        return 0;
    }
```

3. 调用更新函数

```c
ICM42688_Update(&icm42688, dt);
```
