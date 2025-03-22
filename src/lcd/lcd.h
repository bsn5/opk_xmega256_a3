/*
 * lcd.h
 *
 * Created: 24.12.2011 10:15:07
 *  Author: vshivtsev_d
 */ 

/************************************************************************/
/*				��������!!!												*/
/*				����� ������ opk2v0										*/
/************************************************************************/

#ifndef LCD_H_
#define LCD_H_

#include <ioport.h>
#include <stdio.h>

// ����� �������
#define		LCD_PORT_DATA		PORTA
#define		LCD_PORT_DRIVE		PORTB

// ���������� ����������
#define		LED					(1<<0)
// ������������ ������ ��� ���������� ��������
#define		EN					(1<<4)
/* ������ ������ ������� ������ ����������� �� �������
 * 0 - ������ ������, 1 - ������ ������
 */
#define		RS					(1<<6)

/* ����� �����������:
 * 0 - Write (������)
 * 1 - Read (������)
 */
#define		RW					(1<<5)
// ����� ���������� ����������
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

// ������ ��������������� �����
#define		LCD_PUT(Data)		LCD_PORT_DATA.OUT |= Data;

/************************************************************************/
/*				��������� ������� ������ � ��������                     */
/************************************************************************/

/*
 * ������� �������
 */
void LCD_Clear (void);

/*
 * ������������� ������� (������ ��������� ��������, 
 * ��������� �����������, ������� � �.�.)
 */
void LCD_Init (void);

uint8_t LCD_Ready (void);

/*
 * ������ ����� �� ���������� ������ �� �������
 * \ Dest - ����� ������� ������ (0 - ������ ������, 1 - ������ ������)
 * \ Data - �������������� ���� *
 */
void LCD_Write (uint8_t Dest, uint8_t Data);

uint8_t LCD_Read (uint8_t Dest);

/*
 * ����� ������� �� �������
 * \ Str - ����� ������ (0..3)
 * \ Col - ����� ������� (0..19)
 * \ Sym - ��� ������� �� ������� ��������������� �������
 */
void LCD_PutSym (uint8_t Str, uint8_t Col, uint8_t Sym);

/*
 * ����� ������ �� �������
 * \ Str - ����� ������ (0..3)
 * \ Col - ����� ������� (0..19)
 * \ *Mes - ��������� ��������� ������
 */
void LCD_PutStr (uint8_t Str, uint8_t Col, uint8_t *Mes);

void LCD_PutStrNum (uint8_t Str, uint8_t Col, uint8_t Num,  uint8_t *Mes);

/*
 * ����� ����� �� �������
 * \ Str - ����� ������ (0..3)
 * \ Col - ����� ������� (0..19)
 * \ Len - ���������� ���������� �������� ����� (�������������� ��������� ������)
 * \ Val - ��������� �����
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