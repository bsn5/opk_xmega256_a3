/*
 * lcd.h
 *
 * Created: 24.12.2011 10:15:07
 *  Author: vshivtsev_d
 */ 

/************************************************************************/
/*				ВНИМАНИЕ!!!												*/
/*				НОВАЯ ВЕРСИЯ opk2v0										*/
/************************************************************************/

#ifndef LCD_H_
#define LCD_H_

#include <ioport.h>
#include <stdio.h>

// Порты дисплея
#define		LCD_PORT_DATA		PORTA
#define		LCD_PORT_DRIVE		PORTB

// Управление подсветкой
#define		LED					(1<<0)
// Стробирующий сигнал для управления дисплеем
#define		EN					(1<<4)
/* Сигнал выбора области памяти внутреннего МК дисплея
 * 0 - память команд, 1 - память данных
 */
#define		RS					(1<<6)

/* Выбор направления:
 * 0 - Write (Запись)
 * 1 - Read (Чтение)
 */
#define		RW					(1<<5)
// Вывод управления контрастом
#define		CNTR				(1<<3)

#define		RW_SET				LCD_PORT_DRIVE.OUT |= RW
#define		RW_CLR				LCD_PORT_DRIVE.OUT &= ~ RW
#define		RS_SET				LCD_PORT_DRIVE.OUT |= RS
#define		RS_CLR				LCD_PORT_DRIVE.OUT &= ~ RS
#define		EN_SET				LCD_PORT_DRIVE.OUT |= EN
#define		EN_CLR				LCD_PORT_DRIVE.OUT &= ~ EN
#define		LED_SET				LCD_PORT_DRIVE.OUT |= LED
#define		LED_CLR				LCD_PORT_DRIVE.OUT &= ~ LED

#define		LCD_PORT_INIT		LCD_PORT_DATA.DIR = 0xFF;								\
								LCD_PORT_DATA.OUT = 0xFF;								\
								LCD_PORT_DRIVE.DIR |= LED | CNTR | EN | RW | RS;		\
								LCD_PORT_DRIVE.OUT &= ~ (LED | CNTR | EN | RW | RS);
								
#define		LCD_PORT_DATA_OUT	LCD_PORT_DATA.DIR = 0xFF;
#define		LCD_PORT_DATA_IN	LCD_PORT_DATA.DIR = 0x00;

// Запись информационного байта
#define		LCD_PUT(Data)		LCD_PORT_DATA.OUT |= Data;

/************************************************************************/
/*				Прототипы функций работы с дисплеем                     */
/************************************************************************/

/*
 * Очистка дисплея
 */
void LCD_Clear (void);

/*
 * Инициализация дисплея (запись начальных значений, 
 * включение отображения, очистка и т.д.)
 */
void LCD_Init (void);

uint8_t LCD_Ready (void);

/*
 * Запись байта во внутреннюю память МК дисплея
 * \ Dest - выбор области памяти (0 - память команд, 1 - память данных)
 * \ Data - информационный байт *
 */
void LCD_Write (uint8_t Dest, uint8_t Data);

uint8_t LCD_Read (uint8_t Dest);

/*
 * Вывод символа на дисплей
 * \ Str - номер строки (0..3)
 * \ Col - номер столбца (0..19)
 * \ Sym - код символа из таблицы знакогенератора дисплея
 */
void LCD_PutSym (uint8_t Str, uint8_t Col, uint8_t Sym);

/*
 * Вывод строки на дисплей
 * \ Str - номер строки (0..3)
 * \ Col - номер столбца (0..19)
 * \ *Mes - выводимая текстовая строка
 */
void LCD_PutStr (uint8_t Str, uint8_t Col, uint8_t *Mes);

void LCD_PutStrNum (uint8_t Str, uint8_t Col, uint8_t Num,  uint8_t *Mes);

/*
 * Вывод числа на дисплей
 * \ Str - номер строки (0..3)
 * \ Col - номер столбца (0..19)
 * \ Len - количество десятичных разрядов числа (неиспользуемые выводятся нулями)
 * \ Val - выводимое число
 */
void LCD_PutValDec (uint8_t Str, uint8_t Col, uint8_t Len, uint64_t Val);
void LCD_PutValDecPoint (uint8_t Str, uint8_t Col, uint8_t Len, uint8_t Point, uint64_t Val);
void LCD_PutValDecPointMaskNeg (uint8_t Str, uint8_t Col, uint8_t Len, uint8_t Point, uint8_t Mask, uint8_t Neg, uint64_t Val);

void LCD_PutValHex (uint8_t Str, uint8_t Col, uint8_t Len, uint64_t Val);
void LCD_PutValHexPoint (uint8_t Str, uint8_t Col, uint8_t Len, uint8_t Point, uint64_t Val);

void LCD_PutValBin (uint8_t Str, uint8_t Col, uint8_t Len, uint64_t Val);
void LCD_PutValBinPoint (uint8_t Str, uint8_t Col, uint8_t Len, uint8_t Point, uint64_t Val);

void LCD_PutValDecPointMaskNeg_ (uint8_t Str, uint8_t Col, uint8_t Len, uint8_t Point, uint8_t Masked, int64_t Val);

void LCD_UserDefinedSymbols (void);

#endif /* LCD_H_ */