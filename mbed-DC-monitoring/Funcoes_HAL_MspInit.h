#include "mbed.h"
#include "stm32wbxx_hal_adc.h"

#ifndef Funcoes_HAL_mspinit_H
#define Funcoes_HAL_mspinit_H

#ifdef __cplusplus
 extern "C" {
#endif

//Protótipos HAL_MspInit
void HAL_MspInit(void);
void HAL_ADC_MspInit(ADC_HandleTypeDef* hadc);
void HAL_ADC_MspDeInit(ADC_HandleTypeDef* hadc);
void HAL_TIM_Base_MspInit(TIM_HandleTypeDef* htim_base);
void HAL_TIM_Base_MspDeInit(TIM_HandleTypeDef* htim_base);
void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

#ifdef __cplusplus
}
#endif

#endif /* Funcoes_HAL_mspinit_H */