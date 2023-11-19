#include "PinNames.h"
#include "mbed.h"

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

/* Includes ------------------------------------------------------------------*/
#include "stm32wbxx_hal.h"
#include "Funcoes_HAL_Init.h"
#include "Funcoes_HAL_MspInit.h"
#include "HAL_Interrupts.h"
#include "SerialStream.h"

#include <array>


#define PIN_Veloci_Pin GPIO_PIN_4
#define PIN_Veloci_GPIO_Port GPIOE
#define PIN_Veloci_EXTI_IRQn EXTI4_IRQn



// Variaveis externas
extern ADC_HandleTypeDef hadc1;
extern DMA_HandleTypeDef hdma_adc1;
extern I2C_HandleTypeDef hi2c1;
extern TIM_HandleTypeDef htim1;
extern TIM_HandleTypeDef htim2;
extern UART_HandleTypeDef huart1;

extern int angle_cnt;


#endif /* __MAIN_H */