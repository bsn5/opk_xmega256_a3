/*
 * lcd.c
 *
 * Created: 24.12.2011 10:18:34
 *  Author: vshivtsev_d
 */ 
#define		F_CPU					18432000

#include <ioport.h>
#include <stdio.h>
#include <util/delay.h>
#include <lcd/lcd.h>

// Макрос "Очистка содержимого дисплея"
#define		LCD_CLEAR				LCD_Write (0, 0x01)
#define		LCD_RESET				LCD_Write (0, 0x02)

/************************************************************************/
/* Функции управления дисплеем                                          */
/************************************************************************/

/*
 * Очистка дисплея
 */
void LCD_Clear (void) {
	LCD_RESET;
	LCD_CLEAR;
}

/*
 * Инициализация дисплея (запись начальных значений, 
 * включение отображения, очистка и т.д.)
 */
void LCD_Init (void) {
	LCD_PORT_INIT; // Инициализация портов дисплея
	LCD_Write (0, 0b00111000);
	LCD_Write (0, 0b00001100);
	LCD_Write (0, 0x00000110);
	LCD_Clear ();
	LED_SET; // Включение подсветки дисплея
}

const uint8_t StrNum[4] = {0x00, 0x40, 0x14, 0x54};

/*	
switch (Str) {
	case 0:
		Str = 0x00;
		break;
	case 1:
		Str = 0x40;
		break; 
	case 2:
		Str = 0x14;
		break;
	case 3:
		Str = 0x54;
		break;
	default:
		break;
}
*/	

uint8_t LCD_Ready (void) {
	uint8_t CyckleCnt = 100;
	LCD_PORT_DATA_IN;
	do {
		if (-- CyckleCnt) {
			RS_CLR;
			RW_SET;
			EN_SET;
			_delay_us (1);
			EN_CLR;
		} else {
			return 0;
		}
	} while (LCD_PORT_DATA.IN & 0b10000000);
	return 1;
}

/*
 * Запись байта во внутреннюю память МК дисплея
 * \ Dest - выбор области памяти (0 - память команд, 1 - память данных)
 * \ Data - информационный байт *
 */
void LCD_Write (uint8_t Dest, uint8_t Data) {
	if ( LCD_Ready () ) {
		switch (Dest) {
			case 0:
				RS_CLR;
				break;
			case 1:
				RS_SET;
				break;
			default:
				break;
		}
		RW_CLR;
		LCD_PORT_DATA_OUT;
		LCD_PORT_DATA.OUT = 0;
		LCD_PORT_DATA.OUT = Data;
		EN_SET;	
		_delay_us (1);
		EN_CLR;	
		LCD_PORT_DATA_IN;
	}

}

uint8_t LCD_Read (uint8_t Dest) {
	if ( LCD_Ready () ) {
		switch (Dest) {
			case 0:
				RS_CLR;
				break;
			case 1:
				RS_SET;
				break;
			default:
				break;
		}
		LCD_PORT_DATA_IN;
		RW_SET;
		EN_SET;	
		_delay_us(1);
		return LCD_PORT_DATA.IN;
		EN_CLR;
		LCD_PORT_DATA_OUT;
	}
}

/*
 * Вывод символа на дисплей
 * \ Str - номер строки (0..3)
 * \ Col - номер столбца (0..19)
 * \ Sym - код символа из таблицы знакогенератора дисплея
 */
void LCD_PutSym (uint8_t Str, uint8_t Col, uint8_t Sym) {
	Str = StrNum[Str];
	LCD_Write (0, 0x80 + Str + Col);
	LCD_Write (1, Sym);
}

/*
 * Вывод числа на дисплей
 * \ Str - номер строки (0..3)
 * \ Col - номер столбца (0..19)
 * \ Len - количество десятичных разрядов числа (неиспользуемые выводятся нулями)
 * \ Val - выводимое число
 */
void LCD_PutValDec (uint8_t Str, uint8_t Col, uint8_t Len, uint64_t Val) {
	Str = StrNum[Str];
	for (uint8_t i = 1; i <= Len; ++ i) {
		LCD_Write (0, 0x80 + Str + Col + (Len - i));
		LCD_Write (1, 0x30 + Val%10);
		Val /= 10;
	}
}

void LCD_PutValDecPoint (uint8_t Str, uint8_t Col, uint8_t Len, uint8_t Point, uint64_t Val) {
	Str = StrNum[Str];
	for (uint8_t i = 1; i <= Len; ++ i) {
		if ((Point) && (i == ( /*Len - */ Point + 1 ))) {
			LCD_Write (0, 0x80 + Str + Col + (Len - i));
			LCD_Write (1, 0x2E);
		} else {
			LCD_Write (0, 0x80 + Str + Col + (Len - i));
			LCD_Write (1, 0x30 + Val%10);
			Val /= 10;
		}
	}
}

