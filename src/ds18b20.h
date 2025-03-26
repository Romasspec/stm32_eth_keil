#ifndef DS18B20_H_
#define DS18B20_H_

//--------------------------------------------------
//#include <string.h>
//#include <stdlib.h>
//#include <stdint.h>
//#include "MDR32F9Qx_config.h"
//#include "MDR32F9Qx_port.h"
#include "stm32f10x.h"
#include "time.h"

#define DS18b20_pin			GPIO_Pin_11
#define DS18b20_PORT		GPIOB
#define DS18b20_pin_0		(DS18b20_PORT->ODR &= ~DS18b20_pin)
#define DS18b20_pin_1		(DS18b20_PORT->ODR |= DS18b20_pin)
#define DS18b20_pin_read	(DS18b20_PORT->IDR & DS18b20_pin)

#define DS18b20_GND_pin		GPIO_Pin_10
#define DS18b20_GND_PORT	GPIOB

#define TIMEOUT_RESET		1500		// 1500 мкс
#define TIMEOUT_INIT		1000		// 1000 мс
#define CRC_POLINOM			((1<<5)|(1<<4)|1)
//	ROM FUNCTION COMMANDS
#define READ_ROM			0x33
#define MATCH_ROM			0x55
#define SKIP_ROM			0xCC
#define SEARCH_ROM			0xF0
#define ALARM_SEARCH		0xEC

// 	MEMORY COMMAND FUNCTIONS
#define WRITE_SCRATCHPAD	0x4E
#define READ_SCRATCHPAD		0xBE
#define COPY_SCRATCHPAD		0x48
#define CONVERT_T			0x44
#define RECALL_E2			0xB8
#define READ_POWER_SUPPLY	0xB4
#define SIZE_BUF			4

enum {
	RESET_L = 0,
	RESET_H,
	RESET_PRESENCE,
	RESULT
};

enum {
	MODE_SKIP_ROM  = 0,
	MODE_MATCH_ROM
};

enum {
	RESOLUTION_9BIT		= 0x1F,
	RESOLUTION_10BIT	=  0x3F,
	RESOLUTION_11BIT	=  0x5F,
	RESOLUTION_12BIT	=  0x7F
};

enum {
	MEASURE_TEMPER = 0,
	WAIT_MEASURE_TEMPER,
	READ_STRATCPAD,
	CRC_STRATCPAD
};

typedef enum {
	MEASURE_COMPLETE = 0,
	MEASURE_TEMPERATURE,
	ERROR_CRC,
	ERROR_SENSOR
} state_temper_sensor;

uint8_t ds18b20_Reset_delay(void);

uint8_t ds18b20_Reset(void);
state_temper_sensor ds18b20_Tread (void);
uint8_t ds18b20_ReadBit(void);
uint8_t ds18b20_ReadByte(void);
void ds18b20_WriteBit(uint8_t bit);
void ds18b20_WriteByte(uint8_t dt);
uint8_t ds18b20_init(uint8_t mode);
uint8_t ds18b20_MeasureTemperCmd(uint8_t mode, uint8_t DevNum);
uint8_t ds18b20_ReadStratcpad(uint8_t mode, uint8_t *Data, uint8_t DevNum);
uint8_t ds18b20_ReadStratcpad_(uint8_t mode, uint8_t *Data, uint8_t DevNum);
uint16_t ds18b20_Convert(uint16_t *dt);
uint8_t calc_CRC (uint8_t * dt, uint8_t lenght);
uint8_t ds18b20_ReadRom_(uint8_t *Data);
uint16_t ds18b20_GetTemp(void);
//--------------------------------------------------

#endif /* DS18B20_H_ */
