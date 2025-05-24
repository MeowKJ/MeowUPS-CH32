#ifndef UTILS_H
#define UTILS_H

#include "ch32v00x.h"

#define OFFEST 7
#define SAMPLE_COUNT 5  // 取样次数

uint32_t sample_average_voltage (void);
int32_t sample_average_current (void) ;
int32_t xabs (int32_t num);
uint8_t get_Level (uint32_t voltage_mV);

#endif
