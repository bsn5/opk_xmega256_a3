/**
 *	\file mb_config.h
 *
 *	\brief 		Прототипы функции и переменные для работы с MODBUS
 *	\date: 	23.03.2016 15:58:14
	\version	1.0
 *	\author: 	ТолмачевАА
 */ 

 /**
	\addtogroup 	MODBUS
*/
#ifndef MB_CONFIG_H_INCLUDED
#define MB_CONFIG_H_INCLUDED

//#include "modbus.h"
//#include "mb_def_plc.h"
//#include "queue.h"
#include <stdio.h>
#include "status_codes.h"

#define NAME_PULT	(uint8_t)0x04//для команды 17. Биты 0-7 имя прибора
#define VER_PULT	(uint8_t)0x21//						8-15 версия программы
#define NAME_TEST	(uint8_t)0x00//для тестовой версии
#define SPEED_MBUS	(uint8_t)0x01	//адрес регистра для изменения времени ответа по MODBUS


#define ERROR 0x80		//признак ошибки
#define MB0_MAX_REC_SIZE	110
// Адреса устройств на линии  MODBUS
#define MBUS_OPK				0x55

#define TX_EN (1<<5)

//Команды MODBUS
#define MBUS_FUNC_GETREG		0x03
#define MBUS_FUNC_GETREGS		0x04
#define MBUS_FUNC_SETREG		0x06
#define MBUS_FUNC_SETREGS		0x10
#define MBUS_FUNC_GETID			0x11

//Коды ошибок MODBUS
#define ILLEGAL_FUN			0x01	///< 01 ILLEGAL FUNCTION
#define ILLEGAL_ADR			0x02	///< 02 ILLEGAL DATA ADDRESS
#define ILLEGAL_DATA		0x03	///< 03 ILLEGAL DATA VALUE
#define ACKNOWLEDGE			0x05	///< 05 SLAVE BUSY
#define SLAVE_BUSY			0x06	///< 06 SLAVE BUSY
//флаги структуры MODBUS_STATE_t
#define ANSWER				(1<<7)	///< Need answer by MODBUS
#define MBUS_RS485			(1<<6)	///< Answer by RS485
#define MBUS_RS232			(1<<5)	///< Answer by RS232
#define MBUS_RMD			(1<<4)	///< Answer by RMD
#define MBUS_RX_RS485		(1<<3)	///< RX by RS485 
#define MBUS_RX_RS232		(1<<2)	///< RX by RS232
#define MBUS_RX_RMD			(1<<1)	///< RX by RMD
extern uint16_t Time_ans;
volatile uint16_t MBUS_Data;
volatile uint16_t MBUS_Datas[7];
volatile uint8_t MBUS_Buf[MB0_MAX_REC_SIZE];
volatile uint16_t MBUS_Register[4][50];	///< Общий массив для хранения принятой информации
										///< Максимальный размер массива - 50 16bit значений
										///< Адрес ячейки для заполнения берётся из сообщения MODBUS
/**
	\brief		Структура с параметрами работы MODBUS
*/
typedef struct {
	uint8_t address;		///< Адрес устройства на шине MODBUS
	uint8_t command;		///< Принятая команда
	uint8_t Sub_AddressH;	///< Адрес регистра  - номер преобразователя
	uint8_t Sub_AddressL;	///< Адрес регистра данных
	uint8_t Quant;			///< Количество данных
	uint8_t flags;			///< Флаг остояния обработки сообщения
	// флаги модуля MODBUS
	//  бит 0 => была пауза 1.5 символа
	//  бит 1 => была пауза 3.5 символа
	//  бит 2 => идёт приём телеграммы
	//	бит 8 => флаг отправленного ответа 1 - ответ отправлен, можно обрабатывать поступившее сообщение

	uint8_t Buf_byte;	///< Байт для передачи или приёма
	uint8_t lenTX;		///< Длина отправляемого сообщения
	uint8_t lenRx;		///< Длина принимаемого сообщения
	uint8_t lenRx485;		///< Длина принимаемого сообщения
	uint8_t lenRx232;		///< Длина принимаемого сообщения
	uint8_t lenRxrmd;		///< Длина принимаемого сообщения
	uint8_t UART_RxBuf[MB0_MAX_REC_SIZE];///< Буфер приемника
	uint8_t UART_TxBuf[MB0_MAX_REC_SIZE];///< Буфер передатчика
	uint8_t TelCompleet;	///< Флаг завершения приема ответной телеграммы
	uint16_t Error_count;	///< Счётчик ошибок нет ответа ADP
	uint16_t Error_CRC;		///< Счётчик ошибок CRC
	uint16_t Error_Data;	///< Счётчик ошибок неверных адресов, либо диапазона данных
	uint16_t Error_MBUS;	///< Счётчик ошибок неготовых ответов MODBUS
	uint16_t CRC16;			///< Код CRC16 в принятом сообщении
	uint16_t CRC16_OK;		///< Код CRC16 рассчитанный при приёме сообщения
	uint8_t Buf_Ful;		///< Признак переполнения приёмного буфера
	uint8_t SubNum;
	uint16_t MBUS_TimeAns;	///< Время между принятой и отправленной посылками по MODBUS
	uint16_t MBUS_TimeAnsMAX;	///< Максимальное время между принятой и отправленной посылками по MODBUS
} MODBUS_STATE_t;

MODBUS_STATE_t MB0_State;

void MBUS_TransStart (void);
void MBUS_TransContinue (void);
void MBUS_Error_Trans(uint8_t Error);
void MBUS_Send_03 (void);
void MBUS_Send_04 (void);
void MBUS_Send_06 (void);
void MBUS_Send_10 (void);
void MBUS_Send_17 (void);
void MBUS_Init_Reg(void);
void MBUS_Send_Speed (void);

extern uint16_t Crc16 (uint8_t *PcBLOCK_, uint16_t Len);

#endif // MB_CONFIG_H_INCLUDED
