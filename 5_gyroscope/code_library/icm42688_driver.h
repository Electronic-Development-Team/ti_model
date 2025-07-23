#ifndef ICM42688_DRIVER_H
#define ICM42688_DRIVER_H

#include <stdint.h>
#include <stdbool.h>
#include "math.h"

// ICM42688�Ĵ�����ַ����
#define ICM42688_I2C_ADDR          0x68
#define ICM42688_WHO_AM_I_REG       0x75
#define ICM42688_WHO_AM_I_VALUE     0x47

// ���ò���
#define ICM42688_CALIBRATION_SAMPLES    200
#define ICM42688_ALPHA                  0.98f

// ICM42688��̬�ǽṹ��
typedef struct {
    float pitch;                // ������ (��)
    float roll;                 // ����� (��)
    float yaw;                  // ƫ���� (��)
} ICM42688_Attitude_t;

// ICM42688�����ṹ��
typedef struct {
    int16_t raw_accel[3];       // ԭʼ���ٶ�
    int16_t raw_gyro[3];        // ԭʼ������
    float temperature;          // �¶�
    float gyro_offset[3];       // ������ƫ��
    float accel_offset[3];      // ���ٶ�ƫ��
    ICM42688_Attitude_t attitude; // ��̬��
    bool is_initialized;        // ��ʼ��״̬
    bool is_calibrated;         // У׼״̬
} ICM42688_Handle_t;

// �ص���������
typedef void (*ICM42688_ProgressCallback_t)(uint8_t progress);

// ��������
bool ICM42688_Init(ICM42688_Handle_t *handle);
bool ICM42688_TestHardware(void);
bool ICM42688_Calibrate(ICM42688_Handle_t *handle, ICM42688_ProgressCallback_t callback);
void ICM42688_Update(ICM42688_Handle_t *handle, float dt);
void ICM42688_DisplayAngles(ICM42688_Handle_t *handle, uint32_t counter);
void ICM42688_RunSimulation(void);

#endif // ICM42688_DRIVER_H