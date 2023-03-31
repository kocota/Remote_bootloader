/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

#include "gpio.h"
#include "m95.h"
#include "fm25v02.h"
#include "modbus.h"


/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
 IWDG_HandleTypeDef hiwdg;

SPI_HandleTypeDef hspi2;

UART_HandleTypeDef huart3;
DMA_HandleTypeDef hdma_usart3_tx;
DMA_HandleTypeDef hdma_usart3_rx;

osThreadId defaultTaskHandle;
/* USER CODE BEGIN PV */

osMessageQId ModbusQueueHandle;
osTimerId AT_TimerHandle;
osTimerId Ring_Center_TimerHandle;
osMutexId UartMutexHandle;
osSemaphoreId TransmissionStateHandle;


osThreadId M95TaskHandle;
osThreadId ModbusTaskHandle;
osThreadId MainTaskHandle;
osThreadId ModbusPacketTaskHandle;
osThreadId ReadRegistersTaskHandle;


osMutexId Fm25v02MutexHandle;

osSemaphoreId ModbusPacketReceiveHandle;


uint8_t modem_rx_data[256];
char modem_rx_buffer[256];
uint8_t modbus_buffer[20][256];
uint8_t modem_rx_number = 0;
//uint8_t modbus_buffer_number = 0;

volatile uint8_t read_rx_state;

extern bootloader_register_struct bootloader_registers;


typedef void (*pFunction) (void);
pFunction Jump_To_Application;
uint32_t JumpAddress;
uint32_t ApplicationAddress2 = 0x08010000;


uint32_t calculating_firmware_crc;
uint32_t start_address;
uint32_t end_address;
uint32_t firmware_length;
uint16_t firmware_crc;

uint32_t start_default_task_delay = 200;

uint8_t test_data1;



uint8_t temp_id_high;
uint8_t temp_id_low;

uint8_t temp_ip1;
uint8_t temp_ip2;
uint8_t temp_ip3;
uint8_t temp_ip4;

uint8_t temp_port_high;
uint8_t temp_port_low;

extern control_register_struct control_registers;

volatile uint8_t modem_reset_state = 0;


/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_IWDG_Init(void);
static void MX_SPI2_Init(void);
static void MX_USART3_UART_Init(void);
void StartDefaultTask(void const * argument);

/* USER CODE BEGIN PFP */

void Callback_AT_Timer(void const * argument);
void Callback_Ring_Center_Timer(void const * argument);

void ThreadM95Task(void const * argument);
void ThreadModbusTask(void const * argument);
void ThreadMainTask(void const * argument);
void ThreadModbusPacketTask(void const * argument);
void ThreadReadRegistersTask(void const * argument);


/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
	  if (huart->Instance == USART3)
	  {
		  osSemaphoreRelease(TransmissionStateHandle);
	  }
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	  if (huart->Instance == USART3)
	  {
		  modem_rx_buffer[modem_rx_number++] = modem_rx_data[0];
		  osMessagePut(ModbusQueueHandle, (uint32_t)modem_rx_data[0], 2000);
		  HAL_UART_Receive_DMA(&huart3, &modem_rx_data[0], 1);
	  }
}

//void HAL_MspInit(void)
//{


