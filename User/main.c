#include "debug.h"
#include "board.h"
#include "ina219.h"


// 存储按钮状态和充电状态
volatile uint8_t buttonFlag = 0;
volatile uint8_t chargingFlag = 0;

// 该检查电量了
volatile uint8_t checkFlag = 0;

#define OFFEST 7
#define SAMPLE_COUNT 5  // 取样次数

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

    if (voltage_mV >= 4000) {
        level = 4;
    } else if (voltage_mV >= 3700) {
        level = 3;
    } else if (voltage_mV >= 3400) {
        level = 2;
    } else if (voltage_mV >= 3050) {
        level = 1;
    } else {
        level = 0;
    }
    return level;
}

void Sys_Init() {
}

int main (void) {
    NVIC_PriorityGroupConfig (NVIC_PriorityGroup_1);
    SystemCoreClockUpdate();
    Delay_Init();
    USART_Printf_Init (115200);
    printf ("SystemClk: %d\r\n", SystemCoreClock);
    printf ("ChipID: %08x\r\n", DBGMCU_GetCHIPID());

    // 初始化打印串口,可以通过串口接受调试信息
    USARTx_CFG();

    // 初始化外设
    Board_Init();
    TIM2_Init();

    // 初始化检流
    INA219_Init();

    // 默认关闭输出
    set_BOOST (RESET);
    set_OUTPUT (RESET);

    close_LED();

    while (1) {
        // 不在充电,按钮也没打开, 进入睡眠, 睡觉觉节省电力
        if (buttonFlag == 0 && chargingFlag == 0) {
            printf ("Sleep Time!");
            __WFI();
            continue;
        }

        if (checkFlag) {
            int32_t rawCurrent = INA219_GetCurrent();
            if (rawCurrent < -10) {
                chargingFlag = 1;
            } else {
                continue;
            }
            checkFlag = 0;
        }

        uint32_t rawbatVoltage = sample_average_voltage();
        int32_t current = sample_average_current();
        // 根据电流补偿电压
        uint32_t batVoltage = rawbatVoltage + xabs (current) / OFFEST;

        if (current < 0) {
            chargingFlag = 1;

        } else {
            chargingFlag = 0;
        }
        printf ("I:%dmA V:%dmV", current, batVoltage);

        uint8_t level = get_Level (batVoltage);

        if (buttonFlag) {


            // 判断是否没电了, 闪灯提示一下, 然后关闭输出
            if (level == 0) {
                for (uint i = 0; i < 5; i++) {
                    LED_OnOff (0, 1);
                    Delay_Ms (300);
                    LED_OnOff (0, 1);
                    Delay_Ms (300);
                }
                set_OUTPUT (RESET);
                set_BOOST (RESET);
                // 将按钮位设置为0, 准备睡觉。
                buttonFlag = 0;
            }

            // 不在充电的时候才显示这个
            if (!chargingFlag) {
                // 根据电压设定LED
                for (uint8_t i = 0; i < level; i++) {
                    LED_OnOff (i, 1);
                }
            }
        }

        if (chargingFlag) {
            for (uint8_t i = 0; i < level; i++) {

                // 充满电就常亮
                if (current < -20) {
                    Delay_Ms (500);
                }
                LED_OnOff (i, 1);
            }
            Delay_Ms (500);
            close_LED();
        }
    }
}

void EXTI7_0_IRQHandler (void) __attribute__ ((interrupt ("WCH-Interrupt-fast")));

void EXTI7_0_IRQHandler (void) {
    if (EXTI_GetITStatus (EXTI_Line6) != RESET) {
        EXTI_ClearITPendingBit (EXTI_Line6);  // 清除中断标志位！
            if (buttonFlag) {
                // 熄灭所有灯,然后关闭DCDC和OUTPUT
                set_OUTPUT (RESET);
                set_BOOST (RESET);
                close_LED();
                buttonFlag = 0;
            } else {
                // 根据电压打开输出
                set_BOOST (SET);
                set_OUTPUT (SET);
                printf ("Open\n");
                buttonFlag = 1;
            }

    }
}

void TIM2_IRQHandler (void) __attribute__ ((interrupt ("WCH-Interrupt-fast")));

void TIM2_IRQHandler (void) {
    if (TIM_GetITStatus (TIM2, TIM_IT_Update) != RESET) {
        TIM_ClearITPendingBit (TIM2, TIM_IT_Update);
        if (checkFlag == 0) {
            chargingFlag = 1;
        }
    }
}
