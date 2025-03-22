/**
 *	\file MBus.c
 *
 *	\brief 		Функции для работы с MODBUS
 *	\date: 	23.03.2016 15:58:14
	\version	1.0
 *	\author: 	ТолмачевАА
 */ 

#include <modbus/mb_config.h>
#include "usart.h"
#include <lcd/lcd.h>

 /**	
		\defgroup 	MODBUS
		\brief		Функции отправки и сборки сообщений по протоколу MODBUS 
		\{
 */
 
//extern DMA_t DMA_MBUS;
/**
	\brief		Функция передачи сообщения (использует DMA)
	\param
	\retval
*/
void MBUS_TransStart (void) 
{
	//LCD_PutValHex(2,0,2,MB0_State.flags);
	MB0_State.flags &= ~ANSWER;
	//LCD_PutValHex(3,0,2,MB0_State.flags);
	if (MB0_State.MBUS_TimeAns>MB0_State.MBUS_TimeAnsMAX)
	{
		MB0_State.MBUS_TimeAns = MB0_State.MBUS_TimeAnsMAX;
		//LCD_PutValDec(0,0,4,MB0_State.MBUS_TimeAns);
		//LCD_PutValDec(1,0,4,MB0_State.MBUS_TimeAnsMAX);
		MB0_State.MBUS_TimeAns = 0;
	}
	if(MB0_State.flags&MBUS_RS232)
	{
		MB0_State.flags &= ~MBUS_RS232;
		DMA.CH0.SRCADDR0 = ((volatile)MB0_State.UART_TxBuf+1);
		DMA.CH0.SRCADDR1 = ((volatile)(MB0_State.UART_TxBuf+1))>>8;
		DMA.CH0.SRCADDR2 = 0;
		DMA.CH0.REPCNT = 1;
		DMA.CH0.TRFCNT = MB0_State.lenTX-1;
		DMA.CH0.TRIGSRC = DMA_CH_TRIGSRC_USARTF0_DRE_gc;
		usart_putchar (&USARTF0, MB0_State.UART_TxBuf[0]);
		DMA.CH0.CTRLA |= DMA_CH_ENABLE_bm;
		while(DMA.CH0.REPCNT) (void)0;
		DMA.CH0.TRIGSRC = 0;
	}
	else if (MB0_State.flags&MBUS_RS485)
	{
		PORTE.OUT |= TX_EN;
		MB0_State.flags &= ~MBUS_RS485;
		DMA.CH2.SRCADDR0 = ((volatile)MB0_State.UART_TxBuf+1);
		DMA.CH2.SRCADDR1 = ((volatile)(MB0_State.UART_TxBuf+1))>>8;
		DMA.CH2.SRCADDR2 = 0;
	//DMA.CH0.DESTADDR0 = ((volatile)&(USARTC0.DATA));
	//DMA.CH0.DESTADDR1 = ((volatile)&(USARTC0.DATA))>>8;
	//DMA.CH0.DESTADDR2 = 0;
	//DMA.CH0.ADDRCTRL = DMA_CH_SRCDIR_INC_gc | DMA_CH_DESTDIR_FIXED_gc;
		DMA.CH2.REPCNT = 1;
		DMA.CH2.TRFCNT = MB0_State.lenTX-1;
		DMA.CH2.TRIGSRC = DMA_CH_TRIGSRC_USARTE1_DRE_gc;
		usart_putchar (&USARTE1, MB0_State.UART_TxBuf[0]);
		DMA.CH2.CTRLA |= DMA_CH_ENABLE_bm;
		while(DMA.CH2.REPCNT) (void)0;
		DMA.CH2.TRIGSRC = 0;
	}
	else if (MB0_State.flags&MBUS_RMD)
	{
		MB0_State.flags &= ~MBUS_RMD;
		DMA.CH3.SRCADDR0 = ((volatile)MB0_State.UART_TxBuf+1);
		DMA.CH3.SRCADDR1 = ((volatile)(MB0_State.UART_TxBuf+1))>>8;
		DMA.CH3.SRCADDR2 = 0;
	//DMA.CH0.DESTADDR0 = ((volatile)&(USARTC0.DATA));
	//DMA.CH0.DESTADDR1 = ((volatile)&(USARTC0.DATA))>>8;
	//DMA.CH0.DESTADDR2 = 0;
	//DMA.CH0.ADDRCTRL = DMA_CH_SRCDIR_INC_gc | DMA_CH_DESTDIR_FIXED_gc;
		DMA.CH3.REPCNT = 1;
		DMA.CH3.TRFCNT = MB0_State.lenTX-1;
		DMA.CH3.TRIGSRC = DMA_CH_TRIGSRC_USARTC1_DRE_gc;
		usart_putchar (&USARTC1, MB0_State.UART_TxBuf[0]);
		DMA.CH3.CTRLA |= DMA_CH_ENABLE_bm;
		while(DMA.CH3.REPCNT) (void)0;
		DMA.CH3.TRIGSRC = 0;
	}
		MB0_State.lenTX = 0;
	//usart_set_tx_interrupt_level (&USARTC0, USART_TXCINTLVL_MED_gc);

	/*MB0_State.flags = 0x80;
	PORTE.OUT |= TX_EN;
	// отсылка первого байта посылки
	//++ MB0_State.Buf_byte; // инкремент счётчика байт сообщения
	// ѕередача задачи отсылки комманды прерыванию TX
	DMA.CH2.SRCADDR0 = ((volatile)MB0_State.UART_TxBuf+1);
	DMA.CH2.SRCADDR1 = ((volatile)(MB0_State.UART_TxBuf+1))>>8;
	DMA.CH2.SRCADDR2 = 0;
	//DMA.CH0.DESTADDR0 = ((volatile)&(USARTC0.DATA));
	//DMA.CH0.DESTADDR1 = ((volatile)&(USARTC0.DATA))>>8;
	//DMA.CH0.DESTADDR2 = 0;
	//DMA.CH0.ADDRCTRL = DMA_CH_SRCDIR_INC_gc | DMA_CH_DESTDIR_FIXED_gc;
	DMA.CH2.REPCNT = 1;
	DMA.CH2.TRFCNT = MB0_State.lenTX-1;
	DMA.CH2.TRIGSRC = DMA_CH_TRIGSRC_USARTE1_DRE_gc;
	usart_put (&USARTE1, MB0_State.UART_TxBuf[0]);
	DMA.CH2.CTRLA |= DMA_CH_ENABLE_bm;
	while(DMA.CH2.REPCNT) (void)0;
	DMA.CH2.TRIGSRC = 0;*/
	//////////////
	//DMA.CH3.SRCADDR0 = ((volatile)MB0_State.UART_TxBuf+1);
	//DMA.CH3.SRCADDR1 = ((volatile)(MB0_State.UART_TxBuf+1))>>8;
	//DMA.CH3.SRCADDR2 = 0;
	////DMA.CH0.DESTADDR0 = ((volatile)&(USARTC0.DATA));
	////DMA.CH0.DESTADDR1 = ((volatile)&(USARTC0.DATA))>>8;
	////DMA.CH0.DESTADDR2 = 0;
	////DMA.CH0.ADDRCTRL = DMA_CH_SRCDIR_INC_gc | DMA_CH_DESTDIR_FIXED_gc;
	//DMA.CH3.REPCNT = 1;
	//DMA.CH3.TRFCNT = MB0_State.lenTX-1;
	//DMA.CH3.TRIGSRC = DMA_CH_TRIGSRC_USARTC1_DRE_gc;
	//usart_putchar (&USARTC1, MB0_State.UART_TxBuf[0]);
	//DMA.CH3.CTRLA |= DMA_CH_ENABLE_bm;
	//while(DMA.CH3.REPCNT) (void)0;
	//DMA.CH3.TRIGSRC = 0;
	//////////
	//DMA.CH0.SRCADDR0 = ((volatile)MB0_State.UART_TxBuf+1);
	//DMA.CH0.SRCADDR1 = ((volatile)(MB0_State.UART_TxBuf+1))>>8;
	//DMA.CH0.SRCADDR2 = 0;
	////DMA.CH0.DESTADDR0 = ((volatile)&(USARTC0.DATA));
	////DMA.CH0.DESTADDR1 = ((volatile)&(USARTC0.DATA))>>8;
	////DMA.CH0.DESTADDR2 = 0;
	////DMA.CH0.ADDRCTRL = DMA_CH_SRCDIR_INC_gc | DMA_CH_DESTDIR_FIXED_gc;
	//DMA.CH0.REPCNT = 1;
	//DMA.CH0.TRFCNT = MB0_State.lenTX-1;
	//DMA.CH0.TRIGSRC = DMA_CH_TRIGSRC_USARTF0_DRE_gc;
	//usart_putchar (&USARTF0, MB0_State.UART_TxBuf[0]);
	//DMA.CH0.CTRLA |= DMA_CH_ENABLE_bm;
	//while(DMA.CH0.REPCNT) (void)0;
	//DMA.CH0.TRIGSRC = 0;
	//MB0_State.lenTX = 0;
	//usart_set_tx_interrupt_level (&USARTC0, USART_TXCINTLVL_MED_gc);
	/*LCD_PutValHex(1,0,2,MB0_State.UART_TxBuf[0]);
	LCD_PutValHex(1,2,2,MB0_State.UART_TxBuf[1]);
	LCD_PutValHex(1,4,2,MB0_State.UART_TxBuf[2]);
	LCD_PutValHex(1,6,2,MB0_State.UART_TxBuf[3]);
	LCD_PutValHex(1,8,2,MB0_State.UART_TxBuf[4]);*/
}

