#ifndef __BOARD_H
#define __BOARD_H


#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <ch32v00x.h>

#define IIC_BOUND 100000


void Board_Init (void);
void USARTx_CFG (void);
void LED_OnOff (uint8_t num, FlagStatus state);
void LED_Toggle (uint8_t num);
void close_LED(void);

void set_BOOST (FlagStatus bit);
void set_OUTPUT (FlagStatus bit);
void TIM2_Init(void);

void set_BOOST_OUTPUT(FlagStatus bit);

#ifdef __cplusplus
}
#endif

#endif