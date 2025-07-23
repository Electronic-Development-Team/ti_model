#ifndef ICM42688_DRIVER_H
#define ICM42688_DRIVER_H

#include <stdint.h>
#include <stdbool.h>
#include "math.h"

// ICM42688寄存器地址定义
#define ICM42688_I2C_ADDR          0x68
#define ICM42688_WHO_AM_I_REG       0x75
#define ICM42688_WHO_AM_I_VALUE     0x47

// 配置参数
#define ICM42688_CALIBRATION_SAMPLES    200
#define ICM42688_ALPHA                  0.98f

// ICM42688姿态角结构体
typedef struct {
    float pitch;                // 俯仰角 (度)
    float roll;                 // 横滚角 (度)
    float yaw;                  // 偏航角 (度)
} ICM42688_Attitude_t;

// ICM42688驱动结构体
typedef struct {
    int16_t raw_accel[3];       // 原始加速度
    int16_t raw_gyro[3];        // 原始陀螺仪
    float temperature;          // 温度
    float gyro_offset[3];       // 陀螺仪偏移
    float accel_offset[3];      // 加速度偏移
    ICM42688_Attitude_t attitude; // 姿态角
    bool is_initialized;        // 初始化状态
    bool is_calibrated;         // 校准状态
} ICM42688_Handle_t;

// 回调函数类型
typedef void (*ICM42688_ProgressCallback_t)(uint8_t progress);

// 函数声明
bool ICM42688_Init(ICM42688_Handle_t *handle);
bool ICM42688_TestHardware(void);
bool ICM42688_Calibrate(ICM42688_Handle_t *handle, ICM42688_ProgressCallback_t callback);
void ICM42688_Update(ICM42688_Handle_t *handle, float dt);
void ICM42688_DisplayAngles(ICM42688_Handle_t *handle, uint32_t counter);
void ICM42688_RunSimulation(void);

#endif // ICM42688_DRIVER_H