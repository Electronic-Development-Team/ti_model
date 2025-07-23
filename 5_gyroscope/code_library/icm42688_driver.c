#include "icm42688_driver.h"
#include "drv_imu.h"
#include "drv_oled.h"
#include "ti_msp_dl_config.h"
#include <string.h>
#include <stdlib.h>  // ���rand()����ͷ�ļ�

// �ⲿ��������
extern void Single_WriteI2C(unsigned char SlaveAddress, unsigned char REG_Address, unsigned char REG_data);
extern unsigned char Single_ReadI2C(unsigned char SlaveAddress, unsigned char REG_Address);
extern uint8_t icm42688_init(void);
extern void icm42688_read(int16_t *gyro, int16_t *accel, float *temperature);

// �򵥵ķ�������ʱ��ʹ��TI�⺯����
#define SIMPLE_DELAY (800000)  // ������ʱʱ��

// �򵥵�α�����������������stdlib������
static uint32_t simple_rand_seed = 1;
static uint32_t simple_rand(void) {
    simple_rand_seed = simple_rand_seed * 1103515245 + 12345;
    return (simple_rand_seed >> 16) & 0x7FFF;
}

// ȫ��״̬��־
static volatile bool communication_ok = true;
static volatile uint32_t error_count = 0;
static volatile uint32_t consecutive_errors = 0;
static volatile bool force_simulation = false;

// ������I2C��ȡ - ����ʧ�ܻ���
static uint8_t ultra_fast_read_i2c(uint8_t addr, uint8_t reg) {
    // �����������̫�ֱ࣬������I2C����
    if (consecutive_errors > 3) {
        force_simulation = true;
        return 0xFF;
    }
    
    volatile uint32_t timeout = 1000;  // ���̵ĳ�ʱʱ��
    uint8_t result = 0xFF;
    
    // �ǳ��̵ĳ���ʱ��
    for (volatile uint32_t i = 0; i < timeout; i++) {
        result = Single_ReadI2C(addr, reg);
        if (result != 0xFF && result != 0x00) {
            communication_ok = true;
            consecutive_errors = 0;
            return result;
        }
        // ÿ100�ξͷ���
        if (i % 100 == 0 && i > 0) break;
    }
    
    // ����ʧ��
    communication_ok = false;
    error_count++;
    consecutive_errors++;
    return 0xFF;
}

// �������Ĵ��������ݶ�ȡ
static bool safe_icm42688_read(int16_t *gyro, int16_t *accel, float *temperature) {
    // ǿ��ģ��ģʽ
    if (force_simulation || consecutive_errors > 5) {
        // ����ģ�����ݣ���ȫ����I2C����
        for (int i = 0; i < 3; i++) {
            gyro[i] = (int16_t)(simple_rand() % 200 - 100);  // С��Χ���ֵ
            accel[i] = (i == 2) ? 16384 : (int16_t)(simple_rand() % 1000 - 500);
        }
        *temperature = 25.0f + (simple_rand() % 10 - 5);
        return false;
    }
    
    // ���ͨ��״̬
    if (!communication_ok && error_count > 3) {
        // �����������ݣ������һ����I2C����
        consecutive_errors++;
        for (int i = 0; i < 3; i++) {
            gyro[i] = 0;
            accel[i] = (i == 2) ? 16384 : 0;  // Z������
        }
        *temperature = 25.0f;
        return false;
    }
    
    // ���Զ�ȡ��ʵ���ݣ������ϸ��ʱ������
    volatile uint32_t read_timeout = 500;  // ���̵Ķ�ȡ��ʱ
    bool read_success = false;
    
    __disable_irq();
    // ��ʱ��ȡ
    for (volatile uint32_t i = 0; i < read_timeout; i++) {
        icm42688_read(gyro, accel, temperature);
        // �򵥼�����ݺ�����
        if (gyro[0] != 0 || gyro[1] != 0 || gyro[2] != 0 || 
            accel[0] != 0 || accel[1] != 0 || accel[2] != 0) {
            read_success = true;
            consecutive_errors = 0;
            break;
        }
        if (i % 100 == 0 && i > 0) break;  // ÿ100�μ��һ���˳�
    }
    __enable_irq();
    
    if (!read_success) {
        consecutive_errors++;
        communication_ok = false;
    }
    
    return read_success;
}

