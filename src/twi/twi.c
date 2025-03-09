/*
 * twi.c
 *
 * Created: 01.04.2013 17:52:04
 *  Author: vshivtsev_d
 */ 

// #include <asf.h>
#include <ioport.h>
#include <stdio.h>
#include <twi/twi.h>
	
/************************************************************************/
/*             Работа с шиной TWI (PORTE - EEPROM)                      */
/************************************************************************/

void E_TwiInit (void) {	
	TWIE.MASTER.CTRLA |= TWI_MASTER_INTLVL_OFF_gc;
	//TWIE.MASTER.CTRLB |= TWI_MASTER_TIMEOUT_50US_gc;
	TWIE.MASTER.BAUD = TWI_BAUD(F_SYS, F_TWI_EEPROM);
	//TWIE.MASTER.CTRLA |= TWI_MASTER_ENABLE_bm;
}

uint8_t E_TwiBusGetStatus (void) {
	return (TWIE.MASTER.STATUS & 0x03);
}

uint8_t E_TwiBusWaitForIdle (void) {
	uint16_t CycleCnt = 0;
	do {
		if (CycleCnt < CYCLE_COUNTER_EEPROM) {
			CycleCnt++;
		} else {
			return 0;
			break;
		}		
	} while ((TWIE.MASTER.STATUS & 0x03) != TWI_MASTER_BUSSTATE_IDLE_gc);	
	return 1;
}

void E_TwiTransactionStart (uint8_t ControlCode) {	
	TWIE.MASTER.ADDR = ControlCode;	
}

uint8_t E_TwiInterruptGetRif (void) {	
	return (TWIE.MASTER.STATUS & TWI_MASTER_RIF_bm);
}

uint8_t E_TwiInterruptWaitForRif (void) {	
	uint16_t CycleCnt = 0;
	do {
		if (CycleCnt < CYCLE_COUNTER_EEPROM) {
			CycleCnt++;
		} else {
			return 0;
			break;
		}	
	} while ((TWIE.MASTER.STATUS & TWI_MASTER_RIF_bm) == 0);
	return 1;
}

uint8_t E_TwiInterruptGetWif (void) {	
	return (TWIE.MASTER.STATUS & TWI_MASTER_WIF_bm);
}

uint8_t E_TwiInterruptWaitForWif (void) {	
	uint16_t CycleCnt = 0;
	do {
		if (CycleCnt < CYCLE_COUNTER_EEPROM) {
			CycleCnt++;
		} else {
			return 0;
			break;
		}		
	} while ((TWIE.MASTER.STATUS & TWI_MASTER_WIF_bm) == 0);
	return 1;
}

uint8_t E_TwiCheckAck (void) {	
	if ((TWIE.MASTER.STATUS & TWI_MASTER_RXACK_bm) == 0) {		
		return 1;		
	} else {		
		return 0;
	}
}

void E_TwiDataPut (uint8_t Data) {	
	TWIE.MASTER.DATA = Data;
}

uint8_t E_TwiDataGet (void) {	
	return (TWIE.MASTER.DATA);
}

void E_TwiCmdStop (void) {	
	TWIE.MASTER.CTRLC |= TWI_MASTER_CMD_STOP_gc;
}

void E_TwiCmdRepeatedStart (void) {	
	TWIE.MASTER.CTRLC |= TWI_MASTER_CMD_REPSTART_gc;
}

void E_TwiCmdSendAck (void) {	
	TWIE.MASTER.CTRLB &= ~ TWI_MASTER_ACKACT_bm;
	TWIE.MASTER.CTRLC |= TWI_MASTER_CMD_RECVTRANS_gc;	
}

void E_TwiCmdSendNack (void) {	
	TWIE.MASTER.CTRLB |= TWI_MASTER_ACKACT_bm;
	TWIE.MASTER.CTRLC |= TWI_MASTER_CMD_RECVTRANS_gc;
	TWIE.MASTER.CTRLB &= ~ TWI_MASTER_ACKACT_bm;	
}

void E_TwiBusSetIdle (void) {	
	TWIE.MASTER.STATUS |= TWI_MASTER_BUSSTATE_IDLE_gc;
}

void E_TwiEn (void) {	
	TWIE.MASTER.CTRLA |= TWI_MASTER_ENABLE_bm;
}

