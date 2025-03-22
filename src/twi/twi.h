/*
 * twi.h
 *
 * Created: 01.04.2013 17:50:08
 *  Author: vshivtsev_d
 */ 


#ifndef TWI_H_
#define TWI_H_

#include "compiler.h"
#include "pmic.h"

#define TWI_BAUD(F_SYS, F_TWI) ((F_SYS / (2 * F_TWI)) - 5) // Расчёт бодрэйта шины TWI
	
#define CYCLE_COUNTER_RTC 1000
#define CYCLE_COUNTER_EEPROM 1000

#define F_SYS 18432000
#define F_TWI_RTC 300000 // Частота работы шины TWI RTC
#define F_TWI_EEPROM 200000 // Частота работы шины TWI EEPROM
	
// Работа с шиной TWI (PORTC)
void		C_TwiInit (void);
uint8_t		C_TwiBusGetStatus (void);
uint8_t		C_TwiBusWaitForIdle (void);
void		C_TwiTransactionStart (uint8_t Direction);
uint8_t		C_TwiInterruptGetRif (void);
uint8_t		C_TwiInterruptWaitForRif (void);
uint8_t		C_TwiInterruptGetWif (void);
uint8_t		C_TwiInterruptWaitForWif (void);
uint8_t		C_TwiCheckAck (void);
void		C_TwiDataPut (uint8_t Data);
uint8_t		C_TwiDataGet (void);
void		C_TwiCmdStop (void);
void		C_TwiCmdRepeatedStart (void);
void		C_TwiCmdSendAck (void);
void		C_TwiCmdSendNack (void);
void		C_TwiEn (void);
void		C_TwiDisable (void);
void		C_TwiBusSetIdle (void);

// Работа с шиной TWI (PORTE)
void		E_TwiInit (void);
uint8_t		E_TwiBusGetStatus (void);
uint8_t		E_TwiBusWaitForIdle (void);
void		E_TwiTransactionStart (uint8_t Direction);
uint8_t		E_TwiInterruptGetRif (void);
uint8_t		E_TwiInterruptWaitForRif (void);
uint8_t		E_TwiInterruptGetWif (void);
uint8_t		E_TwiInterruptWaitForWif (void);
uint8_t		E_TwiCheckAck (void);
void		E_TwiDataPut (uint8_t Data);
uint8_t		E_TwiDataGet (void);
void		E_TwiCmdStop (void);
void		E_TwiCmdRepeatedStart (void);
void		E_TwiCmdSendAck (void);
void		E_TwiCmdSendNack (void);
void		E_TwiEn (void);
void		E_TwiDisable (void);
void		E_TwiBusSetIdle (void);
	

#endif /* TWI_H_ */