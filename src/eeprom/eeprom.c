/*
 * eeprom.c
 *
 * Created: 01.04.2013 18:48:05
 *  Author: vshivtsev_d
 */ 

#include <ioport.h>
#include <stdio.h>
#include <twi/twi.h>
#include <eeprom/eeprom.h>
#define WP (1<<4)
#define WP_OFF 	PORTE.DIR |= WP;\
				PORTE.OUT &= ~WP;
#define WP_ON 	PORTE.OUT |= WP

/************************************************************************/
/*					Работа с внешней микросхемой EEPROM					*/
/************************************************************************/

/************************************************************************/
/*					Генерация контрольного кода							*/
/************************************************************************/
uint8_t EEPROM_BuildControlCode (uint16_t Adr, uint8_t Direction) {
	// ControlCode = 1010 + B0 + A1 + A0 + R\W
	//uint8_t ControlCode = 0b10100110;
	uint8_t ControlCode = 0b10101110;
	//if (Adr & 0x8000) {
		//ControlCode |= (1<<3);
	//} else {
		//ControlCode &= ~(1<<3);
	//}
	if (Direction) {
		ControlCode |= (1<<0);
	} else {
		ControlCode &= ~(1<<0);	
	}
	return ControlCode;
}

/************************************************************************/
/*                      Запись блока данных в EEPROM                    */
/************************************************************************/
uint8_t EEPROM_WriteBlock (uint16_t Adr, uint8_t * Data, uint8_t BlockSize) {
	uint8_t DataId = 0;
//	uint8_t BlockSize = sizeof (Data) / sizeof (uint8_t);
		
	E_TwiEn (); // Включение модуля TWI	
	E_TwiBusSetIdle (); // Перевод шины в состояние IDLE

	if (E_TwiBusWaitForIdle ()) {	// Ожидание состояния шины IDLE
		E_TwiTransactionStart (EEPROM_BuildControlCode (Adr, 0)); // Отправка контрольного кода с флагом записи
	} else {
		E_TwiCmdStop (); // Отправка условия STOP
		E_TwiDisable (); // Выключение модуля TWI
		return 0; // Функция возвращает 0, если превышен интервал ожидания флага
	}

	if (E_TwiInterruptWaitForWif ()) { // Ожидание флага записи байта WIF
		if (E_TwiCheckAck ()) {	// Проверка, что до этого был получен ACK
			E_TwiDataPut ((Adr & 0xFF00)>>8); // Старший байт адреса
		} else {
			// Если получен ответ NACK
			E_TwiCmdStop (); // Отправка условия STOP
			E_TwiDisable (); // Выключение модуля TWI
			// LCD_PutSym (0, 3, '*');
			return 0; // Функция возвращает 0, если превышен интервал ожидания флага
		}
	} else {
		E_TwiCmdStop (); // Отправка условия STOP
		E_TwiDisable (); // Выключение модуля TWI
		return 0; // Функция возвращает 0, если превышен интервал ожидания флага
	}
	
	if (E_TwiInterruptWaitForWif ()) { // Ожидание флага записи байта WIF
		if (E_TwiCheckAck ()) {	// Проверка, что до этого был получен ACK
			E_TwiDataPut (Adr & 0x00FF); // Младший байт адреса
		} else {
			// Если получен ответ NACK
			E_TwiCmdStop (); // Отправка условия STOP
			E_TwiDisable (); // Выключение модуля TWI
			// LCD_PutSym (0, 3, '*');
			return 0; // Функция возвращает 0, если превышен интервал ожидания флага
		}
	} else {
		E_TwiCmdStop (); // Отправка условия STOP
		E_TwiDisable (); // Выключение модуля TWI
		return 0; // Функция возвращает 0, если превышен интервал ожидания флага
	}

	for (DataId = 0; DataId <= (BlockSize - 1); DataId++) {
		if (E_TwiInterruptWaitForWif ()) { // Ожидание флага записи байта WIF
			if (E_TwiCheckAck ()) {	// Проверка, что до этого был получен ACK
				E_TwiDataPut (*Data); // Передача информационных байт
				Data++;
			} else {
				E_TwiCmdStop (); // Отправка условия STOP
				E_TwiDisable (); // Выключение модуля TWI
				// LCD_PutSym (0, 3, '*');
				return 0; // Функция возвращает 0, если превышен интервал ожидания флага
			}
		} else {
			E_TwiCmdStop (); // Отправка условия STOP
			E_TwiDisable (); // Выключение модуля TWI
			return 0; // Функция возвращает 0, если превышен интервал ожидания флага
		}
	}
	
	if (E_TwiInterruptWaitForWif ()) { // Ожидание флага записи байта WIF
		if (E_TwiCheckAck ()) {	// Проверка, что до этого был получен ACK
			E_TwiCmdStop (); // Отправка условия STOP
		} else {
			// Если получен ответ NACK
			E_TwiCmdStop (); // Отправка условия STOP
			E_TwiDisable (); // Выключение модуля TWI
			// LCD_PutSym (0, 3, '*');
			return 0; // Функция возвращает 0, если превышен интервал ожидания флага
		}
	} else {
		E_TwiCmdStop (); // Отправка условия STOP
		E_TwiDisable (); // Выключение модуля TWI
		return 0; // Функция возвращает 0, если превышен интервал ожидания флага
	}

	//E_TwiCmdStop (); // Отправка условия STOP
	E_TwiDisable (); // Выключение модуля TWI
	return 1; // Функция возвращает 1, если запись была завершена успешно 
}
 