void LCD_PutValDecPointMaskNeg (uint8_t Str, uint8_t Col, uint8_t Len, uint8_t Point, uint8_t Mask, uint8_t Neg, uint64_t Val) {
	Str = StrNum[Str];
	for (uint8_t i = 1; i <= Len; ++ i) {
		if ( (Point) && (i == (Point + 1 )) ) {
			LCD_Write (0, 0x80 + Str + Col + (Len - i));
			LCD_Write (1, 0x2E);
		} else {
			if ( (Mask) && (i > (Len - Mask)) ) {
				LCD_Write (0, 0x80 + Str + Col + (Len - i));
				LCD_Write (1, 0x20);
			} else {
				LCD_Write (0, 0x80 + Str + Col + (Len - i));
				LCD_Write (1, 0x30 + Val%10);
				Val /= 10;
			}
		}
	}
	if (Neg) {
		if (Mask) {
			LCD_Write (0, 0x80 + Str + Col + (Mask - 1));
			LCD_Write (1, 0x2D);
		} else {
			LCD_Write (0, 0x80 + Str + Col);
			LCD_Write (1, 0x2D);
		}
	}
}

void LCD_PutValDecPointMaskNeg_ (uint8_t Str, uint8_t Col, uint8_t Len, uint8_t Point, uint8_t Masked, int64_t Val) {
	
	uint8_t Neg = 0;
	uint8_t Mask = 0;
	
	if (Val >= 0) {
		Neg = 0;
	} else {
		Val = Abs (Val);
		Neg = 1;
	}
	
	if (Masked) {
		if (Val <= 9) {
			Mask = 1;
		} else if (Val <= 99) {
			Mask = 2;
		} else if (Val <= 999) {
			Mask = 3;
		} else if (Val <= 9999) {
			Mask = 4;
		} else if (Val <= 99999) {
			Mask = 5;
		} else if (Val <= 999999) {
			Mask = 6;
		} else {
			Mask = 7;
		}
		if (Len < Mask) {
			Mask = 0;
		} else {
			Mask = Len - Mask /*- 1*/;
			if (Point && Mask) -- Mask;
		}		
	} else {
		Mask = 0;
	}		
	
	Str = StrNum[Str];
	for (uint8_t i = 1; i <= Len; ++ i) {
		if ( (Point) && (i == (Point + 1 )) ) {
			LCD_Write (0, 0x80 + Str + Col + (Len - i));
			LCD_Write (1, 0x2E);			
		} else {
			if ( (Mask) && (i > (Len - Mask)) ) {
				LCD_Write (0, 0x80 + Str + Col + (Len - i));
				LCD_Write (1, 0x20);
			} else {
				LCD_Write (0, 0x80 + Str + Col + (Len - i));
				LCD_Write (1, 0x30 + Val%10);
				Val /= 10;
			}
		}
	}
	if (Neg) {
		if (Mask) {
			LCD_Write (0, 0x80 + Str + Col + (Mask - 1));
			LCD_Write (1, 0x2D);
		} else {
			LCD_Write (0, 0x80 + Str + Col);
			LCD_Write (1, 0x2D);
		}
	}
}

void LCD_PutValHex (uint8_t Str, uint8_t Col, uint8_t Len, uint64_t Val) {
	Str = StrNum[Str];
	for (uint8_t i = 1; i <= Len; ++ i) {
		LCD_Write (0, 0x80 + Str + Col + (Len - i));
		if ( (Val%16) < 10) {	
			LCD_Write (1, 0x30 + Val%16);
		} else {
			LCD_Write (1, 0x41 + (Val%16 - 10));
		}
		Val /= 16;
	}
}

void LCD_PutValHexPoint (uint8_t Str, uint8_t Col, uint8_t Len, uint8_t Point, uint64_t Val) {
	Str = StrNum[Str];
	for (uint8_t i = 1; i <= Len; ++ i) {
		if ((Point) && (i == ( /*Len - */ Point + 1 ))) {
			LCD_Write (0, 0x80 + Str + Col + (Len - i));
			LCD_Write (1, 0x2E);
		} else {
			LCD_Write (0, 0x80 + Str + Col + (Len - i));
			if ( (Val%16) < 10) {	
				LCD_Write (1, 0x30 + Val%16);
			} else {
				LCD_Write (1, 0x41 + (Val%16 - 10));
			}
		}
		Val /= 16;
	}
}

void LCD_PutValBin (uint8_t Str, uint8_t Col, uint8_t Len, uint64_t Val) {
	Str = StrNum[Str];
	for (uint8_t i = 1; i <= Len; ++ i) {
		LCD_Write (0, 0x80 + Str + Col + (Len - i));
		LCD_Write (1, 0x30 + Val%2);
		Val /= 2;
	}
}

