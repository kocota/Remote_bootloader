#include "main.h"
#include "MainTask.h"
#include "modbus.h"
#include "cmsis_os.h"
#include "gpio.h"
#include "fm25v02.h"


extern osThreadId MainTaskHandle;
extern osMutexId Fm25v02MutexHandle;
extern control_register_struct control_registers;
extern bootloader_register_struct bootloader_registers;


uint16_t packet_crc;
uint32_t calculating_packet_crc;
uint8_t buffer_packet_data[256];
uint32_t address_to_read_write;
uint32_t data_to_write;
uint32_t sector_error;

FLASH_EraseInitTypeDef erase_init;


extern SPI_HandleTypeDef hspi2;
extern UART_HandleTypeDef huart3;


/*
typedef void (*pFunction) (void);
pFunction Jump_To_Application;
uint32_t JumpAddress;
uint32_t ApplicationAddress2 = 0x08010000;
*/


void ThreadMainTask(void const * argument)
{

	//uint8_t temp_read_h;
	uint8_t temp_read_l;

	osThreadSuspend(MainTaskHandle); // ждем пока не будут вычитаны регистры и не получен статус фаз А1,А2,В1,В2,С1,С2

	//osMutexWait(Fm25v02MutexHandle, osWaitForever);
	//fm25v02_write(2*READY_DOWNLOAD_REG, 0x00); // устанавливаем регистр готовности к загрузке прошивки
	//fm25v02_write(2*READY_DOWNLOAD_REG+1, 0x01);
	//bootloader_registers.ready_download_reg = 0x0001;
	//osMutexRelease(Fm25v02MutexHandle);

	osMutexWait(Fm25v02MutexHandle, osWaitForever);// обнуляем регистр очисти страниц, чтобы при запуске не произошла очистка
	fm25v02_write(2*CLEAR_PAGE_ON_REG, 0x00);
	fm25v02_write(2*CLEAR_PAGE_ON_REG+1, 0x00);
	bootloader_registers.clear_page_on_reg = 0x0000;
	osMutexRelease(Fm25v02MutexHandle);

	osMutexWait(Fm25v02MutexHandle, osWaitForever);// обнуляем регистр записи в память контроллера, чтобы при запуске не произошла запись
	fm25v02_write(2*WRITE_ARRAY_REG, 0x00);
	fm25v02_write(2*WRITE_ARRAY_REG+1, 0x00);
	bootloader_registers.write_array_reg = 0x0000;
	osMutexRelease(Fm25v02MutexHandle);

	osMutexWait(Fm25v02MutexHandle, osWaitForever);// обнуляем регистр чтения страниц, чтобы при запуске не произошло чтение
	fm25v02_write(2*READ_ARRAY_REG, 0x00);
	fm25v02_write(2*READ_ARRAY_REG+1, 0x00);
	bootloader_registers.read_array_reg = 0x0000;
	osMutexRelease(Fm25v02MutexHandle);




	for(;;)
	{
		if(bootloader_registers.working_mode_reg == 1) // если включен режим обновления программы
		{

			if(bootloader_registers.ready_download_reg == 0x0000)
			{
				osMutexWait(Fm25v02MutexHandle, osWaitForever);
				fm25v02_write(2*READY_DOWNLOAD_REG, 0x00); // устанавливаем регистр готовности к загрузке прошивки
				fm25v02_write(2*READY_DOWNLOAD_REG+1, 0x01);
				bootloader_registers.ready_download_reg = 0x0001;
				osMutexRelease(Fm25v02MutexHandle);
			}

			switch(bootloader_registers.write_array_reg) // запись массива байт в память контроллера
			{
				case(1):

					//address_to_read_write = ((((uint32_t)(bootloader_registers.address_to_write_high_reg))<<24)&0xFF000000) | ((((uint32_t)(bootloader_registers.address_to_write_2_reg))<<16)&0x00FF0000) | ((((uint32_t)(bootloader_registers.address_to_write_3_reg))<<8)&0x0000FF00) | (((uint32_t)(bootloader_registers.address_to_write_low_reg))&0x000000FF); // получаем переменную адреса для записи данных в память контроллера

					address_to_read_write = ((((uint32_t)(bootloader_registers.address_to_write_2_reg))<<24)&0xFF000000) | ((((uint32_t)(bootloader_registers.address_to_write_3_reg))<<16)&0x00FF0000) | ((((uint32_t)(bootloader_registers.address_to_write_high_reg))<<8)&0x0000FF00) | (((uint32_t)(bootloader_registers.address_to_write_low_reg))&0x000000FF); // получаем переменную адреса для записи данных в память контроллера

					packet_crc = (((bootloader_registers.packet_crc_low_reg)<<8)&0xFF00) | ((bootloader_registers.packet_crc_high_reg)&0x00FF); // получаем значение контрольной суммы из регистров контрольной суммы пакета

					for(uint16_t i=0; i<(bootloader_registers.byte_quantity_reg); i++) // заполняем буфер с данными из регистров
					{
						osMutexWait(Fm25v02MutexHandle, osWaitForever);
						//fm25v02_read(2*(PACKET_DATA_0_REG+i), &temp_read_h);
						fm25v02_read(2*(PACKET_DATA_0_REG+i)+1, &temp_read_l);
						osMutexRelease(Fm25v02MutexHandle);
						buffer_packet_data[i] = temp_read_l;
					}

					calculating_packet_crc = CRC16( (unsigned char*)(&buffer_packet_data[0]), (unsigned int)(bootloader_registers.byte_quantity_reg) ); // вычисляем значение контрольной суммы данных из регистров

					if( packet_crc == calculating_packet_crc) // если контрольная сумма из регистров контрольной суммы пакета совпадает с расчетной контрольной суммой данных из регистров с данными
					{
						//osThreadSuspendAll();

						HAL_FLASH_Unlock(); // разблокируем запись памяти контроллера
						for(uint16_t i=0; i<(bootloader_registers.byte_quantity_reg); i++)
						{
							while( HAL_FLASH_Program(FLASH_TYPEPROGRAM_BYTE, address_to_read_write+i, buffer_packet_data[i]) != HAL_OK ) // ничего не делаем пока не выполнится запись в память контроллера
							{

							}
						}
						HAL_FLASH_Lock(); // блокируем запись памяти контроллера

						//osThreadResumeAll();

						osMutexWait(Fm25v02MutexHandle, osWaitForever);

						fm25v02_write(2*WRITE_ARRAY_REG, 0x00); // обнуляем регистр и переменную записи массива
						fm25v02_write(2*WRITE_ARRAY_REG+1, 0x00);
						bootloader_registers.write_array_reg = 0x0000;

						osMutexRelease(Fm25v02MutexHandle);

					}

				break;
			}

			switch(bootloader_registers.read_array_reg) // чтение массива из памяти контроллера
			{
				case(1):

						address_to_read_write = ((((uint32_t)(bootloader_registers.address_to_write_high_reg))<<24)&0xFF000000) | ((((uint32_t)(bootloader_registers.address_to_write_2_reg))<<16)&0x00FF0000) | ((((uint32_t)(bootloader_registers.address_to_write_3_reg))<<8)&0x0000FF00) | (((uint32_t)(bootloader_registers.address_to_write_low_reg))&0x000000FF); // получаем переменную адреса для чтения данных из памяти контроллера

					for(uint16_t i=0; i<(bootloader_registers.byte_quantity_reg); i++)
					{
						osMutexWait(Fm25v02MutexHandle, osWaitForever);

						fm25v02_write(2*(PACKET_DATA_0_REG+i), 0x00);
						fm25v02_write(2*(PACKET_DATA_0_REG+i)+1, *( (uint32_t*)(address_to_read_write+i) ) );

						osMutexRelease(Fm25v02MutexHandle);
					}

					for(uint16_t i=0; i<(bootloader_registers.byte_quantity_reg); i++) // заполняем буфер с данными из регистров
					{
						osMutexWait(Fm25v02MutexHandle, osWaitForever);
						//fm25v02_read(2*(PACKET_DATA_0_REG+i), &temp_read_h);
						fm25v02_read(2*(PACKET_DATA_0_REG+i)+1, &temp_read_l);
						osMutexRelease(Fm25v02MutexHandle);
						buffer_packet_data[i] = temp_read_l;
					}

					calculating_packet_crc = CRC16( (unsigned char*)(&buffer_packet_data[0]), (unsigned int)(bootloader_registers.byte_quantity_reg) ); // вычисляем значение контрольной суммы данных из регистров

					osMutexWait(Fm25v02MutexHandle, osWaitForever);

					fm25v02_write(2*PACKET_CRC_HIGH_REG, 0x00);
					fm25v02_write(2*PACKET_CRC_HIGH_REG+1, (uint8_t)calculating_packet_crc ); //записываем в регистр старший байт контрольной суммы пакета

					fm25v02_write(2*PACKET_CRC_LOW_REG, 0x00);
					fm25v02_write(2*PACKET_CRC_LOW_REG+1, (uint8_t)(calculating_packet_crc>>8) ); //записываем в регистр младший байт контрольной суммы пакета

					fm25v02_write(2*READ_ARRAY_REG, 0x00); // обнуляем регистр и переменную чтения массива
					fm25v02_write(2*READ_ARRAY_REG+1, 0x00);
					bootloader_registers.read_array_reg = 0x0000;

					osMutexRelease(Fm25v02MutexHandle);

					break;
			}

			switch(bootloader_registers.clear_page_on_reg) // очистка указанной страницы памяти контроллера
			{
				case(1):

					erase_init.TypeErase = FLASH_TYPEERASE_SECTORS; // заполняем структуру с параметрами очистки памяти
					erase_init.VoltageRange = FLASH_VOLTAGE_RANGE_3;
					erase_init.Sector = bootloader_registers.clear_page_number_reg;
					erase_init.NbSectors = 1;
					erase_init.Banks = 1;

					//osThreadSuspendAll();

					HAL_FLASH_Unlock(); // разблокируем запись памяти контроллера

					while( HAL_FLASHEx_Erase(&erase_init, &sector_error) != HAL_OK ) // выполняем очистку указанной страницы памяти
					{

					}

					HAL_FLASH_Lock(); // блокируем запись памяти контроллера

					//osThreadResumeAll();

					if(sector_error != 0xFFFFFFFF) // если произошла ошибка очистки сектора памяти
					{
						// здесь должен быть обработчик ошибки очистки сектора памяти
					}

					osMutexWait(Fm25v02MutexHandle, osWaitForever);

					fm25v02_write(2*CLEAR_PAGE_ON_REG, 0x00); // обнуляем регистр и переменную записи массива
					fm25v02_write(2*CLEAR_PAGE_ON_REG+1, 0x00);
					bootloader_registers.clear_page_on_reg = 0x0000;

					osMutexRelease(Fm25v02MutexHandle);

					break;
			}

		}

		else if(bootloader_registers.working_mode_reg == 0) // если включен режим работы
		{

			if(bootloader_registers.ready_download_reg == 0x0001)
			{
				osMutexWait(Fm25v02MutexHandle, osWaitForever);
				fm25v02_write(2*READY_DOWNLOAD_REG, 0x00); // сбрасываем регистр готовности к загрузке прошивки
				fm25v02_write(2*READY_DOWNLOAD_REG+1, 0x00);
				bootloader_registers.ready_download_reg = 0x0000;
				osMutexRelease(Fm25v02MutexHandle);
			}

			if(bootloader_registers.jump_attempt_reg < bootloader_registers.max_jump_attempt_reg)
			{

				NVIC_SystemReset();

			}
		}

		switch(control_registers.reset_control_reg) // удаленная перезагрузка контроллера
		{
			case(1):
				osMutexWait(Fm25v02MutexHandle, osWaitForever);
				fm25v02_write(2*RESET_CONTROL_REG, 0);
				fm25v02_write(2*RESET_CONTROL_REG+1, 0);
				osMutexRelease(Fm25v02MutexHandle);
				NVIC_SystemReset();
			break;

		}




		//HAL_GPIO_TogglePin(GPIOE, GPIO_PIN_1);



		osDelay(10);
	}
}
