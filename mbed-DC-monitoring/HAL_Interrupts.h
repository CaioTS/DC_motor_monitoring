#ifndef HAL_Interrupts_H
#define HAL_Interrupts_H

#include "mbed.h"

extern "C" {
    
void TIM3_IRQHandler(void);
void EXTI15_10_IRQHandler(void);
void UART5_IRQHandler(void);
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart);
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim);
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin);
}

#endif /* HAL_Interrupts_H */