void LCD_PutValBinPoint (uint8_t Str, uint8_t Col, uint8_t Len, uint8_t Point, uint64_t Val) {
	Str = StrNum[Str];
	for (uint8_t i = 1; i <= Len; ++ i) {
		if ((Point) && (i == ( /*Len - */ Point + 1 ))) {
			LCD_Write (0, 0x80 + Str + Col + (Len - i));
			LCD_Write (1, 0x2E);
		} else {
			LCD_Write (0, 0x80 + Str + Col + (Len - i));
			LCD_Write (1, 0x30 + Val%2);
			Val /= 2;
		}
	}
}

/*
 * Вывод строки на дисплей
 * \ Str - номер строки (0..3)
 * \ Col - номер столбца (0..19)
 * \ *Mes - выводимая текстовая строка
 */
void LCD_PutStr (uint8_t Str, uint8_t Col, uint8_t *Mes) {
	Str = StrNum[Str];
	uint8_t i = 0;
	while (*Mes!='\0') {
		LCD_Write (0, 0x80 + Str + Col + i);
		LCD_Write (1, *Mes);
		Mes++;
		i++;
	}
}

void LCD_PutStrNum (uint8_t Str, uint8_t Col, uint8_t Num, uint8_t *Mes) {
	Str = StrNum[Str];
	Num /= (sizeof (*Mes) / sizeof (uint8_t));
	uint8_t i = 0;
	while (Num) {
		LCD_Write (0, 0x80 + Str + Col + i);
		LCD_Write (1, *Mes);
		++ Mes;
		++ i;
		-- Num;
	}
}

void LCD_UserDefinedSymbols (void) {
	LCD_Write (0, 0b01000000);

// Длинная вертикальная черта - 0x00
	LCD_Write (1, 0b00000100);
	LCD_Write (1, 0b00000100);
	LCD_Write (1, 0b00000100);
	LCD_Write (1, 0b00000100);
	LCD_Write (1, 0b00000100);
	LCD_Write (1, 0b00000100);
	LCD_Write (1, 0b00000100);
	LCD_Write (1, 0b00000100);

// Обратный слэш - 0x01
	LCD_Write (1, 0b00000000);
	LCD_Write (1, 0b00010000);
	LCD_Write (1, 0b00001000);
	LCD_Write (1, 0b00000100);
	LCD_Write (1, 0b00000010);
	LCD_Write (1, 0b00000001);
	LCD_Write (1, 0b00000000);
	LCD_Write (1, 0b00000000);

// Стрелка вниз - 0x02
	LCD_Write (1, 0b00000100);
	LCD_Write (1, 0b00000100);
	LCD_Write (1, 0b00000100);
	LCD_Write (1, 0b00000100);
	LCD_Write (1, 0b00000100);
	LCD_Write (1, 0b00010101);
	LCD_Write (1, 0b00001010);
	LCD_Write (1, 0b00000100);
	
// Стрелка вверх - 0x03
	LCD_Write (1, 0b00000100);
	LCD_Write (1, 0b00001010);
	LCD_Write (1, 0b00010101);
	LCD_Write (1, 0b00000100);
	LCD_Write (1, 0b00000100);
	LCD_Write (1, 0b00000100);
	LCD_Write (1, 0b00000100);
	LCD_Write (1, 0b00000100);
	
// Стрелка влево - 0x04
	LCD_Write (1, 0b00000010);
	LCD_Write (1, 0b00000110);
	LCD_Write (1, 0b00001110);
	LCD_Write (1, 0b00011110);
	LCD_Write (1, 0b00001110);
	LCD_Write (1, 0b00000110);
	LCD_Write (1, 0b00000010);
	LCD_Write (1, 0b00000000);
	
// Стрелка вправо - 0x05
	LCD_Write (1, 0b00001000);
	LCD_Write (1, 0b00001100);
	LCD_Write (1, 0b00001110);
	LCD_Write (1, 0b00001111);
	LCD_Write (1, 0b00001110);
	LCD_Write (1, 0b00001100);
	LCD_Write (1, 0b00001000);
	LCD_Write (1, 0b00000000);
	
// Верхнее подчёркивание - 0x06
	LCD_Write (1, 0b00001111);
	LCD_Write (1, 0b00000000);
	LCD_Write (1, 0b00000000);
	LCD_Write (1, 0b00000000);
	LCD_Write (1, 0b00000000);
	LCD_Write (1, 0b00000000);
	LCD_Write (1, 0b00000000);
	LCD_Write (1, 0b00000000);

// Маленький x - 0x07	
	LCD_Write (1, 0b00000000);
	LCD_Write (1, 0b00000000);
	LCD_Write (1, 0b00010001);
	LCD_Write (1, 0b00001010);
	LCD_Write (1, 0b00000100);
	LCD_Write (1, 0b00001010);
	LCD_Write (1, 0b00010001);
	LCD_Write (1, 0b00000000);

}