//}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */


  	uint8_t temp_h;
  	uint8_t temp_l;
  	uint16_t temp_jump_attempt;

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
  MX_DMA_Init();
  MX_IWDG_Init();
  MX_SPI2_Init();
  MX_USART3_UART_Init();
  /* USER CODE BEGIN 2 */

  BUZ_ON(); // пикаем бузером
  HAL_Delay(50);
  BUZ_OFF();

  LED_VD3_ON();

  if( ((RCC->BDCR)&0x02) != 0 ) // Проверяем, запустился ли часовой кварц, если запустился включаем светодиод
  {
	  //LED_VD4_ON();
  }
  else
  {
	  //LED_VD4_OFF();
  }




	//fm25v02_write(2*START_ADDRESS_FIRMWARE_2_REG, 0x00);
	//fm25v02_write(2*START_ADDRESS_FIRMWARE_2_REG+1, 0x08);
	//bootloader_registers.start_address_firmware_2_reg = 0x0008;

	//fm25v02_write(2*START_ADDRESS_FIRMWARE_3_REG, 0x00);
	//fm25v02_write(2*START_ADDRESS_FIRMWARE_3_REG+1, 0x01);
	//bootloader_registers.start_address_firmware_3_reg = 0x0001;

	//fm25v02_write(2*START_ADDRESS_FIRMWARE_HIGH_REG, 0x00);
	//fm25v02_write(2*START_ADDRESS_FIRMWARE_HIGH_REG+1, 0x00);
	//bootloader_registers.start_address_firmware_high_reg = 0x0000;

	//fm25v02_write(2*START_ADDRESS_FIRMWARE_LOW_REG, 0x00);
	//fm25v02_write(2*START_ADDRESS_FIRMWARE_LOW_REG+1, 0x00);
	//bootloader_registers.start_address_firmware_low_reg = 0x0000;



	//fm25v02_write(2*END_ADDRESS_FIRMWARE_2_REG, 0x00);
	//fm25v02_write(2*END_ADDRESS_FIRMWARE_2_REG+1, 0x08);
	//bootloader_registers.end_address_firmware_2_reg = 0x0008;

	//fm25v02_write(2*END_ADDRESS_FIRMWARE_3_REG, 0x00);
	//fm25v02_write(2*END_ADDRESS_FIRMWARE_3_REG+1, 0x01);
	//bootloader_registers.end_address_firmware_3_reg = 0x0001;

	//fm25v02_write(2*END_ADDRESS_FIRMWARE_HIGH_REG, 0x00);
	//fm25v02_write(2*END_ADDRESS_FIRMWARE_HIGH_REG+1, 0xDD);
	//bootloader_registers.end_address_firmware_high_reg = 0x00DD;

	//fm25v02_write(2*END_ADDRESS_FIRMWARE_LOW_REG, 0x00);
	//fm25v02_write(2*END_ADDRESS_FIRMWARE_LOW_REG+1, 0x33);
	//bootloader_registers.end_address_firmware_low_reg = 0x0033;



	//fm25v02_write(2*CRC_FIRMWARE_HIGH_REG, 0x00);
	//fm25v02_write(2*CRC_FIRMWARE_HIGH_REG+1, 0x07);
	//bootloader_registers.crc_firmware_high_reg = 0x0007;

	//fm25v02_write(2*CRC_FIRMWARE_LOW_REG, 0x00);
	//fm25v02_write(2*CRC_FIRMWARE_LOW_REG+1, 0xB4);
	//bootloader_registers.crc_firmware_low_reg = 0x00B4;



  	/*
	fm25v02_write(2*IP_1_REG, 0x00);
	fm25v02_write(2*IP_1_REG+1, 0x00);
	fm25v02_write(2*IP_2_REG, 0x00);
	fm25v02_write(2*IP_2_REG+1, 0x00);
	fm25v02_write(2*IP_3_REG, 0x00);
	fm25v02_write(2*IP_3_REG+1, 0x00);
	fm25v02_write(2*IP_4_REG, 0x00);
	fm25v02_write(2*IP_4_REG+1, 0x00);
	fm25v02_write(2*PORT_HIGH_REG, 0x00);
	fm25v02_write(2*PORT_HIGH_REG+1, 0x00);
	fm25v02_write(2*PORT_LOW_REG, 0x00);
	fm25v02_write(2*PORT_LOW_REG+1, 0x00);

	fm25v02_write(2*ID_HIGH_REG, 0x00);
	fm25v02_write(2*ID_HIGH_REG+1, 0x01);

	fm25v02_write(2*ID_LOW_REG, 0x00);
	fm25v02_write(2*ID_LOW_REG+1, 0x0F);
	*/


	//fm25v02_write(2*WRITE_ARRAY_REG, 0x00); // обнулим регистр записи массива
	//fm25v02_write(2*WRITE_ARRAY_REG+1, 0x00);
	//bootloader_registers.write_array_reg = 0x0000;

	//fm25v02_read(271, &test_data1);

	//fm25v02_write(271, 0x00);

	/*
	fm25v02_read(256, &test_data1); // вычитываем из FRAM памяти регистр старой версии загрузчика

	if( test_data1 !=0 ) // если этот регистр не равен 0, то вычитываем старые параметры ID устройства, IP, номер порта и переписываем их в регистры новой прошивки, затем выставляем значение регистра в 0
	{
		fm25v02_read(4267, &temp_id_high);
		fm25v02_read(4268, &temp_id_low);

		fm25v02_read(4271, &temp_ip1);
		fm25v02_read(4272, &temp_ip2);
		fm25v02_read(4273, &temp_ip3);
		fm25v02_read(4274, &temp_ip4);

		fm25v02_read(4275, &temp_port_high);
		fm25v02_read(4276, &temp_port_low);

		fm25v02_write(2*ID_HIGH_REG, 0);
		fm25v02_write(2*ID_HIGH_REG+1, temp_id_high);
		control_registers.id_high_reg = temp_id_high;

		fm25v02_write(2*ID_LOW_REG, 0);
		fm25v02_write(2*ID_LOW_REG+1, temp_id_low);
		control_registers.id_low_reg = temp_id_low;

		fm25v02_write(2*IP_1_REG, 0);
		fm25v02_write(2*IP_1_REG+1, temp_ip1);
		control_registers.ip1_reg = temp_ip1;

		fm25v02_write(2*IP_2_REG, 0);
		fm25v02_write(2*IP_2_REG+1, temp_ip2);
		control_registers.ip2_reg = temp_ip2;

		fm25v02_write(2*IP_3_REG, 0);
		fm25v02_write(2*IP_3_REG+1, temp_ip3);
		control_registers.ip3_reg = temp_ip3;

		fm25v02_write(2*IP_4_REG, 0);
		fm25v02_write(2*IP_4_REG+1, temp_ip4);
		control_registers.ip4_reg = temp_ip4;

		fm25v02_write(2*PORT_HIGH_REG, 0);
		fm25v02_write(2*PORT_HIGH_REG+1, temp_port_high);
		control_registers.port_high_reg = temp_port_high;

		fm25v02_write(2*PORT_LOW_REG, 0);
		fm25v02_write(2*PORT_LOW_REG+1, temp_port_low);
		control_registers.port_low_reg = temp_port_low;

		fm25v02_write(256, 0); // выставляем значение регистра в 0
	}
	*/




  	read_bootloader_registers_no_rtos(); // читаем ригистры бутлоадера до запуска операционной системы


	start_address = ((((uint32_t)(bootloader_registers.start_address_firmware_high_reg))<<24)&0xFF000000) | ((((uint32_t)(bootloader_registers.start_address_firmware_2_reg))<<16)&0x00FF0000) | ((((uint32_t)(bootloader_registers.start_address_firmware_3_reg))<<8)&0x0000FF00) | (((uint32_t)(bootloader_registers.start_address_firmware_low_reg))&0x000000FF);

	end_address = ((((uint32_t)(bootloader_registers.end_address_firmware_2_reg))<<24)&0xFF000000) | ((((uint32_t)(bootloader_registers.end_address_firmware_3_reg))<<16)&0x00FF0000) | ((((uint32_t)(bootloader_registers.end_address_firmware_high_reg))<<8)&0x0000FF00) | (((uint32_t)(bootloader_registers.end_address_firmware_low_reg))&0x000000FF);

	//end_address = 0x08029EEB;

	firmware_length = end_address - start_address + 1;

	firmware_crc = (((bootloader_registers.crc_firmware_low_reg)<<8)&0xFF00) | ((bootloader_registers.crc_firmware_high_reg)&0x00FF);

	if( (start_address >= 0x08000000) && (start_address <= 0x080FFFFF) && ((start_address + firmware_length) <= 0x080FFFFF) && (firmware_length <= 0xFFFFF) ) // стартовый адресс для расчета контрольной суммы должен входить в диапазон основной памяти контроллера, длина прошивки не должна превышать длину памяти программ контроллера иначе при чтении будет хардфолт
	{
		calculating_firmware_crc = CRC16((unsigned char*)start_address, firmware_length);
	}
	else
	{
		calculating_firmware_crc = 0;
	}



	if( (firmware_crc == calculating_firmware_crc) && (firmware_crc != 0) ) // если рассчетная контрольная сумма прошивки совпадает с указанной и не равна 0
	{


		fm25v02_write(2*FIRMWARE_CORRECTNESS_REG, 0x00); // записываем в регистры и переменную корректность прошивки
		fm25v02_write(2*FIRMWARE_CORRECTNESS_REG+1, 0x01);
		bootloader_registers.firmware_correctness_reg = 0x0001;



	}

	else // если рассчетная контрольная сумма прошивки не совпадает с указанной
	{


		fm25v02_write(2*FIRMWARE_CORRECTNESS_REG, 0x00); // записываем в регистры и переменную ошибку контрольной суммы прошивки
		fm25v02_write(2*FIRMWARE_CORRECTNESS_REG+1, 0x00);
		bootloader_registers.firmware_correctness_reg = 0x0000;



	}

	//----тест, потом удалить
	//fm25v02_write(2*FIRMWARE_CORRECTNESS_REG, 0x00); // записываем в регистры и переменную корректность прошивки
	//fm25v02_write(2*FIRMWARE_CORRECTNESS_REG+1, 0x01);
	//bootloader_registers.firmware_correctness_reg = 0x0001;
	//-----------------------


  	if( bootloader_registers.working_mode_reg == 0 ) // если включен нормальный режим работы
  	{
  		if(bootloader_registers.jump_attempt_reg < bootloader_registers.max_jump_attempt_reg)
  		{
  			if( bootloader_registers.firmware_correctness_reg == 1 ) // если прошивка корректна (контрольная сумма совпадает)
  			{

  				fm25v02_write(2*FIRMWARE_CORRECTNESS_REG, 0x00); // обнуляем корректность прошивки
  				fm25v02_write(2*FIRMWARE_CORRECTNESS_REG+1, 0x00);
  				bootloader_registers.firmware_correctness_reg = 0x0000;

  				fm25v02_write(2*WORKING_MODE_REG, 0x00); // обнуляем корректность прошивки
  				fm25v02_write(2*WORKING_MODE_REG+1, 0x00);
  				bootloader_registers.working_mode_reg = 0x0000;

  				fm25v02_read(271, &test_data1); // обнуляем регистр режима работы с номером 271 в fram памяти, нужно только для старой прошивки


  				HAL_UART_DeInit(&huart3);
  				HAL_SPI_DeInit(&hspi2);
  				HAL_DMA_DeInit(&hdma_usart3_rx);
  				HAL_DMA_DeInit(&hdma_usart3_tx);

  				HAL_RCC_DeInit();

  				HAL_DeInit(); // деинициализируем HAL

  				JumpAddress = *(__IO uint32_t*) (ApplicationAddress2+4); // адрес перехода
  				Jump_To_Application = (pFunction) JumpAddress; // приводим адрес к типу функции
  				__set_MSP (*(__IO uint32_t*) ApplicationAddress2);// устанавливаем указатель стека

  				SCB->VTOR = FLASH_BASE | 0x10000; // смещаем таблицу векторов прерываний

  				Jump_To_Application(); // переход к основной программе

  			}
  			else if( bootloader_registers.firmware_correctness_reg == 0 )
  			{
  				fm25v02_read(2*JUMP_ATTEMPT_REG, &temp_h); // увеличиваем количество попыток перейти в основную программу
  				fm25v02_read(2*JUMP_ATTEMPT_REG+1, &temp_l);
  				temp_jump_attempt = (((uint16_t)temp_h)<<8)|((uint16_t)(temp_l));
  				temp_jump_attempt = temp_jump_attempt + 1;
  				temp_h = (uint8_t)(temp_jump_attempt>>8);
  				temp_l = (uint8_t)(temp_jump_attempt);
  				fm25v02_write(2*JUMP_ATTEMPT_REG, temp_h);
  				fm25v02_write(2*JUMP_ATTEMPT_REG+1, temp_l);

  				HAL_Delay(100);

  				osMutexWait(Fm25v02MutexHandle, osWaitForever); // ждем освобождение мьютекса записи в память
  				NVIC_SystemReset();

  			}
  		}

  		else
  		{
  			start_default_task_delay = 100;
  		}
  	}


  /* USER CODE END 2 */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */

  osMutexDef(UartMutex);
  UartMutexHandle = osMutexCreate(osMutex(UartMutex));

  osMutexDef(Fm25v02Mutex);
  Fm25v02MutexHandle = osMutexCreate(osMutex(Fm25v02Mutex));

  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */

  osSemaphoreDef(TransmissionState);
  TransmissionStateHandle = osSemaphoreCreate(osSemaphore(TransmissionState), 1);

  osSemaphoreDef(ModbusPacketReceive);
  ModbusPacketReceiveHandle = osSemaphoreCreate(osSemaphore(ModbusPacketReceive), 1);

  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */

  osTimerDef(AT_Timer, Callback_AT_Timer);
  AT_TimerHandle = osTimerCreate(osTimer(AT_Timer), osTimerOnce, NULL);

  osTimerDef(Ring_Center_Timer, Callback_Ring_Center_Timer);
  Ring_Center_TimerHandle = osTimerCreate(osTimer(Ring_Center_Timer), osTimerOnce, NULL);

  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */

  osMessageQDef(ModbusQueue, 256, uint8_t);
  ModbusQueueHandle = osMessageCreate(osMessageQ(ModbusQueue), NULL);

  //osMessageQDef(MeterQueue, 256, uint8_t);
  //MeterQueueHandle = osMessageCreate(osMessageQ(MeterQueue), NULL);

  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* definition and creation of defaultTask */
  osThreadDef(defaultTask, StartDefaultTask, osPriorityNormal, 0, 128);
  defaultTaskHandle = osThreadCreate(osThread(defaultTask), NULL);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */

  osThreadDef(M95Task, ThreadM95Task, osPriorityNormal, 0, 128);
  M95TaskHandle = osThreadCreate(osThread(M95Task), NULL);

  osThreadDef(ModbusTask, ThreadModbusTask, osPriorityNormal, 0, 128);
  ModbusTaskHandle = osThreadCreate(osThread(ModbusTask), NULL);

  osThreadDef(MainTask, ThreadMainTask, osPriorityNormal, 0, 128);
  MainTaskHandle = osThreadCreate(osThread(MainTask), NULL);

  osThreadDef(ModbusPacketTask, ThreadModbusPacketTask, osPriorityNormal, 0, 128);
  ModbusPacketTaskHandle = osThreadCreate(osThread(ModbusPacketTask), NULL);

  osThreadDef(ReadRegistersTask, ThreadReadRegistersTask, osPriorityNormal, 0, 128);
  ReadRegistersTaskHandle = osThreadCreate(osThread(ReadRegistersTask), NULL);

  /* USER CODE END RTOS_THREADS */

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
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI|RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 25;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
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

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief IWDG Initialization Function
  * @param None
  * @retval None
  */