// Ӳ������ - ����ʧ�ܰ汾
bool ICM42688_TestHardware(void) {
    OLED_CLS();
    display_6_8_string(0, 0, "Hardware Test");
    display_6_8_string(0, 1, "Quick Test...");
    DL_Common_delayCycles(SIMPLE_DELAY / 2);  // ������ʱ
    
    // �����豸ID - ʹ�ÿ��ٶ�ȡ
    uint8_t device_id = ultra_fast_read_i2c(ICM42688_I2C_ADDR, ICM42688_WHO_AM_I_REG);
    
    OLED_CLS();
    display_6_8_string(0, 0, "Device ID:");
    display_6_8_number_f1(60, 0, device_id);
    
    if (device_id == ICM42688_WHO_AM_I_VALUE) {
        display_6_8_string(0, 1, "ICM42688 Found!");
        display_6_8_string(0, 2, "Hardware OK");
        DL_Common_delayCycles(SIMPLE_DELAY / 2);
        communication_ok = true;
        error_count = 0;
        consecutive_errors = 0;
        force_simulation = false;
        return true;
    } else {
        display_6_8_string(0, 1, "Hardware Failed!");
        display_6_8_string(0, 2, "Force Simulation");
        DL_Common_delayCycles(SIMPLE_DELAY);
        communication_ok = false;
        force_simulation = true;
        return false;
    }
}

// ���ٳ�ʼ���汾
bool ICM42688_Init(ICM42688_Handle_t *handle) {
    if (handle == NULL) return false;
    
    memset(handle, 0, sizeof(ICM42688_Handle_t));
    
    OLED_CLS();
    display_6_8_string(0, 0, "Quick Init...");
    DL_Common_delayCycles(SIMPLE_DELAY / 2);
    
    // ���ǿ��ģ��ģʽ��ֱ��������ʼ��
    if (force_simulation || consecutive_errors > 3) {
        display_6_8_string(0, 1, "Skip Init");
        display_6_8_string(0, 2, "Simulation Mode");
        DL_Common_delayCycles(SIMPLE_DELAY / 2);
        handle->is_initialized = true;
        return true;
    }
    
    // ���ٳ�ʼ������
    volatile uint32_t init_timeout = 1000;  // ���̵ĳ�ʼ��ʱ��
    uint8_t init_result = 0;
    
    for (volatile uint32_t i = 0; i < init_timeout; i++) {
        init_result = icm42688_init();
        if (init_result == 1) break;
        
        if (i % 200 == 0 && i > 0) {
            display_6_8_string(0, 1, "Quick retry...");
            break;  // ���ٷ���
        }
    }
    
    if (init_result != 1) {
        display_6_8_string(0, 1, "Init Failed!");
        display_6_8_string(0, 2, "Use Simulation");
        force_simulation = true;
        consecutive_errors = 10;
        DL_Common_delayCycles(SIMPLE_DELAY / 2);
    }
    
    handle->is_initialized = true;
    return true;  // ���Ƿ��سɹ����ó������
}

// ������У׼�汾
bool ICM42688_Calibrate(ICM42688_Handle_t *handle, ICM42688_ProgressCallback_t callback) {
    if (handle == NULL || !handle->is_initialized) return false;
    
    OLED_CLS();
    display_6_8_string(0, 0, "Fast Calibration");
    
    // ���ǿ��ģ�⣬ʹ��Ĭ��У׼ֵ
    if (force_simulation || consecutive_errors > 3) {
        display_6_8_string(0, 1, "Default Values");
        memset(handle->gyro_offset, 0, sizeof(handle->gyro_offset));
        memset(handle->accel_offset, 0, sizeof(handle->accel_offset));
        handle->accel_offset[2] = -16384.0f;  // Z������ƫ��
        DL_Common_delayCycles(SIMPLE_DELAY / 2);
        handle->is_calibrated = true;
        return true;
    }
    
    display_6_8_string(0, 1, "Quick Sample");
    DL_Common_delayCycles(SIMPLE_DELAY / 4);
    
    OLED_CLS();
    display_6_8_string(0, 0, "Sampling...");
    
    int32_t gyro_sum[3] = {0, 0, 0};
    int32_t accel_sum[3] = {0, 0, 0};
    int valid_samples = 0;
    
    // ���ٵ�У׼������
    const int ultra_fast_samples = 5;  // ֻ��Ҫ5������
    
    for (int i = 0; i < ultra_fast_samples; i++) {
        // ��������ȡ
        if (safe_icm42688_read(handle->raw_gyro, handle->raw_accel, &handle->temperature)) {
            gyro_sum[0] += handle->raw_gyro[0];
            gyro_sum[1] += handle->raw_gyro[1];
            gyro_sum[2] += handle->raw_gyro[2];
            accel_sum[0] += handle->raw_accel[0];
            accel_sum[1] += handle->raw_accel[1];
            accel_sum[2] += handle->raw_accel[2];
            valid_samples++;
        }
        
        // ��ʾ����
        if (callback) {
            uint8_t progress = ((i + 1) * 100) / ultra_fast_samples;
            callback(progress);
        }
        
        // �̵ܶ���ʱ
        for (volatile int j = 0; j < 5000; j++);
        
        // �������ʧ�ܣ������˳�
        if (consecutive_errors > 2) break;
    }
    
    // ��ʹû����Ч����Ҳ������ʹ��Ĭ��ֵ
    if (valid_samples < 1) {
        memset(handle->gyro_offset, 0, sizeof(handle->gyro_offset));
        memset(handle->accel_offset, 0, sizeof(handle->accel_offset));
        handle->accel_offset[2] = -16384.0f;
        
        OLED_CLS();
        display_6_8_string(0, 0, "Use Defaults");
        display_6_8_string(0, 1, "No I2C Data");
    } else {
        // ����ƫ��
        handle->gyro_offset[0] = (float)gyro_sum[0] / valid_samples;
        handle->gyro_offset[1] = (float)gyro_sum[1] / valid_samples;
        handle->gyro_offset[2] = (float)gyro_sum[2] / valid_samples;
        handle->accel_offset[0] = (float)accel_sum[0] / valid_samples;
        handle->accel_offset[1] = (float)accel_sum[1] / valid_samples;
        handle->accel_offset[2] = (float)accel_sum[2] / valid_samples - 16384.0f;
        
        OLED_CLS();
        display_6_8_string(0, 0, "Cal Complete!");
        display_6_8_string(0, 1, "Samples:");
        display_6_8_number_f1(60, 1, valid_samples);
    }
    
    DL_Common_delayCycles(SIMPLE_DELAY / 2);
    handle->is_calibrated = true;
    return true;  // ���Ƿ��سɹ�
}

// �������ĸ��º���
void ICM42688_Update(ICM42688_Handle_t *handle, float dt) {
    if (handle == NULL || !handle->is_initialized) return;
    
    // ��������ȡ����������
    bool read_success = safe_icm42688_read(handle->raw_gyro, handle->raw_accel, &handle->temperature);
    
    if (!read_success) {
        // ͨ��ʧ�ܣ������ϴεĽǶ�ֵ����Ҫ��������
        return;
    }
    
    // Ӧ��У׼ƫ��
    float ax = handle->raw_accel[0] - handle->accel_offset[0];
    float ay = handle->raw_accel[1] - handle->accel_offset[1];
    float az = handle->raw_accel[2] - handle->accel_offset[2];
    float gx = (handle->raw_gyro[0] - handle->gyro_offset[0]) / 16.4f;
    float gy = (handle->raw_gyro[1] - handle->gyro_offset[1]) / 16.4f;
    float gz = (handle->raw_gyro[2] - handle->gyro_offset[2]) / 16.4f;
    
    // ������ٶȼƽǶ�
    float pitch_acc = 57.3f * atan2f(ay, az);
    float roll_acc = -57.3f * atan2f(-ax, sqrtf(ay*ay + az*az));
    
    // �����˲���
    handle->attitude.pitch = ICM42688_ALPHA * (handle->attitude.pitch + gy * dt) + 
                             (1.0f - ICM42688_ALPHA) * pitch_acc;
    handle->attitude.roll = ICM42688_ALPHA * (handle->attitude.roll + gx * dt) + 
                            (1.0f - ICM42688_ALPHA) * roll_acc;
    handle->attitude.yaw = handle->attitude.yaw + gz * dt;
}

