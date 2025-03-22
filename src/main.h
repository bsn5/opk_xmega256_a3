/**
 * \file
 *
 * \brief 
 *
 */

#ifndef MAIN_H
#define MAIN_H

#include <board.h>

#include <ccp.h>

#include <interrupt.h>
#include <pmic.h>
#include <ioport.h>
#include <tc.h>
#include <usart.h>

#include <status_codes.h>
#include <compiler.h>
#include <parts.h>
#include <sysclk.h>

#include <util/delay.h>
#include <avr/pgmspace.h>

#include <lcd/lcd.h>

#include <math.h>
#include <twi/twi.h>
#include <eeprom/eeprom.h>

#include <avr/eeprom.h>

#include <inttypes.h>
#include <avr/io.h>
#include <avr/eeprom.h>
#include <modbus/mb_config.h>
// ������ ������� APD
#include <apd/apd.h>

/************************************************************************/
/*							�������� CRC								*/
/************************************************************************/
#include <crc/crc8.h>
#include <crc/crc16.h>

#define CCP_IOREG 0xD8

#define CONFIG_USART_BAUDRATE 9600
#define RX0 (1<<2)
#define TX0 (1<<3)
#define RX1 (1<<2)
#define TX1 (1<<3)
#define RX2 (1<<6)
#define TX2 (1<<7)


#define RES 0
#define RTS 3
#define RESMBUS 5
#define RTSMBUS 1

/************************************************************************/
/* -> ������ � ������� ������� EEPROM					*/
/************************************************************************/
#define WP (1<<4)

// ���������� ������ �� ������ EEPROM
#define WP_OFF 	PORTE.DIR |= WP;\
				PORTE.OUT &= ~WP;
				//_delay_ms (5);
// ��������� ������ �� ������ EEPROM
//#define WP_ON 	PORTE.DIR &= ~WP;\
				//PORTE.OUT |= WP;
#define WP_ON 	PORTE.OUT |= WP
				//_delay_ms (5);

// ���������� ������� ������\������ EEPROM
#define EEPROM_RW_ATTEMPTS 20

// ������� ������� ������\������ EEPROM
uint8_t EEPROM_AttemptCnt = 0;

#define REG_HEAD_BEGIN 0 // ������ ��������� �������
#define REG_HEAD_WIDTH 2 // ���������� ������ � �������� �������
#define REG_BLOCK_SIZE 8 // ����� ����� ������ � �������
#define REG_BODY_BEGIN 2 // REG_HEAD_BEGIN + REG_HEAD_WIDTH * REG_BLOCK_SIZE // ����� ������� �������
						
#define BUTTON_CNT 800 // �������� ������� ��� ���������

/************************************************************************/
/*								APD										*/
/************************************************************************/
#define ARROW_DOWN	0x02
#define ARROW_UP	0x03
#define ARROW_LEFT	0x04
#define ARROW_RIGHT	0x05

#define APD_MAX_FRAMES 32 // ����� �� ���������� ������ � �������
#define APD_MAX_FIELDS 48 // ����� �� ���������� ����� � �����
#define APD_MAX_ACTIONS 32 // ����� �� ���������� �������������� � �����
// #define APD_MAX_EVENTS 32 // ����� ������� � ������

// ���������� ������������� ��������� �������
#define ADP_EVENT_REQUEST 16

#define EVENT_CODES_MAX 64 // ���������� ����� �������
#define EVENT_FROM_MAX 4 // ���������� ���������
#define DEFAULT_STORAGE_TOP 5
#define DEFAULT_STORAGE_BOT 6
#define EVENT_REC_LIM 8 // ����� �� ���� �������
#define EVENT_REC_WRITE 3 // � ������ ���� ������� �������� � ������
#define Q_TEST_MES		8 //������� �������� ��������� ����� ��������� ��� ����������� ���������� �����

// ���� � ����� ����������� �� ������ ��������� ������������
// "1" - ���������, "0" - ��������
// ��������� ������: ��� ����������� RTS ������ � ����� ������ RES
#define RMD_RESET_ON	_delay_ms (100);\
						PORTD.DIR |= (1<<RTS);\
						PORTD.OUT |= (1<<RTS);\
						_delay_ms (50);\
						PORTD.DIR |= (1<<RES);\
						PORTD.OUT &= ~ (1<<RES);\
						_delay_ms (300);\
						PORTD.OUT |= (1<<RES);\
						PORTD.DIR &= ~(1<<RES);\
						_delay_ms (100)
#define RMDMBUS_RESET_ON	_delay_ms (100);\
							PORTB.DIR |= (1<<RTSMBUS);\
							PORTB.OUT |= (1<<RTSMBUS);\
							_delay_ms (50);\
							PORTF.DIR |= (1<<RESMBUS);\
							PORTF.OUT &= ~ (1<<RESMBUS);\
							_delay_ms (300);\
							PORTF.OUT |= (1<<RESMBUS);\
							PORTF.DIR &= ~(1<<RESMBUS);\
							_delay_ms (100)

// ���������� ������: ��� �������� RTS
#define RMD_RESET_OFF	PORTD.OUT &= ~ (1<<RTS)
#define RMDMBUS_RESET_OFF	PORTB.OUT &= ~ (1<<RTSMBUS)

// ����� �������, ����������� �� �����������
#define RMD_MESSAGE_LENGTH 4

/************************************************************************/
/*								ADP								*/
/************************************************************************/
#define BROKEN_MES 20 // ���������� ���������� ���������

//��������� ���������
#define DEFAULT_CONTRAST 13
#define MENU_MAX 7
#define REG_MENU_MAX 6
#define DEFAULT_DELAY 5
#define DEFAULT_BAUD 3
#define DEFAULT_CYCLE 5
#define DEFAULT_RATE 4
#define DEFAULT_NUM 4
#define SETTINGS_MAX 11
#define TIME_MBUS_ANS 12320
uint16_t Time_ans = 5400;
/************************************************************************/
/*						TIMERS											*/
/************************************************************************/
#define TIMER_1MS_INIT		TCC0.PER = 18432;\
							TCC0.INTCTRLA |= (0x01<<0);\
							TCC0.CTRLA |= (0x01<<0)
///��� �������� 57600 ��� ���������//12320 = 668us
#define TIMER_MBUS485_INIT	TCE1.PER = TIME_MBUS_ANS;\
							TCE1.CTRLA |= (0x00<<0);\
							TCE1.INTCTRLA |= (0x01<<0);

#define TIMER_MBUS485_START	TCE1.CNT = 0x0000;\
							TCE1.CTRLA |= (0x01<<0)
							
#define TIMER_MBUS485_STOP		TCE1.CTRLA = 0x00
//��� �������� 9600 ����� �������� 2 � 36960 = 3,5 �������
#define TIMER_MBUS232_INIT	TCF0.PER = TIME_MBUS_ANS*3;\
							TCF0.CTRLA |= (0x02<<0);\
							TCF0.INTCTRLA |= (0x01<<0);

#define TIMER_MBUS232_START	TCF0.CNT = 0x0000;\
							TCF0.CTRLA |= (0x01<<0)
							
#define TIMER_MBUS232_STOP		TCF0.CTRLA = 0x00	

#define TIMER_MBUSRMD_INIT		TCC1.PER = TIME_MBUS_ANS;\
							TCC1.CTRLA |= (0x00<<0);\
							TCC1.INTCTRLA |= (0x01<<0);

#define TIMER_MBUSRMD_START	TCC1.CNT = 0x0000;\
							TCC1.CTRLA |= (0x01<<0)
							
#define TIMER_MBUSRMD_STOP		TCC1.CTRLA = 0x00
											//7200 = 400ms
#define TIMER_ADP_TIMEOUT_INIT	TCE0.PER = Time_ans;\
								TCE0.INTCTRLA |= 0x01///0x4650 = 18000; 18000/3600=200ms//2328=500ms
								
#define TIMER_ADP_TIMEOUT_START TCE0.CTRLA |= TC_CLKSEL_DIV1024_gc

#define TIMER_ADP_TIMEOUT_STOP	TCE0.CTRLA = 0x00
									///9500 = 515us
#define TIMER_RS232_INIT	TCD1.PER = 9500;\
							TCD1.INTCTRLA |= (0x02<<0)/// ����� ����� ����� ������������ 25000 �������� ����� 1,356�� �� �������� 57600

#define TIMER_RS232_START	TCD1.CNT = 0x0000;\
							TCD1.CTRLA |= (0x01<<0)

#define TIMER_RS232_STOP	TCD1.CTRLA &= ~ (0x01<<0)

#define TIMER_RS232_RESTART	TCD1.CNT = 0x0000
										//28500 = 1.55ms
#define TIMER_ADP_2MS_INIT	TCD0.PER = 28500;\
							TCD0.INTCTRLA |= (0x01<<0)
							
#define TIMER_ADP_2MS_START	TCD0.CNT = 0x0000;\
							TCD0.CTRLA |=(0x01<<0)
							
#define TIMER_ADP_2MS_STOP	TCD0.CTRLA = 0x00

#define SEND_AMOUNT 8
#define SEND_ATTEMPTS 9
#define MAX_QUANT_REG 50

#define DAC_per_VOLT 1241 

static uint8_t ADP_MBUS_Counter[4] = {0,0,0,0};//������� ���������� ��������� ��� �������� �� ADP �� MODBUS
volatile uint8_t ADP_flag;		// ���� ��������� ADP (��� 7 - ��������� ������ ROMBUS 1 - ����� ���������� ����� ���������, 
								//��� 0 - ��� ��������� ������� � ������ � ���������(�� �������������)) 
volatile uint8_t ADP_timeout=0;	
uint16_t Reg2Reg = 0;
uint16_t RegAdr = 0;
volatile uint16_t MBUS_ADP[128];
uint8_t ADP_Buf[128];
uint16_t ADP_MBUS_Adr;
uint16_t ADP_MBUS_Q;
uint16_t ADP_NextStart[4];
uint16_t delta;
uint16_t i,j,ADP_CRC;
uint8_t nu,nu2,MBUS_num;
volatile uint16_t numADPmes;
uint16_t ADP_Rec_CRC_Error;
uint16_t ADP_mis_Error;
uint32_t s_timeout;//������� ��� ������ ������, ���� ����� ��������
uint32_t timeS1;
uint32_t timeS2;
uint8_t s_delta[8];
uint8_t delta_s[8];
uint8_t NumOfFreqCon=0;		//���� 0, �� ���������, ��� 2 �� � ����, ���� 1, �� 4, 
							// �������������� ������ ��������� ����
uint8_t Q_Test_inc=0;

uint16_t comp (const uint16_t *i, const uint16_t *j);

// �������� CRC
extern		uint8_t		Crc8 (uint8_t *PcBLOCK_, uint8_t Len);
extern		uint16_t	Crc16 (uint8_t *PcBLOCK_, uint16_t Len);
// ������ � ������ RS-232 �� ��������� MODBUS
extern void		MBUS_TransStart (void);
extern void		MBUS_TransContinue (void);
extern void		MBUS_Error_Trans(uint8_t Error);
extern void		MBUS_Send_03 (void);
extern void		MBUS_Send_04 (void);
extern void		MBUS_Send_06 (void);
extern void		MBUS_Send_10 (void);
extern void		MBUS_Send_17 (void);
#endif // MAIN_H