/**
	\brief		Функция продолжения передачи
	\warning	\b "Функция пустая, так как работает передача по DMA"
	\param
	\retval
*/
void MBUS_TransContinue (void) 
{
	/*if (MB0_State.Buf_byte < (MB0_State.len)) //продолжаем отсылать, пока посылка не кончится
	{			
		usart_put (&USARTC0, MB0_State.UART_TxBuf[MB0_State.Buf_byte]);
		MB0_State.Buf_byte++;
	} 
	else if (MB0_State.Buf_byte >=	MB0_State.len) //передача закончилась, надо обнулить длину
	{				
		usart_set_tx_interrupt_level (&USARTC0, USART_TXCINTLVL_OFF_gc);
		MB0_State.len = 0;
	}	*/
}

/**
	\brief		Функция формирования ответа с ошибкой
	\param		Error:	Код ошибки
	\retval
*/
void MBUS_Error_Trans(uint8_t Error)
{
	MB0_State.UART_TxBuf[0] = MB0_State.address;
	MB0_State.UART_TxBuf[1] = (MB0_State.command|ERROR);//добавляем бит ошибки
	MB0_State.UART_TxBuf[2] = Error;
	MB0_State.lenTX = 3;
	MB0_State.CRC16 = Crc16 ((uint8_t *)MB0_State.UART_TxBuf, MB0_State.lenTX);
	MB0_State.UART_TxBuf[MB0_State.lenTX] = MB0_State.CRC16;
	MB0_State.UART_TxBuf[MB0_State.lenTX+1] = MB0_State.CRC16>>8;	
	MB0_State.lenTX +=2;
	MBUS_TransStart();
}

