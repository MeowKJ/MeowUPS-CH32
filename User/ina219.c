#include "ina219.h"
#include "ch32v00x_i2c.h"

// 写入 INA219 寄存器
void INA219_WriteRegister(uint8_t reg, uint16_t value) {
    while (I2C_GetFlagStatus(I2C1, I2C_FLAG_BUSY));

    I2C_GenerateSTART(I2C1, ENABLE);
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT));

    I2C_Send7bitAddress(I2C1, INA219_ADDRESS, I2C_Direction_Transmitter);
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));

    I2C_SendData(I2C1, reg);
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED));

    I2C_SendData(I2C1, (value >> 8) & 0xFF);
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED));

    I2C_SendData(I2C1, value & 0xFF);
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED));

    I2C_GenerateSTOP(I2C1, ENABLE);
}

// 读取 INA219 寄存器
uint16_t INA219_ReadRegister(uint8_t reg) {
    uint8_t data[2];
    
    while (I2C_GetFlagStatus(I2C1, I2C_FLAG_BUSY));
    I2C_GenerateSTART(I2C1, ENABLE);
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT));
    
    I2C_Send7bitAddress(I2C1, INA219_ADDRESS, I2C_Direction_Transmitter);
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));

    I2C_SendData(I2C1, reg);
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_TRANSMITTED));

    I2C_GenerateSTART(I2C1, ENABLE);
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_MODE_SELECT));
    
    I2C_Send7bitAddress(I2C1, INA219_ADDRESS, I2C_Direction_Receiver);
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED));

    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_RECEIVED));
    data[0] = I2C_ReceiveData(I2C1);

    I2C_AcknowledgeConfig(I2C1, DISABLE);
    I2C_GenerateSTOP(I2C1, ENABLE);
    
    while (!I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_RECEIVED));
    data[1] = I2C_ReceiveData(I2C1);

    I2C_AcknowledgeConfig(I2C1, ENABLE);
    
    return (data[0] << 8) | data[1];
}

// 初始化 INA219
void INA219_Init(void) {
    // 校准寄存器：0x8312 (33554)，对应10mΩ和4A
    INA219_WriteRegister(INA219_REG_CALIBRATION, 0x8312); 
    
    // 配置寄存器：0x0EE7（16V范围，±40mV，12位ADC，128次平均）
    INA219_WriteRegister(INA219_REG_CONFIG, 0x0EE7);  
}
// 获取总线电压（mV）
int32_t INA219_GetBusVoltage(void) {
    uint16_t raw = INA219_ReadRegister(INA219_REG_BUS_VOLTAGE);
    return (raw >> 3) * 4;  // 转换为 mV
}

// 获取分流电压（μV）
int32_t INA219_GetShuntVoltage(void) {
    int16_t raw = INA219_ReadRegister(INA219_REG_SHUNT_VOLTAGE);
    return raw * 10;  // 转换为 μV
}

// 获取电流（mA）
int32_t INA219_GetCurrent(void) {
    int16_t raw = INA219_ReadRegister(INA219_REG_CURRENT);
    return raw / 10;  // 计算 mA（10mΩ 采样电阻）
}

// 获取功率（mW）
int32_t INA219_GetPower(void) {
    int16_t raw = INA219_ReadRegister(INA219_REG_POWER);
    return raw * 0.2;  // 计算 mW（基于 INA219 校准值）
}