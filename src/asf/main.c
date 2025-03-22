/************************************************************************/
/*                       ����� ��������� ���������                      */
/************************************************************************/
#include <asf.h>

#undef F_CPU
#define F_CPU 18432000

#define CCP_IOREG 0xD8

#define CONFIG_USART_BAUDRATE 9600
#define RX0 (1<<2)
#define TX0 (1<<3)
#define RX1 (1<<2)
#define TX1 (1<<3)

#define RES 0
#define RTS 3
#define F_SYS 18432000
#define F_TWI_RTC 100000 // ������� ������ ���� TWI RTC
#define F_TWI_EEPROM 100000 // ������� ������ ���� TWI EEPROM
#define TWI_BAUD(F_SYS, F_TWI) ((F_SYS / (2 * F_TWI)) - 5) // ������ �������� ���� TWI
#define CYCLE_COUNTER_RTC 1000
#define CYCLE_COUNTER_EEPROM 1000

/************************************************************************/
/*                   ����������� �������                                */
/************************************************************************/
// ������ ������ ����������� � ��
volatile uint16_t TcAdpTrans = 0;
volatile uint16_t TcAdpTransPer = 10000;
volatile uint8_t TcAdpTransEn = 0;

uint8_t TcAdpTransRun = 0;

// ������ ���������� �����
volatile uint16_t TcRtc = 0;
volatile uint16_t TcRtcPer = 500;
volatile uint8_t TcRtcEn = 0;

// ������ ������� ���������������� ��������� �� ����������
volatile uint16_t TcRadCfg = 0;
volatile uint16_t TcRadCfgPer = 100;
volatile uint8_t TcRadCfgEn = 1;

// ������ ��������� �������� ����� ��������� �� ��������� ADP 
volatile uint16_t TcAdpTimeout = 0;
volatile uint16_t TcAdpTimeoutPer = 100;
volatile uint8_t TcAdpTimeoutEn = 0;

// ������ ������� ������� � ��
uint16_t TcAdpEvents = 0;
uint16_t TcAdpEventsPer = 5000;
uint8_t TcAdpEventsEn = 0;

// ������ ��������� ������ ��������
uint16_t TcAdpTransFault = 0;
uint16_t TcAdpTransFaultPer = 1 * 5000;
uint8_t TcAdpTransFaultEn = 0;

/************************************************************************/
/*                     ������ � ������� EEPROM                          */
/************************************************************************/
#define WP (1<<5)

// ���������� ������ �� ������ EEPROM
#define WP_OFF 	PORTD.DIR |= WP;\
				PORTD.OUT &= ~WP;\
//				_delay_ms (3);

// ��������� ������ �� ������ EEPROM				
#define WP_ON 	PORTD.DIR &= ~WP;\
				PORTD.OUT &= ~WP;				

// ����� ������ �� EEPROM
uint8_t EEPROM_ReadBuff[8] = {0, 0, 0, 0, 0, 0, 0, 0};
// ����� ������ � EEPROM
uint8_t EEPROM_WriteBuff[8] = {0, 0, 0, 0, 0, 0, 0, 0};
	
uint16_t EEPROM_MesCnt = 0; // ������� ������� � EEPROM
 
// uint8_t A[3] = {102, 38, 67};
// uint8_t B[3] = {0, 0, 0};
	
// uint8_t Buff1[8], Buff2[8], Buff3[8];

// "������"
uint8_t StringRegister[7] = {0xA3, 0xA9, 0x50, 0x48, 0x41, 0xA7, 0x20};

// ���������� ������� ������\������ EEPROM	
#define EEPROM_OP_ATTEMPTS 100

// ������� ������� ������\������ EEPROM
uint8_t EEPROM_AttemptCnt = 0;

// ������ ��������� �������
#define REGISTER_START 0

uint8_t ButtonFirstPrev = 0;
uint8_t ButtonSecondPrev = 0;
						
#define BUTTON_CNT 500
uint16_t ButtonCnt = BUTTON_CNT;	

/************************************************************************/
/*                     ���������� ������� APD                           */
/************************************************************************/
// ���� ����� � �������
typedef enum apd_field_type {
	APD_FIELD_TYPE_SPLASH = 0, // ��������
	APD_FIELD_TYPE_VALUE = 1, // �����
	APD_FIELD_TYPE_TEXT = 2, // �����	
	APD_FIELD_TYPE_DATE = 3, // ����
	APD_FIELD_TYPE_TIME = 4,  // �����
 } apd_field_type_t;

// ������� ������ ����� 
typedef enum apd_field_format {
	APD_FIELD_FORMAT_BIN = 0x00, // ��������
	APD_FIELD_FORMAT_HEX = 0x10, // �����������������
	APD_FIELD_FORMAT_WITHOUT_NULLS = 0x20, // ���������� �������� ��� ����� �����	
	APD_FIELD_FORMAT_WITH_NULLS = 0x30, // ���������� �������� � ������ �����	
 } apd_field_format_t;
 
 // ���������� �������� 
 uint32_t Rank[] = {
	1, // 0
	10, // 1
	100, // 2
	1000, // 3
	10000, // 4
	100000, // 5
	1000000, // 6
	10000000, // 7
	100000000, // 8
	1000000000, // 9
};

// ������� ������������ 
uint8_t DecToHex[16] = {
	0x30, // 0
	0x31, // 1
	0x32, // 2 
	0x33, // 3 
	0x34, // 4
	0x35, // 5
	0x36, // 6
	0x37, // 7
	0x38, // 8
	0x39, // 9
	0x41, // A
	0x42, // B
	0x43, // C
	0x44, // D
	0x45, // E
	0x46, // F
};
 
// ������ ��������
uint8_t Id8 = 0; // ������� ����
uint8_t IdFrame = 0; // ������� ������
uint8_t IdField = 0; // ������� �����
uint8_t IdAction = 0; // ������� ��������������
uint16_t IdEvent = 0; // ������� �������

#define APD_MAX_FRAMES 16 // ����� �� ���������� ������ � �������
#define APD_MAX_FIELDS 32 // ����� �� ���������� ����� � �����
#define APD_MAX_ACTIONS 32 // ����� �� ���������� �������������� � �����
#define APD_MAX_EVENTS 256 // ����� ������� � ������

// ���������� ������������� ��������� �������
#define ADP_EVENTS_REQUEST 16
// ����� ������������� �������
uint16_t ApdEventsBuf[ADP_EVENTS_REQUEST];

signed int ClockShowString = 0; // ����� ������ ��� ������ �����
uint8_t ClockShowColumn = 12; // ����� ������� ��� ������ �����

signed int DataShowString = 0; // ����� ������ ��� ������ ����
uint8_t DataShowColumn = 0; // ����� ������� ��� ������ �����

uint16_t ApdFrameRefreshRate = 0; // ������ ������ ������ (0 - ����, 1 - 100��, 2 - 200, 3...10)
uint16_t ApdEventRefreshRate = 0; // ������ ������ ������� (0 - ����, 1 - 100��, 2 - 200, 3...10)
uint16_t ApdBaudRate = 0; // �������� ����� � ��� (0 - 9600, 1 - 19200, 2 - 38400, 3 - 57600)

uint8_t ApdFrameAmount = 0; // ���������� ������ � �������
uint8_t ApdFrameStartNumber = 0; // ����� ���������� �����

uint8_t ApdFrameCur = 0; // ������� ���� �������
signed int ApdFrameDisplayShift = 0; // ����� ����� �� �������

uint16_t ApdFramesStartAdr = 0; // ����� �������� ������
uint16_t ApdEventsStartAdr = 0; // ����� �������� ������ �������
uint16_t ApdEventAdr = 0; // ����� ������ ����� ������ �������
uint16_t ApdEventIdAmount = 0; // ���������� �������� �������

uint16_t ApdFrameStartAdr[APD_MAX_FRAMES]; // ������ ������� �������� ������
uint16_t ApdFrameFieldsAndActionsStartAdr[APD_MAX_FRAMES][2]; // ������ �������� ����� � �������� � �����

uint16_t ApdEventStartAdr[APD_MAX_EVENTS]; // ������ ������� �������� �������

uint8_t ApdFrameFieldsAmount[APD_MAX_FRAMES]; // ������ ���������� ����� � ������ �����
uint8_t ApdFrameActionsAmount[APD_MAX_FRAMES]; // ������ ���������� ����� � ������ �����

uint16_t ApdFrameFieldStartAdr[APD_MAX_FRAMES][APD_MAX_FIELDS]; // ������ ������� �������� ����� ������� �����
uint16_t ApdFrameActionStartAdr[APD_MAX_FRAMES][APD_MAX_ACTIONS];// ������ ������� �������� �������������� ������� �����

uint16_t ApdFrameDataFromPlcAdr[APD_MAX_FRAMES][APD_MAX_FIELDS]; // ������ �������, ������������� �� ��� � ������ �����
signed int ApdFrameDataFromPlcData[APD_MAX_FRAMES][APD_MAX_FIELDS]; // ������ ���������, ������������� �� ��� � ������ �����
uint8_t ApdFrameDataFromPlcStatus[APD_MAX_FRAMES][APD_MAX_FIELDS]; // ������ �������� ���������, ������������� �� ��� � ������ �����
uint8_t ApdFrameDataFromPlcString[APD_MAX_FRAMES][APD_MAX_FIELDS]; // ������ ����� ���������, ������������� �� ��� � ������ �����
uint8_t ApdFrameDataFromPlcAmount[APD_MAX_FRAMES]; // ������ ���������� ������������� �� ��� ������� � �����

uint16_t FocusDataAdr[APD_MAX_FIELDS]; // ������ ������� ���������� � ������
signed int FocusData[APD_MAX_FIELDS]; // ������ ��������� � ������
uint8_t FocusDataCntNext[APD_MAX_FIELDS]; // ������ ��������� �������� (������� ���������)
uint8_t  FocusDataCntPrev[APD_MAX_FIELDS]; // ������ ��������� �������� (���������� ���������)
uint8_t FocusDataStatus[APD_MAX_FIELDS]; // ������ �������� ���������
uint8_t FocusDataAmount = 0; // ���������� ������� � ������

uint16_t RequestAdrSpaceArray[APD_MAX_FIELDS]; // ������ ���������� ����� �������� � ������

uint8_t FocusDataMesNew = 0; // ���� ��������� ������-���� �������� � �����

uint16_t BlockRequest[APD_MAX_FIELDS][2]; // ������ ������� � ������ ��� �������
uint8_t BlockAmount = 0; // ���������� ������� � ������ ��� ������� 

uint16_t ApdFrameButtonAction = 255; // ��� �������� �� ������� ������

signed int FrameCheck = 255;
signed int ShiftCheck = 255;

uint8_t FocusNew = 1; // ���� ������ ������ � ����� (����� �����������
// ������������� ����� ������

/*
// ������ ������� APD
uint8_t ApdPrjData[] = {

0x41,0x44,0x02,0x01,0x0A,0x05,0x00,0x03,0x04,0x14,0x03,0x05,0x05,0x00,0x00,0x00,
0x09,0x00,0x00,0x78,0x00,0xC2,0x0D,0x00,0x00,0x00,0x00,0x00,0x00,0x04,0x42,0x31,
0x35,0x4B,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x8A,0x00,0x24,0x02,0x6A,0x05,0xBA,0x05,
0x8B,0x08,0xDB,0x08,0x77,0x09,0x0F,0x0B,0x65,0x0C,0x9B,0x00,0x00,0x02,0x04,0x00,
0x0A,0x5B,0x31,0x5D,0xC3,0xEB,0xE0,0xE2,0xED,0xFB,0xE9,0x0D,0xB6,0x00,0x08,0x01,
0x16,0x01,0x24,0x01,0x46,0x01,0x54,0x01,0x62,0x01,0x84,0x01,0x92,0x01,0xA0,0x01,
0xC2,0x01,0xD0,0x01,0xDE,0x01,0x00,0x04,0x85,0x91,0x31,0x3A,0x20,0x20,0x20,0x41,
0x20,0x20,0x20,0x20,0x80,0x90,0x20,0x20,0x20,0x20,0x20,0x20,0x85,0x91,0x32,0x3A,
0x20,0x20,0x20,0x41,0x20,0x20,0x20,0x20,0x80,0x90,0x20,0x20,0x20,0x20,0x20,0x20,
0x85,0x91,0x33,0x3A,0x20,0x20,0x20,0x41,0x20,0x20,0x20,0x20,0x80,0x90,0x20,0x20,
0x20,0x20,0x20,0x20,0x85,0x91,0x34,0x3A,0x20,0x20,0x20,0x41,0x20,0x20,0x20,0x20,
0x80,0x90,0x20,0x20,0x20,0x20,0x20,0x20,0x01,0x00,0x04,0x03,0x2F,0x04,0x09,0x20,
0xC8,0x00,0x00,0x20,0x00,0x00,0x01,0x00,0x08,0x04,0x71,0x04,0x09,0x20,0x01,0x00,
0x64,0x00,0x00,0x00,0x02,0x00,0x0F,0x05,0x11,0x04,0x09,0x00,0x04,0x41,0x42,0x41,
0x50,0x20,0x80,0x4F,0x54,0x4F,0x42,0x50,0x41,0x8A,0x2E,0x20,0x50,0x45,0x4B,0x06,
0x85,0x85,0x50,0x80,0x20,0x20,0x01,0x01,0x04,0x03,0x17,0x08,0x09,0x20,0xC8,0x00,
0x00,0x20,0x00,0x00,0x01,0x01,0x08,0x04,0x59,0x08,0x09,0x20,0x01,0x00,0x64,0x00,
0x00,0x00,0x02,0x01,0x0F,0x05,0xF9,0x07,0x09,0x00,0x04,0x41,0x42,0x41,0x50,0x20,
0x80,0x4F,0x54,0x4F,0x42,0x50,0x41,0x8A,0x2E,0x20,0x50,0x45,0x4B,0x06,0x85,0x85,
0x50,0x80,0x20,0x20,0x01,0x02,0x04,0x03,0xFF,0x0B,0x09,0x20,0xC8,0x00,0x00,0x20,
0x00,0x00,0x01,0x02,0x08,0x04,0x41,0x0C,0x09,0x20,0x01,0x00,0x64,0x00,0x00,0x00,
0x02,0x02,0x0F,0x05,0xE1,0x0B,0x09,0x00,0x04,0x41,0x42,0x41,0x50,0x20,0x80,0x4F,
0x54,0x4F,0x42,0x50,0x41,0x8A,0x2E,0x20,0x50,0x45,0x4B,0x06,0x85,0x85,0x50,0x80,
0x20,0x20,0x01,0x03,0x04,0x03,0xE7,0x0F,0x09,0x20,0xC8,0x00,0x00,0x20,0x00,0x00,
0x01,0x03,0x08,0x04,0x29,0x10,0x09,0x20,0x01,0x00,0x64,0x00,0x00,0x00,0x02,0x03,
0x0F,0x05,0xC9,0x0F,0x09,0x00,0x04,0x41,0x42,0x41,0x50,0x20,0x80,0x4F,0x54,0x4F,
0x42,0x50,0x41,0x8A,0x2E,0x20,0x50,0x45,0x4B,0x06,0x85,0x85,0x50,0x80,0x20,0x20,
0x07,0x0F,0x02,0x12,0x02,0x15,0x02,0x18,0x02,0x1B,0x02,0x1E,0x02,0x21,0x02,0x09,
0x01,0x05,0x01,0x01,0x01,0x02,0x01,0x02,0x03,0x01,0x03,0x04,0x01,0x04,0x1A,0x01,
0x06,0x14,0x01,0x08,0x37,0x02,0x3C,0x05,0x10,0x00,0x0C,0x5B,0x32,0x5D,0xCF,0xE0,
0xF0,0xE0,0xEC,0xE5,0xF2,0xF0,0xFB,0x1D,0x72,0x02,0xB4,0x03,0xC2,0x03,0xD0,0x03,
0xDE,0x03,0xEC,0x03,0xFA,0x03,0x08,0x04,0x16,0x04,0x24,0x04,0x32,0x04,0x40,0x04,
0x4E,0x04,0x5C,0x04,0x6A,0x04,0x78,0x04,0x86,0x04,0x94,0x04,0xA2,0x04,0xB0,0x04,
0xBE,0x04,0xCC,0x04,0xDA,0x04,0xE8,0x04,0xF6,0x04,0x04,0x05,0x12,0x05,0x20,0x05,
0x2E,0x05,0x00,0x10,0x20,0x20,0x20,0x85,0x41,0x50,0x41,0x4D,0x45,0x54,0x50,0x95,
0x20,0x85,0x91,0x2D,0x31,0x20,0x20,0x20,0x20,0x42,0x58,0x3A,0x20,0x20,0x20,0x20,
0x42,0x20,0x20,0x20,0x20,0x41,0x20,0x20,0x20,0x20,0x80,0x90,0x42,0x95,0x58,0x3A,
0x20,0x20,0x20,0x20,0x42,0x20,0x20,0x20,0x20,0x41,0x20,0x20,0x20,0x20,0x80,0x90,
0x54,0x45,0x4D,0x85,0x45,0x50,0x41,0x54,0x06,0x50,0x41,0x20,0x20,0x20,0x6D,0x61,
0x78,0x20,0x20,0x20,0x20,0x20,0x20,0x85,0x41,0x50,0x41,0x4D,0x45,0x54,0x50,0x95,
0x20,0x85,0x91,0x2D,0x32,0x20,0x20,0x20,0x20,0x42,0x58,0x3A,0x20,0x20,0x20,0x20,
0x42,0x20,0x20,0x20,0x20,0x41,0x20,0x20,0x20,0x20,0x80,0x90,0x42,0x95,0x58,0x3A,
0x20,0x20,0x20,0x20,0x42,0x20,0x20,0x20,0x20,0x41,0x20,0x20,0x20,0x20,0x80,0x90,
0x54,0x45,0x4D,0x85,0x45,0x50,0x41,0x54,0x06,0x50,0x41,0x20,0x20,0x20,0x6D,0x61,
0x78,0x20,0x20,0x20,0x20,0x20,0x20,0x85,0x41,0x50,0x41,0x4D,0x45,0x54,0x50,0x95,
0x20,0x85,0x91,0x2D,0x33,0x20,0x20,0x20,0x20,0x42,0x58,0x3A,0x20,0x20,0x20,0x20,
0x42,0x20,0x20,0x20,0x20,0x41,0x20,0x20,0x20,0x20,0x80,0x90,0x42,0x95,0x58,0x3A,
0x20,0x20,0x20,0x20,0x42,0x20,0x20,0x20,0x20,0x41,0x20,0x20,0x20,0x20,0x80,0x90,
0x54,0x45,0x4D,0x85,0x45,0x50,0x41,0x54,0x06,0x50,0x41,0x20,0x20,0x20,0x6D,0x61,
0x78,0x20,0x20,0x20,0x20,0x20,0x20,0x85,0x41,0x50,0x41,0x4D,0x45,0x54,0x50,0x95,
0x20,0x85,0x91,0x2D,0x34,0x20,0x20,0x20,0x20,0x42,0x58,0x3A,0x20,0x20,0x20,0x20,
0x42,0x20,0x20,0x20,0x20,0x41,0x20,0x20,0x20,0x20,0x80,0x90,0x42,0x95,0x58,0x3A,
0x20,0x20,0x20,0x20,0x42,0x20,0x20,0x20,0x20,0x41,0x20,0x20,0x20,0x20,0x80,0x90,
0x54,0x45,0x4D,0x85,0x45,0x50,0x41,0x54,0x06,0x50,0x41,0x20,0x20,0x20,0x6D,0x61,
0x78,0x20,0x20,0x20,0x01,0x01,0x05,0x03,0x44,0x04,0x09,0x20,0xB0,0x04,0x00,0x20,
0x00,0x00,0x01,0x01,0x0F,0x03,0x75,0x04,0x09,0x20,0x01,0x00,0x64,0x00,0x00,0x00,
0x01,0x02,0x05,0x03,0x3D,0x04,0x09,0x20,0xB0,0x04,0x00,0x20,0x00,0x00,0x01,0x02,
0x0A,0x03,0x2F,0x04,0x09,0x20,0xC8,0x00,0x00,0x20,0x00,0x00,0x01,0x02,0x0F,0x03,
0x71,0x04,0x09,0x20,0x01,0x00,0x64,0x00,0x00,0x00,0x01,0x03,0x0B,0x03,0xB2,0x04,
0x09,0x20,0x7D,0x00,0x3B,0x1E,0x07,0x00,0x01,0x03,0x11,0x03,0x21,0x04,0x09,0x20,
0x7D,0x00,0x3B,0x1E,0x07,0x00,0x01,0x05,0x05,0x03,0x2C,0x08,0x09,0x20,0xB0,0x04,
0x00,0x20,0x00,0x00,0x01,0x05,0x0F,0x03,0x5D,0x08,0x09,0x20,0x01,0x00,0x64,0x00,
0x00,0x00,0x01,0x06,0x05,0x03,0x25,0x08,0x09,0x20,0xB0,0x04,0x00,0x20,0x00,0x00,
0x01,0x06,0x0A,0x03,0x17,0x08,0x09,0x20,0xC8,0x00,0x00,0x20,0x00,0x00,0x01,0x06,
0x0F,0x03,0x59,0x08,0x09,0x20,0x01,0x00,0x64,0x00,0x00,0x00,0x01,0x07,0x0B,0x03,
0x9A,0x08,0x09,0x20,0x7D,0x00,0x3B,0x1E,0x07,0x00,0x01,0x07,0x11,0x03,0x09,0x08,
0x09,0x20,0x7D,0x00,0x3B,0x1E,0x07,0x00,0x01,0x09,0x05,0x03,0x14,0x0C,0x09,0x20,
0xB0,0x04,0x00,0x20,0x00,0x00,0x01,0x09,0x0F,0x03,0x45,0x0C,0x09,0x20,0x01,0x00,
0x64,0x00,0x00,0x00,0x01,0x0A,0x05,0x03,0x0D,0x0C,0x09,0x20,0xB0,0x04,0x00,0x20,
0x00,0x00,0x01,0x0A,0x0A,0x03,0xFF,0x0B,0x09,0x20,0xC8,0x00,0x00,0x20,0x00,0x00,
0x01,0x0A,0x0F,0x03,0x41,0x0C,0x09,0x20,0x01,0x00,0x64,0x00,0x00,0x00,0x01,0x0B,
0x0B,0x03,0x82,0x0C,0x09,0x20,0x7D,0x00,0x3B,0x1E,0x07,0x00,0x01,0x0B,0x11,0x03,
0xF1,0x0B,0x09,0x20,0x7D,0x00,0x3B,0x1E,0x07,0x00,0x01,0x0D,0x05,0x03,0xFC,0x0F,
0x09,0x20,0xB0,0x04,0x00,0x20,0x00,0x00,0x01,0x0D,0x0F,0x03,0x2D,0x10,0x09,0x20,
0x01,0x00,0x64,0x00,0x00,0x00,0x01,0x0E,0x05,0x03,0xF5,0x0F,0x09,0x20,0xB0,0x04,
0x00,0x20,0x00,0x00,0x01,0x0E,0x0A,0x03,0xE7,0x0F,0x09,0x20,0xC8,0x00,0x00,0x20,
0x00,0x00,0x01,0x0E,0x0F,0x03,0x29,0x10,0x09,0x20,0x01,0x00,0x64,0x00,0x00,0x00,
0x01,0x0F,0x0B,0x03,0x6A,0x10,0x09,0x20,0x7D,0x00,0x3B,0x1E,0x07,0x00,0x01,0x0F,
0x11,0x03,0xD9,0x0F,0x09,0x20,0x7D,0x00,0x3B,0x1E,0x07,0x00,0x09,0x4F,0x05,0x52,
0x05,0x55,0x05,0x58,0x05,0x5B,0x05,0x5E,0x05,0x61,0x05,0x64,0x05,0x67,0x05,0x0D,
0x03,0x04,0x0B,0x02,0x04,0x09,0x01,0x05,0x00,0x01,0x00,0x02,0x01,0x02,0x03,0x01,
0x03,0x04,0x01,0x04,0x1A,0x01,0x06,0x14,0x01,0x08,0x7D,0x05,0x96,0x05,0x01,0x00,
0x0C,0x5B,0x33,0x5D,0xD1,0xEE,0xEE,0xE1,0xF9,0xE5,0xED,0xE8,0xFF,0x01,0x80,0x05,
0x00,0x01,0x48,0x45,0x8E,0x43,0x85,0x50,0x41,0x42,0x48,0x4F,0x43,0x54,0x8E,0x20,
0x20,0x20,0x20,0x20,0x20,0x20,0x07,0xA5,0x05,0xA8,0x05,0xAB,0x05,0xAE,0x05,0xB1,
0x05,0xB4,0x05,0xB7,0x05,0x09,0x01,0x05,0x00,0x01,0x00,0x01,0x01,0x01,0x03,0x01,
0x03,0x04,0x01,0x04,0x1A,0x01,0x06,0x14,0x01,0x08,0xCE,0x05,0x53,0x08,0x20,0x00,
0x0D,0x5B,0x34,0x5D,0xD1,0xF2,0xE0,0xF2,0xE8,0xF1,0xF2,0xE8,0xEA,0xE0,0x01,0xD1,
0x05,0x00,0x20,0x20,0x20,0x43,0x54,0x41,0x54,0x8E,0x43,0x54,0x8E,0x4B,0x41,0x20,
0x85,0x91,0x2D,0x31,0x20,0x20,0x20,0x85,0x4F,0x54,0x50,0x2E,0x20,0x4B,0x42,0x54,
0x2A,0x91,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x50,0x45,0x4B,0x06,0x85,
0x20,0x4B,0x42,0x54,0x2A,0x91,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x42,
0x43,0x45,0x80,0x4F,0x20,0x4B,0x42,0x54,0x2A,0x91,0x20,0x20,0x20,0x20,0x20,0x20,
0x20,0x20,0x20,0x20,0x20,0x43,0x54,0x41,0x54,0x8E,0x43,0x54,0x8E,0x4B,0x41,0x20,
0x85,0x91,0x2D,0x31,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,
0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x42,0x50,0x45,0x4D,0x99,
0x20,0x91,0x3A,0x4D,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x3A,0x20,0x20,0x20,
0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x43,0x45,0x4B,
0x20,0x20,0x20,0x20,0x20,0x43,0x54,0x41,0x54,0x8E,0x43,0x54,0x8E,0x4B,0x41,0x20,
0x85,0x91,0x2D,0x32,0x20,0x20,0x20,0x85,0x4F,0x54,0x50,0x2E,0x20,0x4B,0x42,0x54,
0x2A,0x91,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x50,0x45,0x4B,0x06,0x85,
0x20,0x4B,0x42,0x54,0x2A,0x91,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x42,
0x43,0x45,0x80,0x4F,0x20,0x4B,0x42,0x54,0x2A,0x91,0x20,0x20,0x20,0x20,0x20,0x20,
0x20,0x20,0x20,0x20,0x20,0x43,0x54,0x41,0x54,0x8E,0x43,0x54,0x8E,0x4B,0x41,0x20,
0x85,0x91,0x2D,0x32,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,
0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x42,0x50,0x45,0x4D,0x99,
0x20,0x91,0x3A,0x4D,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x3A,0x20,0x20,0x20,
0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x43,0x45,0x4B,
0x20,0x20,0x20,0x20,0x20,0x43,0x54,0x41,0x54,0x8E,0x43,0x54,0x8E,0x4B,0x41,0x20,
0x85,0x91,0x2D,0x33,0x20,0x20,0x20,0x85,0x4F,0x54,0x50,0x2E,0x20,0x4B,0x42,0x54,
0x2A,0x91,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x50,0x45,0x4B,0x06,0x85,
0x20,0x4B,0x42,0x54,0x2A,0x91,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x42,
0x43,0x45,0x80,0x4F,0x20,0x4B,0x42,0x54,0x2A,0x91,0x20,0x20,0x20,0x20,0x20,0x20,
0x20,0x20,0x20,0x20,0x20,0x43,0x54,0x41,0x54,0x8E,0x43,0x54,0x8E,0x4B,0x41,0x20,
0x85,0x91,0x2D,0x33,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,
0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x42,0x50,0x45,0x4D,0x99,
0x20,0x91,0x3A,0x4D,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x3A,0x20,0x20,0x20,
0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x43,0x45,0x4B,
0x20,0x20,0x20,0x20,0x20,0x43,0x54,0x41,0x54,0x8E,0x43,0x54,0x8E,0x4B,0x41,0x20,
0x85,0x91,0x2D,0x34,0x20,0x20,0x20,0x85,0x4F,0x54,0x50,0x2E,0x20,0x4B,0x42,0x54,
0x2A,0x91,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x50,0x45,0x4B,0x06,0x85,
0x20,0x4B,0x42,0x54,0x2A,0x91,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x42,
0x43,0x45,0x80,0x4F,0x20,0x4B,0x42,0x54,0x2A,0x91,0x20,0x20,0x20,0x20,0x20,0x20,
0x20,0x20,0x20,0x20,0x20,0x43,0x54,0x41,0x54,0x8E,0x43,0x54,0x8E,0x4B,0x41,0x20,
0x85,0x91,0x2D,0x34,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,
0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x42,0x50,0x45,0x4D,0x99,
0x20,0x91,0x3A,0x4D,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x3A,0x20,0x20,0x20,
0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x43,0x45,0x4B,
0x20,0x20,0x20,0x0B,0x6A,0x08,0x6D,0x08,0x70,0x08,0x73,0x08,0x76,0x08,0x79,0x08,
0x7C,0x08,0x7F,0x08,0x82,0x08,0x85,0x08,0x88,0x08,0x0D,0x03,0x08,0x0B,0x02,0x08,
0x0C,0x03,0x04,0x11,0x02,0x04,0x09,0x01,0x05,0x00,0x01,0x00,0x01,0x01,0x01,0x02,
0x01,0x02,0x04,0x01,0x04,0x1A,0x01,0x06,0x14,0x01,0x08,0x9E,0x08,0xB7,0x08,0x01,
0x00,0x0C,0x5B,0x35,0x5D,0xCD,0xE0,0xF1,0xF2,0xF0,0xEE,0xE9,0xEA,0xE0,0x01,0xA1,
0x08,0x00,0x01,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,
0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x07,0xC6,0x08,0xC9,0x08,0xCC,0x08,0xCF,0x08,
0xD2,0x08,0xD5,0x08,0xD8,0x08,0x09,0x01,0x05,0x00,0x01,0x00,0x01,0x01,0x01,0x02,
0x01,0x02,0x03,0x01,0x03,0x1A,0x01,0x06,0x14,0x01,0x08,0xF4,0x08,0x53,0x09,0x04,
0x00,0x12,0x28,0x30,0x29,0xC4,0xE0,0xF2,0xE0,0xC2,0xF0,0xE5,0xEC,0xFF,0xC2,0xE5,
0xF0,0xF1,0xE8,0xFF,0x03,0xFB,0x08,0x4D,0x09,0x50,0x09,0x00,0x04,0x42,0x45,0x50,
0x43,0x8E,0x8E,0x20,0x85,0x4F,0x3A,0x20,0x20,0x20,0x85,0x91,0x31,0x20,0xFF,0xFF,
0xFF,0x20,0x85,0x06,0x8F,0x96,0x54,0x20,0xFF,0xFF,0xFF,0x20,0x20,0x20,0x85,0x91,
0x32,0x20,0xFF,0xFF,0xFF,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,
0x20,0x20,0x85,0x91,0x33,0x20,0xFF,0xFF,0xFF,0x20,0x20,0x20,0x20,0x20,0x20,0x20,
0x20,0x20,0x20,0x20,0x20,0x20,0x85,0x91,0x34,0x20,0xFF,0xFF,0xFF,0x03,0x02,0x00,
0x04,0x03,0x00,0x07,0x62,0x09,0x65,0x09,0x68,0x09,0x6B,0x09,0x6E,0x09,0x71,0x09,
0x74,0x09,0x00,0x01,0x00,0x01,0x01,0x01,0x02,0x01,0x02,0x03,0x01,0x03,0x04,0x01,
0x04,0x1A,0x01,0x06,0x14,0x01,0x08,0x8D,0x09,0xE6,0x0A,0x04,0x00,0x0F,0x5B,0x2E,
0x5D,0xD1,0xEB,0xF3,0xE6,0xE5,0xE1,0xED,0xFB,0xE9,0x28,0x30,0x29,0x15,0xB8,0x09,
0xCE,0x09,0xDC,0x09,0xEA,0x09,0xF8,0x09,0x06,0x0A,0x14,0x0A,0x22,0x0A,0x30,0x0A,
0x3E,0x0A,0x4C,0x0A,0x5A,0x0A,0x68,0x0A,0x76,0x0A,0x84,0x0A,0x92,0x0A,0xA0,0x0A,
0xAE,0x0A,0xBC,0x0A,0xCA,0x0A,0xD8,0x0A,0x00,0x01,0x20,0x20,0x20,0x20,0x20,0x20,
0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x01,0x00,
0x00,0x04,0x17,0x04,0x09,0x10,0x00,0x00,0x01,0x00,0x00,0x00,0x01,0x00,0x04,0x04,
0x18,0x04,0x09,0x10,0x00,0x00,0x01,0x00,0x00,0x00,0x01,0x00,0x08,0x04,0x19,0x04,
0x09,0x10,0x00,0x00,0x01,0x00,0x00,0x00,0x01,0x00,0x0C,0x04,0x1A,0x04,0x09,0x10,
0x00,0x00,0x01,0x00,0x00,0x00,0x01,0x00,0x10,0x04,0x07,0x04,0x09,0x10,0x00,0x00,
0x01,0x00,0x00,0x00,0x01,0x01,0x00,0x04,0xFF,0x07,0x09,0x10,0x00,0x00,0x01,0x00,
0x00,0x00,0x01,0x01,0x04,0x04,0x00,0x08,0x09,0x10,0x00,0x00,0x01,0x00,0x00,0x00,
0x01,0x01,0x08,0x04,0x01,0x08,0x09,0x10,0x00,0x00,0x01,0x00,0x00,0x00,0x01,0x01,
0x0C,0x04,0x02,0x08,0x09,0x10,0x00,0x00,0x01,0x00,0x00,0x00,0x01,0x01,0x10,0x04,
0xEF,0x07,0x09,0x10,0x00,0x00,0x01,0x00,0x00,0x00,0x01,0x02,0x00,0x04,0xE7,0x0B,
0x09,0x10,0x00,0x00,0x01,0x00,0x00,0x00,0x01,0x02,0x04,0x04,0xE8,0x0B,0x09,0x10,
0x00,0x00,0x01,0x00,0x00,0x00,0x01,0x02,0x08,0x04,0xE9,0x0B,0x09,0x10,0x00,0x00,
0x01,0x00,0x00,0x00,0x01,0x02,0x0C,0x04,0xEA,0x0B,0x09,0x10,0x00,0x00,0x01,0x00,
0x00,0x00,0x01,0x02,0x10,0x04,0xD7,0x0B,0x09,0x10,0x00,0x00,0x01,0x00,0x00,0x00,
0x01,0x03,0x00,0x04,0xCF,0x0F,0x09,0x10,0x00,0x00,0x01,0x00,0x00,0x00,0x01,0x03,
0x04,0x04,0xD0,0x0F,0x09,0x10,0x00,0x00,0x01,0x00,0x00,0x00,0x01,0x03,0x08,0x04,
0xD1,0x0F,0x09,0x10,0x00,0x00,0x01,0x00,0x00,0x00,0x01,0x03,0x0C,0x04,0xD2,0x0F,
0x09,0x10,0x00,0x00,0x01,0x00,0x00,0x00,0x01,0x03,0x10,0x04,0xBF,0x0F,0x09,0x10,
0x00,0x00,0x01,0x00,0x00,0x00,0x08,0xF7,0x0A,0xFA,0x0A,0xFD,0x0A,0x00,0x0B,0x03,
0x0B,0x06,0x0B,0x09,0x0B,0x0C,0x0B,0x0C,0x01,0x07,0x09,0x01,0x05,0x00,0x01,0x00,
0x01,0x01,0x01,0x02,0x01,0x02,0x03,0x01,0x03,0x04,0x01,0x04,0x14,0x01,0x08,0x23,
0x0B,0x3C,0x0C,0x04,0x00,0x0D,0x5F,0xD1,0xEB,0xF3,0xE6,0xE5,0xE1,0xED,0xFB,0xE9,
0x28,0x31,0x29,0x11,0x46,0x0B,0x5C,0x0B,0x6A,0x0B,0x78,0x0B,0x86,0x0B,0x94,0x0B,
0xA2,0x0B,0xB0,0x0B,0xBE,0x0B,0xCC,0x0B,0xDA,0x0B,0xE8,0x0B,0xF6,0x0B,0x04,0x0C,
0x12,0x0C,0x20,0x0C,0x2E,0x0C,0x00,0x01,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,
0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x01,0x00,0x01,0x04,
0x0D,0x04,0x09,0x10,0x00,0x00,0x01,0x00,0x00,0x00,0x01,0x00,0x06,0x04,0x0E,0x04,
0x09,0x10,0x00,0x00,0x01,0x00,0x00,0x00,0x01,0x00,0x0B,0x04,0x0F,0x04,0x09,0x10,
0x00,0x00,0x01,0x00,0x00,0x00,0x01,0x00,0x10,0x04,0x10,0x04,0x09,0x10,0x00,0x00,
0x01,0x00,0x00,0x00,0x01,0x01,0x01,0x04,0xF5,0x07,0x09,0x10,0x00,0x00,0x01,0x00,
0x00,0x00,0x01,0x01,0x06,0x04,0xF6,0x07,0x09,0x10,0x00,0x00,0x01,0x00,0x00,0x00,
0x01,0x01,0x0B,0x04,0xF7,0x07,0x09,0x10,0x00,0x00,0x01,0x00,0x00,0x00,0x01,0x01,
0x10,0x04,0xF8,0x07,0x09,0x10,0x00,0x00,0x01,0x00,0x00,0x00,0x01,0x02,0x01,0x04,
0xDD,0x0B,0x09,0x10,0x00,0x00,0x01,0x00,0x00,0x00,0x01,0x02,0x06,0x04,0xDE,0x0B,
0x09,0x10,0x00,0x00,0x01,0x00,0x00,0x00,0x01,0x02,0x0B,0x04,0xDF,0x0B,0x09,0x10,
0x00,0x00,0x01,0x00,0x00,0x00,0x01,0x02,0x10,0x04,0xE0,0x0B,0x09,0x10,0x00,0x00,
0x01,0x00,0x00,0x00,0x01,0x03,0x01,0x04,0xC5,0x0F,0x09,0x10,0x00,0x00,0x01,0x00,
0x00,0x00,0x01,0x03,0x06,0x04,0xC6,0x0F,0x09,0x10,0x00,0x00,0x01,0x00,0x00,0x00,
0x01,0x03,0x0B,0x04,0xC7,0x0F,0x09,0x10,0x00,0x00,0x01,0x00,0x00,0x00,0x01,0x03,
0x10,0x04,0xC8,0x0F,0x09,0x10,0x00,0x00,0x01,0x00,0x00,0x00,0x08,0x4D,0x0C,0x50,
0x0C,0x53,0x0C,0x56,0x0C,0x59,0x0C,0x5C,0x0C,0x5F,0x0C,0x62,0x0C,0x11,0x01,0x06,
0x09,0x01,0x05,0x00,0x01,0x00,0x01,0x01,0x01,0x02,0x01,0x02,0x03,0x01,0x03,0x04,
0x01,0x04,0x14,0x01,0x08,0x77,0x0C,0x94,0x0D,0x0E,0x00,0x0B,0x5B,0x46,0x31,0x5D,
0xD1,0xEF,0xF0,0xE0,0xE2,0xEA,0xE0,0x01,0x7A,0x0C,0x00,0x0E,0x31,0x20,0x20,0x43,
0x54,0x41,0x50,0x54,0x4F,0x42,0x95,0x9A,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,
0x32,0x20,0x20,0x85,0x41,0x50,0x41,0x4D,0x45,0x54,0x50,0x95,0x20,0x20,0x20,0x20,
0x20,0x20,0x20,0x20,0x33,0x20,0x20,0x43,0x4F,0x4F,0x8A,0x93,0x45,0x48,0x8E,0x99,
0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x34,0x20,0x20,0x43,0x54,0x41,0x54,0x8E,
0x43,0x54,0x8E,0x4B,0x41,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x35,0x20,0x20,0x48,
0x41,0x43,0x54,0x50,0x4F,0x9A,0x4B,0x41,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,
0x30,0x20,0x20,0x8B,0x41,0x54,0x41,0x2F,0x42,0x50,0x45,0x4D,0x99,0x2C,0x42,0x45,
0x50,0x43,0x8E,0x99,0x2B,0x20,0x20,0x43,0x8F,0x06,0x8C,0x45,0x8A,0x48,0x95,0x9A,
0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x46,0x31,0x20,0x43,0x85,0x50,0x41,0x42,
0x4B,0x41,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x46,0x32,0x20,0x43,
0x4F,0x8A,0x95,0x54,0x8E,0x99,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,
0x46,0x33,0x20,0x4B,0x4F,0x48,0x54,0x50,0x41,0x43,0x54,0x20,0x20,0x20,0x20,0x20,
0x20,0x20,0x20,0x20,0x43,0x54,0x50,0x45,0x8F,0x4B,0x8E,0x20,0x2D,0x3E,0x20,0x42,
0x95,0x8A,0x4F,0x50,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,
0x3C,0x2D,0x20,0x20,0x20,0x20,0x85,0x91,0x20,0x20,0x20,0x20,0x43,0x54,0x50,0x45,
0x8F,0x4B,0x8E,0x20,0x7C,0x5E,0x20,0x43,0x8B,0x42,0x8E,0x80,0x20,0x20,0x20,0x20,
0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x76,0x7C,0x20,0x4B,0x41,0x8B,0x50,0x41,
0x20,0x20,0x20,0x20,0x09,0xA7,0x0D,0xAA,0x0D,0xAD,0x0D,0xB0,0x0D,0xB3,0x0D,0xB6,
0x0D,0xB9,0x0D,0xBC,0x0D,0xBF,0x0D,0x0C,0x03,0x01,0x11,0x02,0x01,0x09,0x01,0x05,
0x00,0x01,0x00,0x01,0x01,0x01,0x02,0x01,0x02,0x03,0x01,0x03,0x04,0x01,0x04,0x1A,
0x01,0x06,0x00,0x00,0x00,0x00,0xCA,0x0D,0x00,0x00,

};
*/

// ������� �������� ������ ������� APD � ���, ��������� ��� ����������� �� LCD �������
uint8_t ApdPrjFontTable[] = {
//	00    01    02    03    04    05    06 �  07    08    09    0A    0B    0C    0D    0E    0F	
	0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0xA9, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
//	10    11    12    13    14    15    16    17    18    19    1A    1B    1C    1D    1E    1F	
	0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
//	20    21 !  22 .  23 #  24 $  25 %  26 &  27 '  28 (  29 )  2A *  2B +  2C ,  2D -  2E .  2F /
	0x20, 0x21, 0x2E, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F,
//	30 0  31 1  32 2  33 3  34 4  35 5  36 6  37 7  38 8  39 9  3A :  3B ;  3C <  3D =  3E >  3F ?	
	0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x30, 0x3E, 0x3F,
//	40 @  41 A  42 B  43 C  44 D  45 E  46 F  47 G  48 H  49 I  4A J  4B K  4C L  4D M  4E N  4F O
	0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F,
//	50 P  51 Q  52 R  53 S  54 T  55 U  56 V  57 W  58 X  59 Y  5A Z  5B [  5C \  5D ]  5E ^  5F _
	0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5A, 0x5B, 0x2F, 0x5D, 0x5E, 0x5F,
//	60 `  61 a  62 b  63 c  64 d  65 e  66 f  67 g  68 h  69 i  6A j  6B k  6C l  6D m  6E n  6F o	
	0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F,
//	70 p  71 q  72 r  73 s  74 t  75 u  76 v  77 w  78 x  79 y  7A z  7B {  7C |  7D }  7E ~  7F	
	0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7A, 0x28, 0xD1, 0x29, 0xE9, 0x20,
//	80 �  81    82    83    84    85 �  86    87 �  88    89    8A �  8B �  8C �  8D �  8E �  8F �	
	0xA1, 0x20, 0x20, 0x20, 0x20, 0xA8, 0x20, 0xAA, 0x20, 0x20, 0xA0, 0xE0, 0xA3, 0xA4, 0xA5, 0xA7,		
//	90 �  91 �  92 �  93 �  94 �  95 �  96 �  97 �  98 �  99 �  9A �  9B    9C    9D    9E    9F	
	0xE1, 0xAB, 0xAC, 0xE2, 0xC2, 0xAE, 0xC4, 0xAF, 0xB0, 0xB1, 0xA6, 0x20, 0x20, 0x20, 0x20, 0x20,
//	A0    A1    A2    A3    A4    A5    A6    A7    A8    A9    AA    AB    AC    AD    AE    AF	
	0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
//	B0    B1    B2    B3    B4    B5    B6    B7    B8    B9    BA    BB    BC    BD    BE    0B	
	0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
//	C0    C1    C2    C3    C4    C5    C6    C7    C8    C9    CA    CB    CC    CD    CE    CF	
	0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
//	D0    D1    D2    D3    D4    D5    D6    D7    D8    D9    DA    DB    DC    DD    DE    DF
	0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
//	E0    E1    E2    E3    E4    E5    E6    E7    E8    E9    EA    EB    EC    ED    EE    EF	
	0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
//	F0    F1    F2    F3    F4    F5    F6    F7    F8    F9    FA    FB    FC    FD    FE    FF SQ
	0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0xFF
};

/************************************************************************/
/*        ������ � ������������ � �������� ������������ �����           */
/************************************************************************/
// ���� � ����� ����������� �� ������ ��������� ������������
// "1" - ���������, "0" - ��������
// ��������� ������: ��� ����������� RTS ������ � ����� ������ RES
#define RADIO_RESET_ON	_delay_ms (100);\
						PORTD.DIR |= (1<<RTS);\
						PORTD.OUT |= (1<<RTS);\
						_delay_ms (50);\
						PORTD.DIR |= (1<<RES);\
						PORTD.OUT &= ~ (1<<RES);\
						_delay_ms (300);\
						PORTD.OUT |= (1<<RES);\
						PORTD.DIR &= ~(1<<RES);\
						_delay_ms (100)
						
// ���������� ������: ��� �������� RTS
#define RADIO_RESET_OFF	PORTD.OUT &= ~ (1<<RTS)

// ����� �������, ����������� �� �����������
#define RADIO_MESSAGE_LENGTH 4

// ������ ���������������� ������ �����������
const uint8_t RadCfg [11][10] = {
	{7, '0', '0', '#', 'I', '9', '6', 3, 0, 0},
	{7, '0', '0', '#', 'R', 'S', '0', 3, 0, 0},
	{7, '0', '0', '#', '8', 'N', '1', 3, 0, 0},
	{7, '0', '0', '#', 'E', '9', '6', 3, 0, 0},
	{7, '0', '0', '#', 'F', '0', '0', 3, 0, 0},
	{6, '0', '0', '#', 'P', 'F', 3, 0, 0, 0},
	{6, '0', '0', '#', 'N', '0', 3, 0, 0, 0},
		
	{9, '0', '0', '#', 'D', 'A', 'A', 'A', 'A', 3}, // ����� �����
	{9, '0', '0', '#', 'A', 'B', 'B', 'B', 'B', 3}, // ����� ��������

//	{9, '0', '0', '#', 'D', '0', '0', 'B', '1', 3}, // ����� �����
//	{9, '0', '0', '#', 'A', '0', '0', 'A', '1', 3}, // ����� ��������
	
	{9, '0', '0', '#', 'C', '0', '0', '0', '0', 3},	
	{6, '0', '0', '#', 'M', '0', 3, 0, 0, 0}
//	{5, '0', '0', '#', '?', 3, 0, 0, 0, 0},	
	};

// ����� ��������� ��������� � �����������
volatile uint8_t RadRecMesBuf[RADIO_MESSAGE_LENGTH] = {0x00, 0x00, 0x00, 0x00};
// �������������� ������� �������������� �� ��������� ������� '$'.
// ������������ ���� RadRecMesInProgress, ����������, ��� ��������������
// ���� ������ ���������.
// �����, ������������ ������� �������� �������� �� �������� ����� ���������.
// ����� ����� ������������ ���� ��������� ��������� RadRecMesReady.
// ���� RadRecMesInProgress ������������, ������������ � ���, ���
// ����� ��������� ����� ���������.

volatile uint8_t RadRecMesCharCnt = 0; // ������� �������� �������� � ��������� �� �����������
volatile uint8_t RadRecMesBufLen = sizeof (RadRecMesBuf) / sizeof (uint8_t); // ����� ��������� ���������
volatile uint8_t RadRecMesCharBuf = 0x00; // ����� ��������� �������
volatile uint8_t RadTransMesCharBuf = 0x00; // ����� ������������� ������� (�� ������������)
volatile uint8_t RadRecMesInProgress = 0x00; // ���� �������� ����� ������ ���������
volatile uint8_t RadRecEn = 0; // ���� ���������� ����� ��������� � �����������
volatile uint8_t RadRecMesReady = 0; // ���� ���������� ��������� ���������
volatile uint8_t RadRecMesCrcOk = 0; // ���� �������� �������� ����������� ����� ��������� ��������� 

// ����������
volatile uint8_t RadCfgAckMes[7] = {0x0D, 0x0A, 0x4F, 0x4B, 0x0A, 0x0D, 0x3E}; // ��������� ������������� �������� ������ ������� ����������������	
volatile uint8_t RadCfgAckMesCharCnt = 0; // ������� �������� ��������� ���������
volatile uint8_t RadCfgAckMesLen = 7; // ����� ��������� ���������
volatile uint8_t RadCfgAckMesReady = 1; // ���� ���������� ��������� ���������
	
volatile uint8_t RadCfgCmdAmount = 11; // ���������� ���������������� ������
volatile uint8_t RadCfgCmdCnt = 0x00; // ������� ���������������� ������
volatile uint8_t RadCfgCmdCharCnt = 0x00; // ������� �������� �������
volatile uint8_t RadCfgCmdAck = 0; // ���� ��������� ������������ �������

/************************************************************************/
/*                        �������� ����� � �� (ADP)                     */
/************************************************************************/
// ������ ��������� �� �����
typedef enum adp_adress {
	ADP_NULL = 0x00, // ��� ������
	ADP_TR1 = 0x51, // �� 1
	ADP_TR2 = 0x52, // �� 2
	ADP_TR3 = 0x53, // �� 3
	ADP_TR4 = 0x54, // �� 4
	ADP_OPK = 0x7D, // �������
	ADP_ATOOLS = 0x7F, // ATools
} adp_adress_t;

#define BROKEN_MES 5 // ���������� ���������� ���������

uint8_t EventReqRecNumber = 0;

// ���� ������� ��������� ADP
typedef enum adp_function {
	ADP_FUNC_TRANSMIT_PARAMS = 0x13, // �������� ���������
	ADP_FUNC_RECIEVE_PARAMS = 0x0B, // ������� ���������
} adp_function_t;

// ����� ���������� �����\�������� ���� UART �� ��������� ADP
volatile uint8_t AdpTransEn = 0x01; // ���������� ��������
volatile uint8_t AdpRecEn = 0x01; // ���������� �����

/************************************************************************/
/*								RS-232 - ADP                            */
/************************************************************************/

/************************************************************************/
/*								��������                                */
/************************************************************************/
volatile uint8_t AdpTransMesBuf[32]; // ����� ������������� ���������
volatile uint16_t AdpTransMesBody[16]; // ����� ���� �������
volatile uint8_t AdpTransMesCharBuf = 0; // ����� ������������� �������
volatile uint16_t AdpTransMesByteCnt = 0; // ������� ���������� ���� 
volatile uint16_t AdpTransMesLen = 0; // ����� ������������ �������

volatile uint8_t AdpTransMesQueue = 0; // ������� ������������ ��������� � ������� �� ���������� �����
volatile uint8_t AdpTransMesQueueCnt = 0; // ������� ������� ���������

/************************************************************************/
/*								��Ȩ�                                   */
/************************************************************************/
volatile uint8_t AdpRecNewChar = 0; // ���� ������ ��������� �������
volatile uint8_t AdpRecMesBuf[32]; // ����� ������������ ���������
volatile uint8_t AdpRecMesReady = 0; // ����: ���� ������� ��������
volatile uint8_t AdpRecMesCrcOk = 0; // ����: ������� ������ �������� ����������� �����
volatile uint8_t AdpRecMesCharBuf = 0; // ����� ������������ ������� �������
volatile uint16_t AdpRecMesByteCnt = 0; // ������� �������� ����
volatile uint16_t AdpRecMesLen = 0; // ����� ����������� �������

volatile uint8_t AdpRecMesTimeout = 1; // ���� �������� ������ ��������� �� ���������������

/************************************************************************/
/*                            �� ������                                 */
/************************************************************************/
// ���� ������ �� ���������� ������
volatile uint8_t LcdRefresh = 1;

// "��������" ��������
uint8_t LoadBar = 8;

// ������������ ������� ������
typedef enum opk_mode_enum {
	OPK_MODE_INTRO = 0, // ��������� �����
	OPK_MODE_NORMAL = 1, // ����� �������� ������
	OPK_MODE_SERVICE = 2, // ����� ���������� ������
	OPK_MODE_DEVELOPE = 3, // ����� ������ ������������
	OPK_MODE_TIMESET = 4, // ����� ��������� ������� � ����
	OPK_MODE_ERROR_DIAGNOSTIC = 5, // ����� ����������� ������
	OPK_MODE_REGISTER = 6, // ����� ����������� ������� �������
	OPK_MODE_SETTINGS = 7, // ����� ���������
} opk_mode_t;

// ����� ����������� ���������� ������
uint8_t OpkErrorRad = 0; // ������ ����������� 
uint8_t OpkErrorPrj = 0; // ������ ������� APD
uint8_t OpkErrorClock = 0; // ������ ����� ��������� �������
uint8_t OpkErrorEeprom = 0; // ������ ���������� EEPROM

typedef enum opk_error_enum {
	OPK_ERROR_RADIO_FAILED = 0,
	OPK_ERROR_APD_FAILED = 1,
	OPK_ERROR_RTC_FAILED = 2,
	OPK_ERROR_EEPROM_FAILED = 3,
} opk_error_t;

uint8_t OpkErrorScoreApd = 0;
uint8_t OpkErrorScoreRtc = 0;
uint8_t OpkErrorScoreEeprom = 0;

// �������� ����������� ������ ������ �����������:
uint8_t OpkErrorScoreRad = 0; // ������� ������: �������� ������
uint8_t OpkErrorScoreRadLimit = 10;
uint8_t OpkErrorScoreRadTry = 0;
uint8_t OpkErrorScoreRadTryLimit = 3; // ������� ������: ������� ��������������
uint8_t OpkErrorScoreRadGlobal = 0; // ������� ������: ������� ����������� ������
uint8_t OpkErrorScoreRadGlobalLimit = 70;

uint8_t OpkMode = OPK_MODE_INTRO; // ������� ����� ������	

// ������� ��������� ���������� ������ ��� �������� � ����������:
uint8_t ServiceModeToggleCnt = 0; // ������� � ��������� �����
uint8_t ServiceModeToggleCntLimit = 15;

uint8_t DevelopeModeToggleCnt = 0; // ������� ����� ������������
uint8_t DevelopeModeToggleCntLimit = 30;

uint8_t ErrorModeToggleCnt = 0; // ������� ����� ������������
uint8_t ErrorModeToggleCntLimit = 15;

// ������� ������:
uint8_t ButtonFirst = 0; // ������ 1
uint8_t ButtonSecond = 0; // ������ 2
// ���� ���������:
// ����� ����� ������� ���� ����������, ����������
// ��������� ������ ������� ������
uint8_t ButtonRelease = 0;

// ��� �������� �� ������� ��� �������� � ������� ��������� �������
uint8_t TimesetModeAction = 255;
uint8_t TimesetModeCursorPosition = 0; // ������� ������� �� ������ ��������� �������

// ��� �������� �� ������� ��� �������� � ����������� �������
uint8_t RegisterModeAction = 255;
uint16_t RegisterModeCurMes = 0; // ������� ������������ ���������
uint8_t RegisterModeShowOnce = 1; // ������ ��������� ������

// ���� ���������� ����������� �����
uint8_t ClockShowEn = 0;
// ���� ���������� ����������� ����
uint8_t DataShowEn = 0;

// ���������� "�������" �������:
// � ��� ������������ "�����" ��������, ���������� �� ��������� RTC
uint8_t TimeGetSeconds = 0x00;
uint8_t TimeGetMinutes = 0x00;
uint8_t TimeGetHours = 0x00;
uint8_t TimeGetDay = 0x00;
uint8_t TimeGetDate = 0x00;
uint8_t TimeGetMonth = 0x00;
uint8_t TimeGetYear = 0x00;

// ���������� ��������������� ��������� �������:
// �������� �������� �����, ����� � �.�.
uint8_t TimeSetSeconds = 0x00;
uint8_t TimeSetMinutes = 0x00;
uint8_t TimeSetHours = 0x0C;
uint8_t TimeSetDay = 0x01;
uint8_t TimeSetDate = 0x01;
uint8_t TimeSetMonth = 0x01;
uint8_t TimeSetYear = 0x00;

// ���������� ������ ��������� �������
// ����, ����������� ������� "������" �������� ������� ��
// ���� ������ ������ ������ ��������� �������.
// ������������ ��� ������ �� ����� ������
uint8_t TimesetModeShowOnce = 1;

// � ���� ���������� ���������� �������� �������� �����, ����� � �.�.
uint8_t TimesetModeShowSeconds = 0x00;
uint8_t TimesetModeShowMinutes = 0x00;
uint8_t TimesetModeShowHours = 0x00;
uint8_t TimesetModeShowDay = 0x00;
uint8_t TimesetModeShowDate = 0x00;
uint8_t TimesetModeShowMonth = 0x00;
uint8_t TimesetModeShowYear = 0x00;
// ���� ������ ����� ��� "�������" ����� � ������ ��������� �������:
// ������������ ��� ������ �� ����� ��������� ������ � �� ������
uint8_t TimesetModeClockFailed = 0x00;

// ���������� ������ ���������
uint8_t SettingsModeShowOnce = 1;
uint8_t MesDelay = 0;
uint8_t SettingsModeAction = 255;

/************************************************************************/
/*                 ��������� �������                                    */
/************************************************************************/

// �������� CRC
uint8_t		Crc8 (uint8_t *PcBlock, uint8_t Len);
uint16_t	Crc16 (uint8_t *PcBlock, uint16_t Len);

// ���������� �������
void		Timer1msInit (void);
void		TimerRs232Init (void);
void		TimerRs232Start (void);
void		TimerRs232Stop (void);

// ������ � ����� TWI (PORTC)
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

// ������ � ����� TWI (PORTE)
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

// ������ � ����������� RTC
uint8_t		RtcSetTime (void);
uint8_t		RtcGetTime (void);

// ������ � ������� EEPROM
uint8_t EepromWriteBlock (uint16_t Adr, uint8_t * Data, uint8_t DataBlockSize);
uint8_t EepromReadBlock (uint16_t Adr, uint8_t * Data, uint8_t DataBlockSize);
// Direction:
// 0 - ������,
// 1 - ������
uint8_t EepromBuildControlCode (uint16_t Adr, uint8_t Direction);

// ��������� �������
uint8_t		TimeSetManual (uint8_t Hours, uint8_t Minutes, uint8_t Seconds, uint8_t Day, uint8_t Date, uint8_t Month, uint8_t Year);

// ����������� ����� � ���� �� ������
void		ClockShowOn (uint8_t String, uint8_t Column);
void		DataShowOn (uint8_t String, uint8_t Column);

// ������ �������
void		ScreenModeIntro (void); // �������� ��� �������
void		ScreenModeService (void); // ��������� �����
void		ScreenModeNormalAction (uint8_t Action); // ������� ����� (�������������� ������� APD)
void		ScreenModeNormal (void); // ������� ����� (��������� ������ ������� APD)
void		ScreenModeDevelope (void); // ����� ����������
uint8_t		ScreenModeTimeset (uint8_t Action); // ��������� �������
void		ScreenModeErrorDiagnostic (void); // ����������� ������
void		ScreenModeRegister (uint8_t Action); // ������ �������
void		ScreenModeSettings (uint8_t Action); // ���������

// ������ � ������ RS-232 �� ��������� ADP
void		AdpTransStart (void);
void		AdpTransContinue (void);
uint16_t	AdpTransMesBuild (uint8_t RecAdr, uint8_t TransAdr, uint8_t CmdCode, uint8_t ClarkAdr, uint16_t AmountOfBytes, uint16_t *CmdBody);
uint8_t		AdpRec (void);
void		AdpFunctionRecParams (void);

// ���������� ������ ��� �������
void		AdrInFocus (void); // ������� ���������� ������� ��� ������� � ������� ������
void		SortAcending (uint16_t * Array, uint8_t Size); // ��������� ������� �� �����������
void		PrepareToRequest (void); // ���������� � �������: ����������� � ��������� ������
void		RequestAdrSpace (void); // ���������� ���������� ����� �������������� ��������	

/************************************************************************/
/*                 �������� �������                                     */
/************************************************************************/

/************************************************************************/
/*                            �������� CRC                              */
/************************************************************************/
/*
  Name  : Crc-8
  Poly  : 0x31    x^8 + x^5 + x^4 + 1
  Init  : 0xFF
  Revert: false
  XorOut: 0x00
  Check : 0xF7 ("123456789")
  MaxLen: 15 ���� (127 ���) - �����������
    ���������, �������, ������� � ���� �������� ������
*/
const uint8_t Crc8Table[256] = {
    0x00, 0x31, 0x62, 0x53, 0xC4, 0xF5, 0xA6, 0x97,
    0xB9, 0x88, 0xDB, 0xEA, 0x7D, 0x4C, 0x1F, 0x2E,
    0x43, 0x72, 0x21, 0x10, 0x87, 0xB6, 0xE5, 0xD4,
    0xFA, 0xCB, 0x98, 0xA9, 0x3E, 0x0F, 0x5C, 0x6D,
    0x86, 0xB7, 0xE4, 0xD5, 0x42, 0x73, 0x20, 0x11,
    0x3F, 0x0E, 0x5D, 0x6C, 0xFB, 0xCA, 0x99, 0xA8,
    0xC5, 0xF4, 0xA7, 0x96, 0x01, 0x30, 0x63, 0x52,
    0x7C, 0x4D, 0x1E, 0x2F, 0xB8, 0x89, 0xDA, 0xEB,
    0x3D, 0x0C, 0x5F, 0x6E, 0xF9, 0xC8, 0x9B, 0xAA,
    0x84, 0xB5, 0xE6, 0xD7, 0x40, 0x71, 0x22, 0x13,
    0x7E, 0x4F, 0x1C, 0x2D, 0xBA, 0x8B, 0xD8, 0xE9,
    0xC7, 0xF6, 0xA5, 0x94, 0x03, 0x32, 0x61, 0x50,
    0xBB, 0x8A, 0xD9, 0xE8, 0x7F, 0x4E, 0x1D, 0x2C,
    0x02, 0x33, 0x60, 0x51, 0xC6, 0xF7, 0xA4, 0x95,
    0xF8, 0xC9, 0x9A, 0xAB, 0x3C, 0x0D, 0x5E, 0x6F,
    0x41, 0x70, 0x23, 0x12, 0x85, 0xB4, 0xE7, 0xD6,
    0x7A, 0x4B, 0x18, 0x29, 0xBE, 0x8F, 0xDC, 0xED,
    0xC3, 0xF2, 0xA1, 0x90, 0x07, 0x36, 0x65, 0x54,
    0x39, 0x08, 0x5B, 0x6A, 0xFD, 0xCC, 0x9F, 0xAE,
    0x80, 0xB1, 0xE2, 0xD3, 0x44, 0x75, 0x26, 0x17,
    0xFC, 0xCD, 0x9E, 0xAF, 0x38, 0x09, 0x5A, 0x6B,
    0x45, 0x74, 0x27, 0x16, 0x81, 0xB0, 0xE3, 0xD2,
    0xBF, 0x8E, 0xDD, 0xEC, 0x7B, 0x4A, 0x19, 0x28,
    0x06, 0x37, 0x64, 0x55, 0xC2, 0xF3, 0xA0, 0x91,
    0x47, 0x76, 0x25, 0x14, 0x83, 0xB2, 0xE1, 0xD0,
    0xFE, 0xCF, 0x9C, 0xAD, 0x3A, 0x0B, 0x58, 0x69,
    0x04, 0x35, 0x66, 0x57, 0xC0, 0xF1, 0xA2, 0x93,
    0xBD, 0x8C, 0xDF, 0xEE, 0x79, 0x48, 0x1B, 0x2A,
    0xC1, 0xF0, 0xA3, 0x92, 0x05, 0x34, 0x67, 0x56,
    0x78, 0x49, 0x1A, 0x2B, 0xBC, 0x8D, 0xDE, 0xEF,
    0x82, 0xB3, 0xE0, 0xD1, 0x46, 0x77, 0x24, 0x15,
    0x3B, 0x0A, 0x59, 0x68, 0xFF, 0xCE, 0x9D, 0xAC
};

uint8_t Crc8 (uint8_t *PcBlock, uint8_t Len) {	
	uint8_t Crc = 0xFF;   
	while (Len--) {
		Crc = Crc8Table[Crc ^ *PcBlock++];
	}		
    return Crc;
}

/*
  Name  : Crc-16
  Poly  : 0x8005    x^16 + x^15 + x^2 + 1
  Init  : 0xFFFF
  Revert: true
  XorOut: 0x0000
  Check : 0x4B37 ("123456789")
  MaxLen: 4095 ���� (32767 ���) - �����������
    ���������, �������, ������� � ���� �������� ������
*/
const uint16_t Crc16Table[256] = {
    0x0000, 0xC0C1, 0xC181, 0x0140, 0xC301, 0x03C0, 0x0280, 0xC241,
    0xC601, 0x06C0, 0x0780, 0xC741, 0x0500, 0xC5C1, 0xC481, 0x0440,
    0xCC01, 0x0CC0, 0x0D80, 0xCD41, 0x0F00, 0xCFC1, 0xCE81, 0x0E40,
    0x0A00, 0xCAC1, 0xCB81, 0x0B40, 0xC901, 0x09C0, 0x0880, 0xC841,
    0xD801, 0x18C0, 0x1980, 0xD941, 0x1B00, 0xDBC1, 0xDA81, 0x1A40,
    0x1E00, 0xDEC1, 0xDF81, 0x1F40, 0xDD01, 0x1DC0, 0x1C80, 0xDC41,
    0x1400, 0xD4C1, 0xD581, 0x1540, 0xD701, 0x17C0, 0x1680, 0xD641,
    0xD201, 0x12C0, 0x1380, 0xD341, 0x1100, 0xD1C1, 0xD081, 0x1040,
    0xF001, 0x30C0, 0x3180, 0xF141, 0x3300, 0xF3C1, 0xF281, 0x3240,
    0x3600, 0xF6C1, 0xF781, 0x3740, 0xF501, 0x35C0, 0x3480, 0xF441,
    0x3C00, 0xFCC1, 0xFD81, 0x3D40, 0xFF01, 0x3FC0, 0x3E80, 0xFE41,
    0xFA01, 0x3AC0, 0x3B80, 0xFB41, 0x3900, 0xF9C1, 0xF881, 0x3840,
    0x2800, 0xE8C1, 0xE981, 0x2940, 0xEB01, 0x2BC0, 0x2A80, 0xEA41,
    0xEE01, 0x2EC0, 0x2F80, 0xEF41, 0x2D00, 0xEDC1, 0xEC81, 0x2C40,
    0xE401, 0x24C0, 0x2580, 0xE541, 0x2700, 0xE7C1, 0xE681, 0x2640,
    0x2200, 0xE2C1, 0xE381, 0x2340, 0xE101, 0x21C0, 0x2080, 0xE041,
    0xA001, 0x60C0, 0x6180, 0xA141, 0x6300, 0xA3C1, 0xA281, 0x6240,
    0x6600, 0xA6C1, 0xA781, 0x6740, 0xA501, 0x65C0, 0x6480, 0xA441,
    0x6C00, 0xACC1, 0xAD81, 0x6D40, 0xAF01, 0x6FC0, 0x6E80, 0xAE41,
    0xAA01, 0x6AC0, 0x6B80, 0xAB41, 0x6900, 0xA9C1, 0xA881, 0x6840,
    0x7800, 0xB8C1, 0xB981, 0x7940, 0xBB01, 0x7BC0, 0x7A80, 0xBA41,
    0xBE01, 0x7EC0, 0x7F80, 0xBF41, 0x7D00, 0xBDC1, 0xBC81, 0x7C40,
    0xB401, 0x74C0, 0x7580, 0xB541, 0x7700, 0xB7C1, 0xB681, 0x7640,
    0x7200, 0xB2C1, 0xB381, 0x7340, 0xB101, 0x71C0, 0x7080, 0xB041,
    0x5000, 0x90C1, 0x9181, 0x5140, 0x9301, 0x53C0, 0x5280, 0x9241,
    0x9601, 0x56C0, 0x5780, 0x9741, 0x5500, 0x95C1, 0x9481, 0x5440,
    0x9C01, 0x5CC0, 0x5D80, 0x9D41, 0x5F00, 0x9FC1, 0x9E81, 0x5E40,
    0x5A00, 0x9AC1, 0x9B81, 0x5B40, 0x9901, 0x59C0, 0x5880, 0x9841,
    0x8801, 0x48C0, 0x4980, 0x8941, 0x4B00, 0x8BC1, 0x8A81, 0x4A40,
    0x4E00, 0x8EC1, 0x8F81, 0x4F40, 0x8D01, 0x4DC0, 0x4C80, 0x8C41,
    0x4400, 0x84C1, 0x8581, 0x4540, 0x8701, 0x47C0, 0x4680, 0x8641,
    0x8201, 0x42C0, 0x4380, 0x8341, 0x4100, 0x81C1, 0x8081, 0x4040 
};

uint16_t Crc16 (uint8_t *PcBlock, uint16_t Len) {
	uint16_t Crc = 0xFFFF; 
	while (Len--) {
		Crc = (Crc >> 8) ^ Crc16Table[(Crc & 0xFF) ^ *PcBlock++];
	}	
	return Crc;
}

/************************************************************************/
/*			��������������� ������� ���������� ������ � �������         */
/************************************************************************/

// ������� ���������� ������� ��� ������� � ������� ������
void AdrInFocus (void) {
	uint8_t IdA = 0;
	uint8_t Cnt = 0;
	
	for (IdA = 0; IdA <= (APD_MAX_FIELDS - 1); IdA++) {
		FocusDataAdr[IdA] = 0;
		FocusData[IdA] = 0;
		FocusDataCntNext[IdA] = 0;
		FocusDataCntPrev[IdA] = 0;
		FocusDataStatus[IdA] = 0;
	}
	FocusDataAmount = 0;
	
	if (ApdFrameDataFromPlcAmount[ApdFrameCur]) {
		for (IdA = 0; IdA <= (ApdFrameDataFromPlcAmount[ApdFrameCur] - 1); IdA++) {
			if ((ApdFrameDataFromPlcString[ApdFrameCur][IdA] >= ApdFrameDisplayShift) && (ApdFrameDataFromPlcString[ApdFrameCur][IdA] <= (ApdFrameDisplayShift + 3))) {
				FocusDataAdr[Cnt] = ApdFrameDataFromPlcAdr[ApdFrameCur][IdA];
				Cnt++;
			}
		}
		FocusDataAmount = Cnt;
	} else {
		FocusDataAmount = 0;
	}		
}

// ��������� ������� �� �����������
void SortAcending (uint16_t * Array, uint8_t Size) {
	uint8_t IdA = 0;
	uint8_t IdB = 0;
	uint8_t Add = 0;
	uint16_t Buf = 0;
//	Size /= sizeof (uint16_t);
	
	if (Size) {
		for (IdA = 0; IdA <= (Size - 1); IdA++) {
			for (IdB = 0; IdB <= (Size - 2 /* - Add */); IdB++) {
				if (*(Array + IdB) > *(Array + IdB + 1)) {
					Buf = *(Array + IdB + 1);
					*(Array + IdB + 1) = *(Array + IdB);
					*(Array + IdB) = Buf;
					Buf = 0;
				}
			}
			Add++;
		}
	}		
}

// ���������� � �������: ����������� � ��������� ������
void PrepareToRequest (void) {
	uint16_t Benefit = 0;
	uint16_t Harm = 0;
	uint8_t RegBenefit = 1;
	uint8_t RegHarm = 0;
	uint8_t IdA = 0;
	uint8_t BlockState = 0;
	uint8_t LastBlock = 0;
	
	uint8_t RegCountPrev = 0;
	uint8_t RegCountCur = 0;
	
	uint16_t BlockAdr = 0;
	uint8_t BlockCnt = 0;
	
	if (FocusDataAmount > 1) {
		while (IdA <= (FocusDataAmount - 1)) {
			switch (BlockState) {
	// ������			
				case 0:
					BlockAdr = FocusDataAdr[IdA];
					RegBenefit = 1;
					RegHarm = 0;
					RegCountCur = RegBenefit + RegHarm;
					if (LastBlock) {
						LastBlock = 0;
						RegCountPrev = RegBenefit + RegHarm;
						BlockState = 2;
					} else {
						BlockState = 1;
						IdA++;				
					}

					break;
	// ��������				
				case 1:
					RegBenefit++;
					RegHarm += RequestAdrSpaceArray[IdA];
					RegCountPrev = RegCountCur;
					RegCountCur = RegBenefit + RegHarm;
					Benefit = 8 * (RegBenefit - 1);
					Harm = 2 * RegHarm;
					if (Harm > Benefit) {
						if (IdA == (FocusDataAmount - 1)) {
							LastBlock = 1;
							BlockState = 2;
						} else {
							BlockState = 2;
						}
					
					} else {
						if (IdA == (FocusDataAmount - 1)) {
							RegCountPrev = RegCountCur;
							BlockState = 2;
						} else {
							BlockState = 1;
							IdA++;
						}
					}
					break;
	// �����				
				case 2:
					BlockRequest[BlockCnt][0] = BlockAdr;
					BlockRequest[BlockCnt][1] = 2*RegCountPrev;
					BlockCnt++;
					BlockState = 0;
					break;				
				
				default:
					break;		
			}
		}
		BlockAmount = BlockCnt;
	} else if (FocusDataAmount == 0) {
		BlockAmount = 0;
	} else if (FocusDataAmount == 1) {
		BlockRequest[0][0] = FocusDataAdr[0];
		BlockRequest[0][1] = 2;		
		BlockAmount = 1;
	}		
}

// ���������� ���������� ����� �������������� ��������	
void RequestAdrSpace (void) {
	uint8_t Id8 = 0;
	
	if (FocusDataAmount) {	
		for (Id8 = 1; Id8 <= (FocusDataAmount - 1); Id8++) {
			RequestAdrSpaceArray[Id8] = FocusDataAdr[Id8] - FocusDataAdr[Id8 - 1] - 1;
		}
	}		
}

/************************************************************************/
/*             ������ � ����� TWI (PORTE - EEPROM)                      */
/************************************************************************/

void E_TwiInit (void) {	
	TWIE.MASTER.CTRLA |= TWI_MASTER_INTLVL_OFF_gc;
//	TWIE.MASTER.CTRLB |= TWI_MASTER_TIMEOUT_50US_gc;
	TWIE.MASTER.BAUD = TWI_BAUD(F_SYS, F_TWI_EEPROM);	
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
/*               ������ � ����� TWI (PORTC - RTC)                       */
/************************************************************************/

void C_TwiInit (void) {	
	TWIC.MASTER.CTRLA |= TWI_MASTER_INTLVL_OFF_gc;
//	TWIC.MASTER.CTRLB |= TWI_MASTER_TIMEOUT_50US_gc;
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

/************************************************************************/
/*               ������ � ������ ��������� ������� RTC                  */
/************************************************************************/

/************************************************************************/
/*                      ������ ��������� �������                        */
/************************************************************************/
uint8_t TimeSetManual (uint8_t Hours, uint8_t Minutes, uint8_t Seconds, uint8_t Day, uint8_t Date, uint8_t Month, uint8_t Year) {	
	C_TwiEn (); // ��������� ������ TWI	
	C_TwiBusSetIdle (); // ������� ���� � ��������� IDLE

	if (C_TwiBusWaitForIdle ()) {	// �������� ��������� ���� IDLE		
		C_TwiTransactionStart (0); // ����� � ������ ������
	} else {
		C_TwiCmdStop (); // �������� ������� STOP	
		C_TwiDisable (); // ���������� ������ TWI		
		return 0; // ������� ���������� 0, ���� �������� �������� �������� �����
	}

	if (C_TwiInterruptWaitForWif ()) { // �������� ����� ������ ����� WIF
		if (C_TwiCheckAck ()) {	// ��������, ��� �� ����� ��� ������� ACK			
			C_TwiDataPut (0); // ������ �������� �������� ������� RTC
		} else {
			// ���� ������� ����� NACK
			C_TwiCmdStop (); // �������� ������� STOP	
			C_TwiDisable (); // ���������� ������ TWI
			// lcd_write_symbol (0, 3, '*');
			return 0; // ������� ���������� 0, ���� �������� �������� �������� �����
		}			
	} else {
		C_TwiCmdStop (); // �������� ������� STOP	
		C_TwiDisable (); // ���������� ������ TWI				
		return 0; // ������� ���������� 0, ���� �������� �������� �������� �����
	}

	if (C_TwiInterruptWaitForWif ()) { // �������� ����� ������ ����� WIF		
		if (C_TwiCheckAck ()) {	// ��������, ��� �� ����� ��� ������� ACK					
			C_TwiDataPut (((Seconds/10)<<4) | (Seconds%10)); // ������ ����� �� �������� (�������)
		} else {
			// ���� ������� ����� NACK
			C_TwiCmdStop (); // �������� ������� STOP	
			C_TwiDisable (); // ���������� ������ TWI
			// lcd_write_symbol (0, 3, '*');
			return 0; // ������� ���������� 0, ���� �������� �������� �������� �����
		}			
	} else {
		C_TwiCmdStop (); // �������� ������� STOP	
		C_TwiDisable (); // ���������� ������ TWI				
		return 0; // ������� ���������� 0, ���� �������� �������� �������� �����
	}
	
	if (C_TwiInterruptWaitForWif ()) { // �������� ����� ������ ����� WIF		
		if (C_TwiCheckAck ()) {	// ��������, ��� �� ����� ��� ������� ACK					
			C_TwiDataPut (((Minutes/10)<<4) | (Minutes%10)); // ������ ����� �� �������� (������)
		} else {
			// ���� ������� ����� NACK
			C_TwiCmdStop (); // �������� ������� STOP	
			C_TwiDisable (); // ���������� ������ TWI
			// lcd_write_symbol (0, 3, '*');
			return 0; // ������� ���������� 0, ���� �������� �������� �������� �����
		}			
	} else {
		C_TwiCmdStop (); // �������� ������� STOP	
		C_TwiDisable (); // ���������� ������ TWI				
		return 0; // ������� ���������� 0, ���� �������� �������� �������� �����
	}
	
	if (C_TwiInterruptWaitForWif ()) { // �������� ����� ������ ����� WIF		
		if (C_TwiCheckAck ()) {	// ��������, ��� �� ����� ��� ������� ACK					
			C_TwiDataPut ((0x00 | (Hours/10)<<4) | (Hours%10)); // ������ ����� �� �������� (����)
		} else {
			// ���� ������� ����� NACK
			C_TwiCmdStop (); // �������� ������� STOP	
			C_TwiDisable (); // ���������� ������ TWI
			// lcd_write_symbol (0, 3, '*');
			return 0; // ������� ���������� 0, ���� �������� �������� �������� �����
		}			
	} else {
		C_TwiCmdStop (); // �������� ������� STOP	
		C_TwiDisable (); // ���������� ������ TWI				
		return 0; // ������� ���������� 0, ���� �������� �������� �������� �����
	}
	
	if (C_TwiInterruptWaitForWif ()) { // �������� ����� ������ ����� WIF		
		if (C_TwiCheckAck ()) {	// ��������, ��� �� ����� ��� ������� ACK					
			C_TwiDataPut (0x00 | Day); // ������ ����� �� �������� (����)
		} else {
			// ���� ������� ����� NACK
			C_TwiCmdStop (); // �������� ������� STOP	
			C_TwiDisable (); // ���������� ������ TWI
			// lcd_write_symbol (0, 3, '*');
			return 0; // ������� ���������� 0, ���� �������� �������� �������� �����
		}			
	} else {
		C_TwiCmdStop (); // �������� ������� STOP	
		C_TwiDisable (); // ���������� ������ TWI				
		return 0; // ������� ���������� 0, ���� �������� �������� �������� �����
	}
	
	if (C_TwiInterruptWaitForWif ()) { // �������� ����� ������ ����� WIF		
		if (C_TwiCheckAck ()) {	// ��������, ��� �� ����� ��� ������� ACK					
			C_TwiDataPut ((0x00 | (Date/10)<<4) | (Date%10)); // ������ ����� �� �������� (����)
		} else {
			// ���� ������� ����� NACK
			C_TwiCmdStop (); // �������� ������� STOP	
			C_TwiDisable (); // ���������� ������ TWI
			// lcd_write_symbol (0, 3, '*');
			return 0; // ������� ���������� 0, ���� �������� �������� �������� �����
		}			
	} else {
		C_TwiCmdStop (); // �������� ������� STOP	
		C_TwiDisable (); // ���������� ������ TWI				
		return 0; // ������� ���������� 0, ���� �������� �������� �������� �����
	}
	
	if (C_TwiInterruptWaitForWif ()) { // �������� ����� ������ ����� WIF		
		if (C_TwiCheckAck ()) {	// ��������, ��� �� ����� ��� ������� ACK					
			C_TwiDataPut ((0x00 | (Month/10)<<4) | (Month%10)); // ������ ����� �� �������� (�����)
		} else {
			// ���� ������� ����� NACK
			C_TwiCmdStop (); // �������� ������� STOP	
			C_TwiDisable (); // ���������� ������ TWI
			// lcd_write_symbol (0, 3, '*');
			return 0; // ������� ���������� 0, ���� �������� �������� �������� �����
		}			
	} else {
		C_TwiCmdStop (); // �������� ������� STOP	
		C_TwiDisable (); // ���������� ������ TWI				
		return 0; // ������� ���������� 0, ���� �������� �������� �������� �����
	}
	
	if (C_TwiInterruptWaitForWif ()) { // �������� ����� ������ ����� WIF		
		if (C_TwiCheckAck ()) {	// ��������, ��� �� ����� ��� ������� ACK					
			C_TwiDataPut ((0x00 | (Year/10)<<4) | (Year%10)); // ������ ����� �� �������� (���)
		} else {
			// ���� ������� ����� NACK
			C_TwiCmdStop (); // �������� ������� STOP	
			C_TwiDisable (); // ���������� ������ TWI
			// lcd_write_symbol (0, 3, '*');
			return 0; // ������� ���������� 0, ���� �������� �������� �������� �����
		}			
	} else {
		C_TwiCmdStop (); // �������� ������� STOP	
		C_TwiDisable (); // ���������� ������ TWI				
		return 0; // ������� ���������� 0, ���� �������� �������� �������� �����
	}
	
	if (C_TwiInterruptWaitForWif ()) { // �������� ����� ������ ����� WIF		
		if (C_TwiCheckAck ()) {	// ��������, ��� �� ����� ��� ������� ACK					
		
		} else {
			// ���� ������� ����� NACK
			C_TwiCmdStop (); // �������� ������� STOP	
			C_TwiDisable (); // ���������� ������ TWI
			// lcd_write_symbol (0, 3, '*');
			return 0; // ������� ���������� 0, ���� �������� �������� �������� �����
		}			
	} else {
		C_TwiCmdStop (); // �������� ������� STOP	
		C_TwiDisable (); // ���������� ������ TWI				
		return 0; // ������� ���������� 0, ���� �������� �������� �������� �����
	}
			
	C_TwiCmdStop (); // �������� ������� STOP	
	C_TwiDisable (); // ���������� ������ TWI	
	return 1; // ������� ���������� 1, ���� ������ ���� ��������� ������� 
}

/************************************************************************/
/*                          ��������� �������                           */
/************************************************************************/
uint8_t RtcSetTime (void) {	
	C_TwiEn (); // ��������� ������ TWI	
	C_TwiBusSetIdle (); // ������� ���� � ��������� IDLE

	if (C_TwiBusWaitForIdle ()) {	// �������� ��������� ���� IDLE		
		C_TwiTransactionStart (0); // ����� � ������ ������
	} else {
		C_TwiCmdStop (); // �������� ������� STOP	
		C_TwiDisable (); // ���������� ������ TWI				
		return 0; // ������� ���������� 0, ���� �������� �������� �������� �����
	}

	if (C_TwiInterruptWaitForWif ()) { // �������� ����� ������ ����� WIF
		if (C_TwiCheckAck ()) {	// ��������, ��� �� ����� ��� ������� ACK			
			C_TwiDataPut (0); // ������ �������� �������� ������� RTC
		} else {
			// ���� ������� ����� NACK
			C_TwiCmdStop (); // �������� ������� STOP	
			C_TwiDisable (); // ���������� ������ TWI
			// lcd_write_symbol (0, 3, '*');
			return 0; // ������� ���������� 0, ���� �������� �������� �������� �����
		}			
	} else {
		C_TwiCmdStop (); // �������� ������� STOP	
		C_TwiDisable (); // ���������� ������ TWI				
		return 0; // ������� ���������� 0, ���� �������� �������� �������� �����
	}

	if (C_TwiInterruptWaitForWif ()) { // �������� ����� ������ ����� WIF		
		if (C_TwiCheckAck ()) {	// ��������, ��� �� ����� ��� ������� ACK					
			C_TwiDataPut ((0x00 | (TimeSetSeconds/10)<<4) | (TimeSetSeconds%10)); // ������ ����� �� �������� (�������)
		} else {
			// ���� ������� ����� NACK
			C_TwiCmdStop (); // �������� ������� STOP	
			C_TwiDisable (); // ���������� ������ TWI
			// lcd_write_symbol (0, 3, '*');
			return 0; // ������� ���������� 0, ���� �������� �������� �������� �����
		}			
	} else {
		C_TwiCmdStop (); // �������� ������� STOP	
		C_TwiDisable (); // ���������� ������ TWI				
		return 0; // ������� ���������� 0, ���� �������� �������� �������� �����
	}
	
	if (C_TwiInterruptWaitForWif ()) { // �������� ����� ������ ����� WIF		
		if (C_TwiCheckAck ()) {	// ��������, ��� �� ����� ��� ������� ACK					
			C_TwiDataPut ((0x00 | (TimeSetMinutes/10)<<4) | (TimeSetMinutes%10)); // ������ ����� �� �������� (������)
		} else {
			// ���� ������� ����� NACK
			C_TwiCmdStop (); // �������� ������� STOP	
			C_TwiDisable (); // ���������� ������ TWI
			// lcd_write_symbol (0, 3, '*');
			return 0; // ������� ���������� 0, ���� �������� �������� �������� �����
		}			
	} else {
		C_TwiCmdStop (); // �������� ������� STOP	
		C_TwiDisable (); // ���������� ������ TWI				
		return 0; // ������� ���������� 0, ���� �������� �������� �������� �����
	}
	
	if (C_TwiInterruptWaitForWif ()) { // �������� ����� ������ ����� WIF		
		if (C_TwiCheckAck ()) {	// ��������, ��� �� ����� ��� ������� ACK					
			C_TwiDataPut ((0x00 | (TimeSetHours/10)<<4) | (TimeSetHours%10)); // ������ ����� �� �������� (����)
		} else {
			// ���� ������� ����� NACK
			C_TwiCmdStop (); // �������� ������� STOP	
			C_TwiDisable (); // ���������� ������ TWI
			// lcd_write_symbol (0, 3, '*');
			return 0; // ������� ���������� 0, ���� �������� �������� �������� �����
		}			
	} else {
		C_TwiCmdStop (); // �������� ������� STOP	
		C_TwiDisable (); // ���������� ������ TWI				
		return 0; // ������� ���������� 0, ���� �������� �������� �������� �����
	}
	
		if (C_TwiInterruptWaitForWif ()) { // �������� ����� ������ ����� WIF		
		if (C_TwiCheckAck ()) {	// ��������, ��� �� ����� ��� ������� ACK					
			C_TwiDataPut (0x00 | TimeSetDay); // ������ ����� �� �������� (����)
		} else {
			// ���� ������� ����� NACK
			C_TwiCmdStop (); // �������� ������� STOP	
			C_TwiDisable (); // ���������� ������ TWI
			// lcd_write_symbol (0, 3, '*');
			return 0; // ������� ���������� 0, ���� �������� �������� �������� �����
		}			
	} else {
		C_TwiCmdStop (); // �������� ������� STOP	
		C_TwiDisable (); // ���������� ������ TWI				
		return 0; // ������� ���������� 0, ���� �������� �������� �������� �����
	}
	
	if (C_TwiInterruptWaitForWif ()) { // �������� ����� ������ ����� WIF		
		if (C_TwiCheckAck ()) {	// ��������, ��� �� ����� ��� ������� ACK					
			C_TwiDataPut ((0x00 | (TimeSetDate/10)<<4) | (TimeSetDate%10)); // ������ ����� �� �������� (����)
		} else {
			// ���� ������� ����� NACK
			C_TwiCmdStop (); // �������� ������� STOP	
			C_TwiDisable (); // ���������� ������ TWI
			// lcd_write_symbol (0, 3, '*');
			return 0; // ������� ���������� 0, ���� �������� �������� �������� �����
		}			
	} else {
		C_TwiCmdStop (); // �������� ������� STOP	
		C_TwiDisable (); // ���������� ������ TWI				
		return 0; // ������� ���������� 0, ���� �������� �������� �������� �����
	}
	
	if (C_TwiInterruptWaitForWif ()) { // �������� ����� ������ ����� WIF		
		if (C_TwiCheckAck ()) {	// ��������, ��� �� ����� ��� ������� ACK					
			C_TwiDataPut ((0x00 | (TimeSetMonth/10)<<4) | (TimeSetMonth%10)); // ������ ����� �� �������� (�����)
		} else {
			// ���� ������� ����� NACK
			C_TwiCmdStop (); // �������� ������� STOP	
			C_TwiDisable (); // ���������� ������ TWI
			// lcd_write_symbol (0, 3, '*');
			return 0; // ������� ���������� 0, ���� �������� �������� �������� �����
		}			
	} else {
		C_TwiCmdStop (); // �������� ������� STOP	
		C_TwiDisable (); // ���������� ������ TWI				
		return 0; // ������� ���������� 0, ���� �������� �������� �������� �����
	}
	
	if (C_TwiInterruptWaitForWif ()) { // �������� ����� ������ ����� WIF		
		if (C_TwiCheckAck ()) {	// ��������, ��� �� ����� ��� ������� ACK					
			C_TwiDataPut ((0x00 | (TimeSetYear/10)<<4) | (TimeSetYear%10)); // ������ ����� �� �������� (���)
		} else {
			// ���� ������� ����� NACK
			C_TwiCmdStop (); // �������� ������� STOP	
			C_TwiDisable (); // ���������� ������ TWI
			// lcd_write_symbol (0, 3, '*');
			return 0; // ������� ���������� 0, ���� �������� �������� �������� �����
		}			
	} else {
		C_TwiCmdStop (); // �������� ������� STOP	
		C_TwiDisable (); // ���������� ������ TWI				
		return 0; // ������� ���������� 0, ���� �������� �������� �������� �����
	}
	
	if (C_TwiInterruptWaitForWif ()) { // �������� ����� ������ ����� WIF		
		if (C_TwiCheckAck ()) {	// ��������, ��� �� ����� ��� ������� ACK					
		
		} else {
			// ���� ������� ����� NACK
			C_TwiCmdStop (); // �������� ������� STOP	
			C_TwiDisable (); // ���������� ������ TWI
			// lcd_write_symbol (0, 3, '*');
			return 0; // ������� ���������� 0, ���� �������� �������� �������� �����
		}			
	} else {
		C_TwiCmdStop (); // �������� ������� STOP	
		C_TwiDisable (); // ���������� ������ TWI				
		return 0; // ������� ���������� 0, ���� �������� �������� �������� �����
	}
			
	C_TwiCmdStop (); // �������� ������� STOP	
	C_TwiDisable (); // ���������� ������ TWI	
	return 1; // ������� ���������� 1, ���� ������ ���� ��������� ������� 
}

/************************************************************************/
/*               ��������� �������� ������� � ����                      */
/************************************************************************/
uint8_t RtcGetTime (void) {	
	C_TwiEn (); // ��������� ������ TWI
	C_TwiBusSetIdle (); // ������� ���� � ��������� IDLE

	if (C_TwiBusWaitForIdle ()) {	// �������� ��������� ���� IDLE
		C_TwiTransactionStart (0); // ����� � ������ ������
	} else {
		C_TwiCmdStop (); // �������� ������� STOP	
		C_TwiDisable (); // ���������� ������ TWI				
		return 0; // ������� ���������� 0, ���� �������� �������� �������� �����
	}

	if (C_TwiInterruptWaitForWif ()) { // �������� ����� ������ ����� WIF
		if (C_TwiCheckAck ()) {	// ��������, ��� �� ����� ��� ������� ACK	
			C_TwiDataPut (0); // ������ �������� �������� ������� RTC
		}			
	} else {
		C_TwiCmdStop (); // �������� ������� STOP	
		C_TwiDisable (); // ���������� ������ TWI				
		return 0; // ������� ���������� 0, ���� �������� �������� �������� �����
	}

	if (C_TwiInterruptWaitForWif ()) { // �������� ����� ������ ����� WIF		
		if (C_TwiCheckAck ()) {	// ��������, ��� �� ����� ��� ������� ACK			
			C_TwiTransactionStart (1); // ��������� ����� � ������ ������
		}			
	} else {
		C_TwiCmdStop (); // �������� ������� STOP	
		C_TwiDisable (); // ���������� ������ TWI				
		return 0; // ������� ���������� 0, ���� �������� �������� �������� �����
	}
	
	if (C_TwiInterruptWaitForRif ()) { // �������� ����� ������ ����� RIF		
		TimeGetSeconds = C_TwiDataGet (); // ������ ����� �� �������� (�������)
		C_TwiCmdSendAck (); // �������� ���� ������������� ACK
	} else {
		C_TwiCmdStop (); // �������� ������� STOP	
		C_TwiDisable (); // ���������� ������ TWI				
		return 0; // ������� ���������� 0, ���� �������� �������� �������� �����
	}	
	
	if (C_TwiInterruptWaitForRif ()) { // �������� ����� ������ ����� RIF		
		TimeGetMinutes = C_TwiDataGet (); // ������ ����� �� �������� (������)
		C_TwiCmdSendAck (); // �������� ���� ������������� ACK
	} else {
		C_TwiCmdStop (); // �������� ������� STOP	
		C_TwiDisable (); // ���������� ������ TWI				
		return 0; // ������� ���������� 0, ���� �������� �������� �������� �����
	}

	if (C_TwiInterruptWaitForRif ()) { // �������� ����� ������ ����� RIF			
		TimeGetHours = C_TwiDataGet (); // ������ ����� �� �������� (����)
		C_TwiCmdSendNack (); // �������� ���� ������������� NACK (����� ���������� �����)			
	}  else {
		C_TwiCmdStop (); // �������� ������� STOP	
		C_TwiDisable (); // ���������� ������ TWI				
		return 0; // ������� ���������� 0, ���� �������� �������� �������� �����
	}
	
	if (C_TwiInterruptWaitForRif ()) { // �������� ����� ������ ����� RIF		
		TimeGetDay = C_TwiDataGet (); // ������ ����� �� �������� (����)
		C_TwiCmdSendAck (); // �������� ���� ������������� ACK
	} else {
		C_TwiCmdStop (); // �������� ������� STOP	
		C_TwiDisable (); // ���������� ������ TWI				
		return 0; // ������� ���������� 0, ���� �������� �������� �������� �����
	}	
	
	if (C_TwiInterruptWaitForRif ()) { // �������� ����� ������ ����� RIF		
		TimeGetDate = C_TwiDataGet (); // ������ ����� �� �������� (����)
		C_TwiCmdSendAck (); // �������� ���� ������������� ACK
	} else {
		C_TwiCmdStop (); // �������� ������� STOP	
		C_TwiDisable (); // ���������� ������ TWI				
		return 0; // ������� ���������� 0, ���� �������� �������� �������� �����
	}

	if (C_TwiInterruptWaitForRif ()) { // �������� ����� ������ ����� RIF			
		TimeGetMonth = C_TwiDataGet (); // ������ ����� �� �������� (�����)
		C_TwiCmdSendNack (); // �������� ���� ������������� NACK (����� ���������� �����)			
	}  else {
		C_TwiCmdStop (); // �������� ������� STOP	
		C_TwiDisable (); // ���������� ������ TWI				
		return 0; // ������� ���������� 0, ���� �������� �������� �������� �����
	}
	
	if (C_TwiInterruptWaitForRif ()) { // �������� ����� ������ ����� RIF			
		TimeGetYear = C_TwiDataGet (); // ������ ����� �� �������� (���)
		C_TwiCmdSendNack (); // �������� ���� ������������� NACK (����� ���������� �����)			
	}  else {
		C_TwiCmdStop (); // �������� ������� STOP	
		C_TwiDisable (); // ���������� ������ TWI				
		return 0; // ������� ���������� 0, ���� �������� �������� �������� �����
	}
			
	C_TwiCmdStop (); // �������� ������� STOP	
	C_TwiDisable (); // ���������� ������ TWI	
	return 1; // ������� ���������� 1, ���� ������ ���� ��������� ������� 
}

/************************************************************************/
/*                ������ � ������� ����������� EEPROM                   */
/************************************************************************/

/************************************************************************/
/*                  ��������� ������������ ����                         */
/************************************************************************/
uint8_t EepromBuildControlCode (uint16_t Adr, uint8_t Direction) {
	// ControlCode = 1010 + B0 + A1 + A0 + R\W
	uint8_t ControlCode = 0b10100110;
	if (Adr & 0x8000) {
		ControlCode |= (1<<3);
	} else {
		ControlCode &= ~(1<<3);
	}
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
uint8_t EepromWriteBlock (uint16_t Adr, uint8_t * Data, uint8_t DataBlockSize) {
	uint8_t DataId = 0;
//	uint8_t DataBlockSize = sizeof (Data) / sizeof (uint8_t);
		
	E_TwiEn (); // ��������� ������ TWI	
	E_TwiBusSetIdle (); // ������� ���� � ��������� IDLE

	if (E_TwiBusWaitForIdle ()) {	// �������� ��������� ���� IDLE		
		E_TwiTransactionStart (EepromBuildControlCode (Adr, 0)); // �������� ������������ ���� � ������ ������
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
			// lcd_write_symbol (0, 3, '*');
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
			// lcd_write_symbol (0, 3, '*');
			return 0; // ������� ���������� 0, ���� �������� �������� �������� �����
		}			
	} else {
		E_TwiCmdStop (); // �������� ������� STOP	
		E_TwiDisable (); // ���������� ������ TWI				
		return 0; // ������� ���������� 0, ���� �������� �������� �������� �����
	}	

	for (DataId = 0; DataId <= (DataBlockSize - 1); DataId++) {
		if (E_TwiInterruptWaitForWif ()) { // �������� ����� ������ ����� WIF		
			if (E_TwiCheckAck ()) {	// ��������, ��� �� ����� ��� ������� ACK					
				E_TwiDataPut (*Data); // �������� �������������� ����
				Data++;
			} else {
				E_TwiCmdStop (); // �������� ������� STOP	
				E_TwiDisable (); // ���������� ������ TWI
				// lcd_write_symbol (0, 3, '*');
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
			// lcd_write_symbol (0, 3, '*');
			return 0; // ������� ���������� 0, ���� �������� �������� �������� �����
		}			
	} else {
		E_TwiCmdStop (); // �������� ������� STOP	
		E_TwiDisable (); // ���������� ������ TWI				
		return 0; // ������� ���������� 0, ���� �������� �������� �������� �����
	}
			
//	E_TwiCmdStop (); // �������� ������� STOP	
	E_TwiDisable (); // ���������� ������ TWI	
	return 1; // ������� ���������� 1, ���� ������ ���� ��������� ������� 
}
 
/************************************************************************/
/*                   ������ ����� ������ �� EEPROM                      */
/************************************************************************/
uint8_t EepromReadBlock (uint16_t Adr, uint8_t * Data, uint8_t DataBlockSize) {
	uint8_t DataId = 0;
//	uint8_t DataBlockSize = sizeof (Data) / sizeof (uint8_t);	
	
	E_TwiEn (); // ��������� ������ TWI
	E_TwiBusSetIdle (); // ������� ���� � ��������� IDLE

	if (E_TwiBusWaitForIdle ()) {	// �������� ��������� ���� IDLE
		E_TwiTransactionStart (EepromBuildControlCode (Adr, 0)); // �������� ������������ ���� � ������ ������
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
			E_TwiTransactionStart (EepromBuildControlCode (Adr, 1)); // ��������� ����� � ������ ������
		}			
	} else {
		E_TwiCmdStop (); // �������� ������� STOP	
		E_TwiDisable (); // ���������� ������ TWI				
		return 0; // ������� ���������� 0, ���� �������� �������� �������� �����
	}

	for (DataId = 0; DataId <= (DataBlockSize - 1); DataId++) {		
		if (E_TwiInterruptWaitForRif ()) { // �������� ����� ������ ����� RIF		
			*Data = E_TwiDataGet (); // ������ ����� �� EEPROM
			Data++;
			if (DataId == (DataBlockSize - 1)) {
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
			
//	E_TwiCmdStop (); // �������� ������� STOP	
	E_TwiDisable (); // ���������� ������ TWI	
	return 1; // ������� ���������� 1, ���� ������ ���� ��������� ������� 
}

/************************************************************************/
/*                        ������� ��������                              */
/************************************************************************/
inline void Timer1msInit (void) {	
	TCC1.PER = 18432;
	TCC1.INTCTRLA |= (0x01<<0); // ������� ���������� �� ������������
	TCC1.CTRLA |= (0x01<<0);		
}

inline void TimerRs232Init (void) {	
	TCD1.PER = 25000;
	TCD1.INTCTRLA |= (0x02<<0); // ������� ���������� �� ������������
}

inline void TimerRs232Start () {
	TCD1.CNT = 0x0000; // ��������� �������� �������� �������
	TCD1.CTRLA |= (0x01<<0); // ����� ��������	
}

inline void TimerRs232Stop () {
	TCD1.CTRLA &= ~ (0x01<<0); // ���� ��������
}

#define TIMER_RS232_INIT	TCD1.PER = 18432;\
							TCD1.INTCTRLA |= (0x02<<0)

#define TIMER_RS232_START	TCD1.CNT = 0x0000;\
							TCD1.CTRLA |= (0x01<<0)

#define TIMER_RS232_STOP	TCD1.CTRLA &= ~ (0x01<<0)

#define TIMER_RS232_RESTART	TCD1.CNT = 0x0000

/************************************************************************/
/*                     ��������� ������ ������� APD                     */
/************************************************************************/

/************************************************************************/
/*                  ����� ����������� ������� �������                   */
/************************************************************************/
void ScreenModeRegister (uint8_t Action) {
	if (RegisterModeShowOnce) {
		RegisterModeShowOnce = 0;
		
		// ������ ��������� �������
		cli();
		EEPROM_AttemptCnt = EEPROM_OP_ATTEMPTS;
		while (EEPROM_AttemptCnt) {
			if (EepromReadBlock (REGISTER_START, EEPROM_ReadBuff, 8)) break;
			-- EEPROM_AttemptCnt;
		}		
		sei();
		// �������� �� �����������
		if (!(Crc8 ((uint8_t *) &EEPROM_ReadBuff, sizeof (EEPROM_ReadBuff)))) {
			EEPROM_MesCnt = EEPROM_ReadBuff[6] | (EEPROM_ReadBuff[5]<<8);
			RegisterModeCurMes = EEPROM_MesCnt;			
		}
	}
		// ������� "������"
		lcd_write_symbol (0, 0, 0xA3);
		lcd_write_symbol (0, 1, 0xA9);
		lcd_write_symbol (0, 2, 0x50);
		lcd_write_symbol (0, 3, 0x48);
		lcd_write_symbol (0, 4, 0x41);
		lcd_write_symbol (0, 5, 0xA7);
		
		// ������ ��������� �������
		cli();
		EEPROM_AttemptCnt = EEPROM_OP_ATTEMPTS;
		while (EEPROM_AttemptCnt) {
			if (EepromReadBlock (REGISTER_START, EEPROM_ReadBuff, 8)) break;
			-- EEPROM_AttemptCnt;
		}						
		sei();
		
		// �������� �� �����������	
		if (!(Crc8 ((uint8_t *) &EEPROM_ReadBuff, sizeof (EEPROM_ReadBuff)))) {
			EEPROM_MesCnt = EEPROM_ReadBuff[6] | (EEPROM_ReadBuff[5]<<8);
		}
	// �������� � ����������� �� ������� ������
	switch (Action) {
		// ��������� ������
		case 0:
			if (RegisterModeCurMes < EEPROM_MesCnt) {
				++ RegisterModeCurMes;
				RegisterModeAction = 255;
			}
			break;
		// ���������� ������	
		case 1:
			if (RegisterModeCurMes >= 2) {
				-- RegisterModeCurMes;
				RegisterModeAction = 255;
			}			
			break;
		// ����� �� �������	
		case 2:
			// ������� � ��������� �����
			OpkMode = OPK_MODE_SERVICE;
			TcRtcEn = 1;
			ClockShowEn = 1;							
			LcdRefresh = 1;

			RegisterModeAction = 255;
			// ���������� ����������		
			RegisterModeShowOnce = 1;
			return 0;
			break;
		// ��������� �����
		case 3:
			if (RegisterModeCurMes < EEPROM_MesCnt) {
				++ RegisterModeCurMes;
				RegisterModeAction = 255;
				LcdRefresh = 1;
			}
			break;
		case 4:
			if (RegisterModeCurMes >= 2) {
				-- RegisterModeCurMes;
				RegisterModeAction = 255;
				LcdRefresh = 1;
			}			
			break;						
			
		default:
			break;		
	}
	
	// ����� ������ ������� �� �����
	// ������ ������� ������
	cli();	
	EEPROM_AttemptCnt = EEPROM_OP_ATTEMPTS;
	while (EEPROM_AttemptCnt) {
		if (EepromReadBlock (REGISTER_START + (RegisterModeCurMes + 1) * 8, EEPROM_ReadBuff, 8)) break;
		-- EEPROM_AttemptCnt;
	}			
	sei();
	// ����� ���� � �������
	if (!(Crc8 ((uint8_t *) &EEPROM_ReadBuff, sizeof (EEPROM_ReadBuff)))) {
		lcd_write_str (1, 0, (uint8_t *) "  :  :  ");
		lcd_write_str (1, 12, (uint8_t *) "  .  .  ");
		lcd_write_value (1, 0, 2, EEPROM_ReadBuff[0] & 0b00111111);
		lcd_write_value (1, 3, 2, EEPROM_ReadBuff[1]);
		lcd_write_value (1, 6, 2, EEPROM_ReadBuff[2]);
		
		lcd_write_value (1, 12, 2, EEPROM_ReadBuff[3]);
		lcd_write_value (1, 15, 2, EEPROM_ReadBuff[4]);
		lcd_write_value (1, 18, 2, EEPROM_ReadBuff[5]);
		// ����� �������� �������
		if (EEPROM_ReadBuff[6] <= ApdEventIdAmount) {
			for (uint8_t IdCol = 0; IdCol <= 19; IdCol++) {
				lcd_write_symbol (2, IdCol, ApdPrjFontTable[ApdPrjData[ApdEventStartAdr[EEPROM_ReadBuff[6]] + IdCol]]);
			}
		} else {
			// ���� ������� � ����� ������� ��� � ������� APD
			lcd_write_str (2, 0, (uint8_t *) " <INCORRECT EVENT>  ");
		}		
		// ������� "��" � ����� ������ ���������������, �� ��������
		// ������ �������
		lcd_write_symbol (3, 0, ApdPrjFontTable[0x85]);
		lcd_write_symbol (3, 1, ApdPrjFontTable[0x91]);
		lcd_write_value (3, 2, 1, (EEPROM_ReadBuff[0]>>6));
	
	} else {
		// ���� � ������ ������� �����-�� �����, �������
		lcd_write_str (1, 0, (uint8_t *) "                    ");
		lcd_write_str (2, 0, (uint8_t *) "   <DATA FAILED>    ");
		lcd_write_str (3, 0, (uint8_t *) "                    ");
	}
	// ����������� � ������� ���������	
	lcd_write_str (0, 11, (uint8_t *) "    :    ");
	lcd_write_value (0, 11, 4, RegisterModeCurMes);
	lcd_write_value (0, 16, 4, EEPROM_MesCnt);		
}

/************************************************************************/
/*                      ����� ��������� �������                         */
/************************************************************************/
uint8_t ScreenModeTimeset (uint8_t Action) {
	uint8_t DateLimit = 0;
	if (TimesetModeShowOnce) {
		// ������ ������ "�������"
		TimesetModeShowOnce = 0;
		// "������" �������� �������
		if (RtcGetTime ()) {
			// ����� ����� ������ �����				
			TimesetModeClockFailed = 0;
			// ������ �������� ������� � ����
			TimesetModeShowSeconds = 10*((TimeGetSeconds & 0x70)>>4) + (TimeGetSeconds & 0x0F);
			TimesetModeShowMinutes = 10*((TimeGetMinutes & 0x70)>>4) + (TimeGetMinutes & 0x0F);
			TimesetModeShowHours = 10*((TimeGetHours & 0x30)>>4) + (TimeGetHours & 0x0F);
			TimesetModeShowDay = (TimeGetDay & 0x07);
			TimesetModeShowDate = 10*((TimeGetDate & 0x30)>>4) + (TimeGetDate & 0x0F);
			TimesetModeShowMonth = 10*((TimeGetMonth & 0x10)>>4) + (TimeGetMonth & 0x0F);
			TimesetModeShowYear = 10*((TimeGetYear & 0xF0)>>4) + (TimeGetYear & 0x0F);
		} else {
			// ��������� ����� ������ �����				
			TimesetModeClockFailed = 1;
			// ��������� �������� ������� � ����
			TimesetModeShowSeconds = 0;
			TimesetModeShowMinutes = 0;
			TimesetModeShowHours = 0;
			TimesetModeShowDay = 0;
			TimesetModeShowDate = 0;
			TimesetModeShowMonth = 0;
			TimesetModeShowYear = 0;
		}
	}	
				
	if (TimesetModeClockFailed) {
		// ��������� ������ "���� �����"
		lcd_clear ();
		lcd_write_str (0, 0, (uint8_t *)"> TIME SET");
		lcd_write_str (1, 0, (uint8_t *)"??:??:?? ??.??.??");
		lcd_write_str (2, 0, (uint8_t *)"ERROR: CLOCK FAILED!");
		lcd_write_str (3, 0, (uint8_t *)"PRESS F1 TO CANCEL");
		// ����� �� ������
		if (Action == 4) {
			// ������� � ��������� �����
			OpkMode = OPK_MODE_SERVICE;
			TcRtcEn = 1;
			ClockShowEn = 1;							
			LcdRefresh = 1;
			TimesetModeCursorPosition = 0; // ������ �� ��������� �������
			// ����� ����� ������ �����	
			TimesetModeClockFailed = 0;
			TimesetModeAction = 255;
			// ���������� "�������" �������� ������� ��� ����� ������ �������		
			TimesetModeShowOnce = 1;
		}				
		return 0;			
	} else {
		lcd_clear ();
		lcd_write_str (0, 0, (uint8_t *)"> TIME SET");
		// ��������� ������� ������:
		// 1. ��������� �������� "������� ����� - ����� ������� �����"
		if (Action == 0) {
			if (TimesetModeCursorPosition == 0) {
				TimesetModeCursorPosition = 15;
			} else {		
				TimesetModeCursorPosition -= 3;
			}			
			TimesetModeAction = 255;			
		}
		// 1. ��������� �������� "������� ������ - ����� ������� ������"
		if (Action == 1) {
			if (TimesetModeCursorPosition == 15) {
				TimesetModeCursorPosition = 0;
			} else {	
				TimesetModeCursorPosition += 3;
			}		
			TimesetModeAction = 255;			
		}
		// 2. ��������� �������� "������� ����� - ��������� ��������"
		if (Action == 2) {
			// ��������� �������� � ����������� �� ��������� �������
			// ����
			if (TimesetModeCursorPosition == 0) {
				if (TimesetModeShowHours == 23) {
					TimesetModeShowHours = 0;
				} else {		
					TimesetModeShowHours++;
				}			
			}
			// ������
			if (TimesetModeCursorPosition == 3) {
				if (TimesetModeShowMinutes == 59) {
					TimesetModeShowMinutes = 0;
				} else {			
					TimesetModeShowMinutes++;
				}				
			}
			// �������
			if (TimesetModeCursorPosition == 6) {
				if (TimesetModeShowSeconds == 59) {
					TimesetModeShowSeconds = 0;
				} else {			
					TimesetModeShowSeconds++;
				}				
			}
			// �����
			if (TimesetModeCursorPosition == 9) {
				if (TimesetModeShowDate == DateLimit) {
					TimesetModeShowDate = 1;
				} else {		
					TimesetModeShowDate++;
				}				
			}
			// ������
			if (TimesetModeCursorPosition == 12) {
				if (TimesetModeShowMonth == 12) {
					TimesetModeShowMonth = 1;
				} else {			
					TimesetModeShowMonth++;
				}				
			}
			// ����
			if (TimesetModeCursorPosition == 15) {
				if (TimesetModeShowYear == 99) {
					TimesetModeShowYear = 0;
				} else {		
					TimesetModeShowYear++;
				}				
			}				
			TimesetModeAction = 255;						
		}
		// 3. ��������� �������� "������� ���� - ��������� ��������"
		if (Action == 3) {
			// ��������� �������� � ����������� �� ��������� �������
			// ����		
			if (TimesetModeCursorPosition == 0) {
				if (TimesetModeShowHours == 0) {
					TimesetModeShowHours = 23;
				} else {		
					TimesetModeShowHours--;
				}				
			}
			// ������		
			if (TimesetModeCursorPosition == 3) {
				if (TimesetModeShowMinutes == 0) {
					TimesetModeShowMinutes = 59;
				} else {		
					TimesetModeShowMinutes--;
				}				
			}
			// �������		
			if (TimesetModeCursorPosition == 6) {
				if (TimesetModeShowSeconds == 0) {
					TimesetModeShowSeconds = 59;
				} else {			
					TimesetModeShowSeconds--;
				}				
			}
			// �����		
			if (TimesetModeCursorPosition == 9) {
				if (TimesetModeShowDate == 1) {
					TimesetModeShowDate = DateLimit;
				} else {		
					TimesetModeShowDate--;
				}				
			}
			// ������		
			if (TimesetModeCursorPosition == 12) {
				if (TimesetModeShowMonth == 1) {
					TimesetModeShowMonth = 12;
				} else {		
					TimesetModeShowMonth--;
				}				
			}
			// ����		
			if (TimesetModeCursorPosition == 15) {
				if (TimesetModeShowYear == 0) {
					TimesetModeShowYear = 99;
				} else {			
					TimesetModeShowYear--;
				}				
			}
			TimesetModeAction = 255;
		}
		// 4. ��������� �������� "F1 - ������������� �����"	
		if (Action == 4) {
			// ���������� ���������
			if (TimeSetManual (TimesetModeShowHours, TimesetModeShowMinutes, TimesetModeShowSeconds, TimesetModeShowDay, TimesetModeShowDate, TimesetModeShowMonth, TimesetModeShowYear)) {
				// ������� � ��������� �����
				OpkMode = OPK_MODE_SERVICE;
				TcRtcEn = 1;
				ClockShowEn = 1;							
				LcdRefresh = 1;
				TimesetModeCursorPosition = 0;
				TimesetModeAction = 255;
				// ���������� "�������" �������� ������� ��� ����� ������ �������
				TimesetModeShowOnce = 1;
			}
		}
		// 5. ��������� �������� "F2 - ������"			
		if (Action == 5) {
			// ������� � ��������� �����
			OpkMode = OPK_MODE_SERVICE;
			TcRtcEn = 1;
			ClockShowEn = 1;							
			LcdRefresh = 1;
			TimesetModeCursorPosition = 0;
			TimesetModeAction = 255;
			// ���������� "�������" �������� ������� ��� ����� ������ �������		
			TimesetModeShowOnce = 1;
		}
		
		// ����� ������ ��� ���� � ����������� �� �������� ������ � ����	
		switch (TimesetModeShowMonth) {
			case 1: // ������
				DateLimit = 31;
				break;
			case 2: // �������
				if (TimesetModeShowYear%4) {
					DateLimit = 28; // �� ����������
				} else {
					DateLimit = 29; // ����������
				}				
				break;
			case 3: // ����
				DateLimit = 31;
				break;
			case 4: // ������
				DateLimit = 30;
				break;
			case 5: // ���
				DateLimit = 31;
				break;
			case 6: // ����
				DateLimit = 30;
				break;
			case 7: // ����
				DateLimit = 31;
				break;
			case 8: // ������
				DateLimit = 31;
				break;
			case 9: // ��������
				DateLimit = 30;
				break;
			case 10: // �������
				DateLimit = 31;
				break;
			case 11: // ������
				DateLimit = 30;
				break;
			case 12: // �������
				DateLimit = 31;
				break;
			default:
				DateLimit = 31;
				break;
		}
		
		// �������� �������� ����� �� ������� �����
		if (TimesetModeShowDate > DateLimit) {
			TimesetModeShowDate = DateLimit;
		}											
		lcd_write_str (1, 0, (uint8_t *)"  :  :     .  .  ");		
		lcd_write_value (1, 0, 2, TimesetModeShowHours);
		lcd_write_value (1, 3, 2, TimesetModeShowMinutes);
		lcd_write_value (1, 6, 2, TimesetModeShowSeconds);
		lcd_write_value (1, 9, 2, TimesetModeShowDate);
		lcd_write_value (1, 12, 2, TimesetModeShowMonth);
		lcd_write_value (1, 15, 2, TimesetModeShowYear);
		lcd_write_str (2, 0, (uint8_t *)"                 ");
	//	lcd_write_str (2, 0 + TimesetModeCursorPosition, (uint8_t *)"==");
		lcd_write_symbol (2, 0 + TimesetModeCursorPosition, 0xD9);
		lcd_write_symbol (2, 0 + TimesetModeCursorPosition + 1, 0xD9);
		lcd_write_str (3, 0, (uint8_t *)"F1 APPLY : F2 CANCEL");
		return 0;
	}
}

/************************************************************************/
/*							���� ���������                              */
/************************************************************************/
void ScreenModeSettings (uint8_t Action) {
	if (SettingsModeShowOnce) {
		SettingsModeShowOnce = 0;
		
		lcd_clear ();

		// ������� "DELAY:"
		lcd_write_symbol (0, 0, 'D');
		lcd_write_symbol (0, 1, 'E');
		lcd_write_symbol (0, 2, 'L');
		lcd_write_symbol (0, 3, 'A');
		lcd_write_symbol (0, 4, 'Y');
		lcd_write_symbol (0, 5, ':');
		
		lcd_write_symbol (1, 0, '<');
		lcd_write_symbol (1, 1, '-');
		
		lcd_write_symbol (1, 7, '-');
		lcd_write_symbol (1, 8, '>');
		
		lcd_write_str (3, 0, (uint8_t *)"F1 APPLY : F2 CANCEL");
		
		MesDelay = TcAdpTimeoutPer;
		
		lcd_write_value (1, 3, 3, MesDelay);
	}		
		
	// �������� � ����������� �� ������� ������
	switch (Action) {
		// ����� - ���������
		case 0:
			if (MesDelay > 1) {
				-- MesDelay;
			}
			SettingsModeAction = 255;			
			break;
			
		// ������ - ���������	
		case 1:
			if (MesDelay < 100) {
				++ MesDelay;
			}
			SettingsModeAction = 255;			
			break;
			
		// �������
		case 2:
			// ���������� ���������
			TcAdpTimeoutPer = MesDelay;
			
			// ������� � ��������� �����
			OpkMode = OPK_MODE_SERVICE;
			TcRtcEn = 1;
			ClockShowEn = 1;							
			LcdRefresh = 1;
			SettingsModeAction = 255;

			SettingsModeShowOnce = 1;
			break;

		// �����		
		case 3:
			// ������� � ��������� �����
			OpkMode = OPK_MODE_SERVICE;
			TcRtcEn = 1;
			ClockShowEn = 1;							
			LcdRefresh = 1;

			SettingsModeAction = 255;
	
			SettingsModeShowOnce = 1;
			break;				
			
		default:
			break;		
	}
	
		lcd_write_value (1, 3, 3, MesDelay);	
}

// ���� �����������
void ScreenModeIntro (void) {
	lcd_write_str (0, 0, (uint8_t *)"OPERATIONAL PANEL");
	lcd_write_str (1, 0, (uint8_t *)"LLC ASC. EKB, RUSSIA");
	lcd_write_str (2, 0, (uint8_t *)"VER.: 1.0.1");
	lcd_write_str (3, 0, (uint8_t *)"LOADING");
}

void ScreenModeDevelope (void) {
//	lcd_write_str (0, 0, (uint8_t *)"> DEVELOPING");
/*
lcd_write_value (0, 16, 3, ApdFrameDataFromPlcAmount[0]);

lcd_write_value (1, 0, 3, ApdFrameDataFromPlcAdr[0][0]);		
lcd_write_value (1, 4, 3, ApdFrameDataFromPlcData[0][0]);
lcd_write_value (1, 8, 1, ApdFrameDataFromPlcStatus[0][0]);

lcd_write_value (2, 0, 3, ApdFrameDataFromPlcAdr[0][1]);		
lcd_write_value (2, 4, 3, ApdFrameDataFromPlcData[0][1]);
lcd_write_value (2, 8, 1, ApdFrameDataFromPlcStatus[0][1]);

lcd_write_value (3, 0, 3, ApdFrameDataFromPlcAdr[0][2]);		
lcd_write_value (3, 4, 3, ApdFrameDataFromPlcData[0][2]);
lcd_write_value (3, 8, 1, ApdFrameDataFromPlcStatus[0][2]);

lcd_write_value (1, 10, 3, ApdFrameDataFromPlcAdr[0][3]);		
lcd_write_value (1, 14, 3, ApdFrameDataFromPlcData[0][3]);
lcd_write_value (1, 18, 1, ApdFrameDataFromPlcStatus[0][3]);

lcd_write_value (2, 10, 3, ApdFrameDataFromPlcAdr[0][4]);		
lcd_write_value (2, 14, 3, ApdFrameDataFromPlcData[0][4]);
lcd_write_value (2, 18, 1, ApdFrameDataFromPlcStatus[0][4]);

lcd_write_value (3, 10, 3, ApdFrameDataFromPlcAdr[0][5]);		
lcd_write_value (3, 14, 3, ApdFrameDataFromPlcData[0][5]);
lcd_write_value (3, 18, 1, ApdFrameDataFromPlcStatus[0][5]);
*/

/*
lcd_write_value (1, 0, 3, ApdEventsStartAdr);		
lcd_write_value (1, 4, 3, ApdEventIdAmount);
lcd_write_value (1, 8, 6, ApdEventRefreshRate);

lcd_write_value (2, 0, 3, ApdEventAdr);

lcd_write_value (3, 0, 3, ApdEventStartAdr[0]);		
lcd_write_value (3, 4, 3, ApdEventStartAdr[1]);
lcd_write_value (3, 8, 3, ApdEventStartAdr[2]);
*/

/*
for (uint8_t IdCnt = 0; IdCnt <= (ApdEventIdAmount - 1); IdCnt++) {
	for (uint8_t IdCol = 0; IdCol <= 19; IdCol++) {
		lcd_write_symbol (IdCnt + 1, IdCol, ApdPrjFontTable[ApdPrjData[ApdEventStartAdr[IdCnt] + IdCol]]);
	}
}
*/
/*	
	SortAcending (C, sizeof (C));
	lcd_write_value (1, 0, 1, C[0]);
	lcd_write_value (1, 2, 1, C[1]);
	lcd_write_value (1, 4, 1, C[2]);
	lcd_write_value (1, 6, 1, C[3]);
	lcd_write_value (1, 8, 1, C[4]);
*/
	
	uint8_t IdA = 0;
//	uint8_t IdB = 0;
	
//	lcd_write_value (0, 19, 1, ApdFrameCur);
//	lcd_write_value (3, 0, 3, FocusDataAmount);
//	lcd_write_value (3, 4, 3, BlockAmount);	

	
//	AdrInFocus ();
	/*
	for (IdA = 0; IdA <= (FocusDataAmount - 1); IdA++) {
		lcd_write_value (0, 4*IdA, 3, FocusDataAdr[IdA]);
	}
	*/
//	SortAcending (FocusDataAdr, FocusDataAmount);
	/*
	for (IdA = 0; IdA <= (FocusDataAmount - 1); IdA++) {
		lcd_write_value (1, 4*IdA, 3, FocusDataAdr[IdA]);
	}
	*/
//	RequestAdrSpace ();
	/*
	for (IdA = 0; IdA <= (FocusDataAmount - 1); IdA++) {
		lcd_write_value (3, 4*IdA, 3, RequestAdrSpaceArray[IdA]);
	}	
	*/
	/*
	PrepareToRequest ();
	for (IdA = 0; IdA <= (BlockAmount - 1); IdA++) {
		lcd_write_value (2, 4*IdA, 3, BlockRequest[IdA][0]);
		lcd_write_value (3, 4*IdA, 3, BlockRequest[IdA][1]);
	}
	*/
/*
	lcd_write_value (1, 0, 3, (AdpRecMesBuf[9]<<8) | AdpRecMesBuf[8]);
	
	for (IdA = 0; IdA <= (FocusDataAmount - 1); IdA++) {
		lcd_write_value (0, 4*IdA, 3, FocusDataAdr[IdA]);
	}	
*/

/************************************************************************/
/*                   �������� ���������                                 */
/************************************************************************/

	if (AdpRecMesLen) {
		for (IdA = 0; IdA <= (AdpRecMesLen - 1); IdA++) {
			lcd_write_symbol (IdA/7, 3*(IdA%7), DecToHex[ (AdpRecMesBuf[IdA]/16) ]);
			lcd_write_symbol (IdA/7, 3*(IdA%7) + 1, DecToHex[ (AdpRecMesBuf[IdA]%16) ]);		
		}
	}

//	lcd_write_value (1, 0, 3, EEPROM_MesCnt);
//	WP_ON;
/*
	uint8_t C[3] = {0, 0, 0};
	WP_OFF;
	EepromWriteBlock (0, C, sizeof (C));
	WP_ON;		
	
	EEPROM_WriteBuff[0] = 0;
	EEPROM_WriteBuff[1] = 0;
	EEPROM_WriteBuff[2] = 0;
	EEPROM_WriteBuff[3] = 0;
	EEPROM_WriteBuff[4] = 0;
	EEPROM_WriteBuff[5] = 0;	
	EEPROM_WriteBuff[6] = 0;
	EEPROM_WriteBuff[7] = Crc8 (EEPROM_WriteBuff, 7);
	WP_OFF;
	EepromWriteBlock (0, EEPROM_WriteBuff, sizeof (EEPROM_WriteBuff));
	WP_ON;

	EepromReadBlock (0, EEPROM_ReadBuff, sizeof (EEPROM_ReadBuff));
	
	//EEPROM_MesCnt = (EEPROM_ReadBuff[5]<<8) | EEPROM_ReadBuff[6];
	if (!(Crc8 ((uint8_t *) &EEPROM_ReadBuff, sizeof (EEPROM_ReadBuff)))) {
		EEPROM_MesCnt = (EEPROM_ReadBuff[5]<<8) | EEPROM_ReadBuff[6];	
	} else {
		
		EEPROM_WriteBuff[0] = 0;
		EEPROM_WriteBuff[1] = 0;
		EEPROM_WriteBuff[2] = 0;
		EEPROM_WriteBuff[3] = 0;
		EEPROM_WriteBuff[4] = 0;
		EEPROM_WriteBuff[5] = 0;	
		EEPROM_WriteBuff[6] = 0;
		EEPROM_WriteBuff[7] = Crc8 (EEPROM_WriteBuff, 7);
		WP_OFF;
		EepromWriteBlock (0, EEPROM_WriteBuff, sizeof (EEPROM_WriteBuff));
		WP_ON;
//		EEPROM_MesCnt = 666;
	}
	
*/

//	lcd_write_value (2, 16, 3, EEPROM_MesCnt);

	
//	A[0] = Crc8 (A, 3);
//	WP_OFF;
/*	
	if (EepromWriteBlock (0, A, sizeof (A))) {
		if (EepromReadBlock (0, B, sizeof (B))) {
			
			EepromReadBlock (0, Buff1, sizeof (Buff1));	
			lcd_write_value (0, 0, 3, Buff1[0]);
			lcd_write_value (0, 4, 3, Buff1[1]);
			lcd_write_value (0, 8, 3, Buff1[2]);
			lcd_write_value (0, 12, 3, Buff1[3]);
			lcd_write_value (0, 16, 3, Buff1[4]);
			lcd_write_value (1, 0, 3, Buff1[5]);
			lcd_write_value (1, 4, 3, Buff1[6]);
			lcd_write_value (1, 8, 3, Buff1[7]);

			EepromReadBlock (8, Buff2, sizeof (Buff2));		
			lcd_write_value (2, 0, 3, Buff2[0]);
			lcd_write_value (2, 4, 3, Buff2[1]);
			lcd_write_value (2, 8, 3, Buff2[2]);
			lcd_write_value (2, 12, 3, Buff2[3]);
			lcd_write_value (2, 16, 3, Buff2[4]);
			lcd_write_value (3, 0, 3, Buff2[5]);
			lcd_write_value (3, 4, 3, Buff2[6]);
			lcd_write_value (3, 8, 3, Buff2[7]);			
			
			lcd_write_value (1, 16, 3, B[0]);
			lcd_write_value (2, 16, 3, B[1]);
			lcd_write_value (3, 16, 3, B[2]);
		}
	}
*/

// ���������� EEPROM
/*
	cli();
	EEPROM_AttemptCnt = EEPROM_OP_ATTEMPTS;
	while (EEPROM_AttemptCnt) {
		if (EepromReadBlock (REGISTER_START, Buff1, 8)) break;
		-- EEPROM_AttemptCnt;
	}	
	sei();
		
	lcd_write_value (0, 0, 3, Buff1[0]);
	lcd_write_value (0, 4, 3, Buff1[1]);
	lcd_write_value (0, 8, 3, Buff1[2]);
	lcd_write_value (0, 12, 3, Buff1[3]);
	lcd_write_value (0, 16, 3, Buff1[4]);
	lcd_write_value (1, 0, 3, Buff1[5]);
	lcd_write_value (1, 4, 3, Buff1[6]);
	lcd_write_value (1, 8, 3, Buff1[7]);

	cli();
	EEPROM_AttemptCnt = EEPROM_OP_ATTEMPTS;
	while (EEPROM_AttemptCnt) {
		if (EepromReadBlock (REGISTER_START + 8, Buff2, 8)) break;
		-- EEPROM_AttemptCnt;
	}		
	sei();
				
	lcd_write_value (2, 0, 3, Buff2[0]);
	lcd_write_value (2, 4, 3, Buff2[1]);
	lcd_write_value (2, 8, 3, Buff2[2]);
	lcd_write_value (2, 12, 3, Buff2[3]);
	lcd_write_value (2, 16, 3, Buff2[4]);
	lcd_write_value (3, 0, 3, Buff2[5]);
	lcd_write_value (3, 4, 3, Buff2[6]);
	lcd_write_value (3, 8, 3, Buff2[7]);	
*/
}

 // ������� ������������ ������� ������ ���� ��������������
uint8_t ApdFrameButtonActionTable[13][13] = {
	{0xFF,    4,    5,    6,    7,   16,    8,    9,   19,    0,    1,    2,    3},
	{0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
	{0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
	{0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
	{0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},				
	{0xFF,   24,   11,   26,   13, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
	{0xFF, 0xFF, 0xFF, 0xFF, 0xFF,   28, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
	{0xFF, 0xFF, 0xFF, 0xFF, 0xFF,   12, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
	{0xFF, 0xFF, 0xFF, 0xFF, 0xFF,   15, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
	{0xFF, 0xFF, 0xFF, 0xFF, 0xFF,   20, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
	{0xFF, 0xFF, 0xFF, 0xFF, 0xFF,   21, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
	{0xFF, 0xFF, 0xFF, 0xFF, 0xFF,   17, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
	{0xFF, 0xFF, 0xFF, 0xFF, 0xFF,   23, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}																												
};

void ApdSendToPlc (uint8_t Id); // �������� "�������� � ���"
void ApdGoToFrame (uint8_t Id); // �������� "������� � �����"
void ApdEditField (uint8_t Id); // �������� "������������� ����"
void ApdFrameShiftUp (uint8_t Id); // �������� "����� ����� �����"
void ApdFrameShiftDown (uint8_t Id); // �������� "����� ����� ����"

void ApdSendToPlc (uint8_t Id) {
	
}
void ApdGoToFrame (uint8_t Id) {
//	lcd_write_symbol (0, 0, '*');
	ApdFrameCur = ApdPrjData[ApdFrameActionStartAdr[ApdFrameCur][Id] + 2];
	ApdFrameDisplayShift = 0;
	LcdRefresh = 1;
}
void ApdEditField (uint8_t Id) {
	
}
void ApdFrameShiftUp (uint8_t Id) {
	//lcd_write_symbol (0, 1, '!');
	uint8_t strings_max = ApdPrjData[ApdFrameStartAdr[ApdFrameCur] + 4];
	uint8_t strings_shift = ApdPrjData[ApdFrameActionStartAdr[ApdFrameCur][Id] + 2];
	if (strings_max > ApdPrjData[8]) {
		ApdFrameDisplayShift -= strings_shift;
		if (ApdFrameDisplayShift <= 0) {
			ApdFrameDisplayShift = 0;
		}	
		LcdRefresh = 1;
	}	
}
void ApdFrameShiftDown (uint8_t Id) {
	//lcd_write_symbol (0, 0, '*');
	uint8_t strings_max = ApdPrjData[ApdFrameStartAdr[ApdFrameCur] + 4];
	uint8_t strings_shift = ApdPrjData[ApdFrameActionStartAdr[ApdFrameCur][Id] + 2];
	if (strings_max > ApdPrjData[8]) {
		ApdFrameDisplayShift += strings_shift;
		if ((ApdFrameDisplayShift + ApdPrjData[8]) >= (strings_max)) {
			ApdFrameDisplayShift = (strings_max - ApdPrjData[8]);
		}
		LcdRefresh = 1;		
	}			
}

typedef void (* FuncPtrArray[]) (uint8_t Id);

const FuncPtrArray ApdFrameActionFunction = {
	ApdSendToPlc,
	ApdGoToFrame,
	ApdFrameShiftUp,
	ApdFrameShiftDown,	
	ApdEditField,		
};

void ScreenModeNormal (void) {
	
	uint8_t ClockAppear = 0;
	uint8_t DataAppear = 0;
	
	signed char ApdValueString = 0x00;
	uint8_t ApdValueColumn = 0x00;
	uint8_t ApdValueLen = 0x00;
	signed long long int ApdValue = 0;
	
	uint16_t ApdValuePlcAdr = 0x0000;
	
	signed long long int ApdValueMultiplier = 0x0000;		
	signed long long int ApdValueDivider = 0x0000;	
	signed long long int ApdValueAddition = 0x0000;
		
	signed long long int ApdValueTemp = 0;
	signed long long int ApdValueResult = 0;
	
	signed char ApdTextString = 0x00;
	uint8_t ApdTextColumn = 0x00;
	uint8_t ApdTextLen = 0x00;
	uint16_t ApdTextPlcAdr = 0x0000;
	uint8_t ApdTextIdMin = 0x00;
	uint8_t ApdTextIdMax = 0x00;
	uint16_t ApdTextId = 0x0000;
	uint8_t ApdTextSymbolId = 0x00;
	
	uint8_t ApdValueFormat = 0x00;
	uint8_t ApdValueId = 0x00;
	uint8_t ApdValueEnd = 0x00;
	
	uint32_t Val = 0x00;
	uint8_t Cnt = 0x00;
	uint8_t Id = 0x00;
	uint8_t Len = 0x00;
	uint8_t Negative = 0x00;
	
	uint8_t PointId = 0x00;
	uint8_t DataOk = 0x00;
	
	for (uint8_t IdField = 0; IdField <= (ApdFrameFieldsAmount[ApdFrameCur]) - 1; IdField++) {
		switch (ApdPrjData[ApdFrameFieldStartAdr[ApdFrameCur][IdField]]) {

/************************************************************************/
/*                            ��������                                  */
/************************************************************************/			
			case APD_FIELD_TYPE_SPLASH:
				for (uint8_t IdString = 0; IdString <= (ApdPrjData[8] - 1); IdString++) {
					for (uint8_t IdColumn = 0; IdColumn <= (ApdPrjData[9] - 1); IdColumn++) {
						lcd_write_symbol (IdString, IdColumn, ApdPrjFontTable[ApdPrjData[ApdFrameFieldStartAdr[ApdFrameCur][IdField] + 2 + ApdPrjData[9]*(IdString + ApdFrameDisplayShift) + IdColumn]]);
					}
				}
				break;
				
/************************************************************************/
/*                               �����                                  */
/************************************************************************/					
			case APD_FIELD_TYPE_VALUE:
				ApdValueString = ApdPrjData[ApdFrameFieldStartAdr[ApdFrameCur][IdField] + 1] - ApdFrameDisplayShift;
				if ((ApdValueString >= 0) && (Abs (ApdValueString) < (ApdPrjData[8]))) {
					
					ApdValueString = Abs (ApdValueString);
					ApdValueColumn = ApdPrjData[ApdFrameFieldStartAdr[ApdFrameCur][IdField] + 2];
					// ����� ����
					ApdValueLen = ApdPrjData[ApdFrameFieldStartAdr[ApdFrameCur][IdField] + 3];
					// ����� ���������� ����
					ApdValuePlcAdr = (ApdPrjData[ApdFrameFieldStartAdr[ApdFrameCur][IdField] + 4]) | (ApdPrjData[ApdFrameFieldStartAdr[ApdFrameCur][IdField] + 5]<<8);
					++ ApdValuePlcAdr;
					// �� ������ ������� ������ �������
					// ApdValue = 0x7FFF;
					DataOk = 0;
					for (Id = 0; Id <= (FocusDataAmount - 1); Id++) {
						if ((ApdValuePlcAdr) == FocusDataAdr[Id]) {
							if (FocusDataStatus[Id]) {
								ApdValue = FocusData[Id];
								DataOk = 1;
							} else {
								DataOk = 0;
							}
						}													
					}
					// ������ ������ ����
					ApdValueFormat = ApdPrjData[ApdFrameFieldStartAdr[ApdFrameCur][IdField] + 7];
					// ��������� (�� �� ����� ������)
					ApdValueMultiplier = (ApdPrjData[ApdFrameFieldStartAdr[ApdFrameCur][IdField] + 9]<<8) | (ApdPrjData[ApdFrameFieldStartAdr[ApdFrameCur][IdField] + 8]);
					// ��������
					ApdValueDivider = (ApdPrjData[ApdFrameFieldStartAdr[ApdFrameCur][IdField] + 11]<<8) | (ApdPrjData[ApdFrameFieldStartAdr[ApdFrameCur][IdField] + 10]);
					// �������
					ApdValueAddition = (ApdPrjData[ApdFrameFieldStartAdr[ApdFrameCur][IdField] + 13]<<8) | (ApdPrjData[ApdFrameFieldStartAdr[ApdFrameCur][IdField] + 12]);	

// ������ ���� �� ��������:
/***************** �������� *******************************************/
					if ((ApdValueFormat & 0xF0) == APD_FIELD_FORMAT_BIN) {
						
						ApdValueResult = ApdValue>>ApdValueMultiplier;						
						
						if (ApdValueLen < 16) {
							ApdValueEnd = ApdValueLen;
						} else {
							ApdValueEnd = 16;
						}
						
						PointId = 0;
						ApdValueId = 0;
						while (PointId <= (ApdValueEnd - 1)) {
							if ((ApdValueFormat & 0x0F) && (PointId == ((ApdValueFormat & 0x0F) - 1))) {
								lcd_write_symbol (ApdValueString, ApdValueColumn + ApdValueLen - 1 - PointId, '.');
								PointId++;
							} else {
								if ((ApdValueColumn + ApdValueLen - 1 - PointId) < ApdValueColumn) {
									
								} else {									
									if (DataOk) {
										lcd_write_symbol (ApdValueString, ApdValueColumn + ApdValueLen - 1 - PointId, DecToHex[(ApdValueResult>>(1*ApdValueId)) & 0x0001]);
									} else {
										lcd_write_symbol (ApdValueString, ApdValueColumn + ApdValueLen - 1 - PointId, 0x2A);
									}
								}																	
								ApdValueId++;
								PointId++;
							}							
						}

/***************** ����������������� ***********************************/											
					} else if ((ApdValueFormat & 0xF0) == APD_FIELD_FORMAT_HEX) {
						
						if (ApdValueLen < 4) {
							ApdValueEnd = ApdValueLen;
						} else {
							ApdValueEnd = 4;
						}
						if (ApdValueFormat & 0x0F) {
							ApdValueEnd++;
						}							
												
						ApdValueResult = ApdValue>>ApdValueMultiplier;
												
						PointId = 0;
						ApdValueId = 0;
						while (PointId <= (ApdValueEnd - 1)) {
							if ((ApdValueFormat & 0x0F) && (PointId == ((ApdValueFormat & 0x0F) - 1))) {
								lcd_write_symbol (ApdValueString, ApdValueColumn + ApdValueLen - 1 - PointId, '.');
								PointId++;
							} else {														
								if (DataOk) {								
									lcd_write_symbol (ApdValueString, ApdValueColumn + ApdValueLen - 1 - PointId, DecToHex[(ApdValueResult>>(4*ApdValueId)) & 0x000F]);
								} else {
									lcd_write_symbol (ApdValueString, ApdValueColumn + ApdValueLen - 1 - PointId, 0x2A);
								}								
								ApdValueId++;
								PointId++;
							}																
						}
						
/***************** �����. �������� ��� ����� ����� *********************/						
					} else if ((ApdValueFormat & 0xF0) == APD_FIELD_FORMAT_WITHOUT_NULLS) {
						// ��������������� �������� ��������
						ApdValueTemp = ApdValue * ApdValueMultiplier;
						
						// ���������� ������������� ��������� ���
						// �������������
						if (ApdValueResult < 0) {
							// ���� �������������, ���� ������
							// � ���������� ���� Negative
							ApdValueResult = Abs (ApdValueResult);
							Negative = 1;
						} else {
							Negative = 0;
						}						
						
						// ����������: (x + div/2) / div =
						ApdValueResult = (ApdValueTemp + (ApdValueDivider >> 1)) / ApdValueDivider + ApdValueAddition;
											
						Len = 0;
						Val = (uint64_t) ApdValueResult;						
						Cnt = 1;
						// ������� ���������� ��������???						
						while ( Val ) {
							Len++;							
							Val -= Val%Rank[Cnt];
							Cnt++;
						};
						// ���� 0, �� ����� ������� ���� ������ � "0"
						if (!Len) {
							Len = 1;
						}
						// ��������������� �� �������� ����
						if (Len > ApdValueLen) {
							Len = ApdValueLen;
						}
						// ���� ������ ����������, �� ������� "*" ��
						// ��� ����� ����
						if (!DataOk) {
							Len = ApdValueLen;
						}														
						
						if (ApdValueLen - Len) {
							PointId = 0;
							while (PointId <= (ApdValueEnd - 1)) {
								lcd_write_symbol (ApdValueString, ApdValueColumn + PointId, 0x20);
								PointId++;
							}	
						}
							
						Val = ApdValueResult;
						PointId = 0;
						ApdValueId = 0;
						ApdValueEnd = Len;
						// ����� ���������� �����������						
						while (PointId <= (ApdValueEnd - 1)) {
							// ������� ��� ������ "."
							if ((ApdValueFormat & 0x0F) && (PointId == ((ApdValueFormat & 0x0F) - 1))) {
								lcd_write_symbol (ApdValueString, ApdValueColumn + ApdValueLen - 1 - PointId, '.');
								PointId++;
							} else {
								if (DataOk) {														
									lcd_write_symbol (ApdValueString, ApdValueColumn + ApdValueLen - 1 - PointId, DecToHex[Val%10]);
								} else {
									lcd_write_symbol (ApdValueString, ApdValueColumn + ApdValueLen - 1 - PointId, 0x2A);
								}										
								Val /= 10;
								ApdValueId++;
								PointId++;
							}																
						}
						// ���� ����� �������������, ������ "-"
						if (Negative) {
							// ������ � ��� ������, ���� ������ ��������
							if (DataOk) {
								// ���� "-" �������� �� ������� ������,
								// ������ ��� � ������ �����
								if (/*((ApdValueColumn + ApdValueLen - 1 - PointId) < 0) || */((ApdValueColumn + ApdValueLen - 1 - PointId) < ApdValueColumn)) {
									lcd_write_symbol (ApdValueString, ApdValueColumn, '-');
								// ���� �� ��������, �� ������ �����
								// ���������� ������� ������
								} else {								
									lcd_write_symbol (ApdValueString, ApdValueColumn + ApdValueLen - 1 - PointId, '-');
								}
							}																
						}
						
/***************** �����. �������� � ������ ����� **********************/							
					} else if ((ApdValueFormat & 0xF0) == APD_FIELD_FORMAT_WITH_NULLS) {
						// ���������� ����������:					
						ApdValueTemp = ApdValue * ApdValueMultiplier;

						// ����������, ������������� ����� ��� ���
						if (ApdValueResult < 0) {
							ApdValueResult = Abs (ApdValueResult);
							Negative = 1;
						} else {
							Negative = 0;
						}
												
						// ����������: (x + div/2) / div =
						ApdValueResult = (ApdValueTemp + (ApdValueDivider >> 1)) / ApdValueDivider + ApdValueAddition;

						Val = ApdValueResult;
						PointId = 0;
						ApdValueId = 0;
						ApdValueEnd = ApdValueLen;						
						while (PointId <= (ApdValueEnd - 1)) {
							if ((ApdValueFormat & 0x0F) && (PointId == ((ApdValueFormat & 0x0F) - 1))) {
								lcd_write_symbol (ApdValueString, ApdValueColumn + ApdValueLen - 1 - PointId, '.');
								PointId++;
							} else {
								if (DataOk) {
									lcd_write_symbol (ApdValueString, ApdValueColumn + ApdValueLen - 1 - PointId, DecToHex[Val%10]);
								} else {
									lcd_write_symbol (ApdValueString, ApdValueColumn + ApdValueLen - 1 - PointId, 0x2A);
								}																						
								Val /= 10;
								ApdValueId++;
								PointId++;
							}																
						}
						if (Negative) {
							if (DataOk) {
								lcd_write_symbol (ApdValueString, ApdValueColumn, '-');
							}								
						}						
					}					
				}				
				break;

/************************************************************************/
/*                               �����                                  */
/************************************************************************/					
			case APD_FIELD_TYPE_TEXT:
				
				ApdTextString = ApdPrjData[ApdFrameFieldStartAdr[ApdFrameCur][IdField] + 1] - ApdFrameDisplayShift;
				if ((ApdTextString >= 0) && (Abs (ApdTextString) < (ApdPrjData[8]))) {
					
					ApdTextString = Abs (ApdTextString);
					ApdTextColumn = ApdPrjData[ApdFrameFieldStartAdr[ApdFrameCur][IdField] + 2];
					ApdTextLen = ApdPrjData[ApdFrameFieldStartAdr[ApdFrameCur][IdField] + 3];	
					
					// ����� �������� � ���������������
					ApdTextPlcAdr = (ApdPrjData[ApdFrameFieldStartAdr[ApdFrameCur][IdField] + 4]) | (ApdPrjData[ApdFrameFieldStartAdr[ApdFrameCur][IdField] + 5]<<8);
					++ ApdTextPlcAdr; // ��������� 1, �.�. � APD ����� ����������� �� 1
					
					// ���� �� APD ����������� � ������������ ������� ������
					ApdTextIdMin = ApdPrjData[ApdFrameFieldStartAdr[ApdFrameCur][IdField] + 7];
					ApdTextIdMax = ApdPrjData[ApdFrameFieldStartAdr[ApdFrameCur][IdField] + 8];
					// �� ������ ������� ������ �������
					DataOk = 0;
					for (Id = 0; Id <= (FocusDataAmount - 1); Id++) {
						// ���������� ����� � ��������������� (�������� ����) � ���� ��������, ������� ����
						// �������� (������ ������ ���� ���������, � �������� �� ��� ������) � ������� ������ �����
						if ((ApdTextPlcAdr) == FocusDataAdr[Id]) {					
							// ���� ������� ����� �������, �� ��������� ��� ������
							if (FocusDataStatus[Id]) {
								// ���� �� �������� �� � �������,
								// �� ����� ����� ������ �� �������� ��� ���������� ���������
								ApdTextId = FocusData[Id];
								DataOk = 1;
							} else {
								DataOk = 0;
							}							
						}						
					}
					// ����� ������ � ����������� �� �������� ��������� � �������					
					if (((ApdTextId <= ApdTextIdMax) && (ApdTextId >= ApdTextIdMin)) && (DataOk)) {
						for (ApdTextSymbolId = 0; ApdTextSymbolId <= (ApdTextLen - 1); ApdTextSymbolId++) {
							// ���� �� � �������, ������ ����� �� ������� APD
							lcd_write_symbol (ApdTextString, ApdTextColumn + ApdTextSymbolId, ApdPrjFontTable[ApdPrjData[ApdFrameFieldStartAdr[ApdFrameCur][IdField] + 9 + ApdTextSymbolId + ApdTextId*ApdTextLen]]);
						}
					} else {
						for (ApdTextSymbolId = 0; ApdTextSymbolId <= (ApdTextLen - 1); ApdTextSymbolId++) {
							// ���� ���, ������ ����� '?'
							lcd_write_symbol (ApdTextString, ApdTextColumn + ApdTextSymbolId, '?');
						}											
					}				
				}								
				break;

/************************************************************************/
/*                                ����                                  */
/************************************************************************/					
			case APD_FIELD_TYPE_DATE:
				DataShowString = ApdPrjData[ApdFrameFieldStartAdr[ApdFrameCur][IdField] + 1] - ApdFrameDisplayShift;
				if ((DataShowString >= 0) && (Abs (DataShowString) < (ApdPrjData[8]))) {
					DataShowString = Abs (DataShowString);	
					DataShowColumn = ApdPrjData[ApdFrameFieldStartAdr[ApdFrameCur][IdField] + 2];
					DataShowEn = 1;
					DataAppear = 1;
				}					
				break;
				
/************************************************************************/
/*                               �����                                  */
/************************************************************************/														
			case APD_FIELD_TYPE_TIME:
				ClockShowString = ApdPrjData[ApdFrameFieldStartAdr[ApdFrameCur][IdField] + 1] - ApdFrameDisplayShift;
				if ((ClockShowString >= 0) && (Abs (ClockShowString) < (ApdPrjData[8]))) {
					ClockShowString = Abs (ClockShowString);				
					ClockShowColumn = ApdPrjData[ApdFrameFieldStartAdr[ApdFrameCur][IdField] + 2];
					ClockShowEn = 1;
					ClockAppear = 1;
				}				
				break;
			default:
				break;			
		}
		if (ClockAppear == 0) {
			ClockShowEn = 0;
		}
		if (DataAppear == 0) {
			DataShowEn = 0;
		}
	}
}

// ��������� ������� ������ � ������� APD
void ScreenModeNormalAction (uint8_t Action) {
	for (uint8_t Id = 0; Id <= ApdFrameActionsAmount[ApdFrameCur]; Id++) {
		if (ApdPrjData[ApdFrameActionStartAdr[ApdFrameCur][Id]] == Action) {
			ApdFrameActionFunction[ApdPrjData[ApdFrameActionStartAdr[ApdFrameCur][Id] + 1]](Id);
		}
	}
}	
	
void ScreenModeService (void) {
	lcd_write_str (0, 0, (uint8_t *)"> SERVICE");
	lcd_write_str (1, 0, (uint8_t *)"F+1 TIME :: F+4 REG ");
	lcd_write_str (2, 0, (uint8_t *)"F+2 DIAG :: F+5 SET ");
	lcd_write_str (3, 0, (uint8_t *)"F+3 DEV  ::         ");
}

void ScreenModeErrorDiagnostic (void) {
	lcd_write_str (0, 0, (uint8_t *)"> DIAGNOSTIC");
	lcd_write_str (1, 0, (uint8_t *)"RADIO...............");
	lcd_write_str (2, 0, (uint8_t *)"APD_PRJ.............");
	lcd_write_str (3, 0, (uint8_t *)"CLOCK...............");
	if (OpkErrorRad) {
		lcd_write_str (1, 15, (uint8_t *)"ERROR");
	} else {
		lcd_write_str (1, 18, (uint8_t *)"OK");		
	}
	if (OpkErrorPrj) {
		lcd_write_str (2, 15, (uint8_t *)"ERROR");
	} else {
		lcd_write_str (2, 18, (uint8_t *)"OK");		
	}
	if (OpkErrorClock) {
		lcd_write_str (3, 15, (uint8_t *)"ERROR");
	} else {
		lcd_write_str (3, 18, (uint8_t *)"OK");		
	}
}

/************************************************************************/

void ClockShowOn (uint8_t String, uint8_t Column) {
	if (RtcGetTime ()) {
		OpkErrorClock = 0;
		if ((TimeGetSeconds & 0x0F)%2) {				
			lcd_write_symbol (String, Column + 2, 0x3A);
			lcd_write_symbol (String, Column + 5, 0x3A);					
		} else {					
			lcd_write_symbol (String, Column + 2, ' ');
			lcd_write_symbol (String, Column + 5, ' ');					
		}					
		lcd_write_value (String, Column, 2, 10*((TimeGetHours & 0x30)>>4) + (TimeGetHours & 0x0F));
		lcd_write_value (String, Column + 3, 2, 10*((TimeGetMinutes & 0x70)>>4) + (TimeGetMinutes & 0x0F));
		lcd_write_value (String, Column + 6, 2, 10*((TimeGetSeconds & 0x70)>>4) + (TimeGetSeconds & 0x0F));
	} else {
		lcd_write_str (String, Column, (uint8_t *)"??:??:??");
		OpkErrorClock = 1;
	}
}

void DataShowOn (uint8_t String, uint8_t Column) {
	if (RtcGetTime ()) {
		OpkErrorClock = 0;
		lcd_write_symbol (String, Column + 2, 0x2E);
		lcd_write_symbol (String, Column + 5, 0x2E);					
		lcd_write_value (String, Column, 2, 10*((TimeGetDate & 0x30)>>4) + (TimeGetDate & 0x0F));
		lcd_write_value (String, Column + 3, 2, 10*((TimeGetMonth & 0x10)>>4) + (TimeGetMonth & 0x0F));
		lcd_write_value (String, Column + 6, 2, 10*((TimeGetYear & 0xF0)>>4) + (TimeGetYear & 0x0F));
	} else {
		lcd_write_str (String, Column, (uint8_t *)"??.??.??");
		OpkErrorClock = 1;
	}
}

void AdpTransStart (void) {
	if ((usart_data_register_is_empty (&USARTC0)) && (AdpTransMesByteCnt == 0x0000) && (AdpTransEn)) {					
		// ������� ������� ����� �������
		usart_put (&USARTC0, AdpTransMesBuf[AdpTransMesByteCnt]);
		AdpTransMesByteCnt++; // ��������� �������� ���� ���������
		// �������� ������ ������� �������� ���������� TX
		usart_set_tx_interrupt_level (&USARTC0, USART_TXCINTLVL_MED_gc);									
	}
}

void AdpTransContinue (void) {
	if ((AdpTransMesByteCnt < AdpTransMesLen) && (AdpTransMesByteCnt)) {			
		usart_put (&USARTC0, AdpTransMesBuf[AdpTransMesByteCnt]);
		AdpTransMesByteCnt++;
	} else if (AdpTransMesByteCnt >= AdpTransMesLen) {				
		usart_set_tx_interrupt_level (&USARTC0, USART_TXCINTLVL_OFF_gc);
		AdpTransMesByteCnt = 0;
		AdpTransMesLen = 0;
		AdpTransMesBody[0] = 0;
		AdpTransMesBody[1] = 0;	
		AdpTransMesQueueCnt++;
		
		TcAdpTimeout = 0;
		TcAdpTimeoutEn = 1;
		
		// TcAdpEventsEn = 1;
	}	
}

uint16_t AdpTransMesBuild (uint8_t RecAdr, uint8_t TransAdr, uint8_t CmdCode, uint8_t ClarkAdr, uint16_t AmountOfBytes, uint16_t *CmdBody) {
	uint16_t Id = 0;
	AdpTransMesBuf[0] = RecAdr;
	AdpTransMesBuf[1] = TransAdr;
	AdpTransMesBuf[2] = CmdCode;
	AdpTransMesBuf[3] = ClarkAdr;
	AdpTransMesBuf[4] = AmountOfBytes;
	AdpTransMesBuf[5] = (AmountOfBytes>>8);
	Id = 6;
	while (Id <= (6 + AmountOfBytes - 1)) {
		AdpTransMesBuf[Id] = (*CmdBody & 0x00FF);
		Id++;
		AdpTransMesBuf[Id] = ((*CmdBody & 0xFF00)>>8);
		Id++;
		CmdBody++;			
	}
	uint16_t MesCrc = 0x0000;
	MesCrc = Crc16 ((uint8_t *)AdpTransMesBuf, 6 + AmountOfBytes);
	AdpTransMesBuf[6 + AmountOfBytes] = MesCrc;
	AdpTransMesBuf[6 + AmountOfBytes + 1] = (MesCrc>>8);	
	return (6 + AmountOfBytes + 2);
}

void AdpFunctionRecParams (void) {
	uint8_t Id16 = 0;
	uint8_t Id8 = 0;
	uint8_t IdA = 0;
	uint8_t IdAdr = 0;
	uint16_t EventBuf = 0;
	uint16_t RegBuf = 0;
	
	uint16_t RegMesCnt = 0;
	uint8_t ReadBuff[8] = {0, 0, 0, 0, 0, 0, 0, 0};
	uint8_t WriteBuff[8] = {0, 0, 0, 0, 0, 0, 0, 0};

	uint16_t ByteAmount =  (AdpRecMesBuf[7]<<8) | AdpRecMesBuf[6];	
	uint16_t StartAdr = (AdpRecMesBuf[9]<<8) | AdpRecMesBuf[8];
	uint16_t CurAdr = StartAdr;
	
	uint8_t LcdRefreshScoreLocal = 0;
	
	if (StartAdr == ApdEventAdr) {
		// ���� ������� � ������� ���, ������ �� ������
		if (ApdEventIdAmount) {	
			// ����������� ��������� �����
			for (Id16 = 0; Id16 <= (ADP_EVENTS_REQUEST /* ByteAmount/2 */ - 1); Id16++) {
				EventBuf = AdpRecMesBuf[10 + Id8];
				Id8++;
				EventBuf |= (AdpRecMesBuf[11 + Id8]<<8);
				Id8++;
				if (ApdEventsBuf[Id16] != EventBuf) {
					for (IdA = 0; IdA <= 15; IdA++) {
						if (((EventBuf & (1<<IdA))) && !(ApdEventsBuf[Id16] & (1<<IdA))) {
							
							cli();
							/************************************************************************/
							/*                     ������ ������� � ������                          */
							/************************************************************************/
							
							// ������ ��������� �������
							//cli();
							EEPROM_AttemptCnt = EEPROM_OP_ATTEMPTS;
							while (EEPROM_AttemptCnt) {
								if (EepromReadBlock (REGISTER_START, (uint8_t *) ReadBuff, 8)) break;
								-- EEPROM_AttemptCnt;
							}								
							//sei();
							
							// ���������� ���������� ��������� � �������
							if (!(Crc8 ((uint8_t *) &ReadBuff, 8))) {
								RegMesCnt = (ReadBuff[5]<<8);
								RegMesCnt |= ReadBuff[6];
								// lcd_write_value (1, 14, 6, RegMesCnt);
							}
							
							// ������ ������� �������
							if (RtcGetTime ()) {
								// �� ������ ��������������� �������?
								WriteBuff[0] = (10*((TimeGetHours & 0x30)>>4) + (TimeGetHours & 0x0F)) | ((AdpRecMesBuf[1]%0x50)<<6);
								WriteBuff[1] = 10*((TimeGetMinutes & 0x70)>>4) + (TimeGetMinutes & 0x0F);
								WriteBuff[2] = 10*((TimeGetSeconds & 0x70)>>4) + (TimeGetSeconds & 0x0F);
								WriteBuff[3] = 10*((TimeGetDate & 0x30)>>4) + (TimeGetDate & 0x0F);
								WriteBuff[4] = 10*((TimeGetMonth & 0x10)>>4) + (TimeGetMonth & 0x0F);
								WriteBuff[5] = 10*((TimeGetYear & 0xF0)>>4) + (TimeGetYear & 0x0F);
							} else {
								// ���� ���� ��������, ����� ��� ����
								// �� ������ ��������������� �������?
								WriteBuff[0] = 0 | ((AdpRecMesBuf[1] % 0x50)<<6);
								WriteBuff[1] = 0;
								WriteBuff[2] = 0;
								WriteBuff[3] = 0;
								WriteBuff[4] = 0;
								WriteBuff[5] = 0;
							}
							
							// ������ �������� �������							
							WriteBuff[6] = (Id16 * 16) + IdA;
							WriteBuff[7] = Crc8 (WriteBuff, 7);
							
							// ������ ������� � ������
							WP_OFF;
							//cli();
							EEPROM_AttemptCnt = EEPROM_OP_ATTEMPTS;
							while (EEPROM_AttemptCnt) {
								if (EepromWriteBlock (REGISTER_START + (RegMesCnt + 1) * 8, (uint8_t *) WriteBuff, 8)) break;
								-- EEPROM_AttemptCnt;
							}	
							//sei();
							WP_ON;
							
							// ��������� �������� �������
							++ RegMesCnt;							
							
							// ���������� ��������� �������
							WriteBuff[0] = rand ();
							WriteBuff[1] = 0;
							WriteBuff[2] = 0;
							WriteBuff[3] = 0;
							WriteBuff[4] = 0;
							WriteBuff[5] = (uint8_t)(RegMesCnt>>8); // ������� ����	
							WriteBuff[6] = (uint8_t)(RegMesCnt); // ������� ����
							WriteBuff[7] = Crc8 (WriteBuff, 7);	
							
							// ������ ��������� �������
							WP_OFF;
							//cli();
							EEPROM_AttemptCnt = EEPROM_OP_ATTEMPTS;
							while (EEPROM_AttemptCnt) {
								if (EepromWriteBlock (REGISTER_START, (uint8_t *) WriteBuff, 8)) {
									// lcd_write_value (3, 14, 6, RegMesCnt);
									break;
								}									
								-- EEPROM_AttemptCnt;
							}							
							//sei();
							WP_ON;
						
							sei();								
						}
					}
				}
				ApdEventsBuf[Id16] = EventBuf;
				CurAdr += Id16;					
			}
		}				
	} else {
		if (FocusDataAmount) {
			CurAdr += 1000 * (AdpRecMesBuf[1] - 0x50);
			Id16 = 0;		
//			for (Id16 = 0; Id16 <= (ByteAmount/2 - 1); Id16++) {
			while (Id16 <= (ByteAmount/2 - 1)) {
				CurAdr += Id16;
				for (IdAdr = 0; IdAdr <= /* APD_MAX_FIELDS */ (FocusDataAmount - 1); IdAdr++) {
					
					if (FocusDataAdr[IdAdr] == CurAdr) {
						RegBuf = (AdpRecMesBuf[10 + Id16*2]);
//						Id8++;
						RegBuf |= (AdpRecMesBuf[10 + Id16*2 + 1]<<8);
//						Id8++;
						
						
						if (RegBuf != FocusData[IdAdr]) LcdRefreshScoreLocal++;
						
						
						FocusData[IdAdr] = RegBuf;
						FocusDataCntNext[IdAdr] = BROKEN_MES;
					
//						FocusDataStatus[IdAdr] = 1;
						
						TcAdpTransFaultEn = 1;
					}
				}
				Id16++;
			}				
			if (LcdRefreshScoreLocal) {
				FocusDataMesNew = 1;
				LcdRefreshScoreLocal = 0;
			}
		}
	}		
}

/************************************************************************/
/*                              MAIN                                    */
/************************************************************************/
int main (void) {	
	
	cli();
	
/************************************************************************/
/*           ��������� ������� � �������� ������ ������                 */
/************************************************************************/	
	OSC.XOSCCTRL |= OSC_FRQRANGE_12TO16_gc | OSC_XOSCSEL_XTAL_16KCLK_gc;
// ���������� ������ ������
	OSC.CTRL |= OSC_XOSCEN_bm;
// �������� ���������� ������	
	while ((OSC.STATUS & OSC_XOSCRDY_bm) == 0) {
	}
/*
	CCP = CCP_IOREG_gc;
//// ����� ������ ��� ��������� ������	
	CLK.CTRL = CLK_SCLKSEL_XOSC_gc;
	CCP = CCP_IOREG_gc;
//// ��������� �����������	
	CLK.PSCTRL = CLK_PSADIV_1_gc | CLK_PSBCDIV_1_1_gc;
	CCP = CCP_IOREG_gc;
//// ���������� ��������� ��������
	CLK.LOCK = CLK_LOCK_bm;
*/
	ccp_write_io ((uint16_t *)&CLK.CTRL, CLK_SCLKSEL_XOSC_gc);
	ccp_write_io ((uint16_t *)&CLK.PSCTRL, CLK_PSADIV_1_gc | CLK_PSBCDIV_1_1_gc);
	ccp_write_io ((uint16_t *)&CLK.LOCK, CLK_LOCK_bm);	
// ���������� ���� �����������, ����� ������	
	OSC.CTRL &= OSC_XOSCEN_bm;
// ��������� PMIC
	pmic_init ();
	pmic_enable_level (PMIC_LVL_MEDIUM);
	
/************************************************************************/
/*                    ��������� ������������ �������                    */
/************************************************************************/		
	sysclk_enable_module (SYSCLK_PORT_C, SYSCLK_USART0);
	sysclk_enable_module (SYSCLK_PORT_E, SYSCLK_USART0);	
	sysclk_enable_module (SYSCLK_PORT_C, SYSCLK_TWI);
	sysclk_enable_module (SYSCLK_PORT_E, SYSCLK_TWI);	
	sysclk_enable_module (SYSCLK_PORT_C, SYSCLK_TC1);		
	sysclk_enable_module (SYSCLK_PORT_D, SYSCLK_TC1);
	
/************************************************************************/
/*                         ������ ������� APD                           */
/************************************************************************/
// �������� ����������� ������� APD
if ((ApdPrjData[0] == 0x41) && (ApdPrjData[1] == 0x44)) {
	OpkErrorPrj = 0;
} else {
	OpkErrorPrj = 1;
}
// ����� ������� ������ ������
if (ApdPrjData[0x000B]) {
	ApdFrameRefreshRate = ApdPrjData[0x000B]*100; // ������ ������ ������ (0-����,1-100��,2-200,3...10)
} else {
	ApdFrameRefreshRate = 2000;
}
TcAdpTransPer = ApdFrameRefreshRate;

// ����� ������� ������ �������
if (ApdPrjData[0x000C]) {
	ApdEventRefreshRate = ApdPrjData[0x000C]*100; // ������ ������ ������� (0-����,1-100��,2-200,3...10)
} else {
	ApdEventRefreshRate = 2000;
}
TcAdpEventsPer = ApdEventRefreshRate;
TcAdpTransFaultPer = 3 * TcAdpTransPer; // ������ ������� ��������� ������ ��������

// ����� �������� ������ � ���
switch (ApdPrjData[0x000A]) {
	case 0:
		ApdBaudRate = 9600;
		break;
	case 1:
		ApdBaudRate = 19200;	
		break;
	case 2:
		ApdBaudRate = 38400;		
		break;	
	case 3:
		ApdBaudRate = 57600;		
		break;
	default:
		ApdBaudRate = 9600;
		break;							
}

ApdFrameAmount = ApdPrjData[0x0010]; // ���������� ������ � �������
ApdFrameStartNumber = ApdPrjData[0x0011]; // ����� ���������� �����

ApdFrameCur = ApdFrameStartNumber; // ������� ���� = ��������� ���� �������

ApdFramesStartAdr = (ApdPrjData[0x0014]<<8) | (ApdPrjData[0x0013]); // ����� �������� ������
ApdEventsStartAdr = (ApdPrjData[0x0016]<<8) | (ApdPrjData[0x0015]); // ����� �������� ������ �������

// ���������� �������� ������ �������
ApdEventIdAmount = (ApdPrjData[ApdEventsStartAdr + 1]<<8) | (ApdPrjData[ApdEventsStartAdr]);
if (ApdEventIdAmount) {	
	// ����� ������ ������ ������� � ���
	ApdEventAdr = (ApdPrjData[ApdEventsStartAdr + 3]<<8) | (ApdPrjData[ApdEventsStartAdr + 2]);
	ApdEventAdr += 1;
	// ���������� ������� ������� �������� �������
	for (IdEvent = 0; IdEvent <= (ApdEventIdAmount - 1); IdEvent++) {
		ApdEventStartAdr[IdEvent] = ApdEventsStartAdr + 8 + IdEvent*20;
	}
}	

// ���������� ������� ������� �������� ������:
// ����� ������� ������ - ����� �������� ������. ����� ������ �����������
// �������� � ������������ � ����������� ������ � �������
for (IdFrame = 0; IdFrame <= (ApdFrameAmount - 1); IdFrame++) {
	ApdFrameStartAdr[IdFrame] |= ApdPrjData[ApdFramesStartAdr + Id8]; // ������� ����
	Id8++;
	ApdFrameStartAdr[IdFrame] |= (ApdPrjData[ApdFramesStartAdr + Id8]<<8); // ������� ����
	Id8++;
}

Id8 = 0;

// ���������� ������� �������� ����� � �������� � �����:
// ����� ������� ������ - ����� �� ������� ������� �������� ������. ����� ������ �����������
// �������� � ������������ � ����������� ������ � �������.
// � ������� ������ - ����� �������� ����� �����, � ������ - ����� ��������
// �������������� �����.
for (IdFrame = 0; IdFrame <= (ApdFrameAmount - 1); IdFrame++) {
	// ����� �������� ����� �����
	ApdFrameFieldsAndActionsStartAdr[IdFrame][0] |= ApdPrjData[ApdFrameStartAdr[IdFrame] + Id8]; // ������� ����
	Id8++;
	ApdFrameFieldsAndActionsStartAdr[IdFrame][0] |= (ApdPrjData[ApdFrameStartAdr[IdFrame] + Id8]<<8); // ������� ����
	Id8++;
	// ����� �������� �������������� �����
	ApdFrameFieldsAndActionsStartAdr[IdFrame][1] |= ApdPrjData[ApdFrameStartAdr[IdFrame] + Id8]; // ������� ����
	Id8++;
	ApdFrameFieldsAndActionsStartAdr[IdFrame][1] |= (ApdPrjData[ApdFrameStartAdr[IdFrame] + Id8]<<8); // ������� ����
	Id8++;
	
	Id8 = 0;	
}

// ��������� ������ ���������� ����� � �������������� ������� ����� 
for (IdFrame = 0; IdFrame <= (ApdFrameAmount - 1); IdFrame++) {
	ApdFrameFieldsAmount[IdFrame] = ApdPrjData[ApdFrameFieldsAndActionsStartAdr[IdFrame][0]];
	ApdFrameActionsAmount[IdFrame] = ApdPrjData[ApdFrameFieldsAndActionsStartAdr[IdFrame][1]];
}	

Id8 = 0;

// ��������� ������ ������� ����� ������� �����
for (IdFrame = 0; IdFrame <= (ApdFrameAmount - 1); IdFrame++) {
	for (IdField = 0; IdField <= ApdFrameFieldsAmount[IdFrame] - 1; IdField++) {
		ApdFrameFieldStartAdr[IdFrame][IdField] |= ApdPrjData[ApdFrameFieldsAndActionsStartAdr[IdFrame][0] + 1 + Id8];
		Id8++;
		ApdFrameFieldStartAdr[IdFrame][IdField] |= (ApdPrjData[ApdFrameFieldsAndActionsStartAdr[IdFrame][0] + 1 + Id8]<<8);
		Id8++;
	}
	Id8 = 0;
}	

Id8 = 0;

// ��������� ������ ������� �������������� ������� �����
for (IdFrame = 0; IdFrame <= (ApdFrameAmount - 1); IdFrame++) {
	for (IdAction = 0; IdAction <= ApdFrameActionsAmount[IdFrame] - 1; IdAction++) {
		ApdFrameActionStartAdr[IdFrame][IdAction] |= ApdPrjData[ApdFrameFieldsAndActionsStartAdr[IdFrame][1] + 1 + Id8];
		Id8++;
		ApdFrameActionStartAdr[IdFrame][IdAction] |= (ApdPrjData[ApdFrameFieldsAndActionsStartAdr[IdFrame][1] + 1 + Id8]<<8);
		Id8++;
	}
	Id8 = 0;	
}

Id8 = 0;

uint8_t ApdFrameDataFromPlcCnt = 0;

// ����� ������ ������������� �� ��� � ������ ����� � ������� ��?
for (IdFrame = 0; IdFrame <= (ApdFrameAmount - 1); IdFrame++) {
	if (ApdFrameFieldsAmount[IdFrame] > 1) {
		for (IdField = 0; IdField <= (ApdFrameFieldsAmount[IdFrame] - 1); IdField++) {
			// ���� ��� ���� - ����� ��� �����:
			if ((ApdPrjData[ApdFrameFieldStartAdr[IdFrame][IdField]] == APD_FIELD_TYPE_VALUE) || (ApdPrjData[ApdFrameFieldStartAdr[IdFrame][IdField]] == APD_FIELD_TYPE_TEXT)) {
				// ��������� ��� ����� � ��� � ������ 16 ���:
				// ������� ����
				ApdFrameDataFromPlcAdr[IdFrame][ApdFrameDataFromPlcCnt] = ApdPrjData[ApdFrameFieldStartAdr[IdFrame][IdField] + 4 + Id8];
				Id8++;
				// ������� ����
				ApdFrameDataFromPlcAdr[IdFrame][ApdFrameDataFromPlcCnt] |= (ApdPrjData[ApdFrameFieldStartAdr[IdFrame][IdField] + 4 + Id8]<<8);
				ApdFrameDataFromPlcAdr[IdFrame][ApdFrameDataFromPlcCnt] += 1;
				Id8++;
				// ���������� ������ ����������� ���������� �� �������
				ApdFrameDataFromPlcString[IdFrame][ApdFrameDataFromPlcCnt] = ApdPrjData[ApdFrameFieldStartAdr[IdFrame][IdField] + 1];
				// ������� ���������� ����� ���� �����
				ApdFrameDataFromPlcCnt++;		
			}
			Id8 = 0;
		}
		// ���������� ��������� ���������� ����� ���� ����� � ������ �������
		ApdFrameDataFromPlcAmount[IdFrame] = ApdFrameDataFromPlcCnt;
		ApdFrameDataFromPlcCnt = 0;
		Id8 = 0;	
	} else {
		ApdFrameDataFromPlcAmount[IdFrame] = 0;		
	}		
}
	
/************************************************************************/
/*                         ��������� UARTE0                             */
/************************************************************************/
// ��������� PIN3 �� ����� (��������)
	PORTE.DIR |= TX0;
//	PORTE.OUT |= TX0;
// ��������� PIN2 �� ���� (����)	
	PORTE.DIR &= ~ RX0;
// ��������� ������������ ������ USARTE0	
//	sysclk_enable_module (SYSCLK_PORT_E, SYSCLK_USART0);
//	sysclk_enable_module (SYSCLK_PORT_C, SYSCLK_TWI);
//	PR.PRPE |= (1<<5);
// ��������� �������� ������	
	usart_set_baudrate (&USARTE0, /*CONFIG_USART_BAUDRATE*/ 9600, /*BOARD_XOSC_HZ*/ 18432000);
//	USARTE0.BAUDCTRLA = 119;
// ����� ������ ������ USARTE0	
	usart_set_mode (&USARTE0, USART_CMODE_ASYNCHRONOUS_gc);
// ��������� ������� �������	
	usart_format_set (&USARTE0, USART_CHSIZE_8BIT_gc, USART_PMODE_DISABLED_gc, false);
// ��������� ��������	
	usart_rx_enable (&USARTE0);
// ��������� �����������	
	usart_tx_enable (&USARTE0);
// ������ ����������
	usart_set_rx_interrupt_level (&USARTE0, USART_RXCINTLVL_MED_gc);

/************************************************************************/
/*                         ��������� UARTC0                             */
/************************************************************************/
// ��������� PIN3 �� ����� (��������)
	PORTC.DIR |= TX0;
//	PORTC.OUT |= TX0;
// ��������� PIN2 �� ���� (����)	
	PORTC.DIR &= ~ RX0;
// ��������� ������������ ������ USARTC0	
//	sysclk_enable_module (SYSCLK_PORT_C, SYSCLK_USART0);
//	sysclk_enable_module (SYSCLK_PORT_C, SYSCLK_TWI);
//	PR.PRPE |= (1<<5);
// ��������� �������� ������	
	usart_set_baudrate (&USARTC0, /*CONFIG_USART_BAUDRATE*/ ApdBaudRate, /*BOARD_XOSC_HZ*/ 18432000);
//	USARTE0.BAUDCTRLA = 119;
// ����� ������ ������ USARTC0	
	usart_set_mode (&USARTC0, USART_CMODE_ASYNCHRONOUS_gc);
// ��������� ������� �������	
	usart_format_set (&USARTC0, USART_CHSIZE_8BIT_gc, USART_PMODE_DISABLED_gc, false);
// ��������� ��������	
	usart_rx_enable (&USARTC0);
// ��������� �����������	
	usart_tx_enable (&USARTC0);
// ������ ����������
	usart_set_rx_interrupt_level (&USARTC0, USART_RXCINTLVL_MED_gc);	
	
/************************************************************************/
/*                         ��������� �������                            */
/************************************************************************/		
	PORTA.DIR |= LED;	
	PORTA.OUT |= LED;
	PORTB.DIR |= EN | RS;
	EN_SET;
	lcd_init ();
	
/************************************************************************/
/*                         ��������� �������                            */
/************************************************************************/
	Timer1msInit ();
	TimerRs232Init ();
	
/************************************************************************/
/*                         ��������� I2C (PORTC - RTC)                  */
/************************************************************************/	
	C_TwiInit ();
	
/************************************************************************/
/*                         ��������� �������                            */
/************************************************************************/		
	if (RtcGetTime ()) {
		OpkErrorClock = 0;
	} else {
		OpkErrorClock = 1;
	}

/************************************************************************/
/*                         ��������� I2C (PORTE - EEPROM)               */
/************************************************************************/	
	E_TwiInit ();
	
/************************************************************************/
/*                         ��������� ������� �������                    */
/************************************************************************/		
	for (uint16_t IdArray = 0; IdArray <= (ADP_EVENTS_REQUEST - 1); IdArray++) {
		ApdEventsBuf[IdArray] = rand ();
	}
			
	WP_ON;
		
	EEPROM_WriteBuff[0] = 0;
	EEPROM_WriteBuff[1] = 0;
	EEPROM_WriteBuff[2] = 0;
	EEPROM_WriteBuff[3] = 0;
	EEPROM_WriteBuff[4] = 0;
	EEPROM_WriteBuff[5] = 0;	
	EEPROM_WriteBuff[6] = 0;
	EEPROM_WriteBuff[7] = Crc8 (EEPROM_WriteBuff, 7);
	
	WP_OFF;
	EEPROM_AttemptCnt = EEPROM_OP_ATTEMPTS;
	while (EEPROM_AttemptCnt) {
		if (EepromWriteBlock (REGISTER_START, EEPROM_WriteBuff, 8)) break;
		-- EEPROM_AttemptCnt;
	}	
	WP_ON;

	EEPROM_AttemptCnt = EEPROM_OP_ATTEMPTS;
	while (EEPROM_AttemptCnt) {
		if (EepromReadBlock (REGISTER_START, EEPROM_ReadBuff, 8)) break;
		-- EEPROM_AttemptCnt;
	}				
	
	if (!(Crc8 ((uint8_t *) &EEPROM_ReadBuff, 8))) {
		EEPROM_MesCnt = EEPROM_ReadBuff[6] | (EEPROM_ReadBuff[5]<<8);	
	} else {
		EEPROM_WriteBuff[0] = 0;
		EEPROM_WriteBuff[1] = 0;
		EEPROM_WriteBuff[2] = 0;
		EEPROM_WriteBuff[3] = 0;
		EEPROM_WriteBuff[4] = 0;
		EEPROM_WriteBuff[5] = 0;	
		EEPROM_WriteBuff[6] = 0;
		EEPROM_WriteBuff[7] = Crc8 (EEPROM_WriteBuff, 7);
		WP_OFF;
		EEPROM_AttemptCnt = EEPROM_OP_ATTEMPTS;
		while (EEPROM_AttemptCnt) {
			if (EepromWriteBlock (REGISTER_START, EEPROM_WriteBuff, 8)) break;
			-- EEPROM_AttemptCnt;
		}					
		WP_ON;
		EEPROM_MesCnt = 0;
	}
	
/************************************************************************/
/*                          ������                                      */
/************************************************************************/
	for (uint8_t IdA = 0; IdA <= (APD_MAX_FIELDS - 1); IdA++) {
		FocusDataAdr[IdA] = 0;
		FocusData[IdA] = 0;
		FocusDataCntNext[IdA] = 0;
		FocusDataCntPrev[IdA] = 0;
		FocusDataStatus[IdA] = 0;
	}
	FocusDataAmount = 0;
	
	uint8_t LcdRefreshScore = 0;
	
	sei();

	while (1) {
		
/************************************************************************/
/*               ��������� ������������ �����������.                    */
/************************************************************************/
		if (TcRadCfg >= TcRadCfgPer) { // ���� �������� ����������� ������			
			TcRadCfg = 0;
			if (OpkErrorRad) {
				OpkErrorScoreRadGlobal++;
				if (OpkErrorScoreRadGlobal > OpkErrorScoreRadGlobalLimit) {
					OpkErrorScoreRadGlobal = 0;
					RADIO_RESET_OFF; // ������ �������� ������ � ���������� ����� ������
					// ������� ��������� ������� ��� ������ ������� ��������� ������������
					RadCfgCmdCnt = 0;
					RadCfgCmdCharCnt = 0;
					RadCfgCmdAck = 1;
					RadCfgAckMes[0] = 0x0D;
					RadCfgAckMes[1] = 0x0A;
					RadCfgAckMes[2] = 0x4F;
					RadCfgAckMes[3] = 0x4B;
					RadCfgAckMes[4] = 0x0A;
					RadCfgAckMes[5] = 0x0D;
					RadCfgAckMes[6] = 0x3E;	
					RadCfgAckMesReady = 1;
				}
			}
			// ��������� ������ �� ����������� �� ������� ������������			
			if (RadCfgAckMesReady) { // ���� ���� �������� ����� ��������� ������������ �������
				RadCfgAckMesReady = 0; // �������� ��������� ����������, ����� ��������� ���
				OpkErrorScoreRad = 0;
				OpkErrorRad = 0;
				// �������� ����������� ���������			
				if ((RadCfgAckMes[0] == 0x0D) && (RadCfgAckMes[1] == 0x0A) && (RadCfgAckMes[2] == 0x4F) && (RadCfgAckMes[3] == 0x4B) && (RadCfgAckMes[4] == 0x0A) && (RadCfgAckMes[5] == 0x0D) && (RadCfgAckMes[6] == 0x3E)) {						
					RadCfgAckMes[0] = 0;
					RadCfgAckMes[1] = 0;
					RadCfgAckMes[2] = 0;
					RadCfgAckMes[3] = 0;
					RadCfgAckMes[4] = 0;
					RadCfgAckMes[5] = 0;
					RadCfgAckMes[6] = 0;																		
					RadCfgCmdAck = 1;	// ���� ������� ����� "��"
				} else {											
					RadCfgCmdAck = 0;	// ���� �� �������� ������, ��� ����� �� "��"
				}
			// ��������� ������ ������������ ����������� (�����������)						
			} else { // ���� ��������� ������������ �������� �� ����
				OpkErrorScoreRad++; // ��������� �������� �������� ������������
				if (OpkErrorScoreRad < OpkErrorScoreRadLimit) {
					// ���������� ���� ������
				} else { // ���� �������� ����� �� �������� ������������
					OpkErrorScoreRadTry++; // ��������� �������� ������� ����������������
					OpkErrorScoreRad = 0; // ����� �������� �������� ������������
					if (OpkErrorScoreRadTry < OpkErrorScoreRadTryLimit) {
						RADIO_RESET_OFF; // ������ �������� ������ � ���������� ����� ������
						// ������� ��������� ������� ��� ������ ������� ��������� ������������
						RadCfgCmdCnt = 0;
						RadCfgCmdCharCnt = 0;
						RadCfgCmdAck = 1;
						RadCfgAckMes[0] = 0x0D;
						RadCfgAckMes[1] = 0x0A;
						RadCfgAckMes[2] = 0x4F;
						RadCfgAckMes[3] = 0x4B;
						RadCfgAckMes[4] = 0x0A;
						RadCfgAckMes[5] = 0x0D;
						RadCfgAckMes[6] = 0x3E;	
						RadCfgAckMesReady = 1;							
					} else { // ���� �������� ����� �� ���������� ������� ����������������
						// ������ ������������ ����������� ��������� �����������:
						// ����� ���������� ��� �����������.
						// ������� ���������� ������ ��-� ����������: ��������� ��� �������
						OpkErrorScoreRadTry = 0;
						RADIO_RESET_OFF; // ������ �������� ������ � ���������� ����� ������
						OpkErrorRad = 1; // ��� ������ "������ ������������ �����������"
						RadCfgCmdCnt = 255;
						RadCfgCmdCharCnt = 255;
						RadCfgCmdAck = 0;
						//TcRadCfgEn = 0; // ������ ������ ������� - ������ ������ ��������� ������������ ������
						//TcRtcEn = 0; // ������ ������ ������� ���������� �����
						//RadRecEn = 0; // ������ ����� �������������� ���������
						OpkMode = OPK_MODE_ERROR_DIAGNOSTIC; // ����� �������� ������ ��� �����������
						LcdRefresh = 1; // ���������� �� �����������
					}							
				}				
			}							
			// ����� ������� �������� ������������ �� �������
			// ���� �� �������� ������� �������, ������ ������������ �� ���������
			if (!OpkErrorRad) { // ���� �� ���������� ������ ���������������� �����������
				if (RadCfgCmdCnt <= (RadCfgCmdAmount - 1)) {
					if (RadCfgCmdCnt == 0) {
						RADIO_RESET_ON; // ��������� ����������� ������ ������
					}
					// ���� DATA ������� ����,
					// � ������� �������� �������� ����� 0,
					// � ������� ����� "��" �� ���������� ��������,
					// ���������� �������� ����� ��������
					if ((usart_data_register_is_empty (&USARTE0)) && (RadCfgCmdCharCnt == 0) && (RadCfgCmdAck)) {					
						RadCfgCmdAck = 0;	// ������������ ���� ������������ ��������
						// ������� ������� ������� ��������
						usart_put (&USARTE0, RadCfg[RadCfgCmdCnt][RadCfgCmdCharCnt + 1]);
						RadCfgCmdCharCnt++; // ��������� �������� �������� ��������
						// RadCfgCmdAck = 0;
						// �������� ������ ������� �������� ���������� TX
						usart_set_tx_interrupt_level (&USARTE0, USART_TXCINTLVL_MED_gc);
						lcd_write_symbol (3, LoadBar, '*');
						++ LoadBar;									
					}				
				} else {
				// ���� �������� ������� �������, ������ ������������ ���������:
				// ��� �������� ����������� �������
					RADIO_RESET_OFF; // ������ �������� ������ � ���������� ����� ������	
					TcRadCfgEn = 0; // ������ ������ ������� - ������ ������ ��������� ������������ ������
					TcRtcEn = 1; // ������ ������� ���������� �����
					
					
					// TcAdpEventsEn = 1; // ������ ������� ������� ������� �� ��

					 // ���������� �� ���� �������������� ���������:
					 // �������� ��������� ���������������� ��� ��������������
					 // � ���������� �� RX (��. ���������� ����������)
					RadRecEn = 1;
					OpkMode = /* OPK_MODE_DEVELOPE */ /* OPK_MODE_SERVICE */ OPK_MODE_NORMAL; // ����� �������� ������ ��� �����������
					LcdRefresh = 1; // ���������� �� �����������
					TcAdpTransRun = 1;
					// TcAdpTransEn = 1; // ������ ������� ������ � ��
				}
			}				
		}

/************************************************************************/
/*               ���� ����������� ������� �������                       */
/************************************************************************/			
		if (LcdRefresh) {						
			LcdRefresh = 0;
//			lcd_clear ();
			switch (OpkMode) {
				case OPK_MODE_INTRO:
					lcd_clear ();
					lcd_clear ();
					lcd_clear ();
					lcd_clear ();
					ScreenModeIntro ();
					break;
				
				case OPK_MODE_NORMAL:
//					lcd_clear ();
					ScreenModeNormal ();
					break;
			
				case OPK_MODE_SERVICE:
					lcd_clear ();
					ScreenModeService ();
					break;
				
				case OPK_MODE_DEVELOPE:
					lcd_clear ();
					ScreenModeDevelope ();
					break;
				
				case OPK_MODE_TIMESET:
//					lcd_clear ();
//					TimesetModeAction = 3;
					ScreenModeTimeset (TimesetModeAction);
//					TimesetModeAction = 255;
					break;
				
				case OPK_MODE_ERROR_DIAGNOSTIC:
					lcd_clear ();
					ScreenModeErrorDiagnostic ();
					break;
				
				case OPK_MODE_REGISTER:
					lcd_clear ();
					ScreenModeRegister (RegisterModeAction);
					break;
				
				case OPK_MODE_SETTINGS:
//					lcd_clear ();
					ScreenModeSettings (SettingsModeAction);
					break;										
				
				default:
					break;	
			}			
		}		
		
/************************************************************************/
/*                      ����� ������� � ��                  	        */
/************************************************************************/			
		// ���� �������� ������� �� ��������
		if (TcAdpTrans >= TcAdpTransPer) {
			
			if (FocusNew) {
				// FocusNew = 0;
			
				AdrInFocus ();
				SortAcending (FocusDataAdr, FocusDataAmount);
				RequestAdrSpace ();			
				PrepareToRequest ();
				
				if (FocusNew) {
					FocusNew = 0;
					++ LcdRefreshScore;
				}				
			}

/************************************************************************/
/*								��������                                */
/************************************************************************/
			/************************************************************************/
			/*                      ������ ������ ��������                          */
			/************************************************************************/
			// �������� �������
			TcAdpTrans = 0;
			TcAdpTransEn = 0;
			
			TcAdpTransFault = 0;
			TcAdpTransFaultEn = 1;
			
			// ��������� ������ ������ �������
			TcAdpEventsEn = 0;
						
			AdpTransMesQueueCnt = 0;
			// ���������� ���������� ��������� � ������ � ������� ��� ��������
			AdpTransMesQueue = BlockAmount;
			// ���� ������� �� �����
			if (AdpTransMesQueue) {
				// �������� ���� ���������
				AdpTransMesBody[0] = BlockRequest[AdpTransMesQueueCnt][1]; // ������� ���� ������?
				AdpTransMesBody[1] = BlockRequest[AdpTransMesQueueCnt][0] % 1000; // � ������ ������?
				// (������� �������������� ������)
				// ���� ����� ��������� �� �������, �������� ��������� ��� ��������
				if (AdpTransMesLen = AdpTransMesBuild (0x50 + (BlockRequest[AdpTransMesQueueCnt][0] / 1000) /* ADP_TR1 */, ADP_OPK, ADP_FUNC_TRANSMIT_PARAMS, ADP_NULL, 0x0004, (uint16_t *)AdpTransMesBody)) {
					// �������� �������� ���������� ��������� � �������
					AdpTransStart ();
				}
			}			
		}

		/************************************************************************/
		/*               �������� ����� ����� ����� �����������                 */
		/************************************************************************/	
		if (TcAdpTimeout >= TcAdpTimeoutPer) {
			TcAdpTimeout = 0;
			TcAdpTimeoutEn = 0;

			// ���� ������� ��������� ��������� (��� ���� ���������� ���������)	� ������� �� �����					
			if ((AdpTransMesQueueCnt < AdpTransMesQueue) && (AdpTransMesQueue)) {
				AdpTransMesBody[0] = BlockRequest[AdpTransMesQueueCnt][1]; // ������� ���� ������?
				AdpTransMesBody[1] = BlockRequest[AdpTransMesQueueCnt][0] % 1000; // � ������ ������?
				// (������� �������������� ������) 
				// ���� ����� ��������� �� �������, �������� ��������� ��� ��������
				if (AdpTransMesLen = AdpTransMesBuild (0x50 + (BlockRequest[AdpTransMesQueueCnt][0] / 1000) /* ADP_TR1 */, ADP_OPK, ADP_FUNC_TRANSMIT_PARAMS, ADP_NULL, 0x0004, (uint16_t *)AdpTransMesBody)) {
					// �������� �������� ���������� ��������� � �������
					AdpTransStart ();
				}			
			// � ��������� ������
			} else {
				// ���������� ������� ���������
				AdpTransMesQueueCnt = 0;
				// �������� �������
				AdpTransMesQueue = 0;
				
				// ��������, ����� �������� ����� ��� ���
				if (FocusDataAmount) {
				
					LcdRefreshScore = 0;
				
					for (uint8_t Id = 0; Id <= (FocusDataAmount - 1); Id++) {
					
						if (FocusDataCntNext[Id]) { 
							-- FocusDataCntNext[Id];
						}					
						if ((FocusDataCntNext[Id] > 0) && (FocusDataCntPrev[Id] == 0)) {
							FocusDataStatus[Id] = 1;
							++ LcdRefreshScore;
						} else if ((FocusDataCntNext[Id] == 0) && (FocusDataCntPrev[Id] > 0)) {
							FocusDataStatus[Id] = 0;
							++ LcdRefreshScore;
						}
						if (FocusDataMesNew) {
							FocusDataMesNew = 0;
							++ LcdRefreshScore;
						}
					
						FocusDataCntPrev[Id] = FocusDataCntNext[Id];
					}
					// ��������� ���-��, ��������� ���������� ������
					if (LcdRefreshScore) LcdRefresh = 1;
				}	
							
				TcAdpTrans = 0;
				TcAdpTransEn = 1;
			
				TcAdpTransFault = 0;
				TcAdpTransFaultEn = 0;
								
				// ��������� ������ ������ �������
				// TcAdpEventsEn = 1;	
			}			
		}

/************************************************************************/
/*                      ������ ������ ������� �� ��            	        */
/************************************************************************/			
		if (TcAdpEvents >= TcAdpEventsPer) {
			TcAdpEvents = 0;
//			TcAdpEventsEn = 0;
//			TcAdpTimeoutEn = 0;			
			AdpTransMesQueueCnt = 0;
			if (ApdEventIdAmount) {
				AdpTransMesQueue = 1;
			} else {
				AdpTransMesQueue = 0;
			}				
			if (AdpTransMesQueue) {
				AdpTransMesBody[0] = ADP_EVENTS_REQUEST;
				AdpTransMesBody[1] = ApdEventAdr;
				if (AdpTransMesLen = AdpTransMesBuild (ADP_TR1 + EventReqRecNumber%4, ADP_OPK, ADP_FUNC_TRANSMIT_PARAMS, ADP_NULL, 0x0004, (uint16_t *)AdpTransMesBody)) {
					EventReqRecNumber++;
					AdpTransStart ();
				}
			}			
		}
		
/************************************************************************/
/*                ���� ��������� ������ ��� �������� ���������	        */
/************************************************************************/			
		if (TcAdpTransFault >= TcAdpTransFaultPer) {
			// ���� � ������� ���������� �������� ������ �� ���� ��������� ��������			
			TcAdpTransFault = 0;
			TcAdpTransFaultEn = 0; // ������������ ������� ������
			
			// ������������� ������ �������� ���������
			TcAdpTrans = 0;
			TcAdpTransEn = 1;
		}
		
/************************************************************************/
/*       ����������� ����� � �������� ������� ������ RTC     	        */
/************************************************************************/						
// ����������� ����� � �������� ������� (�������� ���������� �� ������, ����������� ������ ClockShowEn)
		if (TcRtc >= TcRtcPer) {			
			TcRtc = 0;
			if (ClockShowEn) {
				ClockShowOn (ClockShowString, ClockShowColumn);
			}
			if (DataShowEn) {
				DataShowOn (DataShowString, DataShowColumn);
			}
		}
					
/************************************************************************/
/*       �������� ��������� ��������� �� ������� ������ (Crc-8)	        */
/************************************************************************/	
		if (RadRecMesReady) {
			RadRecMesReady = 0; // ����� ��������� ����������
			if (!(Crc8 ((uint8_t *) &RadRecMesBuf, RadRecMesBufLen))) {
				RadRecMesCrcOk = 1; // ���� ������ Crc-8 �� ����������
				
				// ������� ���� �������� ������
				// lcd_write_value (0, 0, 3, RadRecMesBuf[1]);
				// lcd_write_value (1, 0, 3, RadRecMesBuf[2]);
				
				//if (OpkMode == OPK_MODE_DEVELOPE) {
					//LcdRefresh = 1;
				//}
			}
		}

/************************************************************************/
/*       �������� ��������� ��������� �� ������� ������ (Crc-16)	    */
/************************************************************************/	
		if (AdpRecMesReady) {
			AdpRecMesReady = 0; // ����� ��������� ����������
			if (!(Crc16 ((uint8_t *) &AdpRecMesBuf, AdpRecMesLen))) {
				AdpRecMesCrcOk = 1; // ���� ������ Crc-16 �� ����������
				
				//LcdRefresh = 1;
				
				if (OpkMode == OPK_MODE_DEVELOPE) {
					LcdRefresh = 1;
				}				
			}
		}		

/************************************************************************/
/*       ���� ��������� ��������� �� ����������������                   */
/************************************************************************/	
		if (AdpRecMesCrcOk) {
			AdpRecMesCrcOk = 0;
			// ������ �������� ����������
			// ��� ����������?
			if (AdpRecMesBuf[0] == ADP_OPK) {
				// ��� ����������?
				if ((AdpRecMesBuf[1] == ADP_TR1) || (AdpRecMesBuf[1] == ADP_TR2) || (AdpRecMesBuf[1] == ADP_TR3) || (AdpRecMesBuf[1] == ADP_TR4)) {
					// ��� �������?
					if (AdpRecMesBuf[2] == (ADP_FUNC_TRANSMIT_PARAMS | (1<<7) /* ���� ������ */)) {
						AdpFunctionRecParams ();
//						LcdRefresh = 1;
					}
					//	if (AdpRecMesBuf[2] == ADP_FUNC_TRANSMIT_PARAMS) {
	//						adp_function_transmit_params ();
					//	}
				}									
			}
			// ��������� ������ ��������� ���������
			/*
			for (uint16_t Id = 0; Id <= (AdpRecMesLen - 1); Id++) {
				AdpRecMesBuf[Id] = 0;
			}
			AdpRecMesLen = 0;
			*/
		}


/************************************************************************/
/*       ���� ��������� ���������� � ������ � ������� �������           */
/************************************************************************/			
		// ��������� ������� ��� ����������� �� ������ (����� ������� �������� � ��������� �����)						
		if (RadRecMesCrcOk) {
			RadRecMesCrcOk = 0; // ������������� �������������� ��������� ����������
			// ������ � ����������
			if ((RadRecMesBuf[1] != ButtonFirst) || (RadRecMesBuf[2] != ButtonSecond)) {
				ButtonRelease = 1;
			}			
			// �������� ������� ������ � ����������� ���������������� ���������
			if (((RadRecMesBuf[1] == ButtonFirst) && (ButtonFirst == 5)) && ((RadRecMesBuf[2] == ButtonSecond) && (ButtonSecond == 0))) {
				// ������ �������� ��������� ���������� ������
				if (ServiceModeToggleCnt < ServiceModeToggleCntLimit) {
					ServiceModeToggleCnt++;
				} else {
					// ���� ���������� ������ ������������ � ������� �������,
					// ������������� ServiceModeToggleCntLimit,
					// ���������� ����� ������ ������
					ServiceModeToggleCnt = 0; // ���� ������ ��������, ����� ��������
					if (OpkMode == OPK_MODE_SERVICE) {

						OpkMode = OPK_MODE_NORMAL;
						ButtonRelease = 0;
						
						TcAdpTransEn = 1;
						TcAdpTrans = 0;
						TcAdpTransFaultEn = 1;
						TcAdpTransFault = 0;
					} else {
						ClockShowEn = 1;
						DataShowEn = 0;
						ClockShowString = 0;
						ClockShowColumn = 12;
						OpkMode = OPK_MODE_SERVICE;
						ButtonRelease = 0;
						
						TcAdpTransEn = 0;
						TcAdpTrans = 0;
						TcAdpTransFaultEn = 0;
						TcAdpTransFault = 0;
					}
					LcdRefresh = 1; // ���������� �� �����������
				}
			} else {				
				ServiceModeToggleCnt = 0; // ���� ������ �� ��������, ���-����� ����� ��������					
			}

			// ������ ������� �������� ������ � �����
			ButtonFirst = RadRecMesBuf[1];
			ButtonSecond = RadRecMesBuf[2];	
			
			// ��������� ������� � ����������� �� ������
			if (ButtonRelease) {
				ButtonRelease = 0;
				switch (OpkMode) {
					case OPK_MODE_NORMAL: // ������� �����
						ApdFrameButtonAction = ApdFrameButtonActionTable[ButtonSecond][ButtonFirst];
						ScreenModeNormalAction (ApdFrameButtonAction);
						ApdFrameButtonAction = 255;
						//ClockShowEn = 0;
						//TcRtcEn = 0;
						//LcdRefresh = 1;
						break;
			
					case OPK_MODE_SERVICE: // ��������� �����
						if ((ButtonFirst == 5) && (ButtonSecond == 9)) {
							OpkMode = OPK_MODE_TIMESET;
							ClockShowEn = 0;
							TcRtcEn = 0;
							LcdRefresh = 1;
						} else if ((ButtonFirst == 5) && (ButtonSecond == 10)) {
							OpkMode = OPK_MODE_ERROR_DIAGNOSTIC;
							ClockShowEn = 0;
							TcRtcEn = 0;
							LcdRefresh = 1;
						} else if ((ButtonFirst == 5) && (ButtonSecond == 11)) {
							OpkMode = OPK_MODE_DEVELOPE;
							ClockShowEn = 0;
							TcRtcEn = 0;
							LcdRefresh = 1;

							TcAdpTransEn = 1;
							TcAdpTrans = 0;
							TcAdpTransFaultEn = 0; //
							TcAdpTransFault = 0;
						
						} else if ((ButtonFirst == 5) && (ButtonSecond == 12)) {
							OpkMode = OPK_MODE_REGISTER;
							RegisterModeShowOnce = 1;
							ClockShowEn = 0;
							TcRtcEn = 0;
							LcdRefresh = 1;
						} else if ((ButtonFirst == 1) && (ButtonSecond == 5)) {
							OpkMode = OPK_MODE_SETTINGS;
							ClockShowEn = 0;
							TcRtcEn = 0;
							LcdRefresh = 1;	
						}												
						break;
					
					case OPK_MODE_DEVELOPE: // ����� ������������
						if ((ButtonFirst == 5) && (ButtonSecond == 11)) {
							OpkMode = OPK_MODE_SERVICE;
							ClockShowEn = 1;
							TcRtcEn = 1;
							LcdRefresh = 1;
							
							TcAdpTransEn = 0;
							TcAdpTrans = 0;
							TcAdpTransFaultEn = 0;
							TcAdpTransFault = 0;						
						}
						// ApdFrameButtonAction = ApdFrameButtonActionTable[ButtonSecond][ButtonFirst];
						//ScreenModeNormalAction (ApdFrameButtonAction);
						//ApdFrameButtonAction = 255;

						
//						LcdRefresh = 1;
						break;
					
					case OPK_MODE_TIMESET: // ����� ��������� �������
						if ((ButtonFirst == 2) && (ButtonSecond == 5)) {
							TimesetModeAction = 0;
							LcdRefresh = 1;
						} else if ((ButtonFirst == 4) && (ButtonSecond == 5)) {
							TimesetModeAction = 1;
							LcdRefresh = 1;
						} else if ((ButtonFirst == 5) && (ButtonSecond == 11)) {
							TimesetModeAction = 2;
							LcdRefresh = 1;
						} else if ((ButtonFirst == 5) && (ButtonSecond == 7)) {
							TimesetModeAction = 3;
							LcdRefresh = 1;
						} else if ((ButtonFirst == 5) && (ButtonSecond == 9)) {
							TimesetModeAction = 4;
							//TimesetModeShowOnce = 1;
							LcdRefresh = 1;
						} else if ((ButtonFirst == 5) && (ButtonSecond == 10)) {
							TimesetModeAction = 5;
							//TimesetModeShowOnce = 1;
							LcdRefresh = 1;
						}	
						break;							
				
					case OPK_MODE_ERROR_DIAGNOSTIC: // ����� ����������� ������
						if ((ButtonFirst == 5) && (ButtonSecond == 10)) {
							OpkMode = OPK_MODE_SERVICE;
							ClockShowEn = 1;
							TcRtcEn = 1;
							LcdRefresh = 1;
						}
						break;
					
					case OPK_MODE_REGISTER: // ����� ����������� ������� �������
			
						if ((ButtonFirst == 4) && (ButtonSecond == 5)) {
							ButtonRelease = 1;
							/************************************************************************/
							/*                          ��������� ������                            */
							/************************************************************************/
							if ((ButtonFirst == ButtonFirstPrev) && (ButtonSecond == ButtonSecondPrev)) {
								if (ButtonCnt) -- ButtonCnt;
							} else {
								ButtonFirstPrev = ButtonFirst;
								ButtonSecondPrev = ButtonSecond;
								ButtonCnt = BUTTON_CNT;
							}
							if (ButtonCnt) {
								RegisterModeAction = 0;
								LcdRefresh = 1;
							} else { 
								RegisterModeAction = 3;
								LcdRefresh = 1;
							}
						} else if ((ButtonFirst == 2) && (ButtonSecond == 5)) {
							ButtonRelease = 1;
							/************************************************************************/
							/*                         ����������� ������                           */
							/************************************************************************/								
							if ((ButtonFirst == ButtonFirstPrev) && (ButtonSecond == ButtonSecondPrev)) {
								if (ButtonCnt) -- ButtonCnt;
							} else {
								ButtonFirstPrev = ButtonFirst;
								ButtonSecondPrev = ButtonSecond;
								ButtonCnt = BUTTON_CNT;
							}
							if (ButtonCnt) {
								RegisterModeAction = 1;
								LcdRefresh = 1;
							} else { 
								RegisterModeAction = 4;
								LcdRefresh = 1;
							}							
						} else if ((ButtonFirst == 5) && (ButtonSecond == 12)) {
							/************************************************************************/
							/*                          ����� �� �������                            */
							/************************************************************************/								
							RegisterModeAction = 2;
							LcdRefresh = 1;
						}
						break;								

					case OPK_MODE_SETTINGS: // ����� ���������
						if ((ButtonFirst == 2) && (ButtonSecond == 5)) {
							SettingsModeAction = 0; // �����
							LcdRefresh = 1;
						} else if ((ButtonFirst == 4) && (ButtonSecond == 5)) {
							SettingsModeAction = 1; // ������
							LcdRefresh = 1;
						} else if ((ButtonFirst == 5) && (ButtonSecond == 9)) {
							SettingsModeAction = 2; // �������
							//TimesetModeShowOnce = 1;
							LcdRefresh = 1;
						} else if ((ButtonFirst == 5) && (ButtonSecond == 10)) {
							SettingsModeAction = 3; // �����
							//TimesetModeShowOnce = 1;
							LcdRefresh = 1;
						}	
						break;	
				
					default:
						break;	
				}
			}				
		}	
		

	}
	
	return 0;
}

/************************************************************************/
/*                       ����������� ��������                           */
/************************************************************************/
ISR (TCC1_OVF_vect) { // ������� ������ � �������� 1 ��
	if (TcAdpTransRun) {
		if ((FrameCheck != ApdFrameCur) || (ShiftCheck != ApdFrameDisplayShift)) {
			FrameCheck = ApdFrameCur;
			ShiftCheck = ApdFrameDisplayShift;
			
			// TcAdpTransRun = 0;
			TcAdpTransEn = 1; // ������ ������� ������ � ��
			
			FocusNew = 1;
		}
	}			
	
	if (TcRadCfgEn) {		
		TcRadCfg++;
	}
	
	if (TcRtcEn) {		
		TcRtc++;
	}

	if (TcAdpTransEn) {		
		TcAdpTrans++;
	}
	
	if (TcAdpTimeoutEn) {		
		TcAdpTimeout++;
	}
	
	if (TcAdpEventsEn) {		
		TcAdpEvents++;
	}
	
	if (TcAdpTransFaultEn) {		
		TcAdpTransFault++;
	}		
}

/************************************************************************/
/*                             ����������                               */
/************************************************************************/
ISR (USARTE0_TXC_vect) {	
	if (TcRadCfgEn) {		
		if ((RadCfgCmdCharCnt <= (RadCfg[RadCfgCmdCnt][0] - 1)) && (RadCfgCmdCharCnt != 0)) {			
			usart_put (&USARTE0, RadCfg[RadCfgCmdCnt][RadCfgCmdCharCnt + 1]);
			RadCfgCmdCharCnt++;			
			if (RadCfgCmdCharCnt > RadCfg[RadCfgCmdCnt][0] - 1) {				
				RadCfgCmdCharCnt = 0x00;
				RadCfgCmdCnt++;
				usart_set_tx_interrupt_level (&USARTE0, USART_TXCINTLVL_OFF_gc);		
			}
		}
	}	
}

ISR (USARTE0_RXC_vect) {	
	RadRecMesCharBuf = usart_get (&USARTE0);
	// ���� � ������ ������������
	if (TcRadCfgEn) {		
		if ((RadCfgAckMesCharCnt <= (RadCfgAckMesLen - 1)) && (RadCfgAckMesReady == 0)) {		
			RadCfgAckMes[RadCfgAckMesCharCnt] = RadRecMesCharBuf;
			RadCfgAckMesCharCnt++;		
			if (RadCfgAckMesCharCnt > (RadCfgAckMesLen - 1)) {			
				RadCfgAckMesCharCnt = 0;
				RadCfgAckMesReady = 1;
			}
		}			
	}
	
	// ���� � ������� ������
	if (RadRecEn) {
		// ���� ������ ������ ������� - '$'
		if (RadRecMesCharBuf == '$') {			
			RadRecMesCharCnt = 0;
			RadRecMesInProgress = 1;
			RadRecMesReady = 0;
		}	
		if ((RadRecMesInProgress) && (RadRecMesCharCnt <= RadRecMesBufLen - 1)) {			
			if (RadRecMesCharCnt == RadRecMesBufLen - 1) {				
//				LcdRefresh = 1;
				RadRecMesInProgress = 0;
				RadRecMesReady = 1;
			}			
			RadRecMesBuf[RadRecMesCharCnt] = RadRecMesCharBuf;
			RadRecMesCharCnt++;
		}
	}	
}

/************************************************************************/
/*                             RS-232 ADP                               */
/************************************************************************/
ISR (USARTC0_TXC_vect) {
	// ������ ��������� �������� �������
	AdpTransContinue ();
}

ISR (USARTC0_RXC_vect) {
	AdpRecMesCharBuf = usart_get (&USARTC0);
	TIMER_RS232_START;
	// ���� ��������� ���� ���������� ���������� ���������
	// ����� ���������, ��:
	if (AdpRecMesTimeout) {
		AdpRecMesTimeout = 0;
		// �������� ���� ����� �������. ������ ����������
		AdpRecMesByteCnt = 0;
//		AdpRecMesCharBuf = usart_get (&USARTC0);
		AdpRecMesBuf[AdpRecMesByteCnt] = AdpRecMesCharBuf;

	} else {
		// ���������� ���� ������� �������
		AdpRecMesByteCnt++;
//		AdpRecMesCharBuf = usart_get (&USARTC0);				
		AdpRecMesBuf[AdpRecMesByteCnt] = AdpRecMesCharBuf;
	}
}

// ������ ��������� �������� ������� ��������� ADP
ISR (TCD1_OVF_vect) {
	TIMER_RS232_STOP;
	// �������� ������� ����� ��������� ���������:
	// ���������� ���� �������� ������� �
	// ��������� � ����� �����
	AdpRecMesTimeout = 1;
	if (AdpRecMesByteCnt > 4) {
		AdpRecMesLen = AdpRecMesByteCnt + 1;
		AdpRecMesByteCnt = 0;
		AdpRecMesReady = 1;
	}
}