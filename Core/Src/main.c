/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2025 STMicroelectronics.
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
#include "cmsis_os.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <shell.h>
#include <drv_uart1.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define STACK_SIZE 256
#define TASK1_PRIORITY 1
#define TASK2_PRIORITY 2
#define TASK1_DELAY 1
#define TASK2_DELAY 2
#define QUEUE_LENGTH 5
#define ITEM_SIZE sizeof(uint32_t)

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

TaskHandle_t taskTakeHandle;
SemaphoreHandle_t xSemaphore;
QueueHandle_t xQueue;

SemaphoreHandle_t xMutex;

h_shell_t h_shell;

TaskHandle_t xLedTaskHandle;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void MX_FREERTOS_Init(void);
/* USER CODE BEGIN PFP */
void BlinkTask(void *argument);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

int __io_putchar(int ch) {
	HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1, HAL_MAX_DELAY);
	return ch;
}

void BlinkTask(void *argument)
{
	for(;;)
	{
		HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
		printf("LED toggled\r\n");
		vTaskDelay(100 / portTICK_PERIOD_MS);
	}
}

void taskGive(void *pvParameters)
{
	uint32_t timerValue = 0; // Valeur à envoyer dans la queue
	for (;;)
	{
		printf("taskGive : avant envoi à la queue\r\n");
		if (xQueueSend(xQueue, &timerValue, portMAX_DELAY) == pdPASS)
		{
			printf("taskGive : valeur envoyée : %u\r\n", timerValue);
		}
		timerValue++;
		vTaskDelay(pdMS_TO_TICKS(100));
	}
}

void taskTake(void *pvParameters)
{
	uint32_t receivedValue;
	for (;;)
	{
		printf("taskTake : en attente de valeur dans la queue...\r\n");
		if (xQueueReceive(xQueue, &receivedValue, portMAX_DELAY) == pdPASS)
		{
			printf("taskTake : valeur reçue : %u\r\n", receivedValue);
		}
	}
}

void task_bug(void * pvParameters)
{
	int delay = (int) pvParameters;
	for (;;)
	{
		if (xSemaphoreTake(xMutex, portMAX_DELAY) == pdTRUE)
		{
			printf("Je suis %s et je m'endors pour %d ticks\r\n",
					pcTaskGetName(NULL), delay);
			xSemaphoreGive(xMutex);
		}
		vTaskDelay(pdMS_TO_TICKS(delay));
	}
}

int fonction(int argc, char ** argv)
{
	printf("Je suis une fonction bidon\r\n");

	return 0;
}

void led_task(void *pvParameters){
	uint32_t period = (uint32_t) pvParameters;
	TickType_t xLastWakeTime;
	xLastWakeTime = xTaskGetTickCount();
	for (;;) {
		if (period == 0) {
			HAL_GPIO_WritePin(LED_GPIO_Port,LED_Pin, GPIO_PIN_RESET); // LED éteinte
		} else {
			HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
			vTaskDelay(pdMS_TO_TICKS(period));
		}
	}
}

int sh_led(int argc, char ** argv)
{
	if (argc < 2) {
		int size;
		size = snprintf(h_shell.print_buffer, BUFFER_SIZE, "Mettre le temps en ms\r\n");
		h_shell.drv.transmit(h_shell.print_buffer, size);
		return -1;
	}

	uint32_t period = atoi(argv[2]);

	if (xLedTaskHandle != NULL) {
		vTaskDelete(xLedTaskHandle);
		xLedTaskHandle = NULL;
	}

	BaseType_t ret = xTaskCreate(led_task, "LED_Task", STACK_SIZE, (void *) period, 1, &xLedTaskHandle);
	if (ret != pdPASS) {
		int size;
		size = snprintf(h_shell.print_buffer, BUFFER_SIZE, "Error: Failed to create LED task\r\n");
		h_shell.drv.transmit(h_shell.print_buffer, size);
		return -1;
	}

	int size;
	size = snprintf(h_shell.print_buffer, BUFFER_SIZE, "LED blinking with period %d ms\r\n", period);
	h_shell.drv.transmit(h_shell.print_buffer, size);
	return 0;


}


/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void)
{

	/* USER CODE BEGIN 1 */

	/* USER CODE END 1 */

	/* MCU Configuration--------------------------------------------------------*/

	/* Reset of all peripherals, Initializes the Flash interface and the Systick. */
	HAL_Init();

	/* USER CODE BEGIN Init */

	/* USER CODE END Init */

	/* Configure the system clock */
	SystemClock_Config();

	/* USER CODE BEGIN SysInit */

	/* USER CODE END SysInit */

	/* Initialize all configured peripherals */
	MX_GPIO_Init();
	MX_USART1_UART_Init();
	/* USER CODE BEGIN 2 */

	//HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET);

	//BaseType_t ret;

	//xTaskCreate(BlinkTask, "LED_Task", TASK_STACK, NULL, BLINK_TASK_PRIORITY, NULL);
	//
	//	xSemaphore = xSemaphoreCreateBinary();
	//
	//	if(xSemaphore!=NULL)
	//	{
	//		xTaskCreate(taskGive, "Give", TASK_STACK,NULL,2,NULL);
	//		xTaskCreate(taskTake, "Take", TASK_STACK,NULL,1,NULL);
	//
	//		vTaskStartScheduler();
	//	}


	//	xQueue = xQueueCreate(QUEUE_LENGTH, ITEM_SIZE);
	//	configASSERT(xQueue!=NULL);
	//
	//    ret = xTaskCreate(taskTake, "taskTake", STACK_SIZE, NULL, 2, &taskTakeHandle);
	//    configASSERT(ret == pdPASS);
	//
	//    ret = xTaskCreate(taskGive, "taskGive", STACK_SIZE, NULL, 1, NULL);
	//    configASSERT(ret == pdPASS);
	//
	//    vTaskStartScheduler();

	//
	//	xMutex = xSemaphoreCreateMutex();
	//	configASSERT(xMutex != NULL);
	//
	//	ret = xTaskCreate(task_bug, "Tache 1", STACK_SIZE, (void *) TASK1_DELAY, TASK1_PRIORITY, NULL);
	//	configASSERT(pdPASS == ret);
	//	ret = xTaskCreate(task_bug, "Tache 2", STACK_SIZE, (void *) TASK2_DELAY, TASK2_PRIORITY, NULL);
	//	configASSERT(pdPASS == ret);


	//vTaskStartScheduler();

	h_shell.drv.receive = drv_uart1_receive;
	h_shell.drv.transmit = drv_uart1_transmit;

	shell_init(&h_shell);
	shell_add(&h_shell, 'f', fonction, "Une fonction inutile");
	shell_add(&h_shell, 'l', sh_led, "LED blink with period in ms");
	shell_run(&h_shell);



	/* USER CODE END 2 */

	/* Call init function for freertos objects (in cmsis_os2.c) */
	MX_FREERTOS_Init();

	/* Start scheduler */
	osKernelStart();

	/* We should never get here as control is now taken by the scheduler */

	/* Infinite loop */
	/* USER CODE BEGIN WHILE */
	while (1)
	{
		//		if(	HAL_GPIO_ReadPin(BTN_USER_GPIO_Port, BTN_USER_Pin) == GPIO_PIN_SET){
		//			HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin,GPIO_PIN_SET);
		//		}
		//		else{
		//			HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);
		//		}



		/* USER CODE END WHILE */

		/* USER CODE BEGIN 3 */
	}
	/* USER CODE END 3 */
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void)
{
	RCC_OscInitTypeDef RCC_OscInitStruct = {0};
	RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

	/** Configure the main internal regulator output voltage
	 */
	__HAL_RCC_PWR_CLK_ENABLE();
	__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

	/** Initializes the RCC Oscillators according to the specified parameters
	 * in the RCC_OscInitTypeDef structure.
	 */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
	RCC_OscInitStruct.HSIState = RCC_HSI_ON;
	RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
	RCC_OscInitStruct.PLL.PLLM = 8;
	RCC_OscInitStruct.PLL.PLLN = 216;
	RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
	RCC_OscInitStruct.PLL.PLLQ = 2;
	if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
	{
		Error_Handler();
	}

	/** Activate the Over-Drive mode
	 */
	if (HAL_PWREx_EnableOverDrive() != HAL_OK)
	{
		Error_Handler();
	}

	/** Initializes the CPU, AHB and APB buses clocks
	 */
	RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
			|RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

	if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_7) != HAL_OK)
	{
		Error_Handler();
	}
}

/* USER CODE BEGIN 4 */


/* USER CODE END 4 */

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void)
{
	/* USER CODE BEGIN Error_Handler_Debug */
	/* User can add his own implementation to report the HAL error return state */
	__disable_irq();
	while (1)
	{
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
