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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/


/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/

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

void Recepcao_CPU();
#endif /* __MAIN_H */