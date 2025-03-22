/*
 * eeprom.h
 *
 * Created: 01.04.2013 18:48:18
 *  Author: vshivtsev_d
 */ 


#ifndef EEPROM_H_
#define EEPROM_H_

// Работа с внешней EEPROM
uint8_t EEPROM_WriteBlock (uint16_t Adr, uint8_t * Data, uint8_t BlockSize);
uint8_t EEPROM_ReadBlock (uint16_t Adr, uint8_t * Data, uint8_t BlockSize);

// Direction:
// 0 - запись,
// 1 - чтение
uint8_t EEPROM_BuildControlCode (uint16_t Adr, uint8_t Direction);


#endif /* EEPROM_H_ */