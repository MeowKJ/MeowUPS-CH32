#include "debug.h"
#include "board.h"
#include "utils.h"

volatile FlagStatus WakeUpStatus = RESET;
volatile FlagStatus ButtonStatus = RESET;
volatile FlagStatus ChargeStatus = RESET;
volatile uint8_t sleepCount = 0;

void EXTI7_0_IRQHandler (void) __attribute__ ((interrupt ("WCH-Interrupt-fast")));
void TIM2_IRQHandler (void) __attribute__ ((interrupt ("WCH-Interrupt-fast")));

void Sys_Init() {
    NVIC_PriorityGroupConfig (NVIC_PriorityGroup_1);
    SystemCoreClockUpdate();
    Delay_Init();
    USART_Printf_Init (115200);
    printf ("SystemClk: %d\r\n", SystemCoreClock);
    printf ("ChipID: %08x\r\n", DBGMCU_GetCHIPID());
    USARTx_CFG();
    Board_Init();
    TIM2_Init();
}

int main (void) {

    Sys_Init();

    // 默认关闭输出
    set_BOOST_OUTPUT (RESET);
    close_LED();

    while (1) {
        if (WakeUpStatus == SET || ChargeStatus == SET || ButtonStatus == SET) {

            uint32_t v = sample_average_voltage();
            int32_t i = sample_average_current();
            int32_t w = xabs (((float)v / 1000.0) * i);
            uint8_t level = get_Level (v);

            printf ("[LOOP]Read:V:%dmv, I:%dmA, W:%dmW, Lv:%d;\n", v, i, w, level);

            ChargeStatus = i < -10 ? SET : RESET;

            printf ("[LOOP]:Status:Wake:%d, Charge: %d, Button: %d;\n", WakeUpStatus, ChargeStatus, ButtonStatus);

            if (i < 20 && i > 0 && ButtonStatus == SET) {
                sleepCount++;
                printf ("[LOOP]Sleep Count: %d;\n", sleepCount);
                if (sleepCount > 15) {
                    set_OUTPUT (RESET);
                    close_LED();
                    sleepCount = 0;
                    ButtonStatus = RESET;
                }
            } else {
                sleepCount = 0;
            }


            if (ChargeStatus == SET) {
                for (uint8_t i = 0; i < 4; i++) {
                    LED_OnOff (i, SET);
                    Delay_Ms (500);
                }
                close_LED();
            }

            WakeUpStatus = RESET;

            if (ChargeStatus == RESET && ButtonStatus == SET) {
                if (level == 0) {
                    set_OUTPUT(RESET);
                    ButtonStatus = RESET;
                    LED_OnOff(0, SET);
                    Delay_Ms(200);
                }
                close_LED();
                for (uint8_t i = 0; i < level; i++) {
                    LED_OnOff (i, SET);
                }
                // FOR EXIT CHANGE
                if (ButtonStatus == RESET) {
                    close_LED();
                }
            }
            Delay_Ms (1000);
        } else {
            __WFI();
        }
    }
}

void EXTI7_0_IRQHandler (void) {
    if (EXTI_GetITStatus (EXTI_Line6) != RESET) {
        EXTI_ClearITPendingBit (EXTI_Line6);
        if (ButtonStatus == SET) {
            set_BOOST_OUTPUT (RESET);
            if (ChargeStatus == RESET)
                close_LED();
            ButtonStatus = RESET;
            sleepCount = 0;
            printf ("[EXIT]User Close;\n");
        } else {
            set_BOOST_OUTPUT (SET);
            ButtonStatus = SET;
            printf ("[EXIT]User Opne;\n");
        }
    }
}

void TIM2_IRQHandler (void) {
    if (TIM_GetITStatus (TIM2, TIM_IT_Update) != RESET) {
        TIM_ClearITPendingBit (TIM2, TIM_IT_Update);
        if (WakeUpStatus == RESET) {
            WakeUpStatus = SET;
            printf ("[EXIT]Sys Wake Up;\n");
        }
    }
}