/**
	\brief		Функция формирования ответа на команду 0х03 - чтение регистра
	\param
	\retval
*/
void MBUS_Send_03 (void)	//нам не нужна
{
	uint8_t i;
	MB0_State.UART_TxBuf[0] = MB0_State.address;
	MB0_State.UART_TxBuf[1] = MB0_State.command;
	MB0_State.UART_TxBuf[2] = 2*MB0_State.Quant;
	MB0_State.lenTX = 3;
	//for(i=0;i<MB0_State.Quant;i++)
	//{
		MB0_State.UART_TxBuf[4] = (MBUS_Register[MB0_State.SubNum][MB0_State.Sub_AddressL]>>8);//в спец регистр и увеличиваем длину
		MB0_State.UART_TxBuf[3] = MBUS_Register[MB0_State.SubNum][MB0_State.Sub_AddressL];
		MB0_State.lenTX+=2;
	//}
	MB0_State.CRC16 = Crc16 ((uint8_t *)MB0_State.UART_TxBuf, MB0_State.lenTX);
	MB0_State.UART_TxBuf[MB0_State.lenTX] = MB0_State.CRC16;
	MB0_State.UART_TxBuf[MB0_State.lenTX+1] = MB0_State.CRC16>>8;
	MB0_State.lenTX +=2;// увеличиваем длину для корректной отправки
	MBUS_TransStart();
}
/**
	\brief		Функция формирования ответа на команду 0х04 - чтение регистров
	\param
	\retval
*/
void MBUS_Send_04 (void)
{
	uint8_t i;
	MB0_State.UART_TxBuf[0] = MB0_State.address;
	MB0_State.UART_TxBuf[1] = MB0_State.command;
	MB0_State.UART_TxBuf[2] = 2*MB0_State.Quant;
	MB0_State.lenTX = 3;
	for(i=0;i<MB0_State.Quant;i++)
	//MB0_State.lenTX = 5;
	//for(i=0;i<MB0_State.Quant-2;i++)
	{
		MB0_State.UART_TxBuf[3+i*2] = (MBUS_Register[MB0_State.SubNum-1][i+MB0_State.Sub_AddressL]>>8);//в спец регистр и увеличиваем длину
		MB0_State.UART_TxBuf[4+i*2] = MBUS_Register[MB0_State.SubNum-1][i+MB0_State.Sub_AddressL];
		MB0_State.lenTX+=2;
	}
	MB0_State.CRC16 = Crc16 ((uint8_t *)MB0_State.UART_TxBuf, MB0_State.lenTX);
	MB0_State.UART_TxBuf[MB0_State.lenTX] = MB0_State.CRC16;
	MB0_State.UART_TxBuf[MB0_State.lenTX+1] = MB0_State.CRC16>>8;
	MB0_State.lenTX +=2;// увеличиваем длину для корректной отправки
	MBUS_TransStart();
}

/**
	\brief		Функция формирования ответа на команду 0х06 - запись регистра
	\param
	\retval
*/
void MBUS_Send_06 (void)
{
	MB0_State.UART_TxBuf[0] = MB0_State.address;
	MB0_State.UART_TxBuf[1] = MB0_State.command;
	MB0_State.UART_TxBuf[2] = MB0_State.Sub_AddressH;
	MB0_State.UART_TxBuf[3] = MB0_State.Sub_AddressL;
	//MB0_State.UART_TxBuf[4] = MBUS_Register[MB0_State.SubNum][MB0_State.Sub_AddressL]>>8;
	//MB0_State.UART_TxBuf[5] = MBUS_Register[MB0_State.SubNum][MB0_State.Sub_AddressL];
	MB0_State.UART_TxBuf[4] = MBUS_Data>>8;
	MB0_State.UART_TxBuf[5] = MBUS_Data;
	MB0_State.lenTX = 6;
	MB0_State.CRC16 = Crc16 ((uint8_t *)MB0_State.UART_TxBuf, MB0_State.lenTX);
	MB0_State.UART_TxBuf[MB0_State.lenTX] = MB0_State.CRC16;
	MB0_State.UART_TxBuf[MB0_State.lenTX+1] = MB0_State.CRC16>>8;
	MB0_State.lenTX +=2;
	MBUS_TransStart();
}
/**
	\brief		Функция формирования ответа на команду 0х10 - запись регистра
	\param
	\retval
*/
void MBUS_Send_10 (void)
{
	MB0_State.UART_TxBuf[0] = MB0_State.address;
	MB0_State.UART_TxBuf[1] = MB0_State.command;
	MB0_State.UART_TxBuf[2] = MB0_State.Sub_AddressH;
	MB0_State.UART_TxBuf[3] = MB0_State.Sub_AddressL;
	MB0_State.UART_TxBuf[4] = 0;
	MB0_State.UART_TxBuf[5] = 1;
	MB0_State.lenTX = 6;
	MB0_State.CRC16 = Crc16 ((uint8_t *)MB0_State.UART_TxBuf, MB0_State.lenTX);
	MB0_State.UART_TxBuf[MB0_State.lenTX] = MB0_State.CRC16;
	MB0_State.UART_TxBuf[MB0_State.lenTX+1] = MB0_State.CRC16>>8;
	MB0_State.lenTX +=2;
	MBUS_TransStart();
}
/**
	\brief		Функция формирования ответа на команду 0х17 - чтение имени прибора и версии программы
	\param
	\retval
*/
void MBUS_Send_17 (void)
{
	MB0_State.UART_TxBuf[0] = MB0_State.address;
	MB0_State.UART_TxBuf[1] = MB0_State.command;
	MB0_State.UART_TxBuf[2] = VER_PULT;
	MB0_State.UART_TxBuf[3] = NAME_PULT;
	MB0_State.lenTX = 4;
	MB0_State.CRC16 = Crc16 ((uint8_t *)MB0_State.UART_TxBuf, MB0_State.lenTX);
	MB0_State.UART_TxBuf[MB0_State.lenTX] = MB0_State.CRC16;
	MB0_State.UART_TxBuf[MB0_State.lenTX+1] = MB0_State.CRC16>>8;
	MB0_State.lenTX +=2;
	MBUS_TransStart();
}
void MBUS_Send_Speed (void)
{
	MB0_State.UART_TxBuf[0] = MB0_State.address;
	MB0_State.UART_TxBuf[1] = MB0_State.command;
	MB0_State.UART_TxBuf[2] = Time_ans>>8;
	MB0_State.UART_TxBuf[3] = Time_ans;
	MB0_State.lenTX = 4;
	MB0_State.CRC16 = Crc16 ((uint8_t *)MB0_State.UART_TxBuf, MB0_State.lenTX);
	MB0_State.UART_TxBuf[MB0_State.lenTX] = MB0_State.CRC16;
	MB0_State.UART_TxBuf[MB0_State.lenTX+1] = MB0_State.CRC16>>8;
	MB0_State.lenTX +=2;
	MBUS_TransStart();
}

void MBUS_Init_Reg(void)
{
	MBUS_Register[0][0] =	0xF100;
	MBUS_Register[0][1] =	1000;
	MBUS_Register[0][2] =	2500;
	MBUS_Register[0][3] =	5000;
	MBUS_Register[0][4] =	7500;
	MBUS_Register[0][5] =	0xF105;
	MBUS_Register[0][6] =	0xF106;
	MBUS_Register[0][7] =	0xF107;
	MBUS_Register[0][8] =	0;
	MBUS_Register[0][9] =	0xF109;
	MBUS_Register[0][10] =	0xF10A;
	MBUS_Register[0][11] =	0xF10B;
	MBUS_Register[0][12] =	0xF10C;
	MBUS_Register[0][13] =	0xF10D;
	MBUS_Register[0][14] =	0xF10E;
	MBUS_Register[0][15] =	0xF10F;
	MBUS_Register[0][16] =	4444;
	MBUS_Register[0][17] =	1229;
	MBUS_Register[0][18] =	819;
	MBUS_Register[0][19] =	0x0004;
	MBUS_Register[0][20] =	0x000F;
	MBUS_Register[0][21] =	3227;
	MBUS_Register[0][22] =	3822;
	MBUS_Register[0][23] =	5055;
	MBUS_Register[0][24] =	11264;
	MBUS_Register[0][25] =	17920;
	MBUS_Register[0][26] =	0x0040;
	MBUS_Register[0][27] =	0x0041;
	MBUS_Register[0][28] =	0x0040;
	MBUS_Register[0][29] =	0x0040;
	MBUS_Register[0][30] =	0x0040;
	MBUS_Register[0][31] =	0x0000;
	MBUS_Register[0][32] =	0x0001;
	MBUS_Register[0][33] =	0x0002;
	MBUS_Register[0][34] =	0x0003;
	MBUS_Register[0][35] =	0x0004;
	MBUS_Register[0][36] =	0x0005;
	MBUS_Register[0][37] =	0x0006;
	MBUS_Register[0][38] =	0x0007;
	MBUS_Register[0][39] =	0x0008;
	MBUS_Register[0][40] =	0x0009;
	MBUS_Register[0][41] =	0x0000;
	MBUS_Register[0][42] =	0x0000;
	MBUS_Register[0][43] =	0x0000;
	MBUS_Register[0][44] =	0x0000;
	MBUS_Register[0][45] =	0xF12D;
	MBUS_Register[0][46] =	0xF12E;
	MBUS_Register[0][47] =	0xF12F;
	MBUS_Register[0][48] =	0xF130;
	MBUS_Register[0][49] =	0xF131;

	MBUS_Register[1][0] =	0xF200;
	MBUS_Register[1][1] =	1010;
	MBUS_Register[1][2] =	2525;
	MBUS_Register[1][3] =	5050;
	MBUS_Register[1][4] =	7575;
	MBUS_Register[1][5] =	0xF205;
	MBUS_Register[1][6] =	0xF206;
	MBUS_Register[1][7] =	0xF207;
	MBUS_Register[1][8] =	1;
	MBUS_Register[1][9] =	0xF209;
	MBUS_Register[1][10] =	0xF20A;
	MBUS_Register[1][11] =	0xF20B;
	MBUS_Register[1][12] =	0xF20C;
	MBUS_Register[1][13] =	0xF20D;
	MBUS_Register[1][14] =	0xF20E;
	MBUS_Register[1][15] =	0xF20F;
	MBUS_Register[1][16] =	-4444;
	MBUS_Register[1][17] =	4096;
	MBUS_Register[1][18] =	-819;
	MBUS_Register[1][19] =	0x00E8;
	MBUS_Register[1][20] =	0x0013;
	MBUS_Register[1][21] =	3185;
	MBUS_Register[1][22] =	3504;
	MBUS_Register[1][23] =	-4945;
	MBUS_Register[1][24] =	19712;
	MBUS_Register[1][25] =	20403;
	MBUS_Register[1][26] =	0x0010;
	MBUS_Register[1][27] =	0x0013;
	MBUS_Register[1][28] =	0x0007;
	MBUS_Register[1][29] =	0x0042;
	MBUS_Register[1][30] =	0x0040;
	MBUS_Register[1][31] =	0x0040;
	MBUS_Register[1][32] =	0x0040;
	MBUS_Register[1][33] =	0x0040;
	MBUS_Register[1][34] =	0x0040;
	MBUS_Register[1][35] =	0x0040;
	MBUS_Register[1][36] =	0x0040;
	MBUS_Register[1][37] =	0x0040;
	MBUS_Register[1][38] =	0x0040;
	MBUS_Register[1][39] =	0x0040;
	MBUS_Register[1][40] =	0x0040;
	MBUS_Register[1][41] =	0x0009;
	MBUS_Register[1][42] =	0x0080;
	MBUS_Register[1][43] =	0x0000;
	MBUS_Register[1][44] =	0x0000;
	MBUS_Register[1][45] =	0xF22D;
	MBUS_Register[1][46] =	0xF22E;
	MBUS_Register[1][47] =	0xF22F;
	MBUS_Register[1][48] =	0xF230;
	MBUS_Register[1][49] =	0xF231;
}
/**
		*	\}
*/

/*void MBUS_Init_Reg(void)
{
	MBUS_Register[0][0] =	0xF100;
	MBUS_Register[0][1] =	1000;
	MBUS_Register[0][2] =	2500;
	MBUS_Register[0][3] =	5000;
	MBUS_Register[0][4] =	7500;
	MBUS_Register[0][5] =	0xF105;
	MBUS_Register[0][6] =	0xF106;
	MBUS_Register[0][7] =	0xF107;
	MBUS_Register[0][8] =	0;
	MBUS_Register[0][9] =	0xF109;
	MBUS_Register[0][10] =	0xF10A;
	MBUS_Register[0][11] =	0xF10B;
	MBUS_Register[0][12] =	0xF10C;
	MBUS_Register[0][13] =	0xF10D;
	MBUS_Register[0][14] =	0xF10E;
	MBUS_Register[0][15] =	0xF10F;
	MBUS_Register[0][16] =	4444;
	MBUS_Register[0][17] =	1229;
	MBUS_Register[0][18] =	819;
	MBUS_Register[0][19] =	0x0004;
	MBUS_Register[0][20] =	0x000F;
	MBUS_Register[0][21] =	3227;
	MBUS_Register[0][22] =	3822;
	MBUS_Register[0][23] =	5055;
	MBUS_Register[0][24] =	11264;
	MBUS_Register[0][25] =	17920;
	MBUS_Register[0][26] =	0x0040;
	MBUS_Register[0][27] =	0x0041;
	MBUS_Register[0][28] =	0x0040;
	MBUS_Register[0][29] =	0x0040;
	MBUS_Register[0][30] =	0x0040;
	MBUS_Register[0][31] =	0x0000;
	MBUS_Register[0][32] =	0x0001;
	MBUS_Register[0][33] =	0x0002;
	MBUS_Register[0][34] =	0x0003;
	MBUS_Register[0][35] =	0x0004;
	MBUS_Register[0][36] =	0x0005;
	MBUS_Register[0][37] =	0x0006;
	MBUS_Register[0][38] =	0x0007;
	MBUS_Register[0][39] =	0x0008;
	MBUS_Register[0][40] =	0x0009;
	MBUS_Register[0][41] =	0x0000;
	MBUS_Register[0][42] =	0x0000;
	MBUS_Register[0][43] =	0x0000;
	MBUS_Register[0][44] =	0x0000;
	MBUS_Register[0][45] =	0xF12D;
	MBUS_Register[0][46] =	0xF12E;
	MBUS_Register[0][47] =	0xF12F;
	MBUS_Register[0][48] =	0xF130;
	MBUS_Register[0][49] =	0xF131;

	MBUS_Register[1][0] =	0xF200;
	MBUS_Register[1][1] =	1010;
	MBUS_Register[1][2] =	2525;
	MBUS_Register[1][3] =	5050;
	MBUS_Register[1][4] =	7575;
	MBUS_Register[1][5] =	0xF205;
	MBUS_Register[1][6] =	0xF206;
	MBUS_Register[1][7] =	0xF207;
	MBUS_Register[1][8] =	1;
	MBUS_Register[1][9] =	0xF209;
	MBUS_Register[1][10] =	0xF20A;
	MBUS_Register[1][11] =	0xF20B;
	MBUS_Register[1][12] =	0xF20C;
	MBUS_Register[1][13] =	0xF20D;
	MBUS_Register[1][14] =	0xF20E;
	MBUS_Register[1][15] =	0xF20F;
	MBUS_Register[1][16] =	-4444;
	MBUS_Register[1][17] =	4096;
	MBUS_Register[1][18] =	-819;
	MBUS_Register[1][19] =	0x00E8;
	MBUS_Register[1][20] =	0x0013;
	MBUS_Register[1][21] =	3185;
	MBUS_Register[1][22] =	3504;
	MBUS_Register[1][23] =	-4945;
	MBUS_Register[1][24] =	19712;
	MBUS_Register[1][25] =	20403;
	MBUS_Register[1][26] =	0x0010;
	MBUS_Register[1][27] =	0x0013;
	MBUS_Register[1][28] =	0x0007;
	MBUS_Register[1][29] =	0x0042;
	MBUS_Register[1][30] =	0x0040;
	MBUS_Register[1][31] =	0x0040;
	MBUS_Register[1][32] =	0x0040;
	MBUS_Register[1][33] =	0x0040;
	MBUS_Register[1][34] =	0x0040;
	MBUS_Register[1][35] =	0x0040;
	MBUS_Register[1][36] =	0x0040;
	MBUS_Register[1][37] =	0x0040;
	MBUS_Register[1][38] =	0x0040;
	MBUS_Register[1][39] =	0x0040;
	MBUS_Register[1][40] =	0x0040;
	MBUS_Register[1][41] =	0x0009;
	MBUS_Register[1][42] =	0x0080;
	MBUS_Register[1][43] =	0x0000;
	MBUS_Register[1][44] =	0x0000;
	MBUS_Register[1][45] =	0xF22D;
	MBUS_Register[1][46] =	0xF22E;
	MBUS_Register[1][47] =	0xF22F;
	MBUS_Register[1][48] =	0xF230;
	MBUS_Register[1][49] =	0xF231;
}*/
