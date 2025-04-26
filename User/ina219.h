#ifndef INA219_H
#define INA219_H

#include "ch32v00x.h"

#define INA219_ADDRESS       (0x40 << 1) // I2C 地址（左移1位用于写入）

// INA219 寄存器地址
#define INA219_REG_CONFIG        0x00
#define INA219_REG_SHUNT_VOLTAGE 0x01
#define INA219_REG_BUS_VOLTAGE   0x02
#define INA219_REG_POWER         0x03
#define INA219_REG_CURRENT       0x04
#define INA219_REG_CALIBRATION   0x05

// 函数声明
void INA219_WriteRegister(uint8_t reg, uint16_t value);
uint16_t INA219_ReadRegister(uint8_t reg);
void INA219_Init(void);

int32_t INA219_GetBusVoltage(void);   // 获取总线电压（mV）
int32_t INA219_GetShuntVoltage(void); // 获取分流电压（μV）
int32_t INA219_GetCurrent(void);      // 获取电流（μA）
int32_t INA219_GetPower(void);        // 获取功率（μW）

#endif /* INA219_H */