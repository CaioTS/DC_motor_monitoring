#ifndef Funcoes_HAL_init_H
#define Funcoes_HAL_init_H

#ifdef __cplusplus
 extern "C" {
#endif

//Protótipos HAL_Init
void MX_GPIO_Init(void);
void MX_DMA_Init(void);
void MX_ADC1_Init(void);
void MX_I2C1_Init(void);
void MX_USART1_UART_Init(void); 
void MX_TIM1_Init(void);
void MX_TIM2_Init(void);
void Error_Handler(void);
void SystemClock_Config(void);
void PeriphCommonClock_Config(void);
#ifdef __cplusplus
}
#endif

#endif /* Funcoes_HAL_init_H */