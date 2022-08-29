/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2022 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "openamp.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

#include <stdio.h>

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

#define RPMSG_CHAN_NAME			"openamp_demo"

#define TEST_BUF_SIZE			50

#ifndef HSEM_ID_0
#define HSEM_ID_0 (0U) /* HW semaphore 0*/
#endif
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

size_t app_len;
int32_t status = 0;

char buf[TEST_BUF_SIZE];

static volatile int message_received;
static struct rpmsg_endpoint rp_endpoint;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */

static int rpmsg_recv_callback(struct rpmsg_endpoint *ept, void *data,
		size_t len, uint32_t src, void *priv);

static void OpenAMP_init(void);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void) {
	/* USER CODE BEGIN 1 */
	int cnt = 0;
	char str_cntrl[] = "dasal:";
	/* USER CODE END 1 */

	/* USER CODE BEGIN Boot_Mode_Sequence_1 */
	/*HW semaphore Clock enable*/
	__HAL_RCC_HSEM_CLK_ENABLE();
	/* Activate HSEM notification for Cortex-M4*/
	HAL_HSEM_ActivateNotification(__HAL_HSEM_SEMID_TO_MASK(HSEM_ID_0));
	/*
	 Domain D2 goes to STOP mode (Cortex-M4 in deep-sleep) waiting for Cortex-M7 to
	 perform system initialization (system clock config, external memory configuration.. )
	 */
	HAL_PWREx_ClearPendingEvent();
	HAL_PWREx_EnterSTOPMode(PWR_MAINREGULATOR_ON, PWR_STOPENTRY_WFE,
	PWR_D2_DOMAIN);
	/* Clear HSEM flag */
	__HAL_HSEM_CLEAR_FLAG(__HAL_HSEM_SEMID_TO_MASK(HSEM_ID_0));

	/* USER CODE END Boot_Mode_Sequence_1 */
	/* MCU Configuration--------------------------------------------------------*/

	/* Reset of all peripherals, Initializes the Flash interface and the Systick. */
	HAL_Init();

	/* USER CODE BEGIN Init */

	/* USER CODE END Init */

	/* USER CODE BEGIN SysInit */

	/* USER CODE END SysInit */

	/* Initialize all configured peripherals */
	MX_GPIO_Init();
	MX_UART8_Init();
	MX_TIM13_Init();
	/* USER CODE BEGIN 2 */

	OpenAMP_init();

	HAL_TIM_Base_Start_IT(&htim13);

	/* USER CODE END 2 */

	/* Infinite loop */
	/* USER CODE BEGIN WHILE */
	while (1) {

		if (message_received == 1) {
			HAL_GPIO_TogglePin(GPIOI, GPIO_PIN_14);
			message_received = 0;
			if (NULL != strstr(buf, str_cntrl))
				cnt++;
		}

		if (cnt == 200) {
			HAL_GPIO_TogglePin(GPIOI, GPIO_PIN_15);
			cnt = 0;
		}
		/* USER CODE END WHILE */

		/* USER CODE BEGIN 3 */
	}
	/* USER CODE END 3 */
}

/* USER CODE BEGIN 4 */

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {

	if (htim == &htim13) {
		HAL_GPIO_TogglePin(GPIOI, GPIO_PIN_12);
	}

	OPENAMP_check_for_message();
}

static int rpmsg_recv_callback(struct rpmsg_endpoint *ept, void *data,
		size_t len, uint32_t src, void *priv) {

	memset((void*) buf, 0, sizeof(char) * TEST_BUF_SIZE);
	app_len = len;
	strncpy((char*) buf, (char*) data, len);

	message_received = 1;
	return 0;

}

static void OpenAMP_init(void) {
	MAILBOX_Init();

	if (MX_OPENAMP_Init(RPMSG_REMOTE, NULL) != HAL_OK)
		Error_Handler();

	/* create a endpoint for rmpsg communication */
	status = OPENAMP_create_endpoint(&rp_endpoint, RPMSG_CHAN_NAME,
	RPMSG_ADDR_ANY, rpmsg_recv_callback, NULL);

	if (status < 0) {
		Error_Handler();
	}

	//OPENAMP_DeInit();
}

/* USER CODE END 4 */

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void) {
	/* USER CODE BEGIN Error_Handler_Debug */
	/* User can add his own implementation to report the HAL error return state */
	__disable_irq();
	while (1) {
	}
	/* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
