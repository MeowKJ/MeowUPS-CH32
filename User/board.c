#include "board.h"
#include "ina219.h"
#include <ch32v00x.h>
#include "ch32v00x_tim.h"
#include "ch32v00x_it.h"


void Board_Init (void) {
    GPIO_InitTypeDef GPIO_InitStructure = {0};
    EXTI_InitTypeDef EXTI_InitStructure = {0};
    NVIC_InitTypeDef NVIC_InitStructure = {0};
    RCC_APB2PeriphClockCmd (RCC_APB2Periph_AFIO | RCC_APB2Periph_GPIOD | RCC_APB2Periph_GPIOC, ENABLE);
    // 配置 PC7 PC4
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7 | GPIO_Pin_4 | GPIO_Pin_5;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_30MHz;
    GPIO_Init (GPIOC, &GPIO_InitStructure);

    // 配置 PD2、PD3、PD4
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4;
    GPIO_Init (GPIOD, &GPIO_InitStructure);

    // 配置 PC6 上拉输入, 中断
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init (GPIOC, &GPIO_InitStructure);

    // 配置EXIT下降沿中断
    GPIO_EXTILineConfig (GPIO_PortSourceGPIOC, GPIO_PinSource6);
    EXTI_InitStructure.EXTI_Line = EXTI_Line6;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init (&EXTI_InitStructure);



    // //配置PD7中断
    // GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
    // GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    // GPIO_Init (GPIOD, &GPIO_InitStructure);

    // GPIO_EXTILineConfig (GPIO_PortSourceGPIOD, GPIO_PinSource7);
    // EXTI_InitStructure.EXTI_Line = EXTI_Line7;
    // EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    // EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
    // EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    // EXTI_Init (&EXTI_InitStructure);


    // 配置NVIC
    NVIC_InitStructure.NVIC_IRQChannel = EXTI7_0_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init (&NVIC_InitStructure);


    // 配置PC1-SDA PC2-SCA IIC

    I2C_InitTypeDef I2C_InitStructure = {0};

    // 启用 I2C1 和 GPIOC 时钟
    RCC_APB1PeriphClockCmd (RCC_APB1Periph_I2C1, ENABLE);
    // 配置 PC1 (SDA) 和 PC2 (SCL) 为开漏复用功能
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_OD;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_30MHz;
    GPIO_Init (GPIOC, &GPIO_InitStructure);

    // 初始化 I2C1
    I2C_InitStructure.I2C_ClockSpeed = IIC_BOUND;
    I2C_InitStructure.I2C_Mode = I2C_Mode_I2C;
    I2C_InitStructure.I2C_DutyCycle = I2C_DutyCycle_2;
    I2C_InitStructure.I2C_OwnAddress1 = INA219_ADDRESS;  // 作为主设备，不需要从机地址
    I2C_InitStructure.I2C_Ack = I2C_Ack_Enable;
    I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;

    I2C_Init (I2C1, &I2C_InitStructure);
    I2C_Cmd (I2C1, ENABLE);
}

void TIM2_Init(void) {
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);

    // 配置定时器（x秒一次中断） => 48MHz / 48000 / 1000 = 1Hz
    TIM_TimeBaseStructure.TIM_Period = 5000 - 1;
    TIM_TimeBaseStructure.TIM_Prescaler = 48000 - 1;
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);
    TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);

    // 中断优先级配置
    NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    // 启动定时器
    TIM_Cmd(TIM2, ENABLE);
}

void USARTx_CFG (void) {
    GPIO_InitTypeDef GPIO_InitStructure = {0};
    USART_InitTypeDef USART_InitStructure = {0};

    RCC_APB2PeriphClockCmd (RCC_APB2Periph_GPIOD | RCC_APB2Periph_USART1, ENABLE);

    /* USART1 TX-->D.5   RX-->D.6 */
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_30MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init (GPIOD, &GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init (GPIOD, &GPIO_InitStructure);

    USART_InitStructure.USART_BaudRate = 115200;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;

    USART_Init (USART1, &USART_InitStructure);
    USART_Cmd (USART1, ENABLE);
}

// 控制指定 LED（0: PC7, 1: PD2, 2: PD3, 3: PD4），state = 1 亮，state = 0 灭
void LED_OnOff (uint8_t num, FlagStatus state) {
    switch (num) {
    case 0:
        if (state)
            GPIO_SetBits (GPIOC, GPIO_Pin_7);
        else
            GPIO_ResetBits (GPIOC, GPIO_Pin_7);
        break;
    case 1:
        if (state)
            GPIO_SetBits (GPIOD, GPIO_Pin_2);
        else
            GPIO_ResetBits (GPIOD, GPIO_Pin_2);
        break;
    case 2:
        if (state)
            GPIO_SetBits (GPIOD, GPIO_Pin_3);
        else
            GPIO_ResetBits (GPIOD, GPIO_Pin_3);
        break;
    case 3:
        if (state)
            GPIO_SetBits (GPIOD, GPIO_Pin_4);
        else
            GPIO_ResetBits (GPIOD, GPIO_Pin_4);
        break;
    default:
        break;  // 无效输入
    }
}

//

void LED_Toggle (uint8_t num) {
    switch (num) {
    case 0:
        GPIO_WriteBit (GPIOC, GPIO_Pin_7,
                       (BitAction)(1 - GPIO_ReadOutputDataBit (GPIOC, GPIO_Pin_7)));
        break;
    case 1:
        GPIO_WriteBit (GPIOD, GPIO_Pin_2,
                       (BitAction)(1 - GPIO_ReadOutputDataBit (GPIOD, GPIO_Pin_2)));
        break;
    case 2:
        GPIO_WriteBit (GPIOD, GPIO_Pin_3,
                       (BitAction)(1 - GPIO_ReadOutputDataBit (GPIOD, GPIO_Pin_3)));
        break;
    case 3:
        GPIO_WriteBit (GPIOD, GPIO_Pin_4,
                       (BitAction)(1 - GPIO_ReadOutputDataBit (GPIOD, GPIO_Pin_4)));
        break;
    default:
        break;  // 无效输入
    }
}

void close_LED() {
    for (uint8_t i = 0; i < 4; i++) {
        LED_OnOff (i, 0);
    }
}

// void DCDC_Toggle() {
//     GPIO_WriteBit (GPIOC, GPIO_Pin_4,
//                    (BitAction)(1 - GPIO_ReadOutputDataBit (GPIOC, GPIO_Pin_4)));
// }


void set_BOOST (FlagStatus bit) {
    GPIO_WriteBit (GPIOC, GPIO_Pin_4, (BitAction)bit);
}

void set_OUTPUT (FlagStatus bit) {
    GPIO_WriteBit (GPIOC, GPIO_Pin_5, (BitAction)bit);
}
