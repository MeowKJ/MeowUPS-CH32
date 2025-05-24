#include "utils.h"
#include "ina219.h"

uint32_t sample_average_voltage (void) {
    uint32_t sum = 0;
    for (int i = 0; i < SAMPLE_COUNT; i++) {
        sum += INA219_GetBusVoltage();
        Delay_Ms (5);  // 每次取样间隔
    }
    return sum / SAMPLE_COUNT;
}

int32_t sample_average_current (void) {
    int32_t sum = 0;
    for (int i = 0; i < SAMPLE_COUNT; i++) {
        sum += INA219_GetCurrent();
        Delay_Ms (5);
    }
    return sum / SAMPLE_COUNT;
}

// 取绝对值函数
int32_t xabs (int32_t num) {
    if (num < 0) {
        return 0;
    }
    return num;
}

// 这个函数根据电池电压设置 level，决定亮多少个灯
uint8_t get_Level (uint32_t voltage_mV) {
    uint8_t level = 0;

    if (voltage_mV >= 3900) {
        level = 4;
    } else if (voltage_mV >= 3600) {
        level = 3;
    } else if (voltage_mV >= 3300) {
        level = 2;
    } else if (voltage_mV >= 2900) {
        level = 1;
    } else {
        level = 0;
    }
    return level;
}
