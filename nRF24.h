#ifndef NRF24_H
#define NRF24_H

#include <HAL.h>
#include <util/delay.h>

//Конфигурация частоты работы SPI	

#define spi_PORT	PORTB				// Порт где сидят ноги SPI
#define spi_DDR		DDRB
#define SCK			5					// Биты соответствующих выводов
#define MISO		4
#define MOSI		3
#define SS			1

#define PORT_CE 	PORTB
#define DDR_CE 		DDRB
//#define PORT_IRQ 	PORTC
//#define DDR_IRQ 	DDRC
//#define PIN_IRQ 	PINC
#define CE 			2
//#define IRQ 		1

/*регистр STATUS*/
#define STATUS          0x07
#define RX_DR           6 /*прерывание: данные получены. Для сброса записать 1.*/
#define TX_DS           5 /*прерывание: данные переданы. Для сброса записать 1.*/
#define MAX_RT          4 /*прерывание: данные не переданы. Для сброса записать 1.*/
#define RX_P_NO2        3
#define RX_P_NO1        2
#define RX_P_NO0        1
#define TX_FULL0        0 /*флаг переполнения TX FIFO буфера передачи. 1-переполнен, 0-есть еще место.*/

/*регистр CONFIG*/    //Конфигурационный регистр
#define CONFIG          0x00
#define MASK_RX_DR  	6 //вкл/откл прерывание от бита RX_DR в рег. STATUS. 0-вкл, 1-выкл.
#define MASK_TX_DS  	5 //вкл/откл прерывание от бита TX_DS в рег. STATUS. 0-вкл, 1-выкл.
#define MASK_MAX_RT 	4 //вкл/откл прерывание от бита MAX_RT в рег. STATUS. 0-вкл, 1-выкл.
#define EN_CRC      	3 //включение CRC. По умолчанию вкл. если один из битов регистра EN_AA включен.
#define CRCO        	2 //режим CRC. 0-1 байт, 1-2 байта.
#define PWR_UP      	1 //1-POWER UP, 0-POWER DOWN, по умолчанию 0.
#define PRIM_RX     	0 //0-режим передачи, 1-режим приема.

/*регистр RX_PW_P0*/
#define RX_PW_P0        0x11//указываем в нем из скольких байтов будет состоять наше поле данных для отправки. 1-32 байта

/*регистр SETUP_RETR*/ //Настройки авторетрансляции
#define SETUP_RETR 		0x04
/* 	ARD - SETUP_RETR[7:4] 	Задержка авторетрансляции 
							0 - 250мкс
							1 - 500мкс
							2 - 750мкс
							.....
							по умолчанию 0
	ARC - SETUP_RETR[3:0]	Количество попыток авторетрансляции 
							0 - 0 попыток
							1 - 1 попытка
							....
							по умолчанию 3
*/

/*регистр FEATURE*/ // хитрые настройки чипа
#define FEATURE 		0x1D
#define EN_DPL			2 // Включить динамическую длину данных
#define EN_ACK_PAY		1 // Включить отправку данных вместе с ACK пакетом
#define EN_DYN_ACK 		0 // Включить возможность использовать команду W_TX_PAYLOAD_NOACK


//Описание команд
#define R_REGISTER                      0x00 //читаем регистр
#define W_REGISTER                      0x20 //пишем в регистр
#define R_RX_PAYLOAD                    0x61 //считывание из буфера принятых данных из космоса
#define W_TX_PAYLOAD                    0xA0 //запись данных в буфер для отправки в космос
#define FLUSH_TX                        0xE1 //очистка буфера отправки
#define FLUSH_RX                        0xE2 //очистка буфера приема
#define REUSE_TX_PL                     0xE3 //повторная пересылка пакетов пока CE высокий 
#define ACTIVATE                        0x50 //команда за которой надо отправить байт 0x73, тогда станут доступны команды 
										 	 // R_RX_PL_WID, W_ACK_PAYLOAD, W_TX_PAYLOAD_NOACK, повторная отправка вновь блокирует эти команды    
#define R_RX_PL_WID                     0x60 // считать размер в байтах принятого пакета находящегося сверху RX FIFO
#define W_ACK_PAYLOAD                   0xA8 // записать данные для передачи вместе с пакетом ACK в ответ на прием данных (последние 3 бита это номер PIPE)
#define W_TX_PAYLOAD_NOACK              0x58 // отключает AUTOACK для конктерного входящего пакета
#define NOP                             0xFF //команда заглушка, ничего не делает.

extern u08 ReadReg(u08 addr);
extern void ReadData(u08 *data, u08 size);
extern void WriteReg(u08 addr,u08 byte);
extern void WriteData(u08 *data, u08 size);
extern void RXmod(void);
extern void TXmod(void);
extern void nRF_init(void);	
extern void nRF_send_data(u08 *data, u08 size);
extern BOOL checkStatus(void);
extern u08 nRF_get_byte(void);

#endif