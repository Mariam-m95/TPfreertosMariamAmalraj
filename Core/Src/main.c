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
#include "tim.h"
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
#define STACK_SIZE_LONG 512
#define STACK_SIZE_ERROR 100

#define TASK1_PRIORITY 1
#define TASK2_PRIORITY 2
#define TASK1_DELAY 1
#define TASK2_DELAY 2

#define QUEUE_LENGTH 5
#define ITEM_SIZE sizeof(uint32_t)


#define TASK_SHELL_STACK_DEPTH 512
#define TASK_SHELL_PRIORITY 1
#define TASK_LED_PRIORITY 1
#define TASK_SPAM_PRIORITY 1
#define TASK_OVER_PRIORITY 1


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

TaskHandle_t xLedTaskHandle = NULL;
TaskHandle_t h_task_shell = NULL;

TaskHandle_t xSpamHandle = NULL;
SemaphoreHandle_t xMutexSpam;

TaskHandle_t xOverflowHandle = NULL;
SemaphoreHandle_t xMutexPrint;

typedef struct{
	char* msg;
	int count;
}SpamParams;

static SpamParams spam_params;

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
	uint32_t timerValue = 0;
	for (;;)
	{
		printf("taskGive : avant envoi à la queue\r\n");
		if (xQueueSend(xQueue, &timerValue, portMAX_DELAY) == pdPASS)
		{
			printf("taskGive : valeur envoyée : %u\r\n", timerValue);
		}
		else{
			printf("Erreur envoie de la queue\r\n");
			Error_Handler();
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
			Error_Handler();
		}
		else{
			printf("Erreur valeur non reçue\r\n");
			Error_Handler();
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
			Error_Handler();
		}
		else{
			printf("Erreur Semaphore Mutex");
			Error_Handler();
		}
		vTaskDelay(pdMS_TO_TICKS(delay));
	}
}

int fonction(int argc, char ** argv)
{
	printf("Je suis une fonction bidon\r\n");
	return 0;
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if (huart->Instance == USART1)
	{
		shell_uart_receive_irq_cb();
	}
	else{
		Error_Handler();
	}
}

void led_task(void *pvParameters){
	int period = (int)pvParameters;
	while(1){
		HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin,GPIO_PIN_SET);
		vTaskDelay(period);
		HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin,GPIO_PIN_RESET);
		vTaskDelay(period);
	}
}

int sh_led(int argc, char ** argv)
{
	if(argc>1){
		int period = atoi(argv[1]);
		if(period ==0){
			if (xLedTaskHandle != NULL) {
				vTaskDelete(xLedTaskHandle);
				xLedTaskHandle = NULL;
				HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_RESET);
				return 0;
			}
		}
		else{
			if (xLedTaskHandle != NULL) {
				vTaskDelete(xLedTaskHandle);
				xLedTaskHandle = NULL;
			}
		}
		BaseType_t ret = xTaskCreate(led_task, "LED_Task", STACK_SIZE_LONG, (void*)period, TASK_LED_PRIORITY, &xLedTaskHandle);
		if (ret == pdPASS) {
			printf("Tâche LED lancée avec une période de %d ms\r\n",period);
		} else {
			printf("Erreur création tâche LED\r\n");
			Error_Handler();
		}
		return 1;
	}
	else{
		printf("Erreur : ajouter la période\r\n");
		Error_Handler();
		return -1;
	}
}

void spam_task(void *pvParameters) {
	SpamParams *params = (SpamParams*) pvParameters;
	char* msg = params->msg;
	int count = params->count;
	int i;

	if (xMutexSpam == NULL) {
		xMutexSpam = xSemaphoreCreateMutex();
	}

	if (xMutexSpam != NULL) {
		if (xSemaphoreTake(xMutexSpam, portMAX_DELAY) == pdPASS) {
			for (i = 0; i < count; i++) {
				printf("%s\r\n", msg);
				vTaskDelay(pdMS_TO_TICKS(100));
			}
			xSemaphoreGive(xMutexSpam);
		}
	} else {
		printf("Erreur création du mutex\r\n");
		Error_Handler();
	}

	if (xSpamHandle != NULL) {
		vTaskDelete(xSpamHandle);
		xSpamHandle = NULL;
	}
}

int sh_spam(int argc, char **argv) {
	if (argc > 2) {
		spam_params.msg = argv[1];
		spam_params.count = atoi(argv[2]);

		if (xMutexSpam == NULL) {
			xMutexSpam = xSemaphoreCreateMutex();
		}

		if (xMutexSpam != NULL) {
			if (xSemaphoreTake(xMutexSpam, portMAX_DELAY) == pdPASS) {
				BaseType_t ret = xTaskCreate(spam_task, "SPAM_task", STACK_SIZE_LONG, (void*)&spam_params, TASK_SPAM_PRIORITY, &xSpamHandle);
				if (ret == pdPASS) {
					printf("Debut SPAM : \r\n");
				} else {
					printf("Erreur création tâche SPAM\r\n");
					Error_Handler();
				}
				xSemaphoreGive(xMutexSpam);
			}
		} else {
			printf("Erreur création du mutex\r\n");
			Error_Handler();
			return -1;
		}
		return 1;
	} else {
		printf("Erreur : ajouter le message et le nombre de répétitions\r\n");
		Error_Handler();
		return -1;
	}
}

void overflow_task(void *pvParameters)
{
	if (xMutexPrint != NULL && xSemaphoreTake(xMutexPrint, portMAX_DELAY) == pdPASS) {
		printf("Démarrage de la tâche Overflow...\r\n");
		xSemaphoreGive(xMutexPrint);
	}

	volatile char array[2000];
	int i;
	for(i = 0; i < 2000; i++) {
		array[i] = (char)i;
	}

	if (xMutexPrint != NULL && xSemaphoreTake(xMutexPrint, portMAX_DELAY) == pdPASS) {
		printf("Fin de la tâche Overflow\r\n");
		xSemaphoreGive(xMutexPrint);
	}

	vTaskDelete(NULL);
}

int sh_overflow(int argc, char **argv)
{
	BaseType_t ret = xTaskCreate(overflow_task, "Overflow_task", STACK_SIZE_ERROR, NULL,
			TASK_OVER_PRIORITY, &xOverflowHandle);

	if (xMutexPrint != NULL && xSemaphoreTake(xMutexPrint, portMAX_DELAY) == pdPASS) {
		if (ret == pdPASS) {
			printf("Tâche Overflow créée avec succès !\r\n");
		} else {
			printf("Erreur création tâche Overflow.\r\n");
			Error_Handler();
		}
		xSemaphoreGive(xMutexPrint);
	}

	return 1;
}

void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
	printf("Stack overflow détecté dans la tâche : %s\r\n", pcTaskName);
	Error_Handler();
}


void configureTimerForRunTimeStats(void)
{
	HAL_TIM_Base_Start_IT(&htim6);
}

void stats_task(void *pvParameters)
{
	char stats_buffer[512];
	char task_list_buffer[512];

	printf("\r\n=== Liste des tâches (vTaskList) ===\r\n");
	vTaskList(task_list_buffer);
	printf("Nom\tEtat\tPriorité\tStack\tN°\r\n");
	printf("%s\r\n", task_list_buffer);

	printf("=== Statistiques d'exécution (vTaskGetRunTimeStats) ===\r\n");
	vTaskGetRunTimeStats(stats_buffer);
	printf("Nom\tTemps\t\t%% CPU\r\n");
	printf("%s\r\n", stats_buffer);

	vTaskDelay(pdMS_TO_TICKS(2000));
}

int sh_stats(int argc, char **argv)
{
	BaseType_t ret = xTaskCreate(stats_task, "Stats_task", 512, NULL, tskIDLE_PRIORITY + 1, NULL);
	if (ret == pdPASS) {
		//printf("Tâche de statistiques lancée.\r\n");
	} else {
		//printf("Erreur création tâche de statistiques.\r\n");
		Error_Handler();
	}
	return 0;
}


void task_shell(void * unused){
	shell_init();
	shell_add('f', fonction, "Une fonction inutile");
	shell_add('l', sh_led, "Fonction LED");
	shell_add('s', sh_spam, "Fonction SPAM");
	shell_add('e',sh_stats, "Fonction Stats");
	//shell_add('o',sh_overflow, "Fonction Overflow");
	shell_run();	// boucle infinie
}


extern volatile unsigned long ulHighFrequencyTimerTicks;

unsigned long getRunTimeCounterValue(void)
{
	return ulHighFrequencyTimerTicks++;
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
	MX_TIM6_Init();
	/* USER CODE BEGIN 2 */

	//HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET);

	BaseType_t ret;
	//
	//	xTaskCreate(BlinkTask, "LED_Task", TASK_STACK, NULL, BLINK_TASK_PRIORITY, NULL);


	////////Semaphore
	xSemaphore = xSemaphoreCreateBinary();

	//	if(xSemaphore!=NULL)
	//	{
	//		xTaskCreate(taskGive, "Give", STACK_SIZE,NULL,2,NULL);
	//		xTaskCreate(taskTake, "Take", STACK_SIZE,NULL,1,NULL);
	//
	//		vTaskStartScheduler();
	//	}

	////////////Queues
	char stats_buffer[512];  // Taille suffisante
	vTaskGetRunTimeStats(stats_buffer);
	printf("%s\r\n", stats_buffer);


	//	xQueue = xQueueCreate(QUEUE_LENGTH, ITEM_SIZE);
	//	configASSERT(xQueue!=NULL);
	//
	//	ret = xTaskCreate(taskTake, "taskTake", STACK_SIZE, NULL, 2, &taskTakeHandle);
	//	configASSERT(ret == pdPASS);
	//
	//	ret = xTaskCreate(taskGive, "taskGive", STACK_SIZE, NULL, 1, NULL);
	//	configASSERT(ret == pdPASS);
	//
	//	vTaskStartScheduler();


	//	xMutex = xSemaphoreCreateMutex();
	//	configASSERT(xMutex != NULL);
	//
	//	ret = xTaskCreate(task_bug, "Tache 1", STACK_SIZE, (void *) TASK1_DELAY, TASK1_PRIORITY, NULL);
	//	configASSERT(pdPASS == ret);
	//	ret = xTaskCreate(task_bug, "Tache 2", STACK_SIZE, (void *) TASK2_DELAY, TASK2_PRIORITY, NULL);
	//	configASSERT(pdPASS == ret);


	//vTaskStartScheduler();


	xMutexPrint = xSemaphoreCreateMutex();
	if (xMutexPrint == NULL) {
		Error_Handler();  // Création mutex échouée
	}

	//////////////SHELL

	if (xTaskCreate(task_shell, "Shell", TASK_SHELL_STACK_DEPTH, NULL, TASK_SHELL_PRIORITY, &h_task_shell) != pdPASS)
	{
		printf("Error creating task shell\r\n");
		Error_Handler();
	}


	vTaskStartScheduler();
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
		printf("ERREUR\r\n");
		HAL_Delay(500);
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