void E_TwiDisable (void) {	
	TWIE.MASTER.CTRLA &= ~ TWI_MASTER_ENABLE_bm;
}

/************************************************************************/
/*               Работа с шиной TWI (PORTC - RTC)                       */
/************************************************************************/

void C_TwiInit (void) {	
	TWIC.MASTER.CTRLA |= TWI_MASTER_INTLVL_OFF_gc;
	TWIC.MASTER.CTRLB |= TWI_MASTER_TIMEOUT_50US_gc;
	TWIC.MASTER.BAUD = TWI_BAUD(F_SYS, F_TWI_RTC);	
}

uint8_t C_TwiBusGetStatus (void) {	
	return (TWIC.MASTER.STATUS & 0x03);
}

uint8_t C_TwiBusWaitForIdle (void) {	
	uint16_t CycleCnt = 0;
	do {
		if (CycleCnt < CYCLE_COUNTER_RTC) {
			CycleCnt++;
		} else {
			return 0;
			break;
		}		
	} while ((TWIC.MASTER.STATUS & 0x03) != TWI_MASTER_BUSSTATE_IDLE_gc);	
	return 1;
}

void C_TwiTransactionStart (uint8_t Direction) {	
	if (Direction) {		
		TWIC.MASTER.ADDR = 0xD1;		
	} else {		
		TWIC.MASTER.ADDR = 0xD0;
	}	
}

uint8_t C_TwiInterruptGetRif (void) {	
	return (TWIC.MASTER.STATUS & TWI_MASTER_RIF_bm);
}

uint8_t C_TwiInterruptWaitForRif (void) {	
	uint16_t CycleCnt = 0;
	do {
		if (CycleCnt < CYCLE_COUNTER_RTC) {
			CycleCnt++;
		} else {
			return 0;
			break;
		}	
	} while ((TWIC.MASTER.STATUS & TWI_MASTER_RIF_bm) == 0);
	return 1;
}

uint8_t C_TwiInterruptGetWif (void) {	
	return (TWIC.MASTER.STATUS & TWI_MASTER_WIF_bm);
}

uint8_t C_TwiInterruptWaitForWif (void) {	
	uint16_t CycleCnt = 0;
	do {
		if (CycleCnt < CYCLE_COUNTER_RTC) {
			CycleCnt++;
		} else {
			return 0;
			break;
		}		
	} while ((TWIC.MASTER.STATUS & TWI_MASTER_WIF_bm) == 0);
	return 1;
}

uint8_t C_TwiCheckAck (void) {	
	if ((TWIC.MASTER.STATUS & TWI_MASTER_RXACK_bm) == 0) {		
		return 1;
	} else {
		return 0;
	}
}

void C_TwiDataPut (uint8_t Data) {	
	TWIC.MASTER.DATA = Data;
}

uint8_t C_TwiDataGet (void) {	
	return (TWIC.MASTER.DATA);
}

void C_TwiCmdStop (void) {	
	TWIC.MASTER.CTRLC |= TWI_MASTER_CMD_STOP_gc;
}

void C_TwiCmdRepeatedStart (void) {	
	TWIC.MASTER.CTRLC |= TWI_MASTER_CMD_REPSTART_gc;
}

void C_TwiCmdSendAck (void) {	
	TWIC.MASTER.CTRLB &= ~ TWI_MASTER_ACKACT_bm;
	TWIC.MASTER.CTRLC |= TWI_MASTER_CMD_RECVTRANS_gc;	
}

void C_TwiCmdSendNack (void) {	
	TWIC.MASTER.CTRLB |= TWI_MASTER_ACKACT_bm;
	TWIC.MASTER.CTRLC |= TWI_MASTER_CMD_RECVTRANS_gc;
	TWIC.MASTER.CTRLB &= ~ TWI_MASTER_ACKACT_bm;	
}

void C_TwiBusSetIdle (void) {	
	TWIC.MASTER.STATUS |= TWI_MASTER_BUSSTATE_IDLE_gc;
}

void C_TwiEn (void) {	
	TWIC.MASTER.CTRLA |= TWI_MASTER_ENABLE_bm;
}

void C_TwiDisable (void) {	
	TWIC.MASTER.CTRLA &= ~ TWI_MASTER_ENABLE_bm;
}