// ��ʾ�Ƕ� - ���ͨ��״̬��ʾ
void ICM42688_DisplayAngles(ICM42688_Handle_t *handle, uint32_t counter) {
    if (handle == NULL) return;
    
    // ��һ�У���������״̬
    LCD_clear_L(0, 0);
    display_6_8_number_f1(0, 0, counter);
    if (communication_ok) {
        display_6_8_string(60, 0, "ICM-OK");
    } else {
        display_6_8_string(60, 0, "ICM-ERR");
        display_6_8_number_f1(100, 0, error_count);
    }
    
    // �ڶ��У��¶�
    LCD_clear_L(0, 1);
    display_6_8_string(0, 1, "Temp:");
    display_6_8_number_f1(40, 1, handle->temperature);
    display_6_8_string(80, 1, "C");
    
    // ����
    LCD_clear_L(0, 2);
    
    // �Ƕ���ʾ
    LCD_clear_L(0, 3);
    display_6_8_string(0, 3, "Pitch:");
    display_6_8_number_f1(40, 3, handle->attitude.pitch);
    display_6_8_string(90, 3, "deg");
    
    LCD_clear_L(0, 4);
    display_6_8_string(0, 4, "Roll: ");
    display_6_8_number_f1(40, 4, handle->attitude.roll);
    display_6_8_string(90, 4, "deg");
    
    LCD_clear_L(0, 5);
    display_6_8_string(0, 5, "Yaw:  ");
    display_6_8_number_f1(40, 5, handle->attitude.yaw);
    display_6_8_string(90, 5, "deg");
    
    LCD_clear_L(0, 6);
    LCD_clear_L(0, 7);
}

// ģ��ģʽ���ֲ���
void ICM42688_RunSimulation(void) {
    OLED_CLS();
    display_6_8_string(0, 0, "Hardware Failed");
    display_6_8_string(0, 1, "Simulation Mode");
    DL_Common_delayCycles(SIMPLE_DELAY);
    
    float pitch = 0.0f, roll = 0.0f, yaw = 0.0f;
    float noise = 0.0f;
    uint32_t counter = 0;
    
    while(1) {
        noise += 0.1f;
        
        pitch = 10.0f * sinf(noise * 0.5f);
        roll = 8.0f * cosf(noise * 0.7f);
        yaw += 0.1f;
        if (yaw > 180.0f) yaw -= 360.0f;
        
        float temp = 25.0f + 3.0f * sinf(noise * 0.1f);
        
        LCD_clear_L(0, 0);
        display_6_8_number_f1(0, 0, counter++);
        display_6_8_string(60, 0, "SIM MODE");
        
        LCD_clear_L(0, 1);
        display_6_8_string(0, 1, "Temp:");
        display_6_8_number_f1(40, 1, temp);
        display_6_8_string(80, 1, "C");
        
        LCD_clear_L(0, 2);
        
        LCD_clear_L(0, 3);
        display_6_8_string(0, 3, "Pitch:");
        display_6_8_number_f1(40, 3, pitch);
        display_6_8_string(90, 3, "deg");
        
        LCD_clear_L(0, 4);
        display_6_8_string(0, 4, "Roll: ");
        display_6_8_number_f1(40, 4, roll);
        display_6_8_string(90, 4, "deg");
        
        LCD_clear_L(0, 5);
        display_6_8_string(0, 5, "Yaw:  ");
        display_6_8_number_f1(40, 5, yaw);
        display_6_8_string(90, 5, "deg");
        
        LCD_clear_L(0, 6);
        LCD_clear_L(0, 7);
        
        DL_Common_delayCycles(SIMPLE_DELAY / 5);  // ����ˢ��
        DL_GPIO_togglePins(USER_GPIO_PORT, USER_GPIO_LED_PA0_PIN);
    }
}