static void MX_IWDG_Init(void)
{

  /* USER CODE BEGIN IWDG_Init 0 */

  /* USER CODE END IWDG_Init 0 */

  /* USER CODE BEGIN IWDG_Init 1 */

  /* USER CODE END IWDG_Init 1 */
  hiwdg.Instance = IWDG;
  hiwdg.Init.Prescaler = IWDG_PRESCALER_32;
  hiwdg.Init.Reload = 4000;
  if (HAL_IWDG_Init(&hiwdg) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN IWDG_Init 2 */

  /* USER CODE END IWDG_Init 2 */

}

/**
  * @brief SPI2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI2_Init(void)
{

  /* USER CODE BEGIN SPI2_Init 0 */

  /* USER CODE END SPI2_Init 0 */

  /* USER CODE BEGIN SPI2_Init 1 */

  /* USER CODE END SPI2_Init 1 */
  /* SPI2 parameter configuration*/
  hspi2.Instance = SPI2;
  hspi2.Init.Mode = SPI_MODE_MASTER;
  hspi2.Init.Direction = SPI_DIRECTION_2LINES;
  hspi2.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi2.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi2.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi2.Init.NSS = SPI_NSS_SOFT;
  hspi2.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_32;
  hspi2.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi2.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi2.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi2.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI2_Init 2 */

  /* USER CODE END SPI2_Init 2 */

}

/**
  * @brief USART3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART3_UART_Init(void)
{

  /* USER CODE BEGIN USART3_Init 0 */

  /* USER CODE END USART3_Init 0 */

  /* USER CODE BEGIN USART3_Init 1 */

  /* USER CODE END USART3_Init 1 */
  huart3.Instance = USART3;
  huart3.Init.BaudRate = 9600;
  huart3.Init.WordLength = UART_WORDLENGTH_8B;
  huart3.Init.StopBits = UART_STOPBITS_1;
  huart3.Init.Parity = UART_PARITY_NONE;
  huart3.Init.Mode = UART_MODE_TX_RX;
  huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart3.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART3_Init 2 */

  /* USER CODE END USART3_Init 2 */

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Stream1_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Stream1_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(DMA1_Stream1_IRQn);
  /* DMA1_Stream3_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Stream3_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(DMA1_Stream3_IRQn);

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOI_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOE, GPIO_PIN_6, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOI, GPIO_PIN_8, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOI, GPIO_PIN_10|GPIO_PIN_11, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOF, GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOH, GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_15, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOH, GPIO_PIN_5|GPIO_PIN_6, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_3|GPIO_PIN_5, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_11|GPIO_PIN_12, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOG, GPIO_PIN_10, GPIO_PIN_RESET);

  /*Configure GPIO pin : PE6 */
  GPIO_InitStruct.Pin = GPIO_PIN_6;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /*Configure GPIO pins : PI8 PI10 PI11 */
  GPIO_InitStruct.Pin = GPIO_PIN_8|GPIO_PIN_10|GPIO_PIN_11;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOI, &GPIO_InitStruct);

  /*Configure GPIO pin : PC13 */
  GPIO_InitStruct.Pin = GPIO_PIN_13;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : PF3 PF4 PF5 */
  GPIO_InitStruct.Pin = GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

  /*Configure GPIO pins : PA0 PA3 PA5 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_3|GPIO_PIN_5;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : PH2 PH3 PH5 PH6
                           PH15 */
  GPIO_InitStruct.Pin = GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_5|GPIO_PIN_6
                          |GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOH, &GPIO_InitStruct);

  /*Configure GPIO pin : PH4 */
  GPIO_InitStruct.Pin = GPIO_PIN_4;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOH, &GPIO_InitStruct);

  /*Configure GPIO pin : PB2 */
  GPIO_InitStruct.Pin = GPIO_PIN_2;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : PF11 PF12 PF13 PF14
                           PF15 */
  GPIO_InitStruct.Pin = GPIO_PIN_11|GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14
                          |GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOF, &GPIO_InitStruct);

  /*Configure GPIO pins : PG0 PG1 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

  /*Configure GPIO pins : PD11 PD12 */
  GPIO_InitStruct.Pin = GPIO_PIN_11|GPIO_PIN_12;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /*Configure GPIO pins : PG9 PG14 */
  GPIO_InitStruct.Pin = GPIO_PIN_9|GPIO_PIN_14;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF8_USART6;
  HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

  /*Configure GPIO pin : PG10 */
  GPIO_InitStruct.Pin = GPIO_PIN_10;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLDOWN;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOG, &GPIO_InitStruct);

  /*Configure GPIO pins : PB6 PB7 */
  GPIO_InitStruct.Pin = GPIO_PIN_6|GPIO_PIN_7;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF7_USART1;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : PE0 */
  GPIO_InitStruct.Pin = GPIO_PIN_0;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */

void Callback_AT_Timer(void const * argument)
{

	read_rx_state = NOT_ACTIVE;

}

void Callback_Ring_Center_Timer(void const * argument)
{
	modem_reset_state = 1;
}

/* USER CODE END 4 */

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void const * argument)
{
  /* USER CODE BEGIN 5 */
  /* Infinite loop */
  for(;;)
  {

	HAL_IWDG_Refresh(&hiwdg);
	LED_VD3_TOGGLE();

    osDelay(start_default_task_delay);
  }
  /* USER CODE END 5 */
}

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM1 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM1) {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */

	NVIC_SystemReset();

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