/************************************************************************/
/*                   Чтение блока данных из EEPROM                      */
/************************************************************************/
uint8_t EEPROM_ReadBlock (uint16_t Adr, uint8_t * Data, uint8_t BlockSize) {
	uint8_t DataId = 0;
//	uint8_t BlockSize = sizeof (Data) / sizeof (uint8_t);
	
	E_TwiEn (); // Включение модуля TWI
	E_TwiBusSetIdle (); // Перевод шины в состояние IDLE

	if (E_TwiBusWaitForIdle ()) {	// Ожидание состояния шины IDLE
		E_TwiTransactionStart (EEPROM_BuildControlCode (Adr, 0)); // Отправка контрольного кода с флагом записи
	} else {
		E_TwiCmdStop (); // Отправка условия STOP
		E_TwiDisable (); // Выключение модуля TWI
		return 0; // Функция возвращает 0, если превышен интервал ожидания флага
	}

	if (E_TwiInterruptWaitForWif ()) { // Ожидание флага записи байта WIF
		if (E_TwiCheckAck ()) {	// Проверка, что до этого был получен ACK
			E_TwiDataPut ((Adr & 0xFF00) >>8); // Старший байт адреса
		}
	} else {
		E_TwiCmdStop (); // Отправка условия STOP
		E_TwiDisable (); // Выключение модуля TWI
		return 0; // Функция возвращает 0, если превышен интервал ожидания флага
	}
	
	if (E_TwiInterruptWaitForWif ()) { // Ожидание флага записи байта WIF
		if (E_TwiCheckAck ()) {	// Проверка, что до этого был получен ACK
			E_TwiDataPut (Adr & 0x00FF); // Младший байт адреса
		}
	} else {
		E_TwiCmdStop (); // Отправка условия STOP
		E_TwiDisable (); // Выключение модуля TWI
		return 0; // Функция возвращает 0, если превышен интервал ожидания флага
	}

	if (E_TwiInterruptWaitForWif ()) { // Ожидание флага записи байта WIF
		if (E_TwiCheckAck ()) {	// Проверка, что до этого был получен ACK
			E_TwiTransactionStart (EEPROM_BuildControlCode (Adr, 1)); // Повторный старт с флагом чтения
		}
	} else {
		E_TwiCmdStop (); // Отправка условия STOP
		E_TwiDisable (); // Выключение модуля TWI
		return 0; // Функция возвращает 0, если превышен интервал ожидания флага
	}

	for (DataId = 0; DataId <= (BlockSize - 1); DataId++) {
		if (E_TwiInterruptWaitForRif ()) { // Ожидание флага чтения байта RIF
			*Data = E_TwiDataGet (); // Чтение байта из EEPROM
			Data++;
			if (DataId == (BlockSize - 1)) {
				E_TwiCmdSendNack (); // Отправка бита подтверждения NACK (после последнего байта)
				E_TwiCmdStop (); // Отправка условия STOP
			} else {
				E_TwiCmdSendAck (); // Отправка бита подтверждения ACK
			}
		} else {
			E_TwiCmdStop (); // Отправка условия STOP
			E_TwiDisable (); // Выключение модуля TWI
			return 0; // Функция возвращает 0, если превышен интервал ожидания флага
		}
	}
	//E_TwiCmdStop (); // Отправка условия STOP	
	E_TwiDisable (); // Выключение модуля TWI	
	return 1; // Функция возвращает 1, если чтение было завершено успешно 
}