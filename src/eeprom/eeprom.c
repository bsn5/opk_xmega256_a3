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
/*					������ � ������� ����������� EEPROM					*/
/************************************************************************/

/************************************************************************/
/*					��������� ������������ ����							*/
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
/*                      ������ ����� ������ � EEPROM                    */
/************************************************************************/
uint8_t EEPROM_WriteBlock (uint16_t Adr, uint8_t * Data, uint8_t BlockSize) {
	uint8_t DataId = 0;
//	uint8_t BlockSize = sizeof (Data) / sizeof (uint8_t);
		
	E_TwiEn (); // ��������� ������ TWI	
	E_TwiBusSetIdle (); // ������� ���� � ��������� IDLE

	if (E_TwiBusWaitForIdle ()) {	// �������� ��������� ���� IDLE
		E_TwiTransactionStart (EEPROM_BuildControlCode (Adr, 0)); // �������� ������������ ���� � ������ ������
	} else {
		E_TwiCmdStop (); // �������� ������� STOP
		E_TwiDisable (); // ���������� ������ TWI
		return 0; // ������� ���������� 0, ���� �������� �������� �������� �����
	}

	if (E_TwiInterruptWaitForWif ()) { // �������� ����� ������ ����� WIF
		if (E_TwiCheckAck ()) {	// ��������, ��� �� ����� ��� ������� ACK
			E_TwiDataPut ((Adr & 0xFF00)>>8); // ������� ���� ������
		} else {
			// ���� ������� ����� NACK
			E_TwiCmdStop (); // �������� ������� STOP
			E_TwiDisable (); // ���������� ������ TWI
			// LCD_PutSym (0, 3, '*');
			return 0; // ������� ���������� 0, ���� �������� �������� �������� �����
		}
	} else {
		E_TwiCmdStop (); // �������� ������� STOP
		E_TwiDisable (); // ���������� ������ TWI
		return 0; // ������� ���������� 0, ���� �������� �������� �������� �����
	}
	
	if (E_TwiInterruptWaitForWif ()) { // �������� ����� ������ ����� WIF
		if (E_TwiCheckAck ()) {	// ��������, ��� �� ����� ��� ������� ACK
			E_TwiDataPut (Adr & 0x00FF); // ������� ���� ������
		} else {
			// ���� ������� ����� NACK
			E_TwiCmdStop (); // �������� ������� STOP
			E_TwiDisable (); // ���������� ������ TWI
			// LCD_PutSym (0, 3, '*');
			return 0; // ������� ���������� 0, ���� �������� �������� �������� �����
		}
	} else {
		E_TwiCmdStop (); // �������� ������� STOP
		E_TwiDisable (); // ���������� ������ TWI
		return 0; // ������� ���������� 0, ���� �������� �������� �������� �����
	}

	for (DataId = 0; DataId <= (BlockSize - 1); DataId++) {
		if (E_TwiInterruptWaitForWif ()) { // �������� ����� ������ ����� WIF
			if (E_TwiCheckAck ()) {	// ��������, ��� �� ����� ��� ������� ACK
				E_TwiDataPut (*Data); // �������� �������������� ����
				Data++;
			} else {
				E_TwiCmdStop (); // �������� ������� STOP
				E_TwiDisable (); // ���������� ������ TWI
				// LCD_PutSym (0, 3, '*');
				return 0; // ������� ���������� 0, ���� �������� �������� �������� �����
			}
		} else {
			E_TwiCmdStop (); // �������� ������� STOP
			E_TwiDisable (); // ���������� ������ TWI
			return 0; // ������� ���������� 0, ���� �������� �������� �������� �����
		}
	}
	
	if (E_TwiInterruptWaitForWif ()) { // �������� ����� ������ ����� WIF
		if (E_TwiCheckAck ()) {	// ��������, ��� �� ����� ��� ������� ACK
			E_TwiCmdStop (); // �������� ������� STOP
		} else {
			// ���� ������� ����� NACK
			E_TwiCmdStop (); // �������� ������� STOP
			E_TwiDisable (); // ���������� ������ TWI
			// LCD_PutSym (0, 3, '*');
			return 0; // ������� ���������� 0, ���� �������� �������� �������� �����
		}
	} else {
		E_TwiCmdStop (); // �������� ������� STOP
		E_TwiDisable (); // ���������� ������ TWI
		return 0; // ������� ���������� 0, ���� �������� �������� �������� �����
	}

	//E_TwiCmdStop (); // �������� ������� STOP
	E_TwiDisable (); // ���������� ������ TWI
	return 1; // ������� ���������� 1, ���� ������ ���� ��������� ������� 
}
 
/************************************************************************/
/*                   ������ ����� ������ �� EEPROM                      */
/************************************************************************/
uint8_t EEPROM_ReadBlock (uint16_t Adr, uint8_t * Data, uint8_t BlockSize) {
	uint8_t DataId = 0;
//	uint8_t BlockSize = sizeof (Data) / sizeof (uint8_t);
	
	E_TwiEn (); // ��������� ������ TWI
	E_TwiBusSetIdle (); // ������� ���� � ��������� IDLE

	if (E_TwiBusWaitForIdle ()) {	// �������� ��������� ���� IDLE
		E_TwiTransactionStart (EEPROM_BuildControlCode (Adr, 0)); // �������� ������������ ���� � ������ ������
	} else {
		E_TwiCmdStop (); // �������� ������� STOP
		E_TwiDisable (); // ���������� ������ TWI
		return 0; // ������� ���������� 0, ���� �������� �������� �������� �����
	}

	if (E_TwiInterruptWaitForWif ()) { // �������� ����� ������ ����� WIF
		if (E_TwiCheckAck ()) {	// ��������, ��� �� ����� ��� ������� ACK
			E_TwiDataPut ((Adr & 0xFF00) >>8); // ������� ���� ������
		}
	} else {
		E_TwiCmdStop (); // �������� ������� STOP
		E_TwiDisable (); // ���������� ������ TWI
		return 0; // ������� ���������� 0, ���� �������� �������� �������� �����
	}
	
	if (E_TwiInterruptWaitForWif ()) { // �������� ����� ������ ����� WIF
		if (E_TwiCheckAck ()) {	// ��������, ��� �� ����� ��� ������� ACK
			E_TwiDataPut (Adr & 0x00FF); // ������� ���� ������
		}
	} else {
		E_TwiCmdStop (); // �������� ������� STOP
		E_TwiDisable (); // ���������� ������ TWI
		return 0; // ������� ���������� 0, ���� �������� �������� �������� �����
	}

	if (E_TwiInterruptWaitForWif ()) { // �������� ����� ������ ����� WIF
		if (E_TwiCheckAck ()) {	// ��������, ��� �� ����� ��� ������� ACK
			E_TwiTransactionStart (EEPROM_BuildControlCode (Adr, 1)); // ��������� ����� � ������ ������
		}
	} else {
		E_TwiCmdStop (); // �������� ������� STOP
		E_TwiDisable (); // ���������� ������ TWI
		return 0; // ������� ���������� 0, ���� �������� �������� �������� �����
	}

	for (DataId = 0; DataId <= (BlockSize - 1); DataId++) {
		if (E_TwiInterruptWaitForRif ()) { // �������� ����� ������ ����� RIF
			*Data = E_TwiDataGet (); // ������ ����� �� EEPROM
			Data++;
			if (DataId == (BlockSize - 1)) {
				E_TwiCmdSendNack (); // �������� ���� ������������� NACK (����� ���������� �����)
				E_TwiCmdStop (); // �������� ������� STOP
			} else {
				E_TwiCmdSendAck (); // �������� ���� ������������� ACK
			}
		} else {
			E_TwiCmdStop (); // �������� ������� STOP
			E_TwiDisable (); // ���������� ������ TWI
			return 0; // ������� ���������� 0, ���� �������� �������� �������� �����
		}
	}
	//E_TwiCmdStop (); // �������� ������� STOP	
	E_TwiDisable (); // ���������� ������ TWI	
	return 1; // ������� ���������� 1, ���� ������ ���� ��������� ������� 
}