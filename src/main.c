/************************************************************************/
/*                       ����� ��������� ���������                   */
/************************************************************************/
#include <main.h>

#undef F_CPU
#define F_CPU 18432000

//const uint16_t Redef_Reg[] = {
	//0x01f4,0x01fd,0x0211,0x0225,0x020d,0x0201,0x0222,0x0220,0x0205,0x0202,
	//0x01fe,0x01fb,0x0218,0x01f7,0x020c,0x0212,0x021a,0x01f9,0x020e,0x0219,
	//0x0224,0x0215,0x021f,0x01f8,0x0209,0x01ff,0x021c,0x01f5,0x0216,0x0221,
	//0x01fa,0x0208,0x0206,0x021b,0x0210,0x0200,0x0204,0x021e,0x020b,0x0207,
	//0x0223,0x0203,0x01f6,0x0214,0x0213,0x020a,0x021d,0x0217,0x020f,0x01fc,
//};

const uint16_t Redef_Reg[] = {	0x0258, 0x0259, 0x025A, 0x025B, 0x025C, 0x025D, 0x025E, 0x025F,
								0x0260, 0x0261, 0x0262, 0x0263, 0x0264, 0x0265, 0x0266, 0x0267,
								0x0268, 0x0269, 0x026A, 0x026B, 0x026C, 0x026D, 0x026E, 0x026F,
								0x0270, 0x0271, 0x0272, 0x0273, 0x0274, 0x0275, 0x0276, 0x0277,
								0x0278, 0x0279, 0x027A, 0x027B, 0x027C, 0x027D, 0x027E, 0x027F,
								0x0280, 0x0281, 0x0282, 0x0283, 0x0284, 0x0285, 0x0286, 0x0287,
								0x0288, 0x0289
};
const uint16_t Redef_Reg1 = 0x0258;

uint16_t comp (const uint16_t *i, const uint16_t *j)
{
return *i - *j;
}

ADP_Rec_MBUS(uint8_t* Array_ADP, uint16_t* MBUS)
{
	uint16_t Adr, Q;
	Adr = (Array_ADP[9]<<8) | Array_ADP[8];
	Q = (Array_ADP[7]<<8) | Array_ADP[6];
	//MBUS[0] = Adr;					
	for (i=0;i<Q/2;i++)//���� �� ���������� �� ADP ���������
	{
		for (j=0;j<sizeof(Redef_Reg);j++)
		{
			/*if (Redef_Reg[j]==Adr+i)
			{
				MBUS[j] = (Array_ADP[11 + 2*i]<<8) | (Array_ADP[10 + 2*i]);
			}*/
			if (Redef_Reg[j]==Adr+i)
			{
				*((uint16_t*)MBUS+j) = (Array_ADP[11 + 2*i]<<8) | (Array_ADP[10 + 2*i]);
			}
		}
	}
}
/************************************************************************/
/*                 ��������� �������                                    */
/************************************************************************/
// ���������� �������
void		TIMER_1msInit (void);
void		TIMER_Rs232Init (void);
void		TIMER_Rs232Start (void);
void		TIMER_Rs232Stop (void);
// ���
void		DAC_Init (void);
void		DAC_New (void);
// ������ � ����������� RTC
uint8_t		TIME_Set (void);
uint8_t		TIME_Get (void);
// ��������� �������
uint8_t		TIME_SetManual (uint8_t Hours, uint8_t Minutes, uint8_t Seconds, uint8_t Day, uint8_t Date, uint8_t Month, uint8_t Year);
// ����������� ����� � ���� �� ������
void		CLOCK_ShowOn (uint8_t str, uint8_t col);
void		DATE_ShowOn (uint8_t str, uint8_t col);
// ������ � ������ RS-232 �� ��������� ADP
void		ADP_TransStart (void);
void		ADP_TransContinue (void);
uint16_t	ADP_TransMesBuild (uint8_t RecAdr, uint8_t TransAdr, uint8_t CmdCode, uint8_t ClarkAdr, uint16_t AmountOfBytes, uint16_t *CmdBody);
uint16_t	ADP_TransEventRequest (uint8_t RecAdr, uint8_t TransAdr, uint8_t CmdCode, uint8_t ClarkAdr);

uint16_t	ADP_TransSendToPlcMesBuild (uint8_t RecAdr, uint8_t TransAdr, uint8_t CmdCode, uint8_t ClarkAdr, uint16_t Adress, uint16_t Value);
uint8_t		ADP_Rec (void);

void		ADP_Function_TransParams (void);
void		ADP_Function_RecParams (void);
void		ADP_Function_ReqEvents (void);

// ���������� ������ ��� �������
void		FOCUS_Adr (void); // ������� ���������� ������� ��� ������� � ������� ������
void		FOCUS_Sort (uint16_t * Array, uint8_t Size); // ��������� ������� �� �����������
void		FOCUS_Block (void); // ���������� � �������: ����������� � ��������� ������
void		FOCUS_Space (void); // ���������� ���������� ����� �������������� ��������
void		FOCUS_CntReset (void);
/************************************************************************/
/* -> ����������� �������								*/
/************************************************************************/
// ������ ������ ����������� � ��
volatile uint16_t TIMER_DataRequest = 0;
volatile uint16_t TIMER_DataRequestPer = 10000;
volatile uint8_t TIMER_DataRequestEn = 0;

// ������ ���������� �����
volatile uint16_t TIMER_ClockRefresh = 0;
volatile uint16_t TIMER_ClockRefreshPer = 300;
volatile uint8_t TIMER_ClockRefreshEn = 0;

// ������ ������� ���������������� ��������� �� ���������� ��� ����������
volatile uint16_t TIMER_RMDCfg = 0;
volatile uint16_t TIMER_RMDCfgPer = 100;
volatile uint8_t TIMER_RMDCfgEn = 1;

// ������ ������� ���������������� ��������� �� ���������� ��� ����� �� MODBUS
volatile uint16_t TIMER_RMDCfgMBUS = 0;
volatile uint16_t TIMER_RMDCfgPerMBUS = 100;
volatile uint8_t TIMER_RMDCfgEn_MBUS = 0;

// ������ ��������� �������� ����� ��������� �� ��������� ADP 
volatile uint16_t TIMER_RequestTimeout = 0;
volatile uint16_t TIMER_RequestTimeoutPer = 5;
volatile uint8_t TIMER_RequestTimeoutEn = 0;

// ������ �������� ������� ������� �� �� 
volatile uint16_t TIMER_RegSave = 0;
volatile uint16_t TIMER_RegSavePer = 1000;
volatile uint8_t TIMER_RegSaveEn = 0;

// ������ �������� ������� ������� �� �� 
volatile uint_fast16_t TIMER_EditValBlink = 0;
volatile uint_fast16_t TIMER_EditValBlinkPer = 500;
volatile uint8_t TIMER_EditValBlinkEn = 0;

volatile uint16_t TIMER_FIRSTSCfg=0;
volatile uint16_t TIMER_FIRSTSCCfgPer=450;
volatile uint8_t TIMER_FIRSTCfgEn=0;

uint8_t REGSAVE_MesByteCnt = 0;
uint8_t REGSAVE_MesByteCntLim = REG_BLOCK_SIZE;
uint16_t REGSAVE_MesCnt = 0;
uint16_t REGSAVE_MesLim = 0;
uint8_t REGSAVE_Buff[8] = {0, 0, 0, 0, 0, 0, 0, 0};
	
uint8_t SHOWONCE_RegSaveProgress = 0;

uint8_t StrSpaces[20] = {
	0x20, 0x20, 0x20, 0x20, 0x20,
	0x20, 0x20, 0x20, 0x20, 0x20,
	0x20, 0x20, 0x20, 0x20, 0x20,
	0x20, 0x20, 0x20, 0x20, 0x20,
};

uint8_t StrBlackSq[20] = {
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
};

/************************************************************************/
/* -> ���������� � ��������� ������� APD      */
/************************************************************************/

// ���� ����� � �������
typedef enum APD_field_type {
	APD_FIELD_TYPE_SPLASH = 0, // ��������
	APD_FIELD_TYPE_VALUE = 1, // �����
	APD_FIELD_TYPE_TEXT = 2, // �����
	APD_FIELD_TYPE_DATE = 3, // ����
	APD_FIELD_TYPE_TIME_ = 4,  // �����
	APD_FIELD_TYPE_LIST = 5,  // ������
	APD_FIELD_TYPE_32BIT = 10, // �����
 } APD_field_type_t;

// ������� ������ ����� 
typedef enum APD_field_format {
	APD_FIELD_FORMAT_BIN = 0x00, // ��������
	APD_FIELD_FORMAT_HEX = 0x10, // �����������������
	APD_FIELD_FORMAT_DEC_WITHOUT_NULLS = 0x20, // ���������� �������� ��� ����� �����
	APD_FIELD_FORMAT_DEC_WITH_NULLS = 0x30, // ���������� �������� � ������ �����
 } APD_field_format_t;
 
// ������ ��������
uint8_t Id8 = 0; // ������� ����
uint8_t IdFrame = 0; // ������� ������
uint8_t IdField = 0; // ������� �����
uint8_t IdAction = 0; // ������� ��������������
uint16_t IdEvent = 0; // ������� �������

// ����� ������������� �������
uint16_t APD_EventsBuf[ADP_EVENT_REQUEST];

signed int CLOCK_ShowStr = 0; // ����� ������ ��� ������ �����
uint8_t CLOCK_ShowCol = 12; // ����� ������� ��� ������ �����

signed int DATE_ShowStr = 0; // ����� ������ ��� ������ ����
uint8_t DATE_ShowCol = 0; // ����� ������� ��� ������ �����

uint16_t APD_FrameRefreshRate = 0; // ������ ������ ������ (0 - ����, 1 - 100��, 2 - 200, 3...10)
uint16_t APD_EventReqRatio = 0; // ������ ������ ������� (0 - ����, 1 - 100��, 2 - 200, 3...10)
uint16_t APD_BaudRateIndex = 3; // �������� ����� � ��� (0 - 9600, 1 - 19200, 2 - 38400, 3 - 57600)
uint8_t APD_FrameAmount = 0; // ���������� ������ � �������
uint8_t APD_FrameStartNum = 0; // ����� ���������� �����

volatile uint16_t APD_FrameCur = 0; // ������� ���� �������
signed int APD_FrameDisplayShift = 0; // ����� ����� �� �������

uint16_t APD_FramesStartAdr = 0; // ����� �������� ������
uint16_t APD_EventsStartAdr = 0; // ����� �������� ������ �������
uint16_t APD_EventAdr = 0; // ����� ������ ����� ������ �������
uint16_t APD_EventIdAmount = 0; // ���������� �������� �������

uint8_t APD_EventAdrArray[4] = {0, 0, 0, 0}; // ������ ��������� ��� ������� ������ �������

uint8_t APD_EventAdrNum = 0; // ���������� ��������� ��� ������� ��
uint16_t AdrTemp = 0; // ��������� ����� ������
uint8_t EventEn = 0; // ����: ���������� �� ������ ������ ��
uint8_t EventTransAct = 0; // ����: ����� �������� ������ ��
uint8_t EventTransEn = 0; // ����: ����� ���������� ������ ��

/************************************************************************/
/* -> ��������� ������� �������                      */
/************************************************************************/

int8_t EventStorage[4][EVENT_CODES_MAX]; // ������ ��������� ����������� ������� �� ��

void EventStorageToTOP (int8_t TOP);

void EventStorageToTOP (int8_t TOP) {
	for (uint8_t a = 0; a <= 3; ++ a) {
		for (uint8_t b = 0; b <= (EVENT_CODES_MAX - 1); ++ b) {
			EventStorage[a][b] = TOP;
		}
	}
}

uint8_t EEMEM EventStorage_TOP_Save = DEFAULT_STORAGE_TOP;
uint8_t EEMEM EventStorage_BOT_Save = DEFAULT_STORAGE_BOT;

int8_t EventStorage_TOP = 5 /*3*/; // ������� ������ �������� �������
int8_t EventStorage_BOT = - 6 /*- 4*/; // ������ ������ �������� �������

uint8_t REG_SET_ShowOnce = 1;
uint8_t REG_SET_TOP = 0;
uint8_t REG_SET_BOT = 0;

uint8_t REG_SET_SetMarker = 0;
/************************************************************************/
/* -> ������ ������� APD                                 */
/************************************************************************/
uint16_t APD_FrameStartAdr[APD_MAX_FRAMES]; // ������ ������� �������� ������
uint16_t APD_FrameFieldsAndActionsStartAdr[APD_MAX_FRAMES][2]; // ������ �������� ����� � �������� � �����

uint8_t APD_FrameFieldsAmount[APD_MAX_FRAMES]; // ������ ���������� ����� � ������ �����
uint8_t APD_FrameActionsAmount[APD_MAX_FRAMES]; // ������ ���������� ����� � ������ �����

uint16_t APD_FrameFieldStartAdr[APD_MAX_FRAMES][APD_MAX_FIELDS]; // ������ ������� �������� ����� ������� �����
uint16_t APD_FrameActionStartAdr[APD_MAX_FRAMES][APD_MAX_ACTIONS];// ������ ������� �������� �������������� ������� �����

uint16_t APD_FrameDataFromPlcAdr[APD_MAX_FRAMES][APD_MAX_FIELDS]; // ������ �������, ������������� �� ��� � ������ �����
uint8_t APD_FrameDataFromPlcStr[APD_MAX_FRAMES][APD_MAX_FIELDS]; // ������ ����� ���������, ������������� �� ��� � ������ �����
uint8_t APD_FrameDataFromPlcAmount[APD_MAX_FRAMES]; // ������ ���������� ������������� �� ��� ������� � �����

/* ���������� � ������� � �������:
 * APD_List[N][0] - ����� ������ ���������� ������ ������ � APD_FrameDataFromPlcAdr[N][M]
 * APD_List[N][1] - ���������� �������� (��������� 16-���)
 * APD_List[N][2] - ������� ����� ������
*/
uint8_t APD_List[APD_MAX_FRAMES][3];
// ��������� ������ ������� � �����
uint16_t APD_ListAdress[APD_MAX_FRAMES];

uint16_t FOCUS_DataAdr[APD_MAX_FIELDS]; // ������ ������� ���������� � ������
int16_t FOCUS_Data[APD_MAX_FIELDS]; // ������ ��������� � ������
uint8_t FOCUS_DataCntNext[APD_MAX_FIELDS]; // ������ ��������� �������� (������� ���������)
uint8_t  FOCUS_DataCntPrev[APD_MAX_FIELDS]; // ������ ��������� �������� (���������� ���������)
uint8_t FOCUS_DataStatus[APD_MAX_FIELDS]; // ������ �������� ���������
uint8_t FOCUS_DataAmount = 0; // ���������� ������� � ������

uint8_t FOCUS_DataCntNewNext[APD_MAX_FIELDS]; // ������ ��������� ��������� ��������
uint8_t FOCUS_DataCntNewPrev[APD_MAX_FIELDS]; // ������ ��������� ��������� ��������
uint8_t FOCUS_DataCntNewStatus[APD_MAX_FIELDS]; // ���� ��������� ��������
uint16_t FOCUS_SpaceArray[APD_MAX_FIELDS]; // ������ ���������� ����� �������� � ������
uint8_t FOCUS_DataMesNew = 0; // ���� ��������� ������-���� �������� � �����

 /* ������ ������� � ������ ��� �������:
  * BLOCK_Request[N][0] - ��������� ����� �����
  * BLOCK_Request[N][1] - ���������� ���� � ����� ��� �������
  */
uint16_t BLOCK_Request[APD_MAX_FIELDS][2];
uint8_t BLOCK_Amount = 0; // ���������� ������� � ������ ��� ������� 
uint16_t APD_FrameButtonAction = 255; // ��� �������� �� ������� ������

uint8_t FOCUS_New = 1; // ���� ��������� ������ � ����� (����� �����������
// ������������� ����� ������)

// ���������� �� ��������� ���� ���� �������� � ����� APD
uint8_t SplashRefreshEn = 1;
// ������� �������� ������ ������� APD � ���, ��������� ��� ����������� �� LCD �������
// ������ ������� = ��� ������� � APD
// �������� � ������ = ��, ��� ��������� �� ����� � �������
/*const */ uint8_t APD_PrjFontTable[] = {
//	00    01    02    03    04    05    06 �  07    08    09    0A    0B    0C    0D    0E    0F
	0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0xA9, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
//	10    11    12    13    14    15    16    17    18    19    1A    1B    1C    1D    1E    1F
	0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20,
//	20    21 !  22 .  23 #  24 $  25 %  26 &  27 '  28 (  29 )  2A *  2B +  2C ,  2D -  2E .  2F /
	0x20, 0x21, 0x2E, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F,
//	30 0  31 1  32 2  33 3  34 4  35 5  36 6  37 7  38 8  39 9  3A :  3B ;  3C <  3D =  3E >  3F ?
	0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,
//	40 @  41 A  42 B  43 C  44 D  45 E  46 F  47 G  48 H  49 I  4A J  4B K  4C L  4D M  4E N  4F O
	0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49, 0x4A, 0x4B, 0x4C, 0x4D, 0x4E, 0x4F,
//	50 P  51 Q  52 R  53 S  54 T  55 U  56 V  57 W  58 X  59 Y  5A Z  5B [  5C \  5D ]  5E ^  5F _
	0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59, 0x5A, 0x5B, 0x01, 0x5D, 0x5E, 0x5F,
//	60 `  61 a  62 b  63 c  64 d  65 e  66 f  67 g  68 h  69 i  6A j  6B k  6C l  6D m  6E n  6F o
	0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69, 0x6A, 0x6B, 0x6C, 0x6D, 0x6E, 0x6F,
//	70 p  71 q  72 r  73 s  74 t  75 u  76 v  77 w  78 x  79 y  7A z  7B {  7C |  7D }  7E ~  7F
	0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79, 0x7A, 0x28, 0x00, 0x29, 0xE9, 0x20,
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
//	F0    F1    F2    F3    F4    F5    F6    F7 �  F8    F9    FA    FB    FC    FD    FE    FF SQ
	0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x45, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0xFF
}; // ������ ����� � ����� � (0x45)

uint8_t RusFont[32] = {
0x41, // 0x00 - �
0xA0, // 0x01 - �
0x42, // 0x02 - �
0xA1, // 0x03 - �
0xE0, // 0x04 - �
0x45, // 0x05 - �
0xA3, // 0x06 - �
0xA4, // 0x07 - �
0xA5, // 0x08 - �
0xA6, // 0x09 - �
0x4B, // 0x0A - �
0xA7, // 0x0B - �
0x4D, // 0x0C - �
0x48, // 0x0D - �
0x4F, // 0x0E - �
0xA8, // 0x0F - �
0x50, // 0x10 - �
0x43, // 0x11 - �
0x54, // 0x12 - �
0xA9, // 0x13 - �
0xAA, // 0x14 - �
0x58, // 0x15 - �
0xE1, // 0x16 - �
0xAB, // 0x17 - �
0xAC, // 0x18 - �
0xE2, // 0x19 - �
0xAD, // 0x1A - �
0xAE, // 0x1B - �
0x62, // 0x1C - �
0xAF, // 0x1D - �
0xB0, // 0x1E - �
0xB1, // 0x1F - �
};

/************************************************************************/
/* -> ������ � ������������ � �������� ������������ �����               */
/************************************************************************/
// ������ ���������������� ������ �����������
uint8_t RMD_Cfg [15][10] = {
// [00] - ��������� ������� ����������		
	{7, '0', '0', '#', 'R', 'S', '0', 3, 0, 0},
// [01] - ��������� ���������� ����� ������������ ����������������� ����������
	{7, '0', '0', '#', '8', 'N', '1', 3, 0, 0},
// [02] - ��������� ������� ������� �����������	
	{7, '0', '0', '#', 'F', '0', '0', 3, 0, 0},
// [03] - ��������� �������� �������� �����������
	{6, '0', '0', '#', 'P', 'F', 3, 0, 0, 0},
// [04] - ��������� ������ ����	
	{6, '0', '0', '#', 'N', '0', 3, 0, 0, 0},
// [05] - ��������� ������ �����
	{9, '0', '0', '#', 'D', 'A', 'A', 'A', 'A', 3},
//	{9, '0', '0', '#', 'D', 'B', 'B', 'B', 'B', 3},
// [06] - ��������� ������ ��������	
	{9, '0', '0', '#', 'A', 'B', 'B', 'B', 'B', 3},
//	{9, '0', '0', '#', 'A', 'A', 'A', 'A', 'A', 3},

// [07] - ��������� ����� ���������� ���������
	{9, '0', '0', '#', 'C', '0', '0', '0', '0', 3},
// [08] - ��������� �������� ������ �����������
	{6, '0', '0', '#', 'M', '0', 3, 0, 0, 0},

//	{5, '0', '0', '#', '?', 3, 0, 0, 0, 0},
// [09] - �������� �������� ������ �� ���������������� ����������
	{7, '0', '0', '#', 'I', '9', '6', 3, 0, 0},
// [10] - ��������� �������� �������� ���������� �� �����������	
	{7, '0', '0', '#', 'E', '9', '6', 3, 0, 0},	
// [11] - 57600	
//	{7, '0', '0', '#', 'I', '5', '7', 3, 0, 0},
// [12] - 57600	
//	{7, '0', '0', '#', 'E', '5', '7', 3, 0, 0},

// [13] - 115200
//	{7, '0', '0', '#', 'I', '1', '1', 3, 0, 0},
// [14] - 115200
//	{7, '0', '0', '#', 'E', '1', '1', 3, 0, 0},
};
uint8_t RMD_CfgMBUS [15][10] = {
// [00] - ��������� ������� ����������
	{7, '0', '0', '#', 'R', 'S', '0', 3, 0, 0},
// [01] - ��������� ���������� ����� ������������ ����������������� ����������
	{7, '0', '0', '#', '8', 'N', '1', 3, 0, 0},
// [02] - ��������� ������� ������� �����������
	{7, '0', '0', '#', 'F', '0', '3', 3, 0, 0},
// [03] - ��������� �������� �������� �����������
	{6, '0', '0', '#', 'P', 'F', 3, 0, 0, 0},
// [04] - ��������� ������ ����
	{6, '0', '0', '#', 'N', '0', 3, 0, 0, 0},
// [05] - ��������� ������ �����
	{9, '0', '0', '#', 'D', 'B', 'B', 'B', 'B', 3},
//	{9, '0', '0', '#', 'D', 'B', 'B', 'B', 'B', 3},
// [06] - ��������� ������ ��������
	{9, '0', '0', '#', 'A', 'A', 'A', 'A', 'A', 3},
//	{9, '0', '0', '#', 'A', 'A', 'A', 'A', 'A', 3},

// [07] - ��������� ����� ���������� ���������
	{9, '0', '0', '#', 'C', '0', '0', '0', '0', 3},
// [08] - ��������� �������� ������ �����������
	{6, '0', '0', '#', 'M', '0', 3, 0, 0, 0},
		
//	{5, '0', '0', '#', '?', 3, 0, 0, 0, 0},
// [09] - �������� �������� ������ �� ���������������� ����������
	{7, '0', '0', '#', 'I', '9', '6', 3, 0, 0},
// [10] - ��������� �������� �������� ���������� �� �����������	
	{7, '0', '0', '#', 'E', '9', '6', 3, 0, 0},
// [11] - 57600
//	{7, '0', '0', '#', 'I', '5', '7', 3, 0, 0},
// [12] - 57600
//	{7, '0', '0', '#', 'E', '5', '7', 3, 0, 0},

// [13] - 115200
//	{7, '0', '0', '#', 'I', '1', '1', 3, 0, 0},
// [14] - 115200
//	{7, '0', '0', '#', 'E', '1', '1', 3, 0, 0},
};
// ����� ��������� ��������� � �����������
volatile uint8_t RMD_RecMesBuf[RMD_MESSAGE_LENGTH] = {0x00, 0x00, 0x00, 0x00};
// �������������� ������� �������������� �� ��������� ������� '$'.
// ������������ ���� RMD_RecMesInProgress, ����������, ��� ��������������
// ���� ������ ���������.
// �����, ������������ ������� �������� �������� �� �������� ����� ���������.
// ����� ����� ������������ ���� ��������� ��������� RMD_RecMesReady.
// ���� RMD_RecMesInProgress ������������, ������������ � ���, ���
// ����� ��������� ����� ���������.

volatile uint8_t RMD_RecMesCharCnt = 0; // ������� �������� �������� � ��������� �� �����������
volatile uint8_t RMD_RecMesBufLen = sizeof (RMD_RecMesBuf) / sizeof (uint8_t); // ����� ��������� ���������
volatile uint8_t RMD_RecMesCharBuf = 0; // ����� ��������� �������
volatile uint8_t RMD_TransMesCharBuf = 0; // ����� ������������� ������� (�� ������������)
volatile uint8_t RMD_RecMesInProgress = 0; // ���� �������� ����� ������ ���������
volatile uint8_t RMD_RecEn = 0; // ���� ���������� ����� ��������� � �����������
volatile uint8_t RMD_RecMesReady = 0; // ���� ���������� ��������� ���������
volatile uint8_t RMD_RecMesCrcOk = 0; // ���� �������� �������� ����������� ����� ��������� ��������� 

// ����������
volatile uint8_t RMD_CfgAckMes[7] = {0x0D, 0x0A, 0x4F, 0x4B, 0x0A, 0x0D, 0x3E}; // ��������� ������������� �������� ������ ������� ����������������	
volatile uint8_t RMD_CfgAckMesCharCnt = 0; // ������� �������� ��������� ���������
volatile uint8_t RMD_CfgAckMesLen = 7; // ����� ��������� ���������
volatile uint8_t RMD_CfgAckMesReady = 1; // ���� ���������� ��������� ���������

volatile uint8_t RMD_CfgCmdAmount = 11; // ���������� ���������������� ������
volatile uint8_t RMD_CfgCmdCnt = 0; // ������� ���������������� ������
volatile uint8_t RMD_CfgCmdCharCnt = 0; // ������� �������� �������
volatile uint8_t RMD_CfgCmdAck = 0; // ���� ��������� ������������ �������
//���������� modbus
volatile uint8_t RMD_CfgAckMesMBUS[7] = {0x0D, 0x0A, 0x4F, 0x4B, 0x0A, 0x0D, 0x3E}; // ��������� ������������� �������� ������ ������� ����������������	
volatile uint8_t RMD_CfgAckMesCharCntMBUS = 0; // ������� �������� ��������� ���������
volatile uint8_t RMD_CfgAckMesLenMBUS = 7; // ����� ��������� ���������
volatile uint8_t RMD_CfgAckMesReadyMBUS = 1; // ���� ���������� ��������� ���������

volatile uint8_t RMD_CfgCmdAmountMBUS = 11; // ���������� ���������������� ������
volatile uint8_t RMD_CfgCmdCntMBUS = 0; // ������� ���������������� ������
volatile uint8_t RMD_CfgCmdCharCntMBUS = 0; // ������� �������� �������
volatile uint8_t RMD_CfgCmdAckMBUS = 0; // ���� ��������� ������������ �������

// ������ ��������� �����������
typedef enum rmd_config_state {
	START = 0,
	CHECK = 1,
	SEND =  2,
	END =   3,
} rmd_config_state_t;

uint8_t RMD_Config_STATE = START;
uint8_t RMD_Config_STATEMBUS = START;
uint8_t RMD_Config_FIRST = 1;
uint8_t RMD_Config_ERROR = 0;
uint8_t RMD_Config_FIRSTMBUS = 1;
uint8_t RMD_Config_ERRORMBUS = 0;

uint8_t NewBaudRate = 96;
uint8_t NewBaudRateMBUS = 38;
/************************************************************************/
/* -> �������� ����� ������ � ���������������� ������� (ADP) */
/************************************************************************/
// ������ ��������� �� �����
typedef enum ADP_adress {
	ADP_NULL = 0x00, // ������� �����
	ADP_TR0 = 0x50, // �� 0
	ADP_TR1 = 0x51, // �� 1
	ADP_TR2 = 0x52, // �� 2
	ADP_TR3 = 0x53, // �� 3
	ADP_TR4 = 0x54, // �� 4
	ADP_OPK_MBUS = 0x79,	//������� ����������� ��� ������
	ADP_OPK_RES = 0x7B,	//������� ����������� ���������
	ADP_OPK = 0x7D, // �������
	ADP_ATOOLS = 0x7F, // ATools
} ADP_adress_t;

// ���� ������� ��������� ADP
typedef enum ADP_function {
	ADP_FUNC_TRANS_PAR = 0x13, // �������� ���������
	ADP_FUNC_REC_PAR = 0x0B, // ������� ���������
	ADP_FUNC_EVENT_REQ = 0x05, // ������ ������ �������
} ADP_function_t;

// ����� ���������� �����\�������� ���� UART �� ��������� ADP
volatile uint8_t ADP_TransEn = 0x01; // ���������� ��������
volatile uint8_t ADP_RecEn = 0x01; // ���������� �����

uint8_t DATA_ReqEn = 0; // ���������� ������� ������
uint8_t EVENT_ReqEn = 0; // ���������� ������� ������ �������
/************************************************************************/
/* -> �������� RS-232 - ADP												*/
/************************************************************************/
/************************************************************************/
/*								��������                                */
/************************************************************************/
volatile uint8_t ADP_TransMesBuf[32]; // ����� ������������� ���������
volatile uint16_t ADP_TransMesBody[16]; // ����� ���� �������
volatile uint8_t ADP_TransMesCharBuf=0; // ����� ������������� �������
//volatile uint16_t ADP_TransMesByteCnt; // ������� ���������� ���� 
volatile uint16_t ADP_TransMesLen=0; // ����� ������������ �������
volatile uint8_t ADP_TransMesQueue=0; // ������� ������������ ��������� � ������� �� ���������� �����
volatile uint8_t ADP_TransMesQueueCnt=0; // ������� ������� ���������
volatile uint16_t ADP_MBUS_TransMesBody[16]; // ����� ���� ������� �������������� ����������
/************************************************************************/
/*								��Ȩ�                                   */
/************************************************************************/
volatile uint8_t ADP_RecNewChar=0; // ���� ������ ��������� �������
volatile uint8_t ADP_RecMesBuf[128]; // ����� ������������ ���������
volatile uint8_t ADP_RecMesReady=0; // ����: ���� ������� ��������
volatile uint8_t ADP_RecMesCrcOk=0; // ����: ������� ������ �������� ����������� �����
volatile uint8_t ADP_RecMesCharBuf=0; // ����� ������������ ������� �������
volatile uint16_t ADP_RecMesByteCnt = 0; // ������� �������� ����
volatile uint16_t ADP_RecMesLen=0; // ����� ����������� �������
volatile uint8_t ADP_RecMesTimeout=1; // ���� �������� ������ ��������� �� ���������������

extern MODBUS_STATE_t MB0_State;
/************************************************************************/
/* -> ���������� ����������� ����������� ������ */
/************************************************************************/
// ���� ������ �� ���������� ������
volatile uint8_t LCD_Refresh = 1;
// "��������" ��������
uint8_t LoadBar = 8;
// ����� ����������� ���������� ������
uint8_t RMD_Error = 0; // ������ �����������
uint8_t PRJ_Error = 0; // ������ ������� APD_
uint8_t CLOCK_Error = 0; // ������ ����� ��������� �������
uint8_t EEPROM_Error = 0; // ������ ���������� EEPROM
// ������ ������ ����
typedef enum net_mode_enum {
	NET_MODE_CONFIG = 0, // ������������ �����������
	NET_MODE_MASTER = 1, // ������� �����: ������� ������, ��������������� �����
	NET_MODE_TRANSP = 2, // ���������� �����: ������� - ���� ����� APD � ����������������
	NET_MODE_REGSAVE = 3, // ����� �������� ������� ������� �� ��
} net_mode_t;

uint8_t NET_Mode = NET_MODE_CONFIG; // ������� ����� ������ ����
// ������ ���������� ������ ������
typedef enum error_enum {
	ERROR_RMD = 0,
	ERROR_APD = 1,
	ERROR_RTC = 2,
	ERROR_EEPROM = 3,
} error_t;

uint8_t APD_ErrorScore = 0;
uint8_t EEPROM_ErrorScore = 0;
// ��������-����������� ������ ������ �����������:
uint8_t RMD_ErrorScore = 0; // ������� ������: �������� ������
uint8_t RMD_ErrorScoreLim = 10;
uint8_t RMD_ErrorScoreTry = 0;
uint8_t RMD_ErrorScoreTryLim = 3; // ������� ������: ������� ��������������
uint8_t RMD_ErrorScoreGlobal = 0; // ������� ������: ������� ����������� ������
uint8_t RMD_ErrorScoreGlobalLim = 70;

/************************************************************************/
/* -> ������ � �������� ����������				*/
/************************************************************************/
// ������� ��������� ���������� ������ ��� �������� � ����������:
uint8_t SERVICE_ToggleCnt = 0; // ������� � ��������� �����
uint8_t SERVICE_ToggleCntLim = 40;
uint8_t TRANSP_ToggleCnt = 0; // ������� � ���������� �����
uint8_t TRANSP_ToggleCntLim = 40;
// ������� ������:
uint8_t BUTTON_First = 0; // ������ 1
uint8_t BUTTON_Second = 0; // ������ 2
// ���� ���������:
// ����� ����� ������� ���� ����������, ����������
// ��������� ������ ������� ������
uint8_t BUTTON_Release = 0;

// ��� �������� �� ������� ��� �������� � ������� ����������� ������� ����
uint8_t Action = 255;
/************************************************************************/
/* -> ������ � ������ ��������� �������		*/
/************************************************************************/
uint8_t TIMESET_CurPos = 0; // ������� ������� �� ������ ��������� �������

// ���� ���������� ����������� �����
uint8_t ClockShowEn = 0;
// ���� ���������� ����������� ����
uint8_t DataShowEn = 0;
// ���������� "�������" �������:
// � ��� ������������ "�����" ��������, ���������� �� ��������� RTC
uint8_t TIME_GetSeconds = 0;
uint8_t TIME_GetMinutes = 0;
uint8_t TIME_GetHours = 0;
uint8_t TIME_GetDay = 0;
uint8_t TIME_GetDate = 0;
uint8_t TIME_GetMonth = 0;
uint8_t TIME_GetYear = 0;

// ���������� ��������������� ��������� �������:
// �������� �������� �����, ����� � �.�.
uint8_t TIME_SetSeconds = 0;
uint8_t TIME_SetMinutes = 0;
uint8_t TIME_SetHours = 0;
uint8_t TIME_SetDay = 0;
uint8_t TIME_SetDate = 0;
uint8_t TIME_SetMonth = 0;
uint8_t TIME_SetYear = 0;

// ���������� ������ ��������� �������
// ����, ����������� ������� "������" �������� ������� ��
// ���� ������ ������ ������ ��������� �������.
// ������������ ��� ������ �� ����� ������
uint8_t TIMESET_ShowOnce = 1;
// � ���� ���������� ���������� �������� �������� �����, ����� � �.�.
uint8_t TIMESET_ShowSeconds = 0;
uint8_t TIMESET_ShowMinutes = 0;
uint8_t TIMESET_ShowHours = 0;
uint8_t TIMESET_ShowDay = 0;
uint8_t TIMESET_ShowDate = 0;
uint8_t TIMESET_ShowMonth = 0;
uint8_t TIMESET_ShowYear = 0;

// ���� ������ ����� ��� "�������" ����� � ������ ��������� �������:
// ������������ ��� ������ �� ����� ��������� ������ � �� ������
uint8_t TIMESET_ClockFailed = 0;

// ���������� ������ ��������� ���������
uint8_t CONTRAST_ShowOnce = 1;
uint8_t CONTRAST_Contrast = 0;
uint8_t LCD_Contrast = DEFAULT_CONTRAST;
uint8_t EEMEM CONTRAST_Save = DEFAULT_CONTRAST;
// ������ �������
void		SCREEN_INTRO (void); // �������� ��� �������
void		SCREEN_MENU (void); // ����� ����
void		SCREEN_ACTION (uint8_t Action); // ������� ����� (�������������� ������� APD)
void		SCREEN_NORMAL (void); // ������� ����� (��������� ������ ������� APD)
void		SCREEN_INFO (void); // ����� ����������
uint8_t		SCREEN_TIME (void); // ��������� �������
void		SCREEN_DIAGNOSTIC (void); // ����������� ������
void		SCREEN_SETTINGS (void); // ��������� �����
void		SCREEN_CONTRAST (void); // ��������� ���������
void		SCREEN_EVENTS_LIST (void); // ������ �������
void		SCREEN_EVENTS_CLEAR (void); // ����� ������� �������
void		SCREEN_EVENTS_MENU (void); // ���� �������
void		SCREEN_EVENTS_INFO (void);  // ���������� � �������
void		SCREEN_EVENTS_SETTINGS (void);  // ��������� �������

// ������ �������
void		REG_WriteEvent (uint8_t From, uint8_t EventCode); // ������ ������� � ������
void		REG_Init (void); // ������������� �������
void		REG_Reset (void); // ����� ��������� �������
void		REG_Screen (uint8_t Mode, uint16_t TotMes, uint16_t CurMes, uint16_t NewMes, uint8_t NewFlag, uint8_t *Data); // ��������� ������ �������
// ���������� �����
void		SCREEN_TRANSP_SELECT (void);
void		SCREEN_TRANSP_CONFIRM (void);
void		SCREEN_TRANSP_PROGRESS (void);
void		SCREEN_TRANSP_SCREEN (void);
// �������� ������� �� ��
void		SCREEN_EVENTS_SAVE_MENU (void);
void		SCREEN_EVENTS_SAVE_PROGRESS (void);
void		SCREEN_EVENTS_SAVE_COMPLETED (void);
// ������ ������
void		SCREEN_ERROR_PRJ (void);
void		SCREEN_ERROR_RMD (void);
// ������������ ������� ������
typedef enum mode_enum {
	MODE_INTRO =					0, // ��������� �����
	MODE_NORMAL =					1, // ����� �������� ������
	MODE_MENU =						2, // ����� ����
	MODE_INFO =						3, // ����� ����������
	MODE_TIME =						4, // ����� ��������� ������� � ����
	MODE_DIAGNOSTIC =				5, // ����� ����������� ������
	MODE_SETTINGS =					6, // ����� ��������� �����
	MODE_CONTRAST =					7, // ��������� ���������

	MODE_EVENTS_MENU =				8, // ���� ������� �������
	MODE_EVENTS_LIST =				9, // ����� ����������� ������� �������
	MODE_EVENTS_INFO =				10, // ���� ���������� ������� �������
	MODE_EVENTS_CLEAR =				11, // ���� ������� ������� �������
	MODE_EVENTS_SETTINGS =			12, // ���� �������� ������� �������
	MODE_EVENTS_SAVE_MENU =			13, // ���� ������ �������� ������� �� ��
	MODE_EVENTS_SAVE_PROGRESS =		14, // ���� ������ �������� �������: �������� ���
	MODE_EVENTS_SAVE_COMPLETED =	15, // ���� ������ �������� �������: ����������

	MODE_TRANSP_SELECT =			16, // ���� ��������� ������ ������������
	MODE_TRANSP_CONFIRM =			17, // ���� ����� � ����� ������������
	MODE_TRANSP_PROGRESS =			18, // ���� ����� � ����� ������������
	MODE_TRANSP_SCREEN =			19, // ���� ������ ������������
	
	MODE_ERROR_PRJ =				20, // ����� ������ �������
	MODE_ERROR_RMD =				21, // ����� ������ ������
	
	MODE_EDIT =						22, // ����� �������������� ����
	
	MODE_NORMAL_2 =					23,	//������ ��������� ����� ��� 4 ��

} mode_t;

uint8_t Mode = MODE_INTRO; // ������� ����� ������
typedef void (* SCREEN[]) (void);
const SCREEN SCREEN_ALL = {

	SCREEN_INTRO,
	SCREEN_NORMAL,
	SCREEN_MENU,
	SCREEN_INFO,
	SCREEN_TIME,
	SCREEN_DIAGNOSTIC,
	SCREEN_SETTINGS,
	SCREEN_CONTRAST,

	SCREEN_EVENTS_MENU,
	SCREEN_EVENTS_LIST,
	SCREEN_EVENTS_INFO,
	SCREEN_EVENTS_CLEAR,
	SCREEN_EVENTS_SETTINGS,
	SCREEN_EVENTS_SAVE_MENU,
	SCREEN_EVENTS_SAVE_PROGRESS,
	SCREEN_EVENTS_SAVE_COMPLETED,

	SCREEN_TRANSP_SELECT,
	SCREEN_TRANSP_CONFIRM,
	SCREEN_TRANSP_PROGRESS,
	SCREEN_TRANSP_SCREEN,
	
	SCREEN_ERROR_PRJ,
	SCREEN_ERROR_RMD,
	SCREEN_NORMAL,
};

uint8_t SoftVer = 20;
volatile uint8_t TempChar = 0;

uint8_t EEMEM ErrorPrjText[4][20] = {
{"������! ���� �������"},
{"������ ��������� ���"},
{"    �����������!    "},
{"������������� ������"},
};

uint8_t EEMEM ErrorRmdText[4][20] = {
{"������! ���� ������ "},
{"����������� ����� � "},
{"   ������������!    "},
{"������������� ������"},
};

uint8_t EEMEM InfoText[4][20] = {
{"����������:    0-���"},
{"- ������ ��������� -"},
{"������ ��:          "},
{"��� '���' (c) 2014  "},
};

 /*uint8_t EEMEM IntroText[4][20] = {
 {"LLC ASC. EKB, RUSSIA"},
 {"OPERATIONAL PANEL   "},
 {"SOFT VERSION:       "},
 {"LOADING             "},
 };*/

uint8_t EEMEM IntroText[4][20] = {

{"---- ��� '���' -----"},
{"������������, ������"},
{"������ ���������    "},
{"��������            "},
};

uint8_t MenuShift = 0;
uint8_t MenuMax = MENU_MAX;

// ������� �������������� � ����
uint8_t MenuAction [MENU_MAX - 1] = {
	MODE_TIME,
	MODE_CONTRAST,
	MODE_EVENTS_MENU,
	MODE_SETTINGS,
	MODE_DIAGNOSTIC,
	MODE_INFO,
};

uint8_t EEMEM MenuText[MENU_MAX + 10][20] = {
// {"MENU:               "},
// {"1-TIME              "},
// {"2-CONTRAST          "},
// {"3-EVENTS            "},
// {"4-SETTINGS          "},
// {"5-DIAGNOSTIC        "},
// {"6-INFO              "},
	
{"����:               "},
{"1-�����. �������    "},
{"2-�����. ���������  "},
{"3-������ �������    "},
{"4-�����. �����      "},
{"5-�����������       "},
{"6-����������        "},
	
};

uint8_t EEMEM ContrastText[4][20] = {
// {"CONTRAST:           "},
// {"                    "},
// {"                    "},
// {"1-YES 0-NO 9-DEFAULT"},
	
{"�����. ���������:   "},
{"                    "},
{"                    "},
{"1-���� 0-��� 9-�����"},	
	
};

uint8_t RegMenuShift = 0;
uint8_t RegMenuMax = REG_MENU_MAX;
uint8_t EEMEM RegMenuText[REG_MENU_MAX][20] = {
// {"EVENTS MENU:  0-BACK"},
// {"1-LIST              "},
// {"2-INFO              "},
// {"3-CLEAR             "},
// {"4-SAVE              "},
// {"5-SETTINGS          "},
	
{"����. �������: 0-���"},
{"1-��������          "},
{"2-���������         "},
{"3-�������           "},
{"4-����������        "},
{"5-���������         "},

};

uint8_t EEMEM RegInfoText[4][20] = {
// {"EVENTS INFO:  0-BACK"},
// {"TOT: 0000 NEW: 0000 "},
// {"CLEARED AT:         "},
// {"-> 00:00:00 00.00.00"},

{"���������:     0-���"},
{"���: 0000 ���: 0000 "},
{"��������� �������:  "},
{"-> 00:00:00 00.00.00"},

};

// ���������� ������ ���������
uint8_t SETTINGS_ShowOnce = 1;

uint8_t SETTINGS_Delay = 0;
uint8_t EEMEM SETTINGS_DelaySave = 5;
uint8_t EEMEM SETTINGS_DelaySaveFlag = 0;

uint8_t SETTINGS_Baud = 0;
uint8_t EEMEM SETTINGS_BaudSave = 0;
uint8_t EEMEM SETTINGS_BaudFlag = 0;
uint16_t SETTINGS_BaudValue[4] = {
	9600,
	19200,
	38400,
	57600,
};

uint8_t SETTINGS_Cyckle = 0;
uint8_t EEMEM SETTINGS_CyckleSave = 5;
uint8_t EEMEM SETTINGS_CyckleSaveFlag = 0;
uint16_t SETTINGS_CyckleValue[10] = {
	100,
	200,
	300,
	400,
	500,
	600,
	700,
	800,
	900,
	1000
};

uint8_t SETTINGS_Rate = 0;
uint8_t EEMEM SETTINGS_RateSave = 4;
uint8_t EEMEM SETTINGS_RateSaveFlag = 0;

uint8_t SETTINGS_Num = 0;
uint8_t EEMEM SETTINGS_NumSave = 1;
uint8_t EEMEM SETTINGS_NumSaveFlag = 0;


uint8_t SETTINGS_Adr[DEFAULT_NUM] = {0, 0, 0, 0};
uint8_t EEMEM SETTINGS_AdrSave[DEFAULT_NUM] = {0x51, 0x52, 0x53, 0x54};
uint8_t EEMEM SETTINGS_AdrSaveFlag = 0;

uint8_t SettingsShift = 0;
uint8_t SettingsMax = SETTINGS_MAX;
uint8_t SETTINGS_SetMarker = 1;

uint8_t EEMEM SettingsText[SETTINGS_MAX][20] = {
// {"SETTINGS:           "},
// {" DELAY          MS  "},
// {" BAUD           BPS "},
// {" CYCKLE         MS  "},
// {" RATE               "},
// {" NUM:               "},
// {" 1ST:               "},
// {" 2ND:               "},
// {" 3RD:               "},
// {" 4TH:               "},
// {"1-YES 0-NO 9-DEFAULT"},

{"��������� �����:    "},
{" ��������           "},
{" �������            "},
{" ���� ������        "},
{" �����.������       "},
{" ���-�� ��:         "},
{" 1-� ��� -          "},
{" 2-� ��� -          "},
{" 3-� ��� -          "},
{" 4-� ��� -          "},
{"1-���� 0-��� 9-�����"},

};

uint8_t EEMEM ClockSetText[4][20] = {
{"��������� �������:  "},
{"  :  :     .  .     "},
{"                    "},
{"1-���� 0-��� 9-�����"},
};

uint8_t EEMEM ClockFailedText[4][20] = {
{"��������� �������:  "},
{"??:??:?? ??.??.??   "},
{"������! ���� �����! "},
{"               0-���"},
};

uint8_t EEMEM RegEmptyText[4][20] = {
// {"         (    )/    "},
// {"                    "},
// {"    <NO EVENTS>     "},
// {"              0-BACK"},
	
{"         (    )/    "},
{"                    "},
{"   <������ ����>    "},
{"               0-���"},

};

uint8_t EEMEM RegErrorText[4][20] = {
{"         (    )/    "},
{"                    "},
{"  <������ ������>   "},
{"               0-���"},
};

uint8_t EEMEM RegOutText[4][20] = {
{"         (    )/    "},
{"  :  :        .  .  "},
{" <������. �������>  "},
{"               0-���"},
};

uint8_t EEMEM RegShowText[4][20] = {
{"         (    )/    "},
{"  :  :        .  .  "},
{"                    "},
{"��             0-���"},
};

uint8_t EEMEM RegResetText[4][20] = {
{"������� �������:    "},
{"������� ����� �����-"},
{"��. ����������?     "},
{"1-����         0-���"},
};

uint8_t EEMEM RegSaveText[4][20] = {
{"���������� �������: "},
{"���: 0000 ���: 0000 "},
{"���������� ������.? "},
{"1-����         0-���"},
};

uint8_t EEMEM RegSaveProgressText[4][20] = {
{"���������� �������.."},
{"                    "},
{"���:       ���:     "},
{"                    "},
};

uint8_t EEMEM RegSaveCompletedText[4][20] = {
{"���������� �������: "},
{" ���������� ������� "},
{"     ���������      "},
{"1-����              "},
};

uint8_t EEMEM RegSetText[4][20] = {
{"��������� �������:  "},
{" ������ �������     "},
{" ����� �������      "},
{"1-���� 0-��� 9-�����"},
};

uint8_t EEMEM TranspSelectText[4][20] = {
{"����� ����. ������: "},
{"1 9600              "},
{"2 57600             "},
{"3 115200       0-���"},
};

uint8_t EEMEM TranspConfirmText[4][20] = {
{"���������� �����:   "},
{"  ����������� ����  "},
{"1-����              "},
{"0-���               "},
};

uint8_t EEMEM TranspProgressText[4][20] = {
{"���������� �����:   "},
{"����������������    "},
{"���������...        "},
{"                    "},
};

uint8_t EEMEM TranspActiveText[4][20] = {
{"���������� �����:   "},
{"                    "},
{"    �����������!    "},
{"                    "},
};

uint8_t EEMEM DiagnosticText[4][20] = {
{"�����������:   0-���"},
{"����������.........."},
{"������.............."},
{"����................"},
};
/************************************************************************/
/* -> �����: ������ ������� APD						*/
/************************************************************************/
void SCREEN_ERROR_PRJ (void) {
	uint8_t TempChar = 0;
	for (uint8_t StrIndex = 0; StrIndex <= 3; ++ StrIndex) {
		for (uint8_t ColIndex = 0; ColIndex <= 19; ++ ColIndex) {
			
			TempChar = eeprom_read_byte ((const uint8_t *) ErrorPrjText + 20 * (StrIndex) + ColIndex);
			if (TempChar >= 0xC0) {
				LCD_PutSym (StrIndex, ColIndex, RusFont[TempChar % 0xC0]);
			} else {
				LCD_PutSym (StrIndex, ColIndex, TempChar);
			}
		}
	}
}

/************************************************************************/
/* -> �����: ������ �����������			*/
/************************************************************************/
void SCREEN_ERROR_RMD (void) {
	uint8_t TempChar = 0;
	for (uint8_t StrIndex = 0; StrIndex <= 3; ++ StrIndex) {
		for (uint8_t ColIndex = 0; ColIndex <= 19; ++ ColIndex) {
			
			TempChar = eeprom_read_byte ((const uint8_t *) ErrorRmdText + 20 * (StrIndex) + ColIndex);
			if (TempChar >= 0xC0) {
				LCD_PutSym (StrIndex, ColIndex, RusFont[TempChar % 0xC0]);
			} else {
				LCD_PutSym (StrIndex, ColIndex, TempChar);
			}
		}
	}
}

/************************************************************************/
/* -> �����: ��������� ����							*/
/************************************************************************/
void SCREEN_INTRO (void) {
	uint8_t TempChar = 0;
	for (uint8_t StrIndex = 0; StrIndex <= 3; ++ StrIndex) {
		for (uint8_t ColIndex = 0; ColIndex <= 19; ++ ColIndex) {

			TempChar = eeprom_read_byte ((const uint8_t *) IntroText + 20 * (StrIndex) + ColIndex);
			if (TempChar >= 0xC0) {
				LCD_PutSym (StrIndex, ColIndex, RusFont[TempChar % 0xC0]);
			} else {
				LCD_PutSym (StrIndex, ColIndex, TempChar);
			}
		}
	}
	LCD_PutValDecPointMaskNeg_ (2, 17, 3, 1, 1, SoftVer);	
}

/************************************************************************/
/* -> �����: ����														*/
/************************************************************************/
void SCREEN_MENU (void) {
	uint8_t TempChar = 0;

	for (uint8_t ColIndex = 0; ColIndex <= 19; ++ ColIndex) {
		TempChar = eeprom_read_byte ((const uint8_t *) MenuText + ColIndex);
		if (TempChar >= 0xC0) {
			LCD_PutSym (0, ColIndex, RusFont[TempChar % 0xC0]);
		} else {
			LCD_PutSym (0, ColIndex, TempChar);
		}
	}

	for (uint8_t StrIndex = 1; StrIndex <= 3; ++ StrIndex) {
		for (uint8_t ColIndex = 0; ColIndex <= 19; ++ ColIndex) {
			
			TempChar = eeprom_read_byte ((const uint8_t *) MenuText + 20 * (StrIndex + MenuShift) + ColIndex);
			if (TempChar >= 0xC0) {
				LCD_PutSym (StrIndex, ColIndex, RusFont[TempChar % 0xC0]);
			} else {
				LCD_PutSym (StrIndex, ColIndex, TempChar);
			}
		}
	}
	
	if (MenuShift < (MenuMax - 4)) {
		LCD_PutSym (3, 19, 0x02);
	} else {
		LCD_PutSym (3, 19, 0x00);
	}
	if (MenuShift) {
		LCD_PutSym (1, 19, 0x03);
	} else {
		LCD_PutSym (1, 19, 0x00);
	}
	
	LCD_PutSym (2, 19, 0x00);
}

/************************************************************************/
/* -> �����: ��������� �������						*/
/************************************************************************/
uint8_t SCREEN_TIME (void) {
	uint8_t DateLim = 0;
	uint8_t TempChar = 0;
	if (TIMESET_ShowOnce) {
		// ������ ������ "�������"
		TIMESET_ShowOnce = 0;
		
		for (uint8_t StrIndex = 0; StrIndex <= 3; ++ StrIndex) {
			for (uint8_t ColIndex = 0; ColIndex <= 19; ++ ColIndex) {
				
				TempChar = eeprom_read_byte ((const uint8_t *) ClockSetText + 20 * (StrIndex) + ColIndex);
				if (TempChar >= 0xC0) {
					LCD_PutSym (StrIndex, ColIndex, RusFont[TempChar % 0xC0]);
					} else {
					LCD_PutSym (StrIndex, ColIndex, TempChar);
				}
			}
		}		
		
		// "������" �������� �������
		if (TIME_Get ()) {
			// ����� ����� ������ �����
			TIMESET_ClockFailed = 0;
			// ������ �������� ������� � ����
			TIMESET_ShowSeconds = 10*((TIME_GetSeconds & 0x70)>>4) + (TIME_GetSeconds & 0x0F);
			TIMESET_ShowMinutes = 10*((TIME_GetMinutes & 0x70)>>4) + (TIME_GetMinutes & 0x0F);
			TIMESET_ShowHours = 10*((TIME_GetHours & 0x30)>>4) + (TIME_GetHours & 0x0F);
			TIMESET_ShowDay = (TIME_GetDay & 0x07);
			TIMESET_ShowDate = 10*((TIME_GetDate & 0x30)>>4) + (TIME_GetDate & 0x0F);
			TIMESET_ShowMonth = 10*((TIME_GetMonth & 0x10)>>4) + (TIME_GetMonth & 0x0F);
			TIMESET_ShowYear = 10*((TIME_GetYear & 0xF0)>>4) + (TIME_GetYear & 0x0F);
		} else {
			// ��������� ����� ������ �����
			TIMESET_ClockFailed = 1;
			// ��������� �������� ������� � ����
			TIMESET_ShowSeconds = 0;
			TIMESET_ShowMinutes = 0;
			TIMESET_ShowHours = 0;
			TIMESET_ShowDay = 0;
			TIMESET_ShowDate = 0;
			TIMESET_ShowMonth = 0;
			TIMESET_ShowYear = 0;
		}
	}
	
	if (TIMESET_ClockFailed) {
		// ��������� ������ "���� �����"
		for (uint8_t StrIndex = 0; StrIndex <= 3; ++ StrIndex) {
			for (uint8_t ColIndex = 0; ColIndex <= 19; ++ ColIndex) {
				
				TempChar = eeprom_read_byte ((const uint8_t *) ClockFailedText + 20 * (StrIndex) + ColIndex);
				if (TempChar >= 0xC0) {
					LCD_PutSym (StrIndex, ColIndex, RusFont[TempChar % 0xC0]);
					} else {
					LCD_PutSym (StrIndex, ColIndex, TempChar);
				}
			}
		}
		// (0) BACK
		if (Action == 0) {
			// ������� � ��������� �����
			Mode = MODE_MENU;
			TIMER_ClockRefreshEn = 1;
			ClockShowEn = 1;
			LCD_Refresh = 1;
			TIMESET_CurPos = 0; // ������ �� ��������� �������
			// ����� ����� ������ �����
			TIMESET_ClockFailed = 0;
			Action = 255;
			// ���������� "�������" �������� ������� ��� ����� ������ �������
			TIMESET_ShowOnce = 1;
		}
		return 0;
	} else {
		LCD_Clear ();
		LCD_PutStr (0, 0, (uint8_t *) "TIME SET            ");
		// ��������� ������� ������:
		
		// (7) ����� - ����� ������� �����
		if (Action == 7) {
			if (TIMESET_CurPos == 0) {
				TIMESET_CurPos = 15;
			} else {
				TIMESET_CurPos -= 3;
			}
			Action = 255;
		}
		
		// (8) ������ - ����� ������� ������
		if (Action == 8) {
			if (TIMESET_CurPos == 15) {
				TIMESET_CurPos = 0;
			} else {
				TIMESET_CurPos += 3;
			}
			Action = 255;
		}
		
		// (5) ����� - ��������� ��������
		if (Action == 5) {
			// ��������� �������� � ����������� �� ��������� �������
			// ����
			if (TIMESET_CurPos == 0) {
				if (TIMESET_ShowHours == 23) {
					TIMESET_ShowHours = 0;
				} else {
					++ TIMESET_ShowHours;
				}
			}
			// ������
			if (TIMESET_CurPos == 3) {
				if (TIMESET_ShowMinutes == 59) {
					TIMESET_ShowMinutes = 0;
				} else {
					++ TIMESET_ShowMinutes;
				}
			}
			// �������
			if (TIMESET_CurPos == 6) {
				if (TIMESET_ShowSeconds == 59) {
					TIMESET_ShowSeconds = 0;
				} else {
					++ TIMESET_ShowSeconds;
				}
			}
			// �����
			if (TIMESET_CurPos == 9) {
				if (TIMESET_ShowDate == DateLim) {
					TIMESET_ShowDate = 1;
				} else {
					++ TIMESET_ShowDate;
				}
			}
			// ������
			if (TIMESET_CurPos == 12) {
				if (TIMESET_ShowMonth == 12) {
					TIMESET_ShowMonth = 1;
				} else {
					++ TIMESET_ShowMonth;
				}
			}
			// ����
			if (TIMESET_CurPos == 15) {
				if (TIMESET_ShowYear == 99) {
					TIMESET_ShowYear = 0;
				} else {
					++ TIMESET_ShowYear;
				}
			}
			Action = 255;
		}
		
		// (6) ���� - ��������� ��������
		if (Action == 6) {
			// ��������� �������� � ����������� �� ��������� �������
			// ����
			if (TIMESET_CurPos == 0) {
				if (TIMESET_ShowHours == 0) {
					TIMESET_ShowHours = 23;
				} else {
					-- TIMESET_ShowHours;
				}
			}
			// ������
			if (TIMESET_CurPos == 3) {
				if (TIMESET_ShowMinutes == 0) {
					TIMESET_ShowMinutes = 59;
				} else {
					-- TIMESET_ShowMinutes;
				}
			}
			// �������
			if (TIMESET_CurPos == 6) {
				if (TIMESET_ShowSeconds == 0) {
					TIMESET_ShowSeconds = 59;
				} else {
					-- TIMESET_ShowSeconds;
				}
			}
			// �����
			if (TIMESET_CurPos == 9) {
				if (TIMESET_ShowDate == 1) {
					TIMESET_ShowDate = DateLim;
				} else {
					-- TIMESET_ShowDate;
				}
			}
			// ������
			if (TIMESET_CurPos == 12) {
				if (TIMESET_ShowMonth == 1) {
					TIMESET_ShowMonth = 12;
				} else {
					-- TIMESET_ShowMonth;
				}
			}
			// ����
			if (TIMESET_CurPos == 15) {
				if (TIMESET_ShowYear == 0) {
					TIMESET_ShowYear = 99;
				} else {
					-- TIMESET_ShowYear;
				}
			}
			Action = 255;
		}
		
		// (1) YES
		if (Action == 1) {
			// ���������� ���������
			if (TIME_SetManual (TIMESET_ShowHours, TIMESET_ShowMinutes, TIMESET_ShowSeconds, TIMESET_ShowDay, TIMESET_ShowDate, TIMESET_ShowMonth, TIMESET_ShowYear)) {
				// ������� � ����
				Mode = MODE_MENU;
				TIMER_ClockRefreshEn = 1;
				ClockShowEn = 1;
				LCD_Refresh = 1;
				TIMESET_CurPos = 0;
				Action = 255;
				// ���������� "�������" �������� ������� ��� ����� ������ �������
				TIMESET_ShowOnce = 1;
			}
		}
		
		// (0) NO
		if (Action == 0) {
			// ������� � ����
			Mode = MODE_MENU;
			TIMER_ClockRefreshEn = 1;
			ClockShowEn = 1;
			LCD_Refresh = 1;
			TIMESET_CurPos = 0;
			Action = 255;
			// ���������� "�������" �������� ������� ��� ����� ������ �������
			TIMESET_ShowOnce = 1;
		}

		// (9) - DEFAULT
		if (Action == 9) {
			TIMESET_CurPos = 0;
			TIMESET_ShowHours = 0;
			TIMESET_ShowMinutes = 0;
			TIMESET_ShowSeconds = 0;
			TIMESET_ShowDate = 1;
			TIMESET_ShowMonth = 1;
			TIMESET_ShowYear = 13;
			Action = 255;
		}
				
		// ����� ������ ��� ���� � ����������� �� �������� ������ � ����
		switch (TIMESET_ShowMonth) {
			case 1: // ������
				DateLim = 31;
				break;
			case 2: // �������
				if (TIMESET_ShowYear%4) {
					DateLim = 28; // �� ����������
				} else {
					DateLim = 29; // ����������
				}
				break;
			case 3: // ����
				DateLim = 31;
				break;
			case 4: // ������
				DateLim = 30;
				break;
			case 5: // ���
				DateLim = 31;
				break;
			case 6: // ����
				DateLim = 30;
				break;
			case 7: // ����
				DateLim = 31;
				break;
			case 8: // ������
				DateLim = 31;
				break;
			case 9: // ��������
				DateLim = 30;
				break;
			case 10: // �������
				DateLim = 31;
				break;
			case 11: // ������
				DateLim = 30;
				break;
			case 12: // �������
				DateLim = 31;
				break;
			default:
				DateLim = 31;
				break;
		}
		// �������� �������� ����� �� ������� �����
		if (TIMESET_ShowDate > DateLim) {
			TIMESET_ShowDate = DateLim;
		}

		for (uint8_t StrIndex = 0; StrIndex <= 3; ++ StrIndex) {
			for (uint8_t ColIndex = 0; ColIndex <= 19; ++ ColIndex) {
				
				TempChar = eeprom_read_byte ((const uint8_t *) ClockSetText + 20 * (StrIndex) + ColIndex);
				if (TempChar >= 0xC0) {
					LCD_PutSym (StrIndex, ColIndex, RusFont[TempChar % 0xC0]);
					} else {
					LCD_PutSym (StrIndex, ColIndex, TempChar);
				}
			}
		}

//		LCD_PutStr (1, 0, (uint8_t *)"  :  :     .  .  ");
		LCD_PutValDec (1, 0, 2, TIMESET_ShowHours);
		LCD_PutValDec (1, 3, 2, TIMESET_ShowMinutes);
		LCD_PutValDec (1, 6, 2, TIMESET_ShowSeconds);
		LCD_PutValDec (1, 9, 2, TIMESET_ShowDate);
		LCD_PutValDec (1, 12, 2, TIMESET_ShowMonth);
		LCD_PutValDec (1, 15, 2, TIMESET_ShowYear);
//		LCD_PutStr (2, 0, (uint8_t *)"                 ");
		//	LCD_PutStr (2, 0 + TIMESET_CurPos, (uint8_t *)"==");
		LCD_PutSym (2, 0 + TIMESET_CurPos, /*0xD9*/ 0x06);
		LCD_PutSym (2, 0 + TIMESET_CurPos + 1, /*0xD9*/ 0x06);
//		LCD_PutStr (3, 0, (uint8_t *) "1-YES 0-NO 9-DEFAULT");
		return 0;
	}
}

/************************************************************************/
/* -> �����: ��������� ���������					*/
/************************************************************************/
void SCREEN_CONTRAST (void) {
	static uint8_t CONTRAST_ContrastTemp = 0;
	uint8_t TempChar = 0;
	
	if (CONTRAST_ShowOnce) {
		CONTRAST_ShowOnce = 0;
		
		CONTRAST_Contrast = LCD_Contrast;
		CONTRAST_ContrastTemp = CONTRAST_Contrast;
		
		for (uint8_t StrIndex = 0; StrIndex <= 3; ++ StrIndex) {
			for (uint8_t ColIndex = 0; ColIndex <= 19; ++ ColIndex) {
				
				TempChar = eeprom_read_byte ((const uint8_t *) ContrastText + 20 * (StrIndex) + ColIndex);
				if (TempChar >= 0xC0) {
					LCD_PutSym (StrIndex, ColIndex, RusFont[TempChar % 0xC0]);
				} else {
					LCD_PutSym (StrIndex, ColIndex, TempChar);
				}
			}
		}

		LCD_PutSym (1, 0, ARROW_LEFT);
		LCD_PutSym (1, 19, ARROW_RIGHT);
		LCD_PutStrNum (1, 1, 18, StrSpaces);
		LCD_PutStrNum (1, 1, (CONTRAST_Contrast - 7 - 1), StrBlackSq);
// 		for (uint8_t Index = 0; Index <= (CONTRAST_Contrast - 6 - 1); ++ Index) {
// 			LCD_PutSym (1, 1 + Index, 0xFF);
// 		}
		// LCD_PutValDec (2, 0, 3, CONTRAST_Contrast);
	}
	
	// ��������� ��������
	switch (Action) {
		
		// (7) ����� - ���������
		case 7:
			if (CONTRAST_Contrast > 7) {
				-- CONTRAST_Contrast;
			}
			LCD_Contrast = CONTRAST_Contrast;
			DAC_New ();
			Action = 255;
			break;
		
		// (8) ������ - ���������
		case 8:
			if (CONTRAST_Contrast < 25) {
				++ CONTRAST_Contrast;
			}
			LCD_Contrast = CONTRAST_Contrast;
			DAC_New ();
			Action = 255;
			break;
		
		// (1) YES
		case 1:
			// ���������� ���������
			if (CONTRAST_ContrastTemp != CONTRAST_Contrast) {
				LCD_Contrast = CONTRAST_Contrast;
				DAC_New ();
				eeprom_write_byte ((uint8_t *) &CONTRAST_Save, (uint8_t) CONTRAST_Contrast);
			}
			
			// ������� � ����
			Mode = MODE_MENU;
			TIMER_ClockRefreshEn = 1;
			ClockShowEn = 1;
			LCD_Refresh = 1;
			Action = 255;
			CONTRAST_ShowOnce = 1;
			break;

		// (0) NO
		case 0:
			LCD_Contrast = eeprom_read_byte ((uint8_t *) &CONTRAST_Save);
			DAC_New ();
			// ������� � ����
			Mode = MODE_MENU;
			TIMER_ClockRefreshEn = 1;
			ClockShowEn = 1;
			LCD_Refresh = 1;
			Action = 255;
			CONTRAST_ShowOnce = 1;
			break;
		
		// (9) DEFAULT
		case 9:
			CONTRAST_Contrast = DEFAULT_CONTRAST /*eeprom_read_byte ((uint8_t *) &CONTRAST_Save)*/;
			LCD_Contrast = CONTRAST_Contrast;
			DAC_New ();
			Action = 255;
			break;
		
		default:
			break;
	}
	
// 	LCD_PutStr (1, 1, (uint8_t *) "                  ");
// 
// 	for (uint8_t Index = 0; Index < (CONTRAST_Contrast - 7); ++ Index) {
// 		LCD_PutSym (1, 1 + Index, 0xFF);
// 	}
	LCD_PutStrNum (1, 1, 18, StrSpaces);
	LCD_PutStrNum (1, 1, (CONTRAST_Contrast - 7), StrBlackSq);
	// LCD_PutValDec (2, 0, 3, CONTRAST_Contrast);
}

/************************************************************************/
/* -> �����: ���������									*/
/************************************************************************/
void SCREEN_SETTINGS (void) {

	static uint8_t SETTINGS_Delay_T = 0;
	static uint8_t SETTINGS_Baud_T = 0;
	static uint8_t SETTINGS_Cykle_T = 0;
	static uint8_t SETTINGS_Rate_T = 0;
	static uint8_t SETTINGS_Num_T = 0;
	static uint8_t SETTINGS_Adr_T[DEFAULT_NUM] = {0, 0, 0, 0};
	
	uint8_t TempChar = 0;
	uint8_t CurrentPos = 0;
	uint16_t AdrTemp = 0;
	
	if (SETTINGS_ShowOnce) {
		SETTINGS_ShowOnce = 0;
	
		uint8_t StrIndex = 0;
		for (uint8_t ColIndex = 0; ColIndex <= 19; ++ ColIndex) {
			
			TempChar = eeprom_read_byte ((const uint8_t *) SettingsText + ColIndex);
			if (TempChar >= 0xC0) {
				LCD_PutSym (StrIndex, ColIndex, RusFont[TempChar % 0xC0]);
			} else {
				LCD_PutSym (StrIndex, ColIndex, TempChar);
			}
		}
		StrIndex = 3;
		for (uint8_t ColIndex = 0; ColIndex <= 19; ++ ColIndex) {
				
			TempChar = eeprom_read_byte ((const uint8_t *) SettingsText + 20 * (SettingsMax - 1) + ColIndex);
			if (TempChar >= 0xC0) {
				LCD_PutSym (StrIndex, ColIndex, RusFont[TempChar % 0xC0]);
			} else {
					LCD_PutSym (StrIndex, ColIndex, TempChar);
			}
		}

		for (uint8_t StrIndex = 1; StrIndex <= 2; ++ StrIndex) {
			for (uint8_t ColIndex = 0; ColIndex <= 19; ++ ColIndex) {
				
				TempChar = eeprom_read_byte ((const uint8_t *) SettingsText + 20 * (StrIndex + SettingsShift) + ColIndex);
				if (TempChar >= 0xC0) {
					LCD_PutSym (StrIndex, ColIndex, RusFont[TempChar % 0xC0]);
				} else {
					LCD_PutSym (StrIndex, ColIndex, TempChar);
				}
			}
		}
		
		SETTINGS_Delay = TIMER_RequestTimeoutPer;
		SETTINGS_Delay_T = SETTINGS_Delay;
		
		SETTINGS_Baud = APD_BaudRateIndex;
		SETTINGS_Baud_T = SETTINGS_Baud;
		
		SETTINGS_Cyckle = APD_FrameRefreshRate / 100;
		SETTINGS_Cykle_T = SETTINGS_Cyckle;	
		
		SETTINGS_Rate = APD_EventReqRatio;
		SETTINGS_Rate_T = SETTINGS_Rate;
		
		SETTINGS_Num = APD_EventAdrNum;
		SETTINGS_Num_T = SETTINGS_Num;
		
		SETTINGS_Adr[0] = APD_EventAdrArray[0] + 0x50;
		SETTINGS_Adr_T[0] = SETTINGS_Adr[0];
		SETTINGS_Adr[1] = APD_EventAdrArray[1] + 0x50;
		SETTINGS_Adr_T[1] = SETTINGS_Adr[1];
		SETTINGS_Adr[2] = APD_EventAdrArray[2] + 0x50;
		SETTINGS_Adr_T[2] = SETTINGS_Adr[2];
		SETTINGS_Adr[3] = APD_EventAdrArray[3] + 0x50;
		SETTINGS_Adr_T[3] = SETTINGS_Adr[3];

		for (uint8_t Start = SettingsShift; Start <= (SettingsShift + 1); ++ Start) {
			CurrentPos = Start - SettingsShift + 1;
			switch (Start) {
				case 0:
					LCD_PutValDec (CurrentPos, 15, 3, SETTINGS_Delay);
					LCD_PutSym (CurrentPos, 14, (uint8_t)ARROW_LEFT);
					LCD_PutSym (CurrentPos, 18, (uint8_t)ARROW_RIGHT);
					break;
				case 1:
					LCD_PutValDecPointMaskNeg_ (CurrentPos, 13, 5, 0, 1, SETTINGS_BaudValue[SETTINGS_Baud]);
					LCD_PutSym (CurrentPos, 12, (uint8_t)ARROW_LEFT);
					LCD_PutSym (CurrentPos, 18, (uint8_t)ARROW_RIGHT);
					break;
				case 2:
					LCD_PutValDecPointMaskNeg_ (CurrentPos, 14, 4, 0, 1, SETTINGS_CyckleValue[SETTINGS_Cyckle - 1]);
					LCD_PutSym (CurrentPos, 13, (uint8_t)ARROW_LEFT);
					LCD_PutSym (CurrentPos, 18, (uint8_t)ARROW_RIGHT);
					break;
				case 3:
					LCD_PutValDecPointMaskNeg_ (CurrentPos, 16, 2, 0, 1, SETTINGS_Rate);
					LCD_PutSym (CurrentPos, 15, (uint8_t)ARROW_LEFT);
					LCD_PutSym (CurrentPos, 18, (uint8_t)ARROW_RIGHT);
					break;
				case 4:
					LCD_PutValDecPointMaskNeg_ (CurrentPos, 17, 1, 0, 1, SETTINGS_Num);
					LCD_PutSym (CurrentPos, 16, (uint8_t)ARROW_LEFT);
					LCD_PutSym (CurrentPos, 18, (uint8_t)ARROW_RIGHT);
					break;
				case 5:
					LCD_PutValHex (CurrentPos, 16, 2, SETTINGS_Adr[0]);
					LCD_PutSym (CurrentPos, 14, (uint8_t)'0');
					LCD_PutSym (CurrentPos, 15, (uint8_t) 0x07);
					LCD_PutSym (CurrentPos, 13, (uint8_t)ARROW_LEFT);
					LCD_PutSym (CurrentPos, 18, (uint8_t)ARROW_RIGHT);
					break;
				case 6:
					LCD_PutValHex (CurrentPos, 16, 2, SETTINGS_Adr[1]);
					LCD_PutSym (CurrentPos, 14, (uint8_t)'0');
					LCD_PutSym (CurrentPos, 15, (uint8_t) 0x07);
					LCD_PutSym (CurrentPos, 13, (uint8_t)ARROW_LEFT);
					LCD_PutSym (CurrentPos, 18, (uint8_t)ARROW_RIGHT);
					break;
				case 7:
					LCD_PutValHex (CurrentPos, 16, 2, SETTINGS_Adr[2]);
					LCD_PutSym (CurrentPos, 14, (uint8_t)'0');
					LCD_PutSym (CurrentPos, 15, (uint8_t) 0x07);
					LCD_PutSym (CurrentPos, 13, (uint8_t)ARROW_LEFT);
					LCD_PutSym (CurrentPos, 18, (uint8_t)ARROW_RIGHT);
					break;
				case 8:
					LCD_PutValHex (CurrentPos, 16, 2, SETTINGS_Adr[3]);
					LCD_PutSym (CurrentPos, 14, (uint8_t)'0');
					LCD_PutSym (CurrentPos, 15, (uint8_t) 0x07);
					LCD_PutSym (CurrentPos, 13, (uint8_t)ARROW_LEFT);
					LCD_PutSym (CurrentPos, 18, (uint8_t)ARROW_RIGHT);
					break;
				default:
					break;
			}
		}
		// ������
		LCD_PutSym (SETTINGS_SetMarker, 0, (uint8_t)ARROW_RIGHT);
		if (SettingsShift > 0) LCD_PutSym (1, 19, (uint8_t)ARROW_UP);
		if (SettingsShift < (SettingsMax - 4)) LCD_PutSym (2, 19, (uint8_t)ARROW_DOWN);	
	}
	
	// ��������� �������
		switch (Action) {
			// (7) ����� - ���������
 			case 7:

 				if ((SettingsShift + (SETTINGS_SetMarker - 1)) == 0) {
	 				if (SETTINGS_Delay > 1) -- SETTINGS_Delay;
	 			} else if ((SettingsShift + (SETTINGS_SetMarker - 1)) == 1) {
	 				if (SETTINGS_Baud > 0) -- SETTINGS_Baud;
 				} else if ((SettingsShift + (SETTINGS_SetMarker - 1)) == 2) {
 					if (SETTINGS_Cyckle > 1) -- SETTINGS_Cyckle;
 				} else if ((SettingsShift + (SETTINGS_SetMarker - 1)) == 3) {
 					if (SETTINGS_Rate > 1) -- SETTINGS_Rate;
 				} else if ((SettingsShift + (SETTINGS_SetMarker - 1)) == 4) {
 					if (SETTINGS_Num > 1) -- SETTINGS_Num;
 				} else if ((SettingsShift + (SETTINGS_SetMarker - 1)) == 5) {
 					if (SETTINGS_Adr[0] > 0x50) -- SETTINGS_Adr[0];
 				} else if ((SettingsShift + (SETTINGS_SetMarker - 1)) == 6) {
 					if (SETTINGS_Adr[1] > 0x50) -- SETTINGS_Adr[1];
 				} else if ((SettingsShift + (SETTINGS_SetMarker - 1)) == 7) {
 					if (SETTINGS_Adr[2] > 0x50) -- SETTINGS_Adr[2];
 				} else if ((SettingsShift + (SETTINGS_SetMarker - 1)) == 8) {
 					if (SETTINGS_Adr[3] > 0x50) -- SETTINGS_Adr[3];
 				}
 				
				Action = 255;
 				break;
		
			// (8) ������ - ���������
			case 8:
			
 				if ((SettingsShift + (SETTINGS_SetMarker - 1)) == 0) {
	 				if (SETTINGS_Delay < 10) ++ SETTINGS_Delay;
	 			} else if ((SettingsShift + (SETTINGS_SetMarker - 1)) == 1) {
	 				if (SETTINGS_Baud < 3) ++ SETTINGS_Baud;
	 			} else if ((SettingsShift + (SETTINGS_SetMarker - 1)) == 2) {
	 				if (SETTINGS_Cyckle < 10) ++ SETTINGS_Cyckle;
	 			} else if ((SettingsShift + (SETTINGS_SetMarker - 1)) == 3) {
	 				if (SETTINGS_Rate < 15) ++ SETTINGS_Rate;
	 			} else if ((SettingsShift + (SETTINGS_SetMarker - 1)) == 4) {
	 				if (SETTINGS_Num < 4) ++ SETTINGS_Num;
	 			} else if ((SettingsShift + (SETTINGS_SetMarker - 1)) == 5) {
	 				if (SETTINGS_Adr[0] < 0x5F) ++ SETTINGS_Adr[0];
	 			} else if ((SettingsShift + (SETTINGS_SetMarker - 1)) == 6) {
	 				if (SETTINGS_Adr[1] < 0x5F) ++ SETTINGS_Adr[1];
	 			} else if ((SettingsShift + (SETTINGS_SetMarker - 1)) == 7) {
	 				if (SETTINGS_Adr[2] < 0x5F) ++ SETTINGS_Adr[2];
	 			} else if ((SettingsShift + (SETTINGS_SetMarker - 1)) == 8) {
	 				if (SETTINGS_Adr[3] < 0x5F) ++ SETTINGS_Adr[3];
 				}
				
				Action = 255;
				break;

			// (5) ����� - ������ �����
			case 5:
				if (SETTINGS_SetMarker > 1) {
					-- SETTINGS_SetMarker;
				} else {
					SETTINGS_SetMarker = 1;
					if (SettingsShift > 0) -- SettingsShift;
				}
				Action = 255;
				break;

			// (6) ���� - ������ ����
			case 6:
				if (SETTINGS_SetMarker < 2) {
					++ SETTINGS_SetMarker;
				} else {
					SETTINGS_SetMarker = 2;
					if (SettingsShift < (SettingsMax - 4)) ++ SettingsShift;
				}
				Action = 255;
				break;
		
		// (1) - YES
		case 1:
			cli();
			if (SETTINGS_Delay_T != SETTINGS_Delay) {
				// ���������� ���������
				TIMER_RequestTimeoutPer = SETTINGS_Delay;
				eeprom_write_byte ((uint8_t *) &SETTINGS_DelaySave, (uint8_t) SETTINGS_Delay);
				eeprom_write_byte ((uint8_t *) &SETTINGS_DelaySaveFlag, (uint8_t) 1);
			}
			if (SETTINGS_Baud_T != SETTINGS_Baud) {
				APD_BaudRateIndex = SETTINGS_Baud;
				eeprom_write_byte ((uint8_t *) &SETTINGS_BaudSave, (uint8_t) SETTINGS_Baud);
				eeprom_write_byte ((uint8_t *) &SETTINGS_BaudFlag, (uint8_t) 1);
				// ��������� �������� ������
				usart_set_baudrate (&USARTC0, SETTINGS_BaudValue[APD_BaudRateIndex], F_CPU);
			}
			if (SETTINGS_Cykle_T != SETTINGS_Cyckle) {
				APD_FrameRefreshRate = SETTINGS_CyckleValue[SETTINGS_Cyckle - 1];
				TIMER_DataRequestPer = APD_FrameRefreshRate;
 				eeprom_write_byte ((uint8_t *) &SETTINGS_CyckleSave, (uint8_t) SETTINGS_Cyckle);
 				eeprom_write_byte ((uint8_t *) &SETTINGS_CyckleSaveFlag, (uint8_t) 1);
			}
			if (SETTINGS_Rate_T != SETTINGS_Rate) {
				APD_EventReqRatio = SETTINGS_Rate;
				eeprom_write_byte ((uint8_t *) &SETTINGS_RateSave, (uint8_t) SETTINGS_Rate);
				eeprom_write_byte ((uint8_t *) &SETTINGS_RateSaveFlag, (uint8_t) 1);
			}
// 			if (SETTINGS_Num_T != SETTINGS_Num) {
// 				APD_EventAdrNum = SETTINGS_Num;
// 				eeprom_write_byte ((uint8_t *) &SETTINGS_NumSave, (uint8_t) SETTINGS_Num);
// 				eeprom_write_byte ((uint8_t *) &SETTINGS_NumSaveFlag, (uint8_t) 1);
// 			}
// 			if (SETTINGS_Adr_T[0] != SETTINGS_Adr[0]) {
// 				APD_EventAdrArray[0] = SETTINGS_Adr[0] - 0x50;
// 				eeprom_write_byte ((uint8_t *) SETTINGS_AdrSave, (uint8_t) SETTINGS_Adr[0] - 0x50);
// 				eeprom_write_byte ((uint8_t *) &SETTINGS_AdrSaveFlag, (uint8_t) 1);	
// 			}
// 			if (SETTINGS_Adr_T[1] != SETTINGS_Adr[1]) {
// 				APD_EventAdrArray[1] = SETTINGS_Adr[1] - 0x50;
// 				eeprom_write_byte ((uint8_t *) SETTINGS_AdrSave + 1, (uint8_t) SETTINGS_Adr[1] - 0x50);
// 				eeprom_write_byte ((uint8_t *) &SETTINGS_AdrSaveFlag, (uint8_t) 1);	
// 			}
// 			if (SETTINGS_Adr_T[2] != SETTINGS_Adr[2]) {
// 				APD_EventAdrArray[2] = SETTINGS_Adr[2] - 0x50;
// 				eeprom_write_byte ((uint8_t *) SETTINGS_AdrSave + 2, (uint8_t) SETTINGS_Adr[2] - 0x50);
// 				eeprom_write_byte ((uint8_t *) &SETTINGS_AdrSaveFlag, (uint8_t) 1);
// 			}
// 			if (SETTINGS_Adr_T[3] != SETTINGS_Adr[3]) {
// 				APD_EventAdrArray[3] = SETTINGS_Adr[3] - 0x50;
// 				eeprom_write_byte ((uint8_t *) SETTINGS_AdrSave + 3, (uint8_t) SETTINGS_Adr[3] - 0x50);
// 				eeprom_write_byte ((uint8_t *) &SETTINGS_AdrSaveFlag, (uint8_t) 1);
// 			}
 			if (
				(SETTINGS_Num_T != SETTINGS_Num) ||
				(SETTINGS_Adr_T[0] != SETTINGS_Adr[0]) ||
				(SETTINGS_Adr_T[1] != SETTINGS_Adr[1]) ||
				(SETTINGS_Adr_T[2] != SETTINGS_Adr[2]) ||
				(SETTINGS_Adr_T[3] != SETTINGS_Adr[3])
				) {
				APD_EventAdrNum = SETTINGS_Num;
				eeprom_write_byte ((uint8_t *) &SETTINGS_NumSave, (uint8_t) SETTINGS_Num);
				eeprom_write_byte ((uint8_t *) &SETTINGS_NumSaveFlag, (uint8_t) 1);
				APD_EventAdrArray[0] = SETTINGS_Adr[0] - 0x50;
				eeprom_write_byte ((uint8_t *) SETTINGS_AdrSave, (uint8_t) SETTINGS_Adr[0] - 0x50);
				APD_EventAdrArray[1] = SETTINGS_Adr[1] - 0x50;
				eeprom_write_byte ((uint8_t *) SETTINGS_AdrSave + 1, (uint8_t) SETTINGS_Adr[1] - 0x50);
				APD_EventAdrArray[2] = SETTINGS_Adr[2] - 0x50;
				eeprom_write_byte ((uint8_t *) SETTINGS_AdrSave + 2, (uint8_t) SETTINGS_Adr[2] - 0x50);
				APD_EventAdrArray[3] = SETTINGS_Adr[3] - 0x50;
				eeprom_write_byte ((uint8_t *) SETTINGS_AdrSave + 3, (uint8_t) SETTINGS_Adr[3] - 0x50);
			}
			sei();
		
			// SETTINGS_SetMarker = 1;
		
			// ������� � ����
			Mode = MODE_MENU;
			TIMER_ClockRefreshEn = 1;
			ClockShowEn = 1;
			LCD_Refresh = 1;
			Action = 255;
			SETTINGS_ShowOnce = 1;
			break;

		// (0) - NO
		case 0:
			// SETTINGS_SetMarker = 1;
		
			// ������� � ����
			Mode = MODE_MENU;
			TIMER_ClockRefreshEn = 1;
			ClockShowEn = 1;
			LCD_Refresh = 1;
			Action = 255;
			SETTINGS_ShowOnce = 1;
			break;
		
		// (9) DEFAULT - ������� ���������
		case 9:
			SETTINGS_Delay = DEFAULT_DELAY;
			SETTINGS_Baud = pgm_read_byte ( APD_PrjData + 0x000A );
			SETTINGS_Cyckle = pgm_read_byte ( APD_PrjData + 0x000B );
			SETTINGS_Rate = pgm_read_byte ( APD_PrjData + 0x000C );

			// ������ ������� ���������,
			// � ������� ����� ������������� ������ �������
			SETTINGS_Adr[0] = ((APD_EventAdr % 1000) / 100);
			AdrTemp = APD_EventAdr % 10;
			SETTINGS_Num = AdrTemp - SETTINGS_Adr[0] + 1;
			if (SETTINGS_Num) {
				for (uint8_t AdrId = 1; AdrId <= (SETTINGS_Num - 1); ++ AdrId) {
					SETTINGS_Adr[AdrId] = (SETTINGS_Adr[AdrId - 1] + 1);
				}
			}
			SETTINGS_Adr[0] += 0x50;
			SETTINGS_Adr[1] += 0x50;
			SETTINGS_Adr[2] += 0x50;
			SETTINGS_Adr[3] += 0x50;

			Action = 255;
			break;

		default:
			break;
	}

	for (uint8_t StrIndex = 1; StrIndex <= 2; ++ StrIndex) {
		for (uint8_t ColIndex = 0; ColIndex <= 19; ++ ColIndex) {
			
			TempChar = eeprom_read_byte ((const uint8_t *) SettingsText + 20 * (StrIndex + SettingsShift) + ColIndex);
			if (TempChar >= 0xC0) {
				LCD_PutSym (StrIndex, ColIndex, RusFont[TempChar % 0xC0]);
			} else {
				LCD_PutSym (StrIndex, ColIndex, TempChar);
			}
		}
	}

	for (uint8_t Start = SettingsShift; Start <= (SettingsShift + 1); ++ Start) {
		CurrentPos = Start - SettingsShift + 1;
		switch (Start) {
			case 0:
			LCD_PutValDec (CurrentPos, 15, 3, SETTINGS_Delay);
			LCD_PutSym (CurrentPos, 14, (uint8_t)ARROW_LEFT);
			LCD_PutSym (CurrentPos, 18, (uint8_t)ARROW_RIGHT);
			break;
			case 1:
			LCD_PutValDecPointMaskNeg_ (CurrentPos, 13, 5, 0, 1, SETTINGS_BaudValue[SETTINGS_Baud]);
			LCD_PutSym (CurrentPos, 12, (uint8_t)ARROW_LEFT);
			LCD_PutSym (CurrentPos, 18, (uint8_t)ARROW_RIGHT);
			break;
			case 2:
			LCD_PutValDecPointMaskNeg_ (CurrentPos, 14, 4, 0, 1, SETTINGS_CyckleValue[SETTINGS_Cyckle - 1]);
			LCD_PutSym (CurrentPos, 13, (uint8_t)ARROW_LEFT);
			LCD_PutSym (CurrentPos, 18, (uint8_t)ARROW_RIGHT);
			break;
			case 3:
			LCD_PutValDecPointMaskNeg_ (CurrentPos, 16, 2, 0, 1, SETTINGS_Rate);
			LCD_PutSym (CurrentPos, 15, (uint8_t)ARROW_LEFT);
			LCD_PutSym (CurrentPos, 18, (uint8_t)ARROW_RIGHT);
			break;
			case 4:
			LCD_PutValDecPointMaskNeg_ (CurrentPos, 17, 1, 0, 1, SETTINGS_Num);
			LCD_PutSym (CurrentPos, 16, (uint8_t)ARROW_LEFT);
			LCD_PutSym (CurrentPos, 18, (uint8_t)ARROW_RIGHT);
			break;
			case 5:
			LCD_PutValHex (CurrentPos, 16, 2, SETTINGS_Adr[0]);
			LCD_PutSym (CurrentPos, 14, (uint8_t)'0');
			LCD_PutSym (CurrentPos, 15, (uint8_t) 0x07);
			LCD_PutSym (CurrentPos, 13, (uint8_t)ARROW_LEFT);
			LCD_PutSym (CurrentPos, 18, (uint8_t)ARROW_RIGHT);
			break;
			case 6:
			LCD_PutValHex (CurrentPos, 16, 2, SETTINGS_Adr[1]);
			LCD_PutSym (CurrentPos, 14, (uint8_t)'0');
			LCD_PutSym (CurrentPos, 15, (uint8_t) 0x07);
			LCD_PutSym (CurrentPos, 13, (uint8_t)ARROW_LEFT);
			LCD_PutSym (CurrentPos, 18, (uint8_t)ARROW_RIGHT);
			break;
			case 7:
			LCD_PutValHex (CurrentPos, 16, 2, SETTINGS_Adr[2]);
			LCD_PutSym (CurrentPos, 14, (uint8_t)'0');
			LCD_PutSym (CurrentPos, 15, (uint8_t) 0x07);
			LCD_PutSym (CurrentPos, 13, (uint8_t)ARROW_LEFT);
			LCD_PutSym (CurrentPos, 18, (uint8_t)ARROW_RIGHT);
			break;
			case 8:
			LCD_PutValHex (CurrentPos, 16, 2, SETTINGS_Adr[3]);
			LCD_PutSym (CurrentPos, 14, (uint8_t)'0');
			LCD_PutSym (CurrentPos, 15, (uint8_t) 0x07);
			LCD_PutSym (CurrentPos, 13, (uint8_t)ARROW_LEFT);
			LCD_PutSym (CurrentPos, 18, (uint8_t)ARROW_RIGHT);
			break;
			default:
			break;
		}
	}

	LCD_PutSym (SETTINGS_SetMarker, 0, (uint8_t)ARROW_RIGHT);
	if (SettingsShift > 0) LCD_PutSym (1, 19, (uint8_t)ARROW_UP);
	if (SettingsShift < (SettingsMax - 4)) LCD_PutSym (2, 19, (uint8_t)ARROW_DOWN);
}

/************************************************************************/
/* -> �����: �����������								*/
/************************************************************************/
void SCREEN_DIAGNOSTIC (void) {
	uint8_t TempChar = 0;
	for (uint8_t StrIndex = 0; StrIndex <= 3; ++ StrIndex) {
		for (uint8_t ColIndex = 0; ColIndex <= 19; ++ ColIndex) {
			
			TempChar = eeprom_read_byte ((const uint8_t *) DiagnosticText + 20 * (StrIndex) + ColIndex);
			if (TempChar >= 0xC0) {
				LCD_PutSym (StrIndex, ColIndex, RusFont[TempChar % 0xC0]);
			} else {
				LCD_PutSym (StrIndex, ColIndex, TempChar);
			}
		}
	}
	if (RMD_Error) {
		LCD_PutStr (1, 15, (uint8_t *)"ERROR");
	} else {
		LCD_PutStr (1, 18, (uint8_t *)"OK");
	}
	if (PRJ_Error) {
		LCD_PutStr (2, 15, (uint8_t *)"ERROR");
	} else {
		LCD_PutStr (2, 18, (uint8_t *)"OK");
	}
	if (CLOCK_Error) {
		LCD_PutStr (3, 15, (uint8_t *)"ERROR");
	} else {
		LCD_PutStr (3, 18, (uint8_t *)"OK");
	}
}

/* ��������� ��������� �������:
 * 
 * 1�� ���� - 
 * [0] ���� (���������������)
 * [1] ������ (���������������)
 * [2] ������� (���������������)
 * [3] ����� (���������������)
 * [4] ����� (���������������)
 * [5] ��� (���������������)
 * [6] ��� = 0xFF
 * [7] ����������� �����
 * 
 * 2�� ���� - 
 * [0] ������
 * [1] ������
 * [2] ������
 * [3] ���-�� ����������� ������� (������� ����)
 * [4] ���-�� ����������� ������� (������� ����)
 * [5] ���-�� ������� (������� ����)
 * [6] ���-�� ������� (������� ����)
 * [7] ����������� ����� 
 */

/************************************************************************/
/* -> �����: ���� �������							*/
/************************************************************************/
void SCREEN_EVENTS_MENU (void) {
	uint8_t TempChar = 0;

	for (uint8_t ColIndex = 0; ColIndex <= 19; ++ ColIndex) {
		TempChar = eeprom_read_byte ((const uint8_t *) RegMenuText + ColIndex);
		if (TempChar >= 0xC0) {
			LCD_PutSym (0, ColIndex, RusFont[TempChar % 0xC0]);
		} else {
			LCD_PutSym (0, ColIndex, TempChar);
		}
	}
	
	for (uint8_t StrIndex = 1; StrIndex <= 3; ++ StrIndex) {
		for (uint8_t ColIndex = 0; ColIndex <= 19; ++ ColIndex) {
				
			TempChar = eeprom_read_byte ((const uint8_t *) RegMenuText + 20 * (StrIndex + RegMenuShift) + ColIndex);
			if (TempChar >= 0xC0) {
				LCD_PutSym (StrIndex, ColIndex, RusFont[TempChar % 0xC0]);
			} else {
				LCD_PutSym (StrIndex, ColIndex, TempChar);
			}
		}
	}

	if (RegMenuShift < (RegMenuMax - 4)) {
		LCD_PutSym (3, 19, 0x02);
	} else {
		LCD_PutSym (3, 19, 0x00);
	}
	if (RegMenuShift) {
		LCD_PutSym (1, 19, 0x03);
	} else {
		LCD_PutSym (1, 19, 0x00);
	}
	
	LCD_PutSym (2, 19, 0x00);
}

/************************************************************************/
/* -> �����: ���������� �������						*/
/************************************************************************/
void SCREEN_EVENTS_INFO (void) {
	// uint16_t TotMes = 0; // ���������� ������� � �������
	// uint16_t NewMes = 0; // ���������� ����� ������� � �������
	
	// ����� ������ �� EEPROM 1
	uint8_t ReadBuff1[8] = {0, 0, 0, 0, 0, 0, 0, 0};

	uint8_t TempChar = 0;

	for (uint8_t StrIndex = 0; StrIndex <= 3; ++ StrIndex) {
		for (uint8_t ColIndex = 0; ColIndex <= 19; ++ ColIndex) {
				
			TempChar = eeprom_read_byte ((const uint8_t *) RegInfoText + 20 * (StrIndex) + ColIndex);
			if (TempChar >= 0xC0) {
				LCD_PutSym (StrIndex, ColIndex, RusFont[TempChar % 0xC0]);
			} else {
				LCD_PutSym (StrIndex, ColIndex, TempChar);
			}
		}
	}
	
/************************************************************************/
/* >> ������ ���������													*/
/************************************************************************/
	// 2�� ����
	cli();
	EEPROM_AttemptCnt = EEPROM_RW_ATTEMPTS;
	while (EEPROM_AttemptCnt) {
		if (EEPROM_ReadBlock ((REG_HEAD_BEGIN + 1) * REG_BLOCK_SIZE, ReadBuff1, REG_BLOCK_SIZE)) break;
		-- EEPROM_AttemptCnt;
	}
	//E_TwiCmdStop ();
	//E_TwiDisable ();
	sei();
		
	if (!(Crc8 ((uint8_t *) &ReadBuff1, REG_BLOCK_SIZE))) {
		// CurMes = (ReadBuff1[2]<<8) | ReadBuff1[1];
		// NewMes = (ReadBuff1[4]<<8) | ReadBuff1[3];
		// TotMes = (ReadBuff1[6]<<8) | ReadBuff1[5];
		LCD_PutValDec (1, 5, 4, (ReadBuff1[6]<<8) | ReadBuff1[5]);
		LCD_PutValDec (1, 15, 4, (ReadBuff1[4]<<8) | ReadBuff1[3]);
	}

// 1�� ����
	cli();
	EEPROM_AttemptCnt = EEPROM_RW_ATTEMPTS;
	while (EEPROM_AttemptCnt) {
		if (EEPROM_ReadBlock ((REG_HEAD_BEGIN) * REG_BLOCK_SIZE, ReadBuff1, REG_BLOCK_SIZE)) break;
		-- EEPROM_AttemptCnt;
	}
	//E_TwiCmdStop ();
	//E_TwiDisable ();
	sei();
	
	if (!(Crc8 ((uint8_t *) &ReadBuff1, REG_BLOCK_SIZE))) {
		LCD_PutValDec (3, 3, 2, ReadBuff1[0]);
		LCD_PutValDec (3, 6, 2, ReadBuff1[1]);
		LCD_PutValDec (3, 9, 2, ReadBuff1[2]);
		LCD_PutValDec (3, 12, 2, ReadBuff1[3]);
		LCD_PutValDec (3, 15, 2, ReadBuff1[4]);
		LCD_PutValDec (3, 18, 2, ReadBuff1[5]);
	}
}

/************************************************************************/
/* -> �����: ������ �������									*/
/************************************************************************/
void SCREEN_EVENTS_LIST (void) {
	uint16_t TotMes = 0; // ���������� ������� � �������
	uint16_t CurMes = 0; // ������� ������
	uint16_t NewMes = 0; // ���������� ����� ������� � �������
	
	// ����� ������ �� EEPROM 1
	uint8_t ReadBuff1[8] = {0, 0, 0, 0, 0, 0, 0, 0};
	// ����� ������ �� EEPROM 2
	uint8_t ReadBuff2[8] = {0, 0, 0, 0, 0, 0, 0, 0};
	// ����� ������ � EEPROM 1
	uint8_t WriteBuff1[8] = {0, 0, 0, 0, 0, 0, 0, 0};
	// ����� ������ � EEPROM 2
	uint8_t WriteBuff2[8] = {0, 0, 0, 0, 0, 0, 0, 0};
		
	uint8_t NewFlag = 0;
/************************************************************************/
/* >> ������ ���������													*/
/************************************************************************/
	cli();
	EEPROM_AttemptCnt = EEPROM_RW_ATTEMPTS;
	while (EEPROM_AttemptCnt) {
		if (EEPROM_ReadBlock ((REG_HEAD_BEGIN + 1) * REG_BLOCK_SIZE, ReadBuff1, REG_BLOCK_SIZE)) break;
		-- EEPROM_AttemptCnt;
	}
	//E_TwiCmdStop ();
	//E_TwiDisable ();
	sei();
		
	if (!(Crc8 ((uint8_t *) &ReadBuff1, REG_BLOCK_SIZE))) {
		CurMes = (ReadBuff1[2]<<8) | ReadBuff1[1];
		NewMes = (ReadBuff1[4]<<8) | ReadBuff1[3];
		TotMes = (ReadBuff1[6]<<8) | ReadBuff1[5];
	}
	//LCD_PutValDec(0,0,2,TotMes);

	if (TotMes == 0) {
		CurMes = 0;
		NewMes = 0;
	} else {		
		if (CurMes == 0) {
			++ CurMes;
			// if (NewMes == TotMes) -- NewMes;
		}
	}
	
/************************************************************************/
/* >> ������ ������														*/
/************************************************************************/
	cli();	
	EEPROM_AttemptCnt = EEPROM_RW_ATTEMPTS;
	while (EEPROM_AttemptCnt) {
		if (EEPROM_ReadBlock ((REG_BODY_BEGIN + CurMes - 1) * REG_BLOCK_SIZE, ReadBuff2, REG_BLOCK_SIZE)) break;
		-- EEPROM_AttemptCnt;
	}
	//E_TwiCmdStop ();
	//E_TwiDisable ();
	sei();

/************************************************************************/
/* >> ����� ������� �������												*/
/************************************************************************/
	if (TotMes) {	
		if (!(Crc8 ((uint8_t *) &ReadBuff2, REG_BLOCK_SIZE))) {
			if (ReadBuff2[0] & 0b10000000) {
				NewFlag = 1;
				if (NewMes) -- NewMes;
			}
			
			if (ReadBuff2[6] <= /*0x13*/ (APD_EventIdAmount - 1)) {
				REG_Screen (3, TotMes, CurMes, NewMes, NewFlag, ReadBuff2);
			} else {
				REG_Screen (2, TotMes, CurMes, NewMes, NewFlag, ReadBuff2);
			}
		} else {
			REG_Screen (1, TotMes, CurMes, NewMes, NewFlag, ReadBuff2);
		}
	} else {
		REG_Screen (0, TotMes, CurMes, NewMes, NewFlag, ReadBuff2);
		}
/************************************************************************/
/* >> ���������� ������													*/
/************************************************************************/
	// ���� ����� ���� �������������� ���������
	if (ReadBuff2[0] & 0b10000000) {
		WriteBuff2[0] = ReadBuff2[0] & 0b01111111; // ������� ����
		WriteBuff2[1] = ReadBuff2[1];
		WriteBuff2[2] = ReadBuff2[2];
		WriteBuff2[3] = ReadBuff2[3];
		WriteBuff2[4] = ReadBuff2[4];
		WriteBuff2[5] = ReadBuff2[5];
		WriteBuff2[6] = ReadBuff2[6];
		WriteBuff2[7] = Crc8 (WriteBuff2, REG_BLOCK_SIZE - 1);

		WP_OFF;
		cli();
		EEPROM_AttemptCnt = EEPROM_RW_ATTEMPTS;
		while (EEPROM_AttemptCnt) {
			if (EEPROM_WriteBlock ((REG_BODY_BEGIN + CurMes - 1) * REG_BLOCK_SIZE, (uint8_t *) WriteBuff2, REG_BLOCK_SIZE)) {
				break;
			}
			-- EEPROM_AttemptCnt;
		}
		//E_TwiCmdStop ();
		//E_TwiDisable ();
		sei();
		WP_ON;
	}
/************************************************************************/
/* >> ��������� ��������												*/
/************************************************************************/
	switch (Action) 
	{
// (8) ������ - ����� (+ 1)
		case 8:
			if (TotMes) {
				if (CurMes < TotMes) {
					++ CurMes;
				}
			}
			Action = 255;
			LCD_Refresh = 1;
			break;

// (5) ����� - ����� (+ 10)
		case 5:
			if (TotMes) {
				if ((TotMes - CurMes) >= 10) {
					CurMes += 10;
				}
			}
			Action = 255;
			LCD_Refresh = 1;
			break;

// (7) ����� - ����� (- 1)
		case 7:
			if (TotMes) {
				if (CurMes > 1) {
					-- CurMes;
				}
			}
			Action = 255;
			LCD_Refresh = 1;
			break;
			
// (6) ���� - ����� (- 10)	
		case 6:
			if (TotMes) {
				if (CurMes >= 11) {
					CurMes -= 10;
				}
			}
			Action = 255;
			LCD_Refresh = 1;
			break;
			
// (0) BACK - ����� �� �������
		case 0:
			// ������� � ���� �������
			Mode = MODE_EVENTS_MENU;
			Action = 255;
			LCD_Refresh = 1;
			return 0;
			break;
		default:
			break;
	}
/************************************************************************/
/* >> ���������� ���������												*/
/************************************************************************/
	WriteBuff1[0] = ReadBuff1[0];
	WriteBuff1[1] = CurMes;
	WriteBuff1[2] = CurMes>>8;
	WriteBuff1[3] = NewMes;
	WriteBuff1[4] = NewMes>>8;
	WriteBuff1[5] = TotMes;
	WriteBuff1[6] = TotMes>>8;
	WriteBuff1[7] = Crc8 (WriteBuff1, REG_BLOCK_SIZE - 1);

	WP_OFF;
	cli();
	EEPROM_AttemptCnt = EEPROM_RW_ATTEMPTS;
	while (EEPROM_AttemptCnt) {
		if (EEPROM_WriteBlock ((REG_HEAD_BEGIN + 1) * REG_BLOCK_SIZE, (uint8_t *) WriteBuff1, REG_BLOCK_SIZE)) {
			break;
		}
		-- EEPROM_AttemptCnt;
	}
	//E_TwiCmdStop ();
	//E_TwiDisable ();
	sei();
	WP_ON;
}
/************************************************************************/
/* -> �����: ������� �������								*/
/************************************************************************/
void SCREEN_EVENTS_CLEAR (void /*uint8_t Action*/) 
{
	uint8_t TempChar = 0;
	for (uint8_t StrIndex = 0; StrIndex <= 3; ++ StrIndex) {
		for (uint8_t ColIndex = 0; ColIndex <= 19; ++ ColIndex) {
			TempChar = eeprom_read_byte ((const uint8_t *) RegResetText + 20 * (StrIndex) + ColIndex);
			if (TempChar >= 0xC0) {
				LCD_PutSym (StrIndex, ColIndex, RusFont[TempChar % 0xC0]);
			} else {
				LCD_PutSym (StrIndex, ColIndex, TempChar);
			}
		}
	}
	// �������� � ����������� �� ������� ������
	switch (Action) {
		
		// (1) YES
		case 1:
		// ������� ������� � ������� � ���� �������
		REG_Reset ();
		Mode = MODE_EVENTS_MENU;
		LCD_Refresh = 1;
		Action = 255;
//		REG_Reset ();
		break;
		
		// (0) NO
		case 0:
		// ������� � ���� ������� ��� ������� �������
		Mode = MODE_EVENTS_MENU;
		LCD_Refresh = 1;
		Action = 255;
		break;

		default:
		break;
	}
}
/************************************************************************/
/* -> ��������������� ������� �������        */
/************************************************************************/
void REG_Reset (void) {
	// ����� ������ � EEPROM 1
	uint8_t WriteBuff1[8] = {0, 0, 0, 0, 0, 0, 0, 0};
	
/* ��������� ��������� �������:
 * 
 * 1�� ���� - 
 * [0] ���� (���������������)
 * [1] ������ (���������������)
 * [2] ������� (���������������)
 * [3] ����� (���������������)
 * [4] ����� (���������������)
 * [5] ��� (���������������)
 * [6] ��� = 0xFF
 * [7] ����������� �����
 * 
 * 2�� ���� - 
 * [0] ������
 * [1] ������
 * [2] ������
 * [3] ���-�� ����������� ������� (������� ����)
 * [4] ���-�� ����������� ������� (������� ����)
 * [5] ���-�� ������� (������� ����)
 * [6] ���-�� ������� (������� ����)
 * [7] ����������� ����� 
 * 
/************************************************************************/
/* >> ���������� ������� �����											*/
/************************************************************************/
	WriteBuff1[0] = 10*((TIME_GetHours & 0x30)>>4) + (TIME_GetHours & 0x0F);
	WriteBuff1[1] = 10*((TIME_GetMinutes & 0x70)>>4) + (TIME_GetMinutes & 0x0F);
	WriteBuff1[2] = 10*((TIME_GetSeconds & 0x70)>>4) + (TIME_GetSeconds & 0x0F);
	WriteBuff1[3] = 10*((TIME_GetDate & 0x30)>>4) + (TIME_GetDate & 0x0F);
	WriteBuff1[4] = 10*((TIME_GetMonth & 0x10)>>4) + (TIME_GetMonth & 0x0F);
	WriteBuff1[5] = 10*((TIME_GetYear & 0xF0)>>4) + (TIME_GetYear & 0x0F);
	WriteBuff1[6] = 0xFF;
	WriteBuff1[7] = Crc8 (WriteBuff1, REG_BLOCK_SIZE - 1);
	
/************************************************************************/
/* >> ������ ������� �����												*/
/************************************************************************/
	WP_OFF;
	cli();
	EEPROM_AttemptCnt = EEPROM_RW_ATTEMPTS;
	while (EEPROM_AttemptCnt) {
		if (EEPROM_WriteBlock ((REG_HEAD_BEGIN) * REG_BLOCK_SIZE, (uint8_t *) WriteBuff1, REG_BLOCK_SIZE)) {
			break;
		}
		-- EEPROM_AttemptCnt;
	}
	//E_TwiCmdStop ();
	//E_TwiDisable ();
	sei();
	WP_ON;
/************************************************************************/
/* >> ���������� ������� �����											*/
/************************************************************************/
	WriteBuff1[0] = 0;
	WriteBuff1[1] = 0;
	WriteBuff1[2] = 0;
	WriteBuff1[3] = 0;
	WriteBuff1[4] = 0;
	WriteBuff1[5] = 0;
	WriteBuff1[6] = 0;
	WriteBuff1[7] = Crc8 (WriteBuff1, REG_BLOCK_SIZE - 1);
	
/************************************************************************/
/* >> ������ ������� �����												*/
/************************************************************************/
	WP_OFF;
	cli();
	EEPROM_AttemptCnt = EEPROM_RW_ATTEMPTS;
	while (EEPROM_AttemptCnt) {
		if (EEPROM_WriteBlock ((REG_HEAD_BEGIN + 1) * REG_BLOCK_SIZE, (uint8_t *) WriteBuff1, REG_BLOCK_SIZE)) {
			break;
		}
		-- EEPROM_AttemptCnt;
	}
	//E_TwiCmdStop ();
	//E_TwiDisable ();
	sei();
	WP_ON;
}

/************************************************************************/
/* -> ��������� ������� �������							*/
/************************************************************************/
void REG_Screen (uint8_t Mode, uint16_t TotMes, uint16_t CurMes, uint16_t NewMes, uint8_t NewFlag, uint8_t * Data) {
	uint8_t TempChar = 0;
	switch (Mode) {
/************************************************************************/
/* >> ������ ����														*/
/************************************************************************/
		case 0:
			// ����������
			for (uint8_t StrIndex = 0; StrIndex <= 3; ++ StrIndex) {
				for (uint8_t ColIndex = 0; ColIndex <= 19; ++ ColIndex) {
					TempChar = eeprom_read_byte ((const uint8_t *) RegEmptyText + 20 * (StrIndex) + ColIndex);
					if (TempChar >= 0xC0) {
						LCD_PutSym (StrIndex, ColIndex, RusFont[TempChar % 0xC0]);
					} else {
						LCD_PutSym (StrIndex, ColIndex, TempChar);
					}
				}
			}
			// ����������
			LCD_PutValDec (0, 5, 4, CurMes);
			LCD_PutValDec (0, 10, 4, NewMes);
			LCD_PutValDec (0, 16, 4, TotMes);
			break;
/************************************************************************/
/* >> ������ CRC ������													*/
/************************************************************************/
		case 1:
			// ����������
			for (uint8_t StrIndex = 0; StrIndex <= 3; ++ StrIndex) {
				for (uint8_t ColIndex = 0; ColIndex <= 19; ++ ColIndex) {
					TempChar = eeprom_read_byte ((const uint8_t *) RegErrorText + 20 * (StrIndex) + ColIndex);
					if (TempChar >= 0xC0) {
						LCD_PutSym (StrIndex, ColIndex, RusFont[TempChar % 0xC0]);
					} else {
						LCD_PutSym (StrIndex, ColIndex, TempChar);
					}
				}
			}
			// ����������
			LCD_PutValDec (0, 5, 4, CurMes);
			LCD_PutValDec (0, 10, 4, NewMes);
			LCD_PutValDec (0, 16, 4, TotMes);
			// ����� ���������
			if (NewFlag) {
				NewFlag = 0;
				// LCD_PutSym (0, 3, '*');
				LCD_PutStr (0, 0, (uint8_t *) "HOB!");
			}
			break;
/************************************************************************/
/* >> ��� ������� ������� �� ����� �����������							*/
/************************************************************************/ 
		case 2:
			// ����������
			for (uint8_t StrIndex = 0; StrIndex <= 3; ++ StrIndex) {
				for (uint8_t ColIndex = 0; ColIndex <= 19; ++ ColIndex) {
					TempChar = eeprom_read_byte ((const uint8_t *) RegOutText + 20 * (StrIndex) + ColIndex);
					if (TempChar >= 0xC0) {
						LCD_PutSym (StrIndex, ColIndex, RusFont[TempChar % 0xC0]);
					} else {
						LCD_PutSym (StrIndex, ColIndex, TempChar);
					}
				}
			}
			// �����
			LCD_PutValDec (1, 0, 2, *(Data + 0) & 0b01111111);
			LCD_PutValDec (1, 3, 2, *(Data + 1));
			LCD_PutValDec (1, 6, 2, *(Data + 2));
			// ����
			LCD_PutValDec (1, 12, 2, *(Data + 3));
			LCD_PutValDec (1, 15, 2, *(Data + 4) & 0x0F);
			LCD_PutValDec (1, 18, 2, *(Data + 5));
			// ����������
			LCD_PutValDec (0, 5, 4, CurMes);
			LCD_PutValDec (0, 10, 4, NewMes);
			LCD_PutValDec (0, 16, 4, TotMes);
			// ����� ���������������
			LCD_PutValDec (3, 3, 2, (*(Data + 4) & 0xF0)>>4 );
			// ����� ���������
			if (NewFlag) {
				NewFlag = 0;
				// LCD_PutSym (0, 3, '*');
				LCD_PutStr (0, 0, (uint8_t *) "HOB!");
			}
			break;

/************************************************************************/
/* >> ����������� ������� ������										*/
/************************************************************************/
		case 3:
			// ����������
			for (uint8_t StrIndex = 0; StrIndex <= 3; ++ StrIndex) {
				for (uint8_t ColIndex = 0; ColIndex <= 19; ++ ColIndex) {
					TempChar = eeprom_read_byte ((const uint8_t *) RegShowText + 20 * (StrIndex) + ColIndex);
					if (TempChar >= 0xC0) {
						LCD_PutSym (StrIndex, ColIndex, RusFont[TempChar % 0xC0]);
					} else {
						LCD_PutSym (StrIndex, ColIndex, TempChar);
					}
				}
			}
			// �����
			LCD_PutValDec (1, 0, 2, *(Data + 0) & 0b01111111);
			LCD_PutValDec (1, 3, 2, *(Data + 1));
			LCD_PutValDec (1, 6, 2, *(Data + 2));
			// ����
			LCD_PutValDec (1, 12, 2, *(Data + 3));
			LCD_PutValDec (1, 15, 2, *(Data + 4) & 0x0F);
			LCD_PutValDec (1, 18, 2, *(Data + 5));
			// ����������
			LCD_PutValDec (0, 5, 4, CurMes);
			LCD_PutValDec (0, 10, 4, NewMes);
			LCD_PutValDec (0, 16, 4, TotMes);
			// ����� ���������������
			LCD_PutValDec (3, 3, 2, (*(Data + 4) & 0xF0)>>4 );
			// �����
			for (uint8_t IdCol = 0; IdCol <= 19; IdCol++) {
				LCD_PutSym (2, IdCol, APD_PrjFontTable[pgm_read_byte ( APD_PrjData + APD_EventsStartAdr + 8 + *(Data + 6)*20 /*APD_EventStartAdr[*(Data + 6)]*/ + IdCol )]);
			}
			// ����� ���������
			if (NewFlag) {
				NewFlag = 0;
				// LCD_PutSym (0, 3, '*');
				LCD_PutStr (0, 0, (uint8_t *) "HOB!");
			}
			break;
		
		default:
			break;	
	}
}

/************************************************************************/
/* -> �����: ���������� �������						*/
/************************************************************************/
void SCREEN_EVENTS_SAVE_MENU (void) {
	uint8_t TempChar = 0;
	// ����� ������ �� EEPROM 1
	uint8_t ReadBuff1[8] = {0, 0, 0, 0, 0, 0, 0, 0};
	for (uint8_t StrIndex = 0; StrIndex <= 3; ++ StrIndex) {
		for (uint8_t ColIndex = 0; ColIndex <= 19; ++ ColIndex) {
			TempChar = eeprom_read_byte ((const uint8_t *) RegSaveText + 20 * (StrIndex) + ColIndex);
			if (TempChar >= 0xC0) {
				LCD_PutSym (StrIndex, ColIndex, RusFont[TempChar % 0xC0]);
			} else {
				LCD_PutSym (StrIndex, ColIndex, TempChar);
			}
		}
	}
	/************************************************************************/
	/* ������ ��������� *****************************************************/
	/************************************************************************/
	// 2�� ����
	cli();
	EEPROM_AttemptCnt = EEPROM_RW_ATTEMPTS;
	while (EEPROM_AttemptCnt) {
		if (EEPROM_ReadBlock ((REG_HEAD_BEGIN + 1) * REG_BLOCK_SIZE, ReadBuff1, REG_BLOCK_SIZE)) break;
		-- EEPROM_AttemptCnt;
	}
	//E_TwiCmdStop ();
	//E_TwiDisable ();
	sei();

	if (!(Crc8 ((uint8_t *) &ReadBuff1, REG_BLOCK_SIZE))) {
	
		REGSAVE_MesLim = (ReadBuff1[6]<<8) | ReadBuff1[5];
	
		LCD_PutValDec (1, 5, 4, /*(ReadBuff1[6]<<8) | ReadBuff1[5]*/ REGSAVE_MesLim);
		LCD_PutValDec (1, 15, 4, (ReadBuff1[4]<<8) | ReadBuff1[3]);
	}
	switch (Action) {
		// (0) NO
		case 0:
			// ������� ���� �������
			Mode = MODE_EVENTS_MENU;
			LCD_Refresh = 1;
			Action = 255;
			break;
	
		// (1) YES
		case 1:
			// ��������� ��� ������ ������� ������ � ��������� ������ ������ �
			// ������ �������
			TIMER_DataRequestEn = 0;
			TIMER_DataRequest = 0;
			TIMER_RequestTimeoutEn = 0;
			TIMER_RequestTimeout = 0;
			DATA_ReqEn = 0;
			EVENT_ReqEn = 0;

			// �������� ������ �������� �������
			TIMER_RegSaveEn = 1;
			TIMER_RegSave = 0;
	
			REGSAVE_MesCnt = 1;
			NET_Mode = NET_MODE_REGSAVE;
			Mode = MODE_EVENTS_SAVE_PROGRESS;
			SHOWONCE_RegSaveProgress = 1;
			LCD_Refresh = 1;
	
			Action = 255;
			break;
		default:
			break;
	}
}
/************************************************************************/
/* -> �����: �������� ���������� �������		*/
/************************************************************************/
void SCREEN_EVENTS_SAVE_PROGRESS (void) {
	uint8_t TempChar = 0;
	uint16_t ProgressBarScale = REGSAVE_MesLim / 18;
	if (SHOWONCE_RegSaveProgress) {
		SHOWONCE_RegSaveProgress = 0;
		for (uint8_t StrIndex = 0; StrIndex <= 3; ++ StrIndex) {
			for (uint8_t ColIndex = 0; ColIndex <= 19; ++ ColIndex) {
				TempChar = eeprom_read_byte ((const uint8_t *) RegSaveProgressText + 20 * (StrIndex) + ColIndex);
				if (TempChar >= 0xC0) {
					LCD_PutSym (StrIndex, ColIndex, RusFont[TempChar % 0xC0]);
				} else {
					LCD_PutSym (StrIndex, ColIndex, TempChar);
				}
			}
		}
		LCD_PutSym (1, 0, ARROW_LEFT);
		LCD_PutSym (1, 19, ARROW_RIGHT);
	}	
	LCD_PutValDec (2, 5, 4, REGSAVE_MesLim);
	LCD_PutValDec (2, 16, 4, REGSAVE_MesCnt % REGSAVE_MesLim);
	LCD_PutSym (1, 1 + REGSAVE_MesCnt / ProgressBarScale, 0xFF);
}

/************************************************************************/
/* -> �����: ���������� ������� ���������		*/
/************************************************************************/
void SCREEN_EVENTS_SAVE_COMPLETED (void) {
	uint8_t TempChar = 0;
	for (uint8_t StrIndex = 0; StrIndex <= 3; ++ StrIndex) {
		for (uint8_t ColIndex = 0; ColIndex <= 19; ++ ColIndex) {
			TempChar = eeprom_read_byte ((const uint8_t *) RegSaveCompletedText + 20 * (StrIndex) + ColIndex);
			if (TempChar >= 0xC0) {
				LCD_PutSym (StrIndex, ColIndex, RusFont[TempChar % 0xC0]);
			} else {
				LCD_PutSym (StrIndex, ColIndex, TempChar);
			}
		}
	}
}

/************************************************************************/
/* -> �����: ��������� �������								*/
/************************************************************************/
void SCREEN_EVENTS_SETTINGS (void) {
	static uint8_t REG_SET_TempTOP = 0;
	static uint8_t REG_SET_TempBOT = 0;
	uint8_t TempChar = 0;
	
	if (REG_SET_ShowOnce) {
		REG_SET_ShowOnce = 0;
		
		for (uint8_t StrIndex = 0; StrIndex <= 3; ++ StrIndex) {
			for (uint8_t ColIndex = 0; ColIndex <= 19; ++ ColIndex) {
				
				TempChar = eeprom_read_byte ((const uint8_t *) RegSetText + 20 * (StrIndex) + ColIndex);
				if (TempChar >= 0xC0) {
					LCD_PutSym (StrIndex, ColIndex, RusFont[TempChar % 0xC0]);
				} else {
					LCD_PutSym (StrIndex, ColIndex, TempChar);
				}
			}
		}
		LCD_PutSym (1, 16, ARROW_LEFT);
		LCD_PutSym (1, 19, ARROW_RIGHT);
		LCD_PutSym (2, 16, ARROW_LEFT);
		LCD_PutSym (2, 19, ARROW_RIGHT);
		
		REG_SET_TOP = (uint8_t) EventStorage_TOP;
		REG_SET_TempTOP = REG_SET_TOP;
		REG_SET_BOT = (- 1) * (uint8_t) EventStorage_BOT;
		REG_SET_TempBOT = REG_SET_BOT;
		
		LCD_PutValDecPointMaskNeg_ (1, 17, 2, 0, 1, REG_SET_TOP);
		LCD_PutValDecPointMaskNeg_ (2, 17, 2, 0, 1, REG_SET_BOT);
		// ������
		LCD_PutSym (1 + REG_SET_SetMarker, 0, ARROW_RIGHT);
	}
	// �������� � ����������� �� ������� ������
	switch (Action) {
		// (7) ����� - ���������
		case 7:
		switch (REG_SET_SetMarker) {
			case 0:
			if (REG_SET_TOP > 1) {
				-- REG_SET_TOP;
			}
			break;
			case 1:
			if (REG_SET_BOT > 1) {
				-- REG_SET_BOT;
			}
			break;
		}
		Action = 255;
		break;
		
		// (8) ������ - ���������
		case 8:
		switch (REG_SET_SetMarker) {
			case 0:
			if (REG_SET_TOP < 10) {
				++ REG_SET_TOP;
			}
			break;
			case 1:
			if (REG_SET_BOT <10) {
				++ REG_SET_BOT;
			}
			break;
		}
		Action = 255;
		break;

		// (5) ����� - ������ �����
		case 5:
		if (REG_SET_SetMarker == 0) {
			REG_SET_SetMarker = 1;
		} else {
			REG_SET_SetMarker = 0;
		}
		Action = 255;
		break;

		// (6) ���� - ������ ����
		case 6:
		if (REG_SET_SetMarker == 0) {
			REG_SET_SetMarker = 1;
		} else {
			REG_SET_SetMarker = 0;
		}
		Action = 255;
		break;
		
		// (1) YES
		case 1:
		cli();
		// ���������� ���������
		if (REG_SET_TempTOP != REG_SET_TOP) {
			EventStorage_TOP = (int8_t) REG_SET_TOP;
			eeprom_write_byte ((uint8_t *) &EventStorage_TOP_Save, (uint8_t) REG_SET_TOP);
		}
		if (REG_SET_TempBOT != REG_SET_BOT) {
			EventStorage_BOT = (- 1) * (int8_t) REG_SET_BOT;
			eeprom_write_byte ((uint8_t *) &EventStorage_BOT_Save, (uint8_t) REG_SET_BOT);
		}
		sei();
		
		REG_SET_SetMarker = 0;
		
		// ������� � ��������� �����
		Mode = MODE_EVENTS_MENU;
		LCD_Refresh = 1;
		Action = 255;
		REG_SET_ShowOnce = 1;
		break;

		// (0) - NO
		case 0:
		REG_SET_SetMarker = 0;
		// ������� � ��������� �����
		Mode = MODE_EVENTS_MENU;
		LCD_Refresh = 1;
		Action = 255;
		REG_SET_ShowOnce = 1;
		break;
		
		// (9) DEFAULT - ������� ���������
		case 9:
		REG_SET_TOP = DEFAULT_STORAGE_TOP;
		REG_SET_BOT = DEFAULT_STORAGE_BOT;
		Action = 255;
		break;
		default:
		break;
	}
	
	LCD_PutValDecPointMaskNeg_ (1, 17, 2, 0, 1, REG_SET_TOP);
	LCD_PutValDecPointMaskNeg_ (2, 17, 2, 0, 1, REG_SET_BOT);
	// ������
	if (REG_SET_SetMarker == 0) {
		LCD_PutSym (2, 0, 0x20);
	} else {
		LCD_PutSym (1, 0, 0x20);
	}
	LCD_PutSym (1 + REG_SET_SetMarker, 0, ARROW_RIGHT);
}

/************************************************************************/
/* -> ���������� �����: �����							*/
/************************************************************************/
void SCREEN_TRANSP_SELECT (void) {
	uint8_t TempChar = 0;
	for (uint8_t StrIndex = 0; StrIndex <= 3; ++ StrIndex) {
		for (uint8_t ColIndex = 0; ColIndex <= 19; ++ ColIndex) {
			TempChar = eeprom_read_byte ((const uint8_t *) TranspSelectText + 20 * (StrIndex) + ColIndex);
			if (TempChar >= 0xC0) {
				LCD_PutSym (StrIndex, ColIndex, RusFont[TempChar % 0xC0]);
			} else {
				LCD_PutSym (StrIndex, ColIndex, TempChar);
			}
		}
	}

	// �������� � ����������� �� ������� ������
	switch (Action) {
		// (0) BACK
		case 0:
			// ������� � ��������� �����
			Mode = MODE_MENU;
			TIMER_ClockRefreshEn = 1;
			ClockShowEn = 1;
			LCD_Refresh = 1;
			Action = 255;
			break;
			
		// (1) 9600	
		case 1:
// 			// ��������� ��� ������ ������� ������ � ��������� ������ ������ � 
// 			// ������ �������
	
			NewBaudRate = 96;
			Mode = MODE_TRANSP_CONFIRM;
			LCD_Refresh = 1;
			Action = 255;
			break;
			
		// (2) 57600
		case 2:
			NewBaudRate = 57;
			Mode = MODE_TRANSP_CONFIRM;
			LCD_Refresh = 1;
			Action = 255;
			break;

		// (3) 115200
		case 3:
			NewBaudRate = 11;
			Mode = MODE_TRANSP_CONFIRM;
			LCD_Refresh = 1;
			Action = 255;
			break;
			
		default:
			break;
	}
}

/************************************************************************/
/* -> ���������� �����: �������������				*/
/************************************************************************/
void SCREEN_TRANSP_CONFIRM (void) {
	uint8_t TempChar = 0;
	for (uint8_t StrIndex = 0; StrIndex <= 3; ++ StrIndex) {
		for (uint8_t ColIndex = 0; ColIndex <= 19; ++ ColIndex) {
			TempChar = eeprom_read_byte ((const uint8_t *) TranspConfirmText + 20 * (StrIndex) + ColIndex);
			if (TempChar >= 0xC0) {
				LCD_PutSym (StrIndex, ColIndex, RusFont[TempChar % 0xC0]);
			} else {
				LCD_PutSym (StrIndex, ColIndex, TempChar);
			}
		}
	}	
	
	// �������� � ����������� �� ������� ������
	switch (Action) {
		// (0) NO
		case 0:
			// ������� � ��������� �����
			Mode = MODE_MENU;
			TIMER_ClockRefreshEn = 1;
			ClockShowEn = 1;
			LCD_Refresh = 1;
			Action = 255;
			break;
			
		// (1) YES	
		case 1:
 			// ��������� ��� ������ ������� ������ � ��������� ������ ������ � 
			// ������ �������
 			TIMER_DataRequestEn = 0;
 			TIMER_DataRequest = 0;	
 			TIMER_RequestTimeoutEn = 0;
 			TIMER_RequestTimeout = 0;
 			DATA_ReqEn = 0;
 			EVENT_ReqEn = 0;
  			RMD_Config_STATE = START;
			Mode = MODE_TRANSP_PROGRESS;
			LCD_Refresh = 1;
			TIMER_RMDCfgPer = 300;
			TIMER_RMDCfg = 0;
 			TIMER_RMDCfgEn = 1;
			Action = 255;
			break;

		default:
			break;
	}
}

/************************************************************************/
/* -> ���������� �����: ��������							*/
/************************************************************************/
void SCREEN_TRANSP_PROGRESS (void) {
	uint8_t TempChar = 0;
	for (uint8_t StrIndex = 0; StrIndex <= 3; ++ StrIndex) {
		for (uint8_t ColIndex = 0; ColIndex <= 19; ++ ColIndex) {
			TempChar = eeprom_read_byte ((const uint8_t *) TranspProgressText + 20 * (StrIndex) + ColIndex);
			if (TempChar >= 0xC0) {
				LCD_PutSym (StrIndex, ColIndex, RusFont[TempChar % 0xC0]);
			} else {
				LCD_PutSym (StrIndex, ColIndex, TempChar);
			}
		}
	}
}	

/************************************************************************/
/* -> ���������� �����: ��������							*/
/************************************************************************/
void SCREEN_TRANSP_SCREEN (void) {
	uint8_t TempChar = 0;
	for (uint8_t StrIndex = 0; StrIndex <= 3; ++ StrIndex) {
		for (uint8_t ColIndex = 0; ColIndex <= 19; ++ ColIndex) {
			TempChar = eeprom_read_byte ((const uint8_t *) TranspActiveText + 20 * (StrIndex) + ColIndex);
			if (TempChar >= 0xC0) {
				LCD_PutSym (StrIndex, ColIndex, RusFont[TempChar % 0xC0]);
			} else {
				LCD_PutSym (StrIndex, ColIndex, TempChar);
			}
		}
	}
}

/************************************************************************/
/* -> �����: ����														*/
/************************************************************************/
void SCREEN_INFO (void) {
	uint8_t TempChar = 0;
	for (uint8_t StrIndex = 0; StrIndex <= 3; ++ StrIndex) {
		for (uint8_t ColIndex = 0; ColIndex <= 19; ++ ColIndex) {
			TempChar = eeprom_read_byte ((const uint8_t *) InfoText + 20 * (StrIndex) + ColIndex);
			if (TempChar >= 0xC0) {
				LCD_PutSym (StrIndex, ColIndex, RusFont[TempChar % 0xC0]);
			} else {
				LCD_PutSym (StrIndex, ColIndex, TempChar);
			}
		}
	}
	LCD_PutValDecPointMaskNeg_ (2, 13, 3, 1, 1, SoftVer);


//	LCD_PutStr (0, 0, (uint8_t *)"> DEVELOPING");
/*
LCD_PutValDec (0, 16, 3, APD_FrameDataFromPlcAmount[0]);

LCD_PutValDec (1, 0, 3, APD_FrameDataFromPlcAdr[0][0]);
LCD_PutValDec (1, 4, 3, APD_FrameDataFromPLCD_ata[0][0]);
LCD_PutValDec (1, 8, 1, APD_FrameDataFromPlcStatus[0][0]);

LCD_PutValDec (2, 0, 3, APD_FrameDataFromPlcAdr[0][1]);
LCD_PutValDec (2, 4, 3, APD_FrameDataFromPLCD_ata[0][1]);
LCD_PutValDec (2, 8, 1, APD_FrameDataFromPlcStatus[0][1]);

LCD_PutValDec (3, 0, 3, APD_FrameDataFromPlcAdr[0][2]);
LCD_PutValDec (3, 4, 3, APD_FrameDataFromPLCD_ata[0][2]);
LCD_PutValDec (3, 8, 1, APD_FrameDataFromPlcStatus[0][2]);

LCD_PutValDec (1, 10, 3, APD_FrameDataFromPlcAdr[0][3]);
LCD_PutValDec (1, 14, 3, APD_FrameDataFromPLCD_ata[0][3]);
LCD_PutValDec (1, 18, 1, APD_FrameDataFromPlcStatus[0][3]);

LCD_PutValDec (2, 10, 3, APD_FrameDataFromPlcAdr[0][4]);
LCD_PutValDec (2, 14, 3, APD_FrameDataFromPLCD_ata[0][4]);
LCD_PutValDec (2, 18, 1, APD_FrameDataFromPlcStatus[0][4]);

LCD_PutValDec (3, 10, 3, APD_FrameDataFromPlcAdr[0][5]);
LCD_PutValDec (3, 14, 3, APD_FrameDataFromPLCD_ata[0][5]);
LCD_PutValDec (3, 18, 1, APD_FrameDataFromPlcStatus[0][5]);
*/

/*
LCD_PutValDec (1, 0, 3, APD_EventsStartAdr);
LCD_PutValDec (1, 4, 3, APD_EventIdAmount);
LCD_PutValDec (1, 8, 6, APD_EventReqRatio);

LCD_PutValDec (2, 0, 3, APD_EventAdr);

LCD_PutValDec (3, 0, 3, APD_EventStartAdr[0]);
LCD_PutValDec (3, 4, 3, APD_EventStartAdr[1]);
LCD_PutValDec (3, 8, 3, APD_EventStartAdr[2]);
*/

/*
for (uint8_t IdCnt = 0; IdCnt <= (APD_EventIdAmount - 1); IdCnt++) {
	for (uint8_t IdCol = 0; IdCol <= 19; IdCol++) {
		LCD_PutSym (IdCnt + 1, IdCol, APD_PrjFontTable[APD_PrjData[APD_EventStartAdr[IdCnt] + IdCol]]);
	}
}
*/
/*	
	FOCUS_Sort (C, sizeof (C));
	LCD_PutValDec (1, 0, 1, C[0]);
	LCD_PutValDec (1, 2, 1, C[1]);
	LCD_PutValDec (1, 4, 1, C[2]);
	LCD_PutValDec (1, 6, 1, C[3]);
	LCD_PutValDec (1, 8, 1, C[4]);
*/

//	uint8_t IdA = 0;
//	uint8_t IdB = 0;
	
//	LCD_PutValDec (0, 19, 1, APD_FrameCur);
//	LCD_PutValDec (3, 0, 3, FOCUS_DataAmount);
//	LCD_PutValDec (3, 4, 3, BLOCK_Amount);	

	
//	FOCUS_Adr ();
	/*
	for (IdA = 0; IdA <= (FOCUS_DataAmount - 1); IdA++) {
		LCD_PutValDec (0, 4*IdA, 3, FOCUS_DataAdr[IdA]);
	}
	*/
//	FOCUS_Sort (FOCUS_DataAdr, FOCUS_DataAmount);
	/*
	for (IdA = 0; IdA <= (FOCUS_DataAmount - 1); IdA++) {
		LCD_PutValDec (1, 4*IdA, 3, FOCUS_DataAdr[IdA]);
	}
	*/
//	FOCUS_Space ();
	/*
	for (IdA = 0; IdA <= (FOCUS_DataAmount - 1); IdA++) {
		LCD_PutValDec (3, 4*IdA, 3, FOCUS_SpaceArray[IdA]);
	}	
	*/
	/*
	FOCUS_Block ();
	for (IdA = 0; IdA <= (BLOCK_Amount - 1); IdA++) {
		LCD_PutValDec (2, 4*IdA, 3, BLOCK_Request[IdA][0]);
		LCD_PutValDec (3, 4*IdA, 3, BLOCK_Request[IdA][1]);
	}
	*/
/*
	LCD_PutValDec (1, 0, 3, (ADP_RecMesBuf[9]<<8) | ADP_RecMesBuf[8]);
	
	for (IdA = 0; IdA <= (FOCUS_DataAmount - 1); IdA++) {
		LCD_PutValDec (0, 4*IdA, 3, FOCUS_DataAdr[IdA]);
	}
*/

/************************************************************************/
/*                   �������� ���������                                 */
/************************************************************************/

// 	if (ADP_RecMesLen) {
// 		for (uint8_t IdA_ = 0; IdA_ <= (ADP_RecMesLen - 1); ++ IdA_) {
// 			LCD_PutSym (IdA_/7, 3*(IdA_%7), DecToHex[ (ADP_RecMesBuf[IdA_]/16) ]);
// 			LCD_PutSym (IdA_/7, 3*(IdA_%7) + 1, DecToHex[ (ADP_RecMesBuf[IdA_]%16) ]);
// 		}
// 	}

// LCD_PutValDec (0, 0, 3, APD_EventIdAmount);

/*
	uint16_t Adress = 0;

	Adress = (ADP_RecMesBuf[9]<<8) | ADP_RecMesBuf[8];

	if (ADP_RecMesLen) {
		if (Adress >= 138) {
			for (IdA = 0; IdA <= (ADP_RecMesLen - 1); IdA++) {
				LCD_PutSym (IdA/7, 3*(IdA%7), DecToHex[ (ADP_RecMesBuf[IdA]/16) ]);
				LCD_PutSym (IdA/7, 3*(IdA%7) + 1, DecToHex[ (ADP_RecMesBuf[IdA]%16) ]);
			}
		}
	}
*/

/*	
	for (IdA = 0; IdA <= (FOCUS_DataAmount - 1 - 1); ++ IdA) {
		LCD_PutValDec (0, 5*IdA, 4, FOCUS_DataAdr[IdA]);
		LCD_PutValDec (1, 5*IdA, 4, FOCUS_Data[IdA]);
		LCD_PutValDec (2, 5*IdA, 4, FOCUS_DataStatus[IdA]);
	}
*/


//	IdA = 6;
	
//	LCD_PutValDec (1, 0, 4, FOCUS_DataAdr[IdA]);
//	LCD_PutValDec (2, 0, 4, FOCUS_Data[IdA]);
//	LCD_PutValDec (3, 0, 4, FOCUS_DataStatus[IdA]);

/************************************************************************/
/*                                                                      */
/************************************************************************/

//	LCD_PutValDec (1, 0, 3, TotMes);
//	WP_ON;
/*
	uint8_t C[3] = {0, 0, 0};
	WP_OFF;
	EEPROM_WriteBlock (0, C, sizeof (C));
	WP_ON;
	
	WriteBuff1[0] = 0;
	WriteBuff1[1] = 0;
	WriteBuff1[2] = 0;
	WriteBuff1[3] = 0;
	WriteBuff1[4] = 0;
	WriteBuff1[5] = 0;
	WriteBuff1[6] = 0;
	WriteBuff1[7] = Crc8 (WriteBuff1, 7);
	WP_OFF;
	EEPROM_WriteBlock (0, WriteBuff1, sizeof (WriteBuff1));
	WP_ON;

	EEPROM_ReadBlock (0, ReadBuff1, sizeof (ReadBuff1));
	
	//TotMes = (ReadBuff1[5]<<8) | ReadBuff1[6];
	if (!(Crc8 ((uint8_t *) &ReadBuff1, sizeof (ReadBuff1)))) {
		TotMes = (ReadBuff1[5]<<8) | ReadBuff1[6];	
	} else {

		WriteBuff1[0] = 0;
		WriteBuff1[1] = 0;
		WriteBuff1[2] = 0;
		WriteBuff1[3] = 0;
		WriteBuff1[4] = 0;
		WriteBuff1[5] = 0;
		WriteBuff1[6] = 0;
		WriteBuff1[7] = Crc8 (WriteBuff1, 7);
		WP_OFF;
		EEPROM_WriteBlock (0, WriteBuff1, sizeof (WriteBuff1));
		WP_ON;
//		TotMes = 666;
	}

*/

//	LCD_PutValDec (2, 16, 3, TotMes);

	
//	A[0] = Crc8 (A, 3);
//	WP_OFF;
/*	
	if (EEPROM_WriteBlock (0, A, sizeof (A))) {
		if (EEPROM_ReadBlock (0, B, sizeof (B))) {
			
			EEPROM_ReadBlock (0, Buff1, sizeof (Buff1));
			LCD_PutValDec (0, 0, 3, Buff1[0]);
			LCD_PutValDec (0, 4, 3, Buff1[1]);
			LCD_PutValDec (0, 8, 3, Buff1[2]);
			LCD_PutValDec (0, 12, 3, Buff1[3]);
			LCD_PutValDec (0, 16, 3, Buff1[4]);
			LCD_PutValDec (1, 0, 3, Buff1[5]);
			LCD_PutValDec (1, 4, 3, Buff1[6]);
			LCD_PutValDec (1, 8, 3, Buff1[7]);

			EEPROM_ReadBlock (8, Buff2, sizeof (Buff2));
			LCD_PutValDec (2, 0, 3, Buff2[0]);
			LCD_PutValDec (2, 4, 3, Buff2[1]);
			LCD_PutValDec (2, 8, 3, Buff2[2]);
			LCD_PutValDec (2, 12, 3, Buff2[3]);
			LCD_PutValDec (2, 16, 3, Buff2[4]);
			LCD_PutValDec (3, 0, 3, Buff2[5]);
			LCD_PutValDec (3, 4, 3, Buff2[6]);
			LCD_PutValDec (3, 8, 3, Buff2[7]);			
			
			LCD_PutValDec (1, 16, 3, B[0]);
			LCD_PutValDec (2, 16, 3, B[1]);
			LCD_PutValDec (3, 16, 3, B[2]);
		}
	}
*/

/*

uint8_t Buff1[8] = {0, 1, 2, 3, 4, 5, 6, 7};
uint8_t Buff2[8] = {7, 6, 5, 4, 3, 2, 1, 0};

// ���������� EEPROM
	cli();
	WP_OFF;
	EEPROM_AttemptCnt = EEPROM_RW_ATTEMPTS;
	while (EEPROM_AttemptCnt) {
		if (EEPROM_WriteBlock (REG_HEAD_BEGIN, Buff2, 8)) break;
		-- EEPROM_AttemptCnt;
	}
	WP_ON;
	sei();
	
	cli();
	EEPROM_AttemptCnt = EEPROM_RW_ATTEMPTS;
	while (EEPROM_AttemptCnt) {
		if (EEPROM_ReadBlock (REG_HEAD_BEGIN, Buff1, 8)) break;
		-- EEPROM_AttemptCnt;
	}
	sei();

	LCD_PutValDec (0, 0, 3, Buff1[0]);
	LCD_PutValDec (0, 4, 3, Buff1[1]);
	LCD_PutValDec (0, 8, 3, Buff1[2]);
	LCD_PutValDec (0, 12, 3, Buff1[3]);
	LCD_PutValDec (0, 16, 3, Buff1[4]);
	LCD_PutValDec (1, 0, 3, Buff1[5]);
	LCD_PutValDec (1, 4, 3, Buff1[6]);
	LCD_PutValDec (1, 8, 3, Buff1[7]);

	cli();
	EEPROM_AttemptCnt = EEPROM_RW_ATTEMPTS;
	while (EEPROM_AttemptCnt) {
		if (EEPROM_ReadBlock (REG_HEAD_BEGIN + 8, Buff2, 8)) break;
		-- EEPROM_AttemptCnt;
	}		
	sei();
				
	LCD_PutValDec (2, 0, 3, Buff2[0]);
	LCD_PutValDec (2, 4, 3, Buff2[1]);
	LCD_PutValDec (2, 8, 3, Buff2[2]);
	LCD_PutValDec (2, 12, 3, Buff2[3]);
	LCD_PutValDec (2, 16, 3, Buff2[4]);
	LCD_PutValDec (3, 0, 3, Buff2[5]);
	LCD_PutValDec (3, 4, 3, Buff2[6]);
	LCD_PutValDec (3, 8, 3, Buff2[7]);
*/
/*
	for (uint8_t Index_ = 0; Index_ <= APD_FrameAmount; ++ Index_) {
		LCD_PutValDec (0, 5*Index_, 4, APD_List[Index_][0]);
		LCD_PutValDec (1, 5*Index_, 4, APD_List[Index_][1]);
		LCD_PutValDec (2, 5*Index_, 4, APD_List[Index_][2]);
	}
*/
/*
	LCD_PutValDec (0, 0, 3, APD_EventAdrArray[0]);
	LCD_PutValDec (0, 4, 3, APD_EventAdrArray[1]);
	LCD_PutValDec (0, 8, 3, APD_EventAdrArray[2]);
	LCD_PutValDec (0, 12, 3, APD_EventAdrArray[3]);
	LCD_PutValDec (2, 0, 3, APD_EventAdrNum);
	LCD_PutValDec (3, 0, 4, APD_EventAdr);
*/
/*
	LCD_PutValDec (0, 0, 3, EventStorage[0][0x07][0]);
	LCD_PutValDec (0, 4, 3, EventStorage[0][0x2B][0]);
	LCD_PutValDec (0, 8, 3, EventStorage[0][0x13][0]);
	LCD_PutValDec (0, 12, 3, EventStorage[0][0x08][0]);
	
	LCD_PutValDec (1, 0, 3, EventStorage[1][0x07][0]);
	LCD_PutValDec (1, 4, 3, EventStorage[1][0x2B][0]);
	LCD_PutValDec (1, 8, 3, EventStorage[1][0x13][0]);
	LCD_PutValDec (1, 12, 3, EventStorage[1][0x08][0]);
	
	LCD_PutValDec (2, 0, 3, EventStorage[2][0x07][0]);
	LCD_PutValDec (2, 4, 3, EventStorage[2][0x2B][0]);
	LCD_PutValDec (2, 8, 3, EventStorage[2][0x13][0]);
	LCD_PutValDec (2, 12, 3, EventStorage[2][0x08][0]);
	
	LCD_PutValDec (3, 0, 3, EventStorage[3][0x05][0]);
	LCD_PutValDec (3, 4, 3, EventStorage[3][0x06][0]);
	LCD_PutValDec (3, 8, 3, EventStorage[3][0x07][0]);
	LCD_PutValDec (3, 12, 3, EventStorage[3][0x08][0]);
*/	
// 	LCD_PutValDec (0, 0, 1, EventStorage[3][0x07][0]);
// 	LCD_PutValDec (0, 2, 1, EventStorage[3][0x08][0]);
// 	LCD_PutValDec (0, 4, 1, EventStorage[3][0x13][0]);
// 	LCD_PutValDec (0, 6, 1, EventStorage[3][0x2B][0]);
// 	
// 	LCD_PutValDec (1, 0, 1, EventStorage[3][0x07][2]);
// 	LCD_PutValDec (1, 2, 1, EventStorage[3][0x08][2]);
// 	LCD_PutValDec (1, 4, 1, EventStorage[3][0x13][2]);
// 	LCD_PutValDec (1, 6, 1, EventStorage[3][0x2B][2]);

// 	LCD_PutValHex (0, 0, 2, RMD_RecMesBuf[0]);
// 	LCD_PutValHex (0, 3, 2, RMD_RecMesBuf[1]);
// 	LCD_PutValHex (0, 6, 2, RMD_RecMesBuf[2]);
// 	LCD_PutValHex (0, 9, 2, RMD_RecMesBuf[3]);

// LCD_PutValDecPointMaskNeg_ (1, 0, 4, 2, 1, 321);
}

/************************************************************************/
/*				�������� �������										*/
/************************************************************************/
/************************************************************************/
/*			��������������� ������� ���������� ������ � �������*/
/************************************************************************/
void FOCUS_CntReset (void) {
	for (uint8_t IdA = 0; IdA <= (FOCUS_DataAmount - 1); IdA++) {
	
		FOCUS_DataCntNext[IdA] = 0;
		FOCUS_DataCntPrev[IdA] = 0;
		FOCUS_DataStatus[IdA] = 0;
		
		FOCUS_DataCntNewNext[IdA] = BROKEN_MES;
		FOCUS_DataCntNewPrev[IdA] = 0;
		FOCUS_DataCntNewStatus[IdA] = 1;
	}
}

// ������� ���������� ������� ��� ������� � ������� ������
void FOCUS_Adr (void) {
	uint8_t IdA = 0;
	uint8_t Cnt = 0;
	
	for (IdA = 0; IdA <= (APD_MAX_FIELDS - 1); IdA++) {
		FOCUS_DataAdr[IdA] = 0;
		FOCUS_Data[IdA] = 0;
		FOCUS_DataCntNewStatus[IdA] = 1;
	}

	FOCUS_DataAmount = 0;

	if (APD_FrameDataFromPlcAmount[APD_FrameCur]) {
		for (IdA = 0; IdA <= (APD_FrameDataFromPlcAmount[APD_FrameCur] - 1); IdA++) {
			if ((APD_FrameDataFromPlcStr[APD_FrameCur][IdA] >= APD_FrameDisplayShift) && (APD_FrameDataFromPlcStr[APD_FrameCur][IdA] <= (APD_FrameDisplayShift + 3))) {
				FOCUS_DataAdr[Cnt] = APD_FrameDataFromPlcAdr[APD_FrameCur][IdA];
				++ Cnt;
			}
		}
		FOCUS_DataAmount = Cnt;
	} else {
		FOCUS_DataAmount = 0;
	}
}

// ���������� ������� �� �����������
void FOCUS_Sort (uint16_t * Array, uint8_t Size) {
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
void FOCUS_Block (void) {
	uint16_t Benefit = 0;
	uint16_t Harm = 0;
	uint8_t RegBenefit = 1;
	uint8_t RegHarm = 0;
	uint8_t IdA = 0;
	uint8_t BLOCK_State = 0;
	uint8_t BLOCK_Last = 0;
	
	uint8_t RegCountPrev = 0;
	uint8_t RegCountCur = 0;
	uint16_t BLOCK_Adr = 0;
	uint8_t BLOCK_Cnt = 0;
	
	if (FOCUS_DataAmount > 1) {
		while (IdA <= (FOCUS_DataAmount - 1)) {
			switch (BLOCK_State) {
	// ������
				case 0:
					BLOCK_Adr = FOCUS_DataAdr[IdA];
					RegBenefit = 1;
					RegHarm = 0;
					RegCountCur = RegBenefit + RegHarm;
					if (BLOCK_Last) {
						BLOCK_Last = 0;
						RegCountPrev = RegBenefit + RegHarm;
						BLOCK_State = 2;
					} else {
						BLOCK_State = 1;
						++ IdA;
					}
					break;
	// ��������
				case 1:
					++ RegBenefit;
					RegHarm += FOCUS_SpaceArray[IdA];
					RegCountPrev = RegCountCur;
					RegCountCur = RegBenefit + RegHarm;
					Benefit = 8 * (RegBenefit - 1);
					Harm = 2 * RegHarm;
					if (Harm > Benefit) {
						if (IdA == (FOCUS_DataAmount - 1)) {
							BLOCK_Last = 1;
							BLOCK_State = 2;
						} else {
							BLOCK_State = 2;
						}
					} else {
						if (IdA == (FOCUS_DataAmount - 1)) {
							RegCountPrev = RegCountCur;
							BLOCK_State = 2;
						} else {
							BLOCK_State = 1;
							++ IdA;
						}
					}
					break;
	// �����
				case 2:
					BLOCK_Request[BLOCK_Cnt][0] = BLOCK_Adr;
					BLOCK_Request[BLOCK_Cnt][1] = 2*RegCountPrev;
					++ BLOCK_Cnt;
					BLOCK_State = 0;
					break;
				
				default:
					break;
			}
		}
		BLOCK_Amount = BLOCK_Cnt;
	} else if (FOCUS_DataAmount == 0) {
		BLOCK_Amount = 0;
	} else if (FOCUS_DataAmount == 1) {
		BLOCK_Request[0][0] = FOCUS_DataAdr[0];
		BLOCK_Request[0][1] = 2;
		BLOCK_Amount = 1;
	}
}

// ���������� ���������� ����� �������������� ��������
void FOCUS_Space (void) {
	uint8_t Id8 = 0;
	
	if (FOCUS_DataAmount) {	
		for (Id8 = 1; Id8 <= (FOCUS_DataAmount - 1); Id8++) {
			FOCUS_SpaceArray[Id8] = FOCUS_DataAdr[Id8] - FOCUS_DataAdr[Id8 - 1] - 1;
		}
	}
}

/************************************************************************/
/*               ������ � ������ ��������� ������� RTC          */
/************************************************************************/
/************************************************************************/
/* -> ������ ��������� �������								*/
/************************************************************************/
uint8_t TIME_SetManual (uint8_t Hours, uint8_t Minutes, uint8_t Seconds, uint8_t Day, uint8_t Date, uint8_t Month, uint8_t Year) {	
	C_TwiEn (); // ��������� ������ TWI	
	C_TwiBusSetIdle (); // ������� ���� � ��������� IDLE

	if (C_TwiBusWaitForIdle ()) {	// �������� ��������� ���� IDLE
		C_TwiTransactionStart (0); // ����� � ������ ������
	} else {
		C_TwiCmdStop (); // �������� ������� STOP
		C_TwiDisable (); // ���������� ������ TWI
		return 0;} // ������� ���������� 0, ���� �������� �������� �������� �����
	if (C_TwiInterruptWaitForWif ()) { // �������� ����� ������ ����� WIF
		if (C_TwiCheckAck ()) {	// ��������, ��� �� ����� ��� ������� ACK
			C_TwiDataPut (0); // ������ �������� �������� ������� RTC
		} else {
			// ���� ������� ����� NACK
			C_TwiCmdStop (); // �������� ������� STOP
			C_TwiDisable (); // ���������� ������ TWI
			// LCD_PutSym (0, 3, '*');
			return 0;} // ������� ���������� 0, ���� �������� �������� �������� �����
	} else {
		C_TwiCmdStop (); // �������� ������� STOP
		C_TwiDisable (); // ���������� ������ TWI
		return 0;} // ������� ���������� 0, ���� �������� �������� �������� �����
	if (C_TwiInterruptWaitForWif ()) { // �������� ����� ������ ����� WIF
		if (C_TwiCheckAck ()) {	// ��������, ��� �� ����� ��� ������� ACK
			C_TwiDataPut (((Seconds/10)<<4) | (Seconds%10)); // ������ ����� �� �������� (�������)
		} else {
			// ���� ������� ����� NACK
			C_TwiCmdStop (); // �������� ������� STOP
			C_TwiDisable (); // ���������� ������ TWI
			// LCD_PutSym (0, 3, '*');
			return 0;} // ������� ���������� 0, ���� �������� �������� �������� �����
	} else {
		C_TwiCmdStop (); // �������� ������� STOP
		C_TwiDisable (); // ���������� ������ TWI
		return 0;} // ������� ���������� 0, ���� �������� �������� �������� �����
	if (C_TwiInterruptWaitForWif ()) { // �������� ����� ������ ����� WIF
		if (C_TwiCheckAck ()) {	// ��������, ��� �� ����� ��� ������� ACK
			C_TwiDataPut (((Minutes/10)<<4) | (Minutes%10)); // ������ ����� �� �������� (������)
		} else {
			// ���� ������� ����� NACK
			C_TwiCmdStop (); // �������� ������� STOP
			C_TwiDisable (); // ���������� ������ TWI
			// LCD_PutSym (0, 3, '*');
			return 0;} // ������� ���������� 0, ���� �������� �������� �������� �����
	} else {
		C_TwiCmdStop (); // �������� ������� STOP
		C_TwiDisable (); // ���������� ������ TWI
		return 0;} // ������� ���������� 0, ���� �������� �������� �������� �����
	if (C_TwiInterruptWaitForWif ()) { // �������� ����� ������ ����� WIF
		if (C_TwiCheckAck ()) {	// ��������, ��� �� ����� ��� ������� ACK
			C_TwiDataPut ((0x00 | (Hours/10)<<4) | (Hours%10)); // ������ ����� �� �������� (����)
		} else {
			// ���� ������� ����� NACK
			C_TwiCmdStop (); // �������� ������� STOP
			C_TwiDisable (); // ���������� ������ TWI
			// LCD_PutSym (0, 3, '*');
			return 0;} // ������� ���������� 0, ���� �������� �������� �������� �����
	} else {
		C_TwiCmdStop (); // �������� ������� STOP
		C_TwiDisable (); // ���������� ������ TWI
		return 0;} // ������� ���������� 0, ���� �������� �������� �������� �����
	if (C_TwiInterruptWaitForWif ()) { // �������� ����� ������ ����� WIF
		if (C_TwiCheckAck ()) {	// ��������, ��� �� ����� ��� ������� ACK
			C_TwiDataPut (0x00 | Day); // ������ ����� �� �������� (����)
		} else {
			// ���� ������� ����� NACK
			C_TwiCmdStop (); // �������� ������� STOP
			C_TwiDisable (); // ���������� ������ TWI
			// LCD_PutSym (0, 3, '*');
			return 0;} // ������� ���������� 0, ���� �������� �������� �������� �����
	} else {
		C_TwiCmdStop (); // �������� ������� STOP
		C_TwiDisable (); // ���������� ������ TWI
		return 0;} // ������� ���������� 0, ���� �������� �������� �������� �����
	if (C_TwiInterruptWaitForWif ()) { // �������� ����� ������ ����� WIF
		if (C_TwiCheckAck ()) {	// ��������, ��� �� ����� ��� ������� ACK
			C_TwiDataPut ((0x00 | (Date/10)<<4) | (Date%10)); // ������ ����� �� �������� (����)
		} else {
			// ���� ������� ����� NACK
			C_TwiCmdStop (); // �������� ������� STOP
			C_TwiDisable (); // ���������� ������ TWI
			// LCD_PutSym (0, 3, '*');
			return 0;} // ������� ���������� 0, ���� �������� �������� �������� �����	
	} else {
		C_TwiCmdStop (); // �������� ������� STOP	
		C_TwiDisable (); // ���������� ������ TWI
		return 0;} // ������� ���������� 0, ���� �������� �������� �������� �����
	if (C_TwiInterruptWaitForWif ()) { // �������� ����� ������ ����� WIF
		if (C_TwiCheckAck ()) {	// ��������, ��� �� ����� ��� ������� ACK
			C_TwiDataPut ((0x00 | (Month/10)<<4) | (Month%10)); // ������ ����� �� �������� (�����)
		} else {
			// ���� ������� ����� NACK
			C_TwiCmdStop (); // �������� ������� STOP
			C_TwiDisable (); // ���������� ������ TWI
			// LCD_PutSym (0, 3, '*');
			return 0;} // ������� ���������� 0, ���� �������� �������� �������� �����
	} else {
		C_TwiCmdStop (); // �������� ������� STOP	
		C_TwiDisable (); // ���������� ������ TWI
		return 0;} // ������� ���������� 0, ���� �������� �������� �������� �����
	if (C_TwiInterruptWaitForWif ()) { // �������� ����� ������ ����� WIF
		if (C_TwiCheckAck ()) {	// ��������, ��� �� ����� ��� ������� ACK
			C_TwiDataPut ((0x00 | (Year/10)<<4) | (Year%10)); // ������ ����� �� �������� (���)
		} else {
			// ���� ������� ����� NACK
			C_TwiCmdStop (); // �������� ������� STOP
			C_TwiDisable (); // ���������� ������ TWI
			// LCD_PutSym (0, 3, '*');
			return 0;} // ������� ���������� 0, ���� �������� �������� �������� �����
	} else {
		C_TwiCmdStop (); // �������� ������� STOP
		C_TwiDisable (); // ���������� ������ TWI
		return 0;} // ������� ���������� 0, ���� �������� �������� �������� �����
	if (C_TwiInterruptWaitForWif ()) { // �������� ����� ������ ����� WIF
		if (C_TwiCheckAck ()) {	// ��������, ��� �� ����� ��� ������� ACK
		
		} else {
			// ���� ������� ����� NACK
			C_TwiCmdStop (); // �������� ������� STOP	
			C_TwiDisable (); // ���������� ������ TWI
			// LCD_PutSym (0, 3, '*');
			return 0;} // ������� ���������� 0, ���� �������� �������� �������� �����
	} else {
		C_TwiCmdStop (); // �������� ������� STOP
		C_TwiDisable (); // ���������� ������ TWI
		return 0;} // ������� ���������� 0, ���� �������� �������� �������� �����
	C_TwiCmdStop (); // �������� ������� STOP
	C_TwiDisable (); // ���������� ������ TWI
	return 1; // ������� ���������� 1, ���� ������ ���� ��������� ������� 
}

/************************************************************************/
/* -> ��������� �������											*/
/************************************************************************/
uint8_t TIME_Set (void) {	
	C_TwiEn (); // ��������� ������ TWI	
	C_TwiBusSetIdle (); // ������� ���� � ��������� IDLE

	if (C_TwiBusWaitForIdle ()) {	// �������� ��������� ���� IDLE
		C_TwiTransactionStart (0); // ����� � ������ ������
	} else {
		C_TwiCmdStop (); // �������� ������� STOP
		C_TwiDisable (); // ���������� ������ TWI
		return 0;}// ������� ���������� 0, ���� �������� �������� �������� �����
	if (C_TwiInterruptWaitForWif ()) { // �������� ����� ������ ����� WIF
		if (C_TwiCheckAck ()) {	// ��������, ��� �� ����� ��� ������� ACK
			C_TwiDataPut (0); // ������ �������� �������� ������� RTC
		} else {
			// ���� ������� ����� NACK
			C_TwiCmdStop (); // �������� ������� STOP
			C_TwiDisable (); // ���������� ������ TWI
			return 0;} // ������� ���������� 0, ���� �������� �������� �������� �����
	} else {
		C_TwiCmdStop (); // �������� ������� STOP
		C_TwiDisable (); // ���������� ������ TWI
		return 0;} // ������� ���������� 0, ���� �������� �������� �������� �����
	if (C_TwiInterruptWaitForWif ()) { // �������� ����� ������ ����� WIF
		if (C_TwiCheckAck ()) {	// ��������, ��� �� ����� ��� ������� ACK
			C_TwiDataPut ((0x00 | (TIME_SetSeconds/10)<<4) | (TIME_SetSeconds%10)); // ������ ����� �� �������� (�������)
		} else {
			// ���� ������� ����� NACK
			C_TwiCmdStop (); // �������� ������� STOP
			C_TwiDisable (); // ���������� ������ TWI
			return 0;} // ������� ���������� 0, ���� �������� �������� �������� �����
	} else {
		C_TwiCmdStop (); // �������� ������� STOP
		C_TwiDisable (); // ���������� ������ TWI
		return 0;} // ������� ���������� 0, ���� �������� �������� �������� �����
	if (C_TwiInterruptWaitForWif ()) { // �������� ����� ������ ����� WIF
		if (C_TwiCheckAck ()) {	// ��������, ��� �� ����� ��� ������� ACK
			C_TwiDataPut ((0x00 | (TIME_SetMinutes/10)<<4) | (TIME_SetMinutes%10)); // ������ ����� �� �������� (������)
		} else {
			// ���� ������� ����� NACK
			C_TwiCmdStop (); // �������� ������� STOP
			C_TwiDisable (); // ���������� ������ TWI
			return 0;} // ������� ���������� 0, ���� �������� �������� �������� �����
	} else {
		C_TwiCmdStop (); // �������� ������� STOP
		C_TwiDisable (); // ���������� ������ TWI
		return 0;} // ������� ���������� 0, ���� �������� �������� �������� �����
	if (C_TwiInterruptWaitForWif ()) { // �������� ����� ������ ����� WIF
		if (C_TwiCheckAck ()) {	// ��������, ��� �� ����� ��� ������� ACK
			C_TwiDataPut ((0x00 | (TIME_SetHours/10)<<4) | (TIME_SetHours%10)); // ������ ����� �� �������� (����)
		} else {
			// ���� ������� ����� NACK
			C_TwiCmdStop (); // �������� ������� STOP	
			C_TwiDisable (); // ���������� ������ TWI
			return 0;} // ������� ���������� 0, ���� �������� �������� �������� �����
	} else {
		C_TwiCmdStop (); // �������� ������� STOP
		C_TwiDisable (); // ���������� ������ TWI
		return 0;} // ������� ���������� 0, ���� �������� �������� �������� �����
		if (C_TwiInterruptWaitForWif ()) { // �������� ����� ������ ����� WIF
		if (C_TwiCheckAck ()) {	// ��������, ��� �� ����� ��� ������� ACK
			C_TwiDataPut (0x00 | TIME_SetDay); // ������ ����� �� �������� (����)
		} else {
			// ���� ������� ����� NACK
			C_TwiCmdStop (); // �������� ������� STOP	
			C_TwiDisable (); // ���������� ������ TWI
			return 0;} // ������� ���������� 0, ���� �������� �������� �������� �����
	} else {
		C_TwiCmdStop (); // �������� ������� STOP
		C_TwiDisable (); // ���������� ������ TWI
		return 0;} // ������� ���������� 0, ���� �������� �������� �������� �����
	if (C_TwiInterruptWaitForWif ()) { // �������� ����� ������ ����� WIF
		if (C_TwiCheckAck ()) {	// ��������, ��� �� ����� ��� ������� ACK
			C_TwiDataPut ((0x00 | (TIME_SetDate/10)<<4) | (TIME_SetDate%10)); // ������ ����� �� �������� (����)
		} else {
			// ���� ������� ����� NACK
			C_TwiCmdStop (); // �������� ������� STOP
			C_TwiDisable (); // ���������� ������ TWI
			return 0;} // ������� ���������� 0, ���� �������� �������� �������� �����
	} else {
		C_TwiCmdStop (); // �������� ������� STOP
		C_TwiDisable (); // ���������� ������ TWI
		return 0;} // ������� ���������� 0, ���� �������� �������� �������� �����
	if (C_TwiInterruptWaitForWif ()) { // �������� ����� ������ ����� WIF
		if (C_TwiCheckAck ()) {	// ��������, ��� �� ����� ��� ������� ACK
			C_TwiDataPut ((0x00 | (TIME_SetMonth/10)<<4) | (TIME_SetMonth%10)); // ������ ����� �� �������� (�����)
		} else {
			// ���� ������� ����� NACK
			C_TwiCmdStop (); // �������� ������� STOP	
			C_TwiDisable (); // ���������� ������ TWI
			return 0;} // ������� ���������� 0, ���� �������� �������� �������� �����
	} else {
		C_TwiCmdStop (); // �������� ������� STOP
		C_TwiDisable (); // ���������� ������ TWI
		return 0;} // ������� ���������� 0, ���� �������� �������� �������� �����
	if (C_TwiInterruptWaitForWif ()) { // �������� ����� ������ ����� WIF
		if (C_TwiCheckAck ()) {	// ��������, ��� �� ����� ��� ������� ACK
			C_TwiDataPut ((0x00 | (TIME_SetYear/10)<<4) | (TIME_SetYear%10)); // ������ ����� �� �������� (���)
		} else {
			// ���� ������� ����� NACK
			C_TwiCmdStop (); // �������� ������� STOP
			C_TwiDisable (); // ���������� ������ TWI
			return 0;} // ������� ���������� 0, ���� �������� �������� �������� �����
	} else {
		C_TwiCmdStop (); // �������� ������� STOP
		C_TwiDisable (); // ���������� ������ TWI
		return 0;} // ������� ���������� 0, ���� �������� �������� �������� �����
	if (C_TwiInterruptWaitForWif ()) { // �������� ����� ������ ����� WIF
		if (C_TwiCheckAck ()) {	// ��������, ��� �� ����� ��� ������� ACK
		
		} else {
			// ���� ������� ����� NACK
			C_TwiCmdStop (); // �������� ������� STOP
			C_TwiDisable (); // ���������� ������ TWI
			return 0;} // ������� ���������� 0, ���� �������� �������� �������� �����
	} else {
		C_TwiCmdStop (); // �������� ������� STOP
		C_TwiDisable (); // ���������� ������ TWI
		return 0;} // ������� ���������� 0, ���� �������� �������� �������� �����
	C_TwiCmdStop (); // �������� ������� STOP	
	C_TwiDisable (); // ���������� ������ TWI	
	return 1; // ������� ���������� 1, ���� ������ ���� ��������� ������� 
}

/************************************************************************/
/* -> ��������� �������� ���� � �������			*/
/************************************************************************/
uint8_t TIME_Get (void) {	
	C_TwiEn (); // ��������� ������ TWI
	C_TwiBusSetIdle (); // ������� ���� � ��������� IDLE

	if (C_TwiBusWaitForIdle ()) {	// �������� ��������� ���� IDLE
		C_TwiTransactionStart (0); // ����� � ������ ������
	} else {
		C_TwiCmdStop (); // �������� ������� STOP
		C_TwiDisable (); // ���������� ������ TWI
		return 0;} // ������� ���������� 0, ���� �������� �������� �������� �����
	if (C_TwiInterruptWaitForWif ()) { // �������� ����� ������ ����� WIF
		if (C_TwiCheckAck ()) {	// ��������, ��� �� ����� ��� ������� ACK
			C_TwiDataPut (0);} // ������ �������� �������� ������� RTC
	} else {
		C_TwiCmdStop (); // �������� ������� STOP
		C_TwiDisable (); // ���������� ������ TWI
		return 0;} // ������� ���������� 0, ���� �������� �������� �������� �����
	if (C_TwiInterruptWaitForWif ()) { // �������� ����� ������ ����� WIF
		if (C_TwiCheckAck ()) {	// ��������, ��� �� ����� ��� ������� ACK
			C_TwiTransactionStart (1);} // ��������� ����� � ������ ������
	} else {
		C_TwiCmdStop (); // �������� ������� STOP
		C_TwiDisable (); // ���������� ������ TWI
		return 0;} // ������� ���������� 0, ���� �������� �������� �������� �����
	if (C_TwiInterruptWaitForRif ()) { // �������� ����� ������ ����� RIF
		TIME_GetSeconds = C_TwiDataGet (); // ������ ����� �� �������� (�������)
		C_TwiCmdSendAck (); // �������� ���� ������������� ACK
	} else {
		C_TwiCmdStop (); // �������� ������� STOP
		C_TwiDisable (); // ���������� ������ TWI
		return 0;} // ������� ���������� 0, ���� �������� �������� �������� �����
	if (C_TwiInterruptWaitForRif ()) { // �������� ����� ������ ����� RIF
		TIME_GetMinutes = C_TwiDataGet (); // ������ ����� �� �������� (������)
		C_TwiCmdSendAck (); // �������� ���� ������������� ACK
	} else {
		C_TwiCmdStop (); // �������� ������� STOP
		C_TwiDisable (); // ���������� ������ TWI
		return 0;} // ������� ���������� 0, ���� �������� �������� �������� �����
	if (C_TwiInterruptWaitForRif ()) { // �������� ����� ������ ����� RIF
		TIME_GetHours = C_TwiDataGet (); // ������ ����� �� �������� (����)
		C_TwiCmdSendNack (); // �������� ���� ������������� NACK (����� ���������� �����)
	}  else {
		C_TwiCmdStop (); // �������� ������� STOP
		C_TwiDisable (); // ���������� ������ TWI
		return 0;} // ������� ���������� 0, ���� �������� �������� �������� �����
	if (C_TwiInterruptWaitForRif ()) { // �������� ����� ������ ����� RIF
		TIME_GetDay = C_TwiDataGet (); // ������ ����� �� �������� (����)
		C_TwiCmdSendAck (); // �������� ���� ������������� ACK
	} else {
		C_TwiCmdStop (); // �������� ������� STOP
		C_TwiDisable (); // ���������� ������ TWI
		return 0;} // ������� ���������� 0, ���� �������� �������� �������� �����
	if (C_TwiInterruptWaitForRif ()) { // �������� ����� ������ ����� RIF
		TIME_GetDate = C_TwiDataGet (); // ������ ����� �� �������� (����)
		C_TwiCmdSendAck (); // �������� ���� ������������� ACK
	} else {
		C_TwiCmdStop (); // �������� ������� STOP
		C_TwiDisable (); // ���������� ������ TWI
		return 0;} // ������� ���������� 0, ���� �������� �������� �������� �����
	if (C_TwiInterruptWaitForRif ()) { // �������� ����� ������ ����� RIF
		TIME_GetMonth = C_TwiDataGet (); // ������ ����� �� �������� (�����)
		C_TwiCmdSendNack (); // �������� ���� ������������� NACK (����� ���������� �����)
	}  else {
		C_TwiCmdStop (); // �������� ������� STOP
		C_TwiDisable (); // ���������� ������ TWI
		return 0;} // ������� ���������� 0, ���� �������� �������� �������� �����
	if (C_TwiInterruptWaitForRif ()) { // �������� ����� ������ ����� RIF
		TIME_GetYear = C_TwiDataGet (); // ������ ����� �� �������� (���)
		C_TwiCmdSendNack (); // �������� ���� ������������� NACK (����� ���������� �����)
	}  else {
		C_TwiCmdStop (); // �������� ������� STOP
		C_TwiDisable (); // ���������� ������ TWI
		return 0;} // ������� ���������� 0, ���� �������� �������� �������� �����
	C_TwiCmdStop (); // �������� ������� STOP	
	C_TwiDisable (); // ���������� ������ TWI	
	return 1; // ������� ���������� 1, ���� ������ ���� ��������� ������� 
}

/************************************************************************/
/* -> ������� ��������												*/
/************************************************************************/
//inline void TIMER_1msInit (void) {	
	//TCC1.PER = 18432;
	//TCC1.INTCTRLA |= (0x01<<0); // ������� ���������� �� ������������
	//TCC1.CTRLA |= (0x01<<0);		
//}

//inline void TIMER_Rs232Init (void) {	
	//TCD1.PER = 25000;
	//TCD1.INTCTRLA |= (0x02<<0); // ������� ���������� �� ������������
//}

//inline void TIMER_Rs232Start () {
	//TCD1.CNT = 0x0000; // ��������� �������� �������� �������
	//TCD1.CTRLA |= (0x01<<0); // ����� ��������
//}
//
//inline void TIMER_Rs232Stop () {
	//TCD1.CTRLA &= ~ (0x01<<0); // ���� ��������
//}
/************************************************************************/
/*				������ � ���������������� ������� APD*/
/************************************************************************/
/* ������� ������������ ������� ������ ���� ��������������
 * ������ == [������ 1]
 * ������� == [������ 2]
 * �� ����������� == ��� ��������������
 * ��� ������� == 0xFF
 */
uint8_t APD_FrameButtonActionTable[13][13] = {
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

/*
 * ���������� ������� �������������� ������� APD
 */
void APD_SendToPlc (uint8_t Id); // �������� "�������� � ���"
void APD_GoToFrame (uint8_t Id); // �������� "������� � �����"
void APD_EditField (uint8_t Id); // �������� "������������� ����"
void APD_FrameShiftUp (uint8_t Id); // �������� "����� ����� �����"
void APD_FrameShiftDown (uint8_t Id); // �������� "����� ����� ����"

void APD_ListShiftUp (uint8_t Id); // �������� "����� ������ �����"
void APD_ListShiftDown (uint8_t Id); // �������� "����� ������ ����"

uint8_t SendToPlc_Recieved = 0;

uint16_t SendToPlc_Adress[SEND_AMOUNT];
int16_t SendToPlc_Value[SEND_AMOUNT];
uint8_t SendToPlc_Ready[SEND_AMOUNT];

uint8_t SendToPlc_Amount = 0;
uint8_t ADP_SendToPlcQueue = 0;
uint8_t ADP_SendToPlcQueueCnt = 0;
uint8_t ADP_EventRequestQueue = 0;
uint8_t ADP_EventReqRatioCnt = 0;
uint8_t ADP_EventReqReady = 0;
uint8_t TRANS_STATUS = 0;

typedef enum trans_status {
	DATA = 0, // ������ ������
	EVENTS = 1, // ������ ������ �������
 } trans_status_t;

/*
 * �������� ������� �������������� ������� APD
 */
void APD_SendToPlc (uint8_t Id) {
	SendToPlc_Adress[SendToPlc_Amount - 1] = ((pgm_read_byte ( APD_PrjData + APD_FrameActionStartAdr[APD_FrameCur][Id] + 3 ))<<8) | (pgm_read_byte ( APD_PrjData + APD_FrameActionStartAdr[APD_FrameCur][Id] + 2 )) + 1;
	SendToPlc_Value[SendToPlc_Amount - 1] = ((pgm_read_byte ( APD_PrjData + APD_FrameActionStartAdr[APD_FrameCur][Id] + 5 ))<<8) | (pgm_read_byte ( APD_PrjData + APD_FrameActionStartAdr[APD_FrameCur][Id] + 4 ));
	SendToPlc_Ready[SendToPlc_Amount - 1] = SEND_ATTEMPTS;
	// LCD_PutSym (3, 0, 'A');
}

void APD_GoToFrame (uint8_t Id) {
//	LCD_PutSym (0, 0, '*');

// 	if ( (APD_FrameCur == 14) && (Id == 9) ) {
// 		// TestValue_1 = APD_FrameCur;
// 		TestFlag = 1;
// 	}

/*	cli();*/

	//TestValue_2 = pgm_read_byte ( APD_PrjData + APD_FrameActionStartAdr[APD_FrameCur][Id] + 2);
	APD_FrameCur = pgm_read_byte ( APD_PrjData + APD_FrameActionStartAdr[APD_FrameCur][Id] + 2);
	//TestValue_1 = APD_FrameCur;
/*	sei();*/
	
	APD_List[APD_FrameCur][2] = 0;
	if (APD_ListAdress[APD_FrameCur]) {
		for ( uint8_t ListAdrId = 0; ListAdrId <= ( APD_List[APD_FrameCur][1] - 1 ); ++ ListAdrId ) {
			APD_FrameDataFromPlcAdr[APD_FrameCur][ ( APD_List[APD_FrameCur][0] + ListAdrId ) ] = APD_ListAdress[APD_FrameCur] + ListAdrId;
		}
	}		
	APD_FrameDisplayShift = 0;
//	FOCUS_New = 1;
						
	FOCUS_Adr ();
	FOCUS_Sort (FOCUS_DataAdr, FOCUS_DataAmount);
	FOCUS_Space ();			
	FOCUS_Block ();
	FOCUS_CntReset ();
	SplashRefreshEn = 1;
	LCD_Refresh = 1;	
}
// (APD_ValStr, APD_ValCol, APD_ValLen, PointPos, APD_ValLen - Len, Neg, Val);

uint8_t EDIT_IdField = 0;
uint8_t EDIT_FieldGet = 0;

uint8_t EDIT_ValStr = 0;
uint8_t EDIT_ValCol = 0;
uint8_t EDIT_ValLen = 0;
int64_t EDIT_Val = 0;
int64_t EDIT_ValTemp = 0;

int32_t EDIT_ValMultiplier = 0;
int32_t EDIT_ValDivider = 0;
int32_t EDIT_ValAddition = 0;

uint8_t EDIT_ValPoint = 0;
uint8_t EDIT_ValNeg = 0;
uint16_t EDIT_ValPlcAdr = 0;
uint8_t EDIT_BlinkMarker = 0;
uint8_t EDIT_ValCapture = 0;

void APD_EditField (uint8_t Id) {
	EDIT_IdField = pgm_read_byte ( APD_PrjData + APD_FrameActionStartAdr[APD_FrameCur][Id] + 2 );
	Mode = MODE_EDIT;
	EDIT_FieldGet = 1;
	
	TIMER_EditValBlinkEn = 1;
	TIMER_EditValBlink = 0;
	LCD_Refresh = 1;
//	LCD_PutValDec (0, 0, 3, EDIT_IdField);
}
void APD_FrameShiftUp (uint8_t Id) {
	//LCD_PutSym (0, 1, '!');
	uint8_t Strs_max = pgm_read_byte ( APD_PrjData + APD_FrameStartAdr[APD_FrameCur] + 4 );
	uint8_t Strs_shift = pgm_read_byte ( APD_PrjData + APD_FrameActionStartAdr[APD_FrameCur][Id] + 2 );
	if (Strs_max > pgm_read_byte ( APD_PrjData + 8 )) {
		APD_FrameDisplayShift -= Strs_shift;
		if (APD_FrameDisplayShift <= 0) {
			APD_FrameDisplayShift = 0;
		}	
//		FOCUS_New = 1;
		FOCUS_Adr ();
		FOCUS_Sort (FOCUS_DataAdr, FOCUS_DataAmount);
		FOCUS_Space ();			
		FOCUS_Block ();
		FOCUS_CntReset ();
		
		SplashRefreshEn = 1;
		LCD_Refresh = 1;
	}	
}
void APD_FrameShiftDown (uint8_t Id) {
	//LCD_PutSym (0, 0, '*');
	uint8_t Strs_max = pgm_read_byte ( APD_PrjData + APD_FrameStartAdr[APD_FrameCur] + 4 );
	uint8_t Strs_shift = pgm_read_byte ( APD_PrjData + APD_FrameActionStartAdr[APD_FrameCur][Id] + 2 );
	if (Strs_max > pgm_read_byte ( APD_PrjData + 8 )) {
		APD_FrameDisplayShift += Strs_shift;
		if ((APD_FrameDisplayShift + pgm_read_byte ( APD_PrjData + 8) ) >= (Strs_max)) {
			APD_FrameDisplayShift = (Strs_max - pgm_read_byte ( APD_PrjData + 8) );
		}
//		FOCUS_New = 1;
		FOCUS_Adr ();
		FOCUS_Sort (FOCUS_DataAdr, FOCUS_DataAmount);
		FOCUS_Space ();			
		FOCUS_Block ();
		FOCUS_CntReset ();
		
		SplashRefreshEn = 1;
		LCD_Refresh = 1;
	}
}
void APD_ListShiftUp (uint8_t Id) {
	if ((APD_List[2]) && (APD_FrameDataFromPlcAdr[APD_FrameCur][ ( APD_List[APD_FrameCur][0] ) ] > APD_ListAdress[APD_FrameCur])) {
		-- APD_List[APD_FrameCur][2];
		for ( uint8_t ListAdrId = 0; ListAdrId <= ( APD_List[APD_FrameCur][1] - 1 ); ++ ListAdrId ) {
			-- APD_FrameDataFromPlcAdr[APD_FrameCur][ ( APD_List[APD_FrameCur][0] + ListAdrId ) ];
		}
		// FOCUS_New = 1;
		FOCUS_Adr ();
		FOCUS_Sort (FOCUS_DataAdr, FOCUS_DataAmount);
		FOCUS_Space ();			
		FOCUS_Block ();
		FOCUS_CntReset ();
		// SplashRefreshEn = 1;
		LCD_Refresh = 1;		
	}		
}
void APD_ListShiftDown (uint8_t Id) {
	if ( APD_FrameDataFromPlcAdr[APD_FrameCur][ ( APD_List[APD_FrameCur][0] ) ] < (APD_ListAdress[APD_FrameCur] + 256) ) {
		++ APD_List[APD_FrameCur][2];
		for ( uint8_t ListAdrId = 0; ListAdrId <= ( APD_List[APD_FrameCur][1] - 1 ); ++ ListAdrId ) {
			++ APD_FrameDataFromPlcAdr[APD_FrameCur][ ( APD_List[APD_FrameCur][0] + ListAdrId ) ];
		}
		// FOCUS_New = 1;
		FOCUS_Adr ();
		FOCUS_Sort (FOCUS_DataAdr, FOCUS_DataAmount);
		FOCUS_Space ();			
		FOCUS_Block ();
		FOCUS_CntReset ();
		// SplashRefreshEn = 1;
		LCD_Refresh = 1;
	}
}

typedef void (* FuncPtrArray[]) (uint8_t Id);

/* ��� ��� ������� � APD:
 * 0 - �������� = �������� � ���
 * 1 - �������� = ������� �� ����
 * 2 - �������� = ����� ����� �����
 * 3 - �������� = ����� ����� ����
 * 4 - �������� = ����� ����� ����-������
 * 5 - �������� = ����� ���� ����-������
 * 6 - �������� = ������������� ����
 */
const FuncPtrArray APD_FrameActionFunction = {
	APD_SendToPlc,
	APD_GoToFrame,
	APD_FrameShiftUp,
	APD_FrameShiftDown,	
	APD_ListShiftUp,
	APD_ListShiftDown,
	APD_EditField,
};

uint8_t SendToPlc_One = 0;

// ��������� ������� ������ � ������� APD
void SCREEN_ACTION (uint8_t Action) {
//	cli();
	for (uint8_t Id = 0; Id <= APD_FrameActionsAmount[APD_FrameCur]; ++ Id) {
		if (pgm_read_byte ( APD_PrjData + APD_FrameActionStartAdr[APD_FrameCur][Id] ) == Action) {
			if ( (SendToPlc_One) && ( (pgm_read_byte ( APD_PrjData + APD_FrameActionStartAdr[APD_FrameCur][Id] + 1 )) == 0) ) {
				SendToPlc_One = 0;
				SendToPlc_Amount = 0;
			}
			if ( SendToPlc_Amount < SEND_AMOUNT ) ++ SendToPlc_Amount;
			APD_FrameActionFunction[pgm_read_byte ( APD_PrjData + APD_FrameActionStartAdr[APD_FrameCur][Id] + 1 )](Id);
		}
	}
	
//	sei();
}

uint8_t EDIT_StrSpaces[20] = {
	0x20, 0x20, 0x20, 0x20, 0x20,
	0x20, 0x20, 0x20, 0x20, 0x20,
	0x20, 0x20, 0x20, 0x20, 0x20,
	0x20, 0x20, 0x20, 0x20, 0x20,
};

// ����� �������������� ���� � �������� ��������������
void EDIT_ValBlink (void);
void EDIT_ValBlink (void) {
	// LCD_PutValDec (0, 19, 1, EDIT_BlinkMarker);
	if (EDIT_BlinkMarker) {
		EDIT_BlinkMarker = 0;
		// LCD_PutSym (0, 10, 'A');
		
		// LCD_PutValDec (3, 0, 2, Mode);
// 		LCD_PutValDec (0, 16, 2, EDIT_IdField);
// 		LCD_PutValDec (1, 16, 4, EDIT_Val);
// 		LCD_PutValDec (2, 16, 4, EDIT_ValStr);
// 		LCD_PutValDec (3, 16, 4, EDIT_ValCol);
		LCD_PutValDecPointMaskNeg_ (EDIT_ValStr, EDIT_ValCol, EDIT_ValLen, EDIT_ValPoint, 0, EDIT_Val);	
	} else {
		EDIT_BlinkMarker = 1;
		// LCD_PutSym (0, 10, 'B');
		
		// LCD_PutValDec (3, 0, 2, Mode);
// 		LCD_PutValDec (0, 16, 2, EDIT_IdField);
// 		LCD_PutValDec (1, 16, 4, EDIT_Val);
// 		LCD_PutValDec (2, 16, 4, EDIT_ValStr);
// 		LCD_PutValDec (3, 16, 4, EDIT_ValCol);
		
		LCD_PutStrNum (EDIT_ValStr, EDIT_ValCol, EDIT_ValLen, (uint8_t *) EDIT_StrSpaces);
	}
}

void EDIT_ValAdd (uint8_t Add);
void EDIT_ValAdd (uint8_t Add) {
	uint64_t Divider = 1;
	
	if (EDIT_ValCapture) {
		EDIT_ValCapture = 0;
		EDIT_Val = 0;
	}
	for (uint8_t Id = EDIT_ValLen /*- 1*/; Id; -- Id) {
		Divider *= 10;
	}
		// Divider /= 10;
	EDIT_Val = ( ( EDIT_Val * 10 ) + Add ) % ( Divider );
	// LCD_PutValDec (3, 0, 8, EDIT_Val);
	// LCD_PutValDec (2, 0, 8, Divider);
}

/************************************************************************/
/*					������ � �������� �������                 */
/************************************************************************/
/************************************************************************/
/* -> ���������� ����� (������������ ����� ������� APD)  */
/************************************************************************/
void SCREEN_NORMAL (void) {
	
	uint8_t ClockAppear = 0;
	uint8_t DataAppear = 0;
	
	int8_t APD_ValStr = 0;
	uint8_t APD_ValCol = 0;
	uint8_t APD_ValLen = 0;
	uint64_t APD_Val_ = 0;
	uint64_t APD_Val = 0;
	uint16_t APD_ValPlcAdr = 0;
	
	
	int64_t APD_ValMultiplier = 0;
	int64_t APD_ValDivider = 0;
	int64_t APD_ValAddition = 0;
		
//	int64_t APD_ValTemp = 0;
	int64_t APD_ValResult = 0;
	
	int8_t APD_TextStr = 0;
	
	uint8_t APD_TextCol = 0;
	uint8_t APD_TextLen = 0;
	uint16_t APD_TextPlcAdr = 0;
	uint8_t APD_TextIdMin = 0;
	uint8_t APD_TextIdMax = 0;
	uint16_t APD_TextId = 0;
	uint8_t APD_TextSymbolId = 0;
	
	uint8_t APD_ValFormat = 0;
//	uint32_t Val = 0;
	
	uint8_t Id = 0;
	uint8_t IdActual = 0;
//	uint8_t Len = 0;
//	uint8_t Neg = 0;
	uint8_t DataOk = 0;
	uint8_t SpaceOk = 0;

	uint16_t APD_ListPlcAdr = 0;
	uint8_t APD_ListCol = 0;
	uint8_t APD_ListStr = 0;
	uint8_t APD_ListLen = 0;
	uint8_t APD_ListHei = 0;
	uint8_t APD_ListIdMin = 0;
	uint8_t APD_ListIdMax = 0;
	uint8_t APD_ListStrId = 0;
	uint8_t APD_ListId = 0;
	uint8_t APD_ListSymbolId = 0;
	
	uint8_t SplashStrAmount = 0;
	uint8_t SplashColAmount = 0;
	
	uint8_t PointPos = 0;
	for (uint8_t IdField = 0; IdField <= (APD_FrameFieldsAmount[APD_FrameCur] - 1); ++ IdField) {
		switch (pgm_read_byte ( APD_PrjData + APD_FrameFieldStartAdr[APD_FrameCur][IdField] )) {

			/************************************************************************/
			/* -> ��� ����: ��������												*/
			/************************************************************************/
			case APD_FIELD_TYPE_SPLASH:
				if (SplashRefreshEn) {
					SplashRefreshEn = 0;
					LCD_Clear ();
					
					SplashStrAmount = pgm_read_byte ( APD_PrjData + APD_FrameFieldStartAdr[APD_FrameCur][IdField] + 1 );
					if (SplashStrAmount > 4) SplashStrAmount = 4;
					
					SplashColAmount = pgm_read_byte ( APD_PrjData + 9 );
					
					for (uint8_t IdStr = 0; IdStr <= (SplashStrAmount - 1); IdStr++) {
						for (uint8_t IdCol = 0; IdCol <= (SplashColAmount - 1); IdCol++) {
							LCD_PutSym (IdStr, IdCol, APD_PrjFontTable[pgm_read_byte ( APD_PrjData + APD_FrameFieldStartAdr[APD_FrameCur][IdField] + 2 + pgm_read_byte ( APD_PrjData + 9 )*(IdStr + APD_FrameDisplayShift) + IdCol )]);
						}
					}
				}
				break;
				
			/************************************************************************/
			/* -> ��� ����: �����													*/
			/************************************************************************/
			case APD_FIELD_TYPE_VALUE:

				APD_ValStr = pgm_read_byte ( APD_PrjData + APD_FrameFieldStartAdr[APD_FrameCur][IdField] + 1 ) - APD_FrameDisplayShift;
				if ((APD_ValStr >= 0) && (Abs (APD_ValStr) < (pgm_read_byte ( APD_PrjData + 8 )))) {

					APD_ValStr = Abs (APD_ValStr);
					APD_ValCol = pgm_read_byte ( APD_PrjData + APD_FrameFieldStartAdr[APD_FrameCur][IdField] + 2 );
					// ����� ����
					APD_ValLen = pgm_read_byte ( APD_PrjData + APD_FrameFieldStartAdr[APD_FrameCur][IdField] + 3 );
					// ����� ���������� ����
					APD_ValPlcAdr = (pgm_read_byte ( APD_PrjData + APD_FrameFieldStartAdr[APD_FrameCur][IdField] + 4 )) | (pgm_read_byte ( APD_PrjData + APD_FrameFieldStartAdr[APD_FrameCur][IdField] + 5 ) << 8);
					++ APD_ValPlcAdr;
					// �� ������ ������� ������ �������
					DataOk = 0;
					//DataOk = 1;
					for (Id = 0; Id <= /*APD_MAX_FIELDS*/ (FOCUS_DataAmount - 1); Id++) {
						if ((APD_ValPlcAdr) == FOCUS_DataAdr[Id]) {

							if (FOCUS_DataStatus[Id]) {
								APD_Val_ = FOCUS_Data[Id];
								DataOk = 1;
								if (pgm_read_byte ( APD_PrjData + APD_FrameFieldStartAdr[APD_FrameCur][IdField] + 6 ) == APD_FIELD_TYPE_32BIT)
								{
									if (FOCUS_Data[Id]<0){
										APD_Val_ = 65536+FOCUS_Data[Id];//convert to positive
									}
									else APD_Val_ = FOCUS_Data[Id];
									
									if (FOCUS_Data[Id+1]<0){
										APD_Val_ += (4294967296+((int32_t)FOCUS_Data[Id+1]<<16));//convert to positive
									}
									else APD_Val_ += ((uint32_t)FOCUS_Data[Id+1]<<16);
									//Id++;
								}
							} else {	
								DataOk = 0;
							}
							if (FOCUS_DataCntNewStatus[Id]) {
								SpaceOk = 1;
							} else {
								SpaceOk = 0;
							}
						}
					}
					// ���������� � ������� ������
					APD_Val = (uint64_t) APD_Val_;
					// ������ ������ ����
					APD_ValFormat = pgm_read_byte ( APD_PrjData + APD_FrameFieldStartAdr[APD_FrameCur][IdField] + 7 );
					// ��������� (�� �� ����� ������)
					APD_ValMultiplier = (int64_t) (pgm_read_byte ( APD_PrjData + APD_FrameFieldStartAdr[APD_FrameCur][IdField] + 9 ) << 8) | (pgm_read_byte ( APD_PrjData + APD_FrameFieldStartAdr[APD_FrameCur][IdField] + 8 ));
					// ��������
					APD_ValDivider = (int64_t) (pgm_read_byte ( APD_PrjData + APD_FrameFieldStartAdr[APD_FrameCur][IdField] + 11 ) << 8) | (pgm_read_byte ( APD_PrjData + APD_FrameFieldStartAdr[APD_FrameCur][IdField] + 10 ));
					// �������
					APD_ValAddition = (int64_t) (pgm_read_byte ( APD_PrjData + APD_FrameFieldStartAdr[APD_FrameCur][IdField] + 13 ) << 8) | (pgm_read_byte ( APD_PrjData + APD_FrameFieldStartAdr[APD_FrameCur][IdField] + 12 ));	

					// ������� �����
					PointPos = (APD_ValFormat & 0x0F);
					
					// LCD_PutValDecPointMaskNeg (0, 0, 10, 1, 6, 1, 123);
					
					if ( (Mode == MODE_EDIT) &&  (EDIT_IdField == IdField) ) {
						// LCD_PutSym (0, 15, 'M');
						if (EDIT_FieldGet) {
							LCD_PutStrNum (APD_ValStr, APD_ValCol, APD_ValLen, (uint8_t *) StrSpaces);
							// LCD_PutSym (0, 14, 'B');
						}
					} else {
						// ��������
						LCD_PutStrNum (APD_ValStr, APD_ValCol, APD_ValLen, (uint8_t *) StrSpaces);
					}

					/************************************************************************/
					/* -> ������ � ��������� ���������                                      */
					/************************************************************************/
					/************************************************************************/
					/* ==>> ������: ��������                                                */
					/************************************************************************/
					if ((APD_ValFormat & 0xF0) == APD_FIELD_FORMAT_BIN) {
						if (DataOk) {

							// �����
							APD_ValResult = APD_Val>>APD_ValMultiplier;
		
							LCD_PutValBinPoint (APD_ValStr, APD_ValCol, APD_ValLen, PointPos, APD_ValResult);

						} else {
							if (SpaceOk) {
								LCD_PutStrNum (APD_ValStr, APD_ValCol, APD_ValLen, (uint8_t *) StrSpaces);
							} else {
								LCD_PutStrNum (APD_ValStr, APD_ValCol, APD_ValLen, (uint8_t *) StrBlackSq);
							}
							SpaceOk = 0;
						}

					/************************************************************************/
					/* ==>> ������: �����������������                                       */
					/************************************************************************/
					} else if ((APD_ValFormat & 0xF0) == APD_FIELD_FORMAT_HEX) {
						if (DataOk) {

							// �����
							APD_ValResult = APD_Val>>APD_ValMultiplier;

							LCD_PutValHexPoint (APD_ValStr, APD_ValCol, APD_ValLen, PointPos, APD_ValResult);

						} else {
							if (SpaceOk) {
								LCD_PutStrNum (APD_ValStr, APD_ValCol, APD_ValLen, (uint8_t *) StrSpaces);
							} else {
								LCD_PutStrNum (APD_ValStr, APD_ValCol, APD_ValLen, (uint8_t *) StrBlackSq);
							}
							SpaceOk = 0;														
						}

					/************************************************************************/
					/* ==>> ������: ���������� �������� ��� ����� �����                     */
					/************************************************************************/							
					} else if ((APD_ValFormat & 0xF0) == APD_FIELD_FORMAT_DEC_WITHOUT_NULLS) {
						if (DataOk) {
//							APD_ValResult = ( ( ( APD_Val * ( APD_ValMultiplier << 1 ) ) / APD_ValDivider ) >> 1 ) + APD_ValAddition;
							if((int64_t)APD_Val>0)
								APD_ValResult = (int64_t) ( round ( (double) APD_Val * ( (double) APD_ValMultiplier  / (double) APD_ValDivider ) ) ) + APD_ValAddition;
							else
								APD_ValResult = (int64_t) (((int32_t)APD_ValMultiplier*((int32_t)APD_Val/(int32_t)APD_ValDivider))+APD_ValAddition);// * ( (double) APD_ValMultiplier  / (double) APD_ValDivider ) ) ) + APD_ValAddition;
							

							
// 							if (APD_Val) {
// 								// ��������������� �������� �������� � ����������
// 								APD_ValResult = ( ( ( ( ( APD_Val * APD_ValMultiplier ) * 10 ) / APD_ValDivider ) + 5 ) / 10 ) + APD_ValAddition;
// 							} else {	
// 								// ��������������� �������� �������� � ����������
// 								APD_ValResult = ( - 1 ) * ( ( ( ( ( Abs (APD_Val) * APD_ValMultiplier ) * 10 ) / APD_ValDivider ) + 5 ) / 10 ) + APD_ValAddition;
// 							}
						
							if ( (Mode == MODE_EDIT) && (EDIT_IdField == IdField) ) {
								// LCD_PutSym (3, 4, 'C');
								if (EDIT_FieldGet) {
									EDIT_FieldGet = 0;
									// ������� ��������� ����
									EDIT_ValStr = APD_ValStr;
									EDIT_ValCol = APD_ValCol;
									EDIT_ValLen = APD_ValLen;
									EDIT_Val = APD_ValResult;
									EDIT_ValMultiplier = APD_ValMultiplier;
									EDIT_ValDivider = APD_ValDivider;
									EDIT_ValAddition = APD_ValAddition;
									EDIT_ValPlcAdr = APD_ValPlcAdr;
									EDIT_ValPoint = PointPos;
									EDIT_ValCapture = 1;
								}
							} else {
								// LCD_PutValDecPointMaskNeg (APD_ValStr, APD_ValCol, APD_ValLen, PointPos, APD_ValLen - Len - 1, Neg, Val);
								LCD_PutValDecPointMaskNeg_ (APD_ValStr, APD_ValCol, APD_ValLen, PointPos, 1, APD_ValResult);
							}

						} else {
							if ( (Mode == MODE_EDIT) && (EDIT_IdField == IdField) ) {
								
								if (EDIT_FieldGet) {
									EDIT_FieldGet = 0;
									// ������� ��������� ����
									EDIT_ValStr = APD_ValStr;
									EDIT_ValCol = APD_ValCol;
									EDIT_ValLen = APD_ValLen;
									EDIT_Val = 0;
									EDIT_ValMultiplier = APD_ValMultiplier;
									EDIT_ValDivider = APD_ValDivider;
									EDIT_ValAddition = APD_ValAddition;
									EDIT_ValPlcAdr = APD_ValPlcAdr;
									EDIT_ValPoint = PointPos;
									EDIT_ValCapture = 1;
								}
							} else {
								if (SpaceOk) {
									LCD_PutStrNum (APD_ValStr, APD_ValCol, APD_ValLen, (uint8_t *) StrSpaces);
								} else {
									LCD_PutStrNum (APD_ValStr, APD_ValCol, APD_ValLen, (uint8_t *) StrBlackSq);
								}
								SpaceOk = 0;
							}
						}

					/************************************************************************/
					/* ==>> ������: ���������� �������� � ������ �����                      */
					/************************************************************************/
					} else if ((APD_ValFormat & 0xF0) == APD_FIELD_FORMAT_DEC_WITH_NULLS) {
						if (DataOk) {
//							APD_ValResult = ( ( ( APD_Val * ( APD_ValMultiplier << 1 ) ) / APD_ValDivider ) >> 1 ) + APD_ValAddition;
							APD_ValResult = (int64_t) ( round ( (double) APD_Val * ( (double) APD_ValMultiplier  / (double) APD_ValDivider ) ) ) + APD_ValAddition;
							
// 							if (APD_Val) {
// 								// ��������������� �������� �������� � ����������
// 								APD_ValResult = ( ( ( ( ( APD_Val * APD_ValMultiplier ) * 10 ) / APD_ValDivider ) + 5 ) / 10 ) + APD_ValAddition;
// 							} else {	
// 								// ��������������� �������� �������� � ����������
// 								APD_ValResult = ( - 1 ) * ( ( ( ( ( Abs (APD_Val) * APD_ValMultiplier ) * 10 ) / APD_ValDivider ) + 5 ) / 10 ) + APD_ValAddition;
// 							}
							
							if ( (Mode == MODE_EDIT) && (EDIT_IdField == IdField) ) {
								// LCD_PutSym (0, 12, 'C');
								if (EDIT_FieldGet) {
									EDIT_FieldGet = 0;
									// ������� ��������� ����
									EDIT_ValStr = APD_ValStr;
									EDIT_ValCol = APD_ValCol;
									EDIT_ValLen = APD_ValLen;
									EDIT_Val = APD_ValResult;
									EDIT_ValMultiplier = APD_ValMultiplier;
									EDIT_ValDivider = APD_ValDivider;
									EDIT_ValAddition = APD_ValAddition;
									EDIT_ValPlcAdr = APD_ValPlcAdr;
									EDIT_ValPoint = PointPos;
									EDIT_ValCapture = 1;
									// EDIT_ValNeg = Neg;
								}
							} else {
								LCD_PutValDecPointMaskNeg_ (APD_ValStr, APD_ValCol, APD_ValLen, PointPos, 0, APD_ValResult);
							}

						} else {
							if ( (Mode == MODE_EDIT) && (EDIT_IdField == IdField) ) {
								if (EDIT_FieldGet) {
									EDIT_FieldGet = 0;
									// ������� ��������� ����
									EDIT_ValStr = APD_ValStr;
									EDIT_ValCol = APD_ValCol;
									EDIT_ValLen = APD_ValLen;
									EDIT_Val = 0;
									EDIT_ValMultiplier = APD_ValMultiplier;
									EDIT_ValDivider = APD_ValDivider;
									EDIT_ValAddition = APD_ValAddition;
									EDIT_ValPlcAdr = APD_ValPlcAdr;
									EDIT_ValPoint = PointPos;
									EDIT_ValCapture = 1;
									// EDIT_ValNeg = Neg;
								}
							} else {
								if (SpaceOk) {
									LCD_PutStrNum (APD_ValStr, APD_ValCol, APD_ValLen, (uint8_t *) StrSpaces);
								} else {
									LCD_PutStrNum (APD_ValStr, APD_ValCol, APD_ValLen, (uint8_t *) StrBlackSq);
								}
								SpaceOk = 0;
							}
						}
					}
				}
				break;

			/************************************************************************/
			/* -> ��� ����: �����													*/
			/************************************************************************/
			case APD_FIELD_TYPE_TEXT:
				
				APD_TextStr = pgm_read_byte ( APD_PrjData + APD_FrameFieldStartAdr[APD_FrameCur][IdField] + 1 ) - APD_FrameDisplayShift;
				if ((APD_TextStr >= 0) && (Abs (APD_TextStr) < (pgm_read_byte ( APD_PrjData + 8 )))) {
					
					APD_TextStr = Abs (APD_TextStr);
					APD_TextCol = pgm_read_byte ( APD_PrjData + APD_FrameFieldStartAdr[APD_FrameCur][IdField] + 2 );
					APD_TextLen = pgm_read_byte ( APD_PrjData + APD_FrameFieldStartAdr[APD_FrameCur][IdField] + 3 );	
					
					// ����� �������� � ���������������
					APD_TextPlcAdr = (pgm_read_byte ( APD_PrjData + APD_FrameFieldStartAdr[APD_FrameCur][IdField] + 4 )) | (pgm_read_byte ( APD_PrjData + APD_FrameFieldStartAdr[APD_FrameCur][IdField] + 5 ) << 8);
					++ APD_TextPlcAdr; // ��������� 1, �.�. � APD_ ����� ����������� �� 1
					
					// ���� �� APD_ ����������� � ������������ ������� ������
					APD_TextIdMin = pgm_read_byte ( APD_PrjData + APD_FrameFieldStartAdr[APD_FrameCur][IdField] + 7 );
					APD_TextIdMax = pgm_read_byte ( APD_PrjData + APD_FrameFieldStartAdr[APD_FrameCur][IdField] + 8 );
					// �� ������ ������� ������ �������
					DataOk = 0;
					//DataOk = 1;
					for (Id = 0; Id <= (FOCUS_DataAmount - 1); Id++) {
						// ���������� ����� � ��������������� (�������� ����) � ���� ��������, ������� ����
						// �������� (������ ������ ���� ���������, � �������� �� ��� ������) � ������� ������ �����
						if ((APD_TextPlcAdr) == FOCUS_DataAdr[Id]) {
							// ���� ������� ����� �������, �� ��������� ��� ������
							IdActual = Id; // ��������� Id
							if (FOCUS_DataStatus[Id]) {
								// ���� �� �������� �� � �������,
								// �� ����� ����� ������ �� �������� ��� ���������� ���������
								APD_TextId = FOCUS_Data[Id];
								
								DataOk = 1;
							} else {
								DataOk = 0;
							}
						}
					}
					// ����� ������ � ����������� �� �������� �������� � �������
					if (((APD_TextId <= APD_TextIdMax) && (APD_TextId >= APD_TextIdMin)) && (DataOk)) {
						for (APD_TextSymbolId = 0; APD_TextSymbolId <= (APD_TextLen - 1); APD_TextSymbolId++) {
							// ���� �� � �������, ������ ����� �� ������� APD_
							LCD_PutSym (APD_TextStr, APD_TextCol + APD_TextSymbolId, APD_PrjFontTable[pgm_read_byte ( APD_PrjData + APD_FrameFieldStartAdr[APD_FrameCur][IdField] + 9 + APD_TextSymbolId + APD_TextId*APD_TextLen )]);
						}
					} else {
						if (FOCUS_DataCntNewStatus[IdActual]) {
							for (APD_TextSymbolId = 0; APD_TextSymbolId <= (APD_TextLen - 1); APD_TextSymbolId++) {
								// ������ �������
								LCD_PutSym (APD_TextStr, APD_TextCol + APD_TextSymbolId, 0x20);
							}
						} else {
							for (APD_TextSymbolId = 0; APD_TextSymbolId <= (APD_TextLen - 1); APD_TextSymbolId++) {
								// ������ ����� '*'
								LCD_PutSym (APD_TextStr, APD_TextCol + APD_TextSymbolId, 0xFF);
							}
						}
					}
				}
				break;

			/************************************************************************/
			/* -> ��� ����: ����													*/
			/************************************************************************/
			case APD_FIELD_TYPE_DATE:
				DATE_ShowStr = pgm_read_byte ( APD_PrjData + APD_FrameFieldStartAdr[APD_FrameCur][IdField] + 1 ) - APD_FrameDisplayShift;
				if ((DATE_ShowStr >= 0) && (Abs (DATE_ShowStr) < (pgm_read_byte ( APD_PrjData + 8 )))) {
					DATE_ShowStr = Abs (DATE_ShowStr);	
					DATE_ShowCol = pgm_read_byte ( APD_PrjData + APD_FrameFieldStartAdr[APD_FrameCur][IdField] + 2 );
					DataShowEn = 1;
					DataAppear = 1;
				}
				break;
				
			/************************************************************************/
			/* -> ��� ����: �����													*/
			/************************************************************************/
			case APD_FIELD_TYPE_TIME_:
				CLOCK_ShowStr = pgm_read_byte ( APD_PrjData + APD_FrameFieldStartAdr[APD_FrameCur][IdField] + 1 ) - APD_FrameDisplayShift;
				if ((CLOCK_ShowStr >= 0) && (Abs (CLOCK_ShowStr) < (pgm_read_byte ( APD_PrjData + 8 )))) {
					CLOCK_ShowStr = Abs (CLOCK_ShowStr);				
					CLOCK_ShowCol = pgm_read_byte ( APD_PrjData + APD_FrameFieldStartAdr[APD_FrameCur][IdField] + 2 );
					ClockShowEn = 1;
					ClockAppear = 1;
				}
				break;

			/************************************************************************/
			/* -> ��� ����: ������													*/
			/************************************************************************/
			case APD_FIELD_TYPE_LIST:
				APD_ListStr = pgm_read_byte ( APD_PrjData + APD_FrameFieldStartAdr[APD_FrameCur][IdField] + 1 ) /* - APD_FrameDisplayShift */;
				// if ((APD_TextStr >= 0) && (Abs (APD_TextStr) < (APD_PrjData[8]))) {
					
				//	APD_TextStr = Abs (APD_TextStr);
				APD_ListCol = pgm_read_byte ( APD_PrjData + APD_FrameFieldStartAdr[APD_FrameCur][IdField] + 2 );
				APD_ListLen = pgm_read_byte ( APD_PrjData + APD_FrameFieldStartAdr[APD_FrameCur][IdField] + 3 );	
					
				// ����� �������� � ���������������
				APD_ListPlcAdr = (pgm_read_byte ( APD_PrjData + APD_FrameFieldStartAdr[APD_FrameCur][IdField] + 4 )) | (pgm_read_byte ( APD_PrjData + APD_FrameFieldStartAdr[APD_FrameCur][IdField] + 5 ) << 8);
				++ APD_ListPlcAdr; // ��������� 1, �.�. � APD_ ����� ����������� �� 1
					
				// ���� �� APD_ ����������� � ������������ ������� ������ � ������
				APD_ListIdMin = pgm_read_byte ( APD_PrjData + APD_FrameFieldStartAdr[APD_FrameCur][IdField] + 7 );
				APD_ListIdMax = pgm_read_byte ( APD_PrjData + APD_FrameFieldStartAdr[APD_FrameCur][IdField] + 8 );
				APD_ListHei = pgm_read_byte ( APD_PrjData + APD_FrameFieldStartAdr[APD_FrameCur][IdField] + 9 );
				
				DataOk = 0;
				for (APD_ListStrId = 0; APD_ListStrId <= (APD_ListHei - 1); APD_ListStrId++) {
					// �� ������ ������� ������ �������
					for (Id = 0; Id <= (FOCUS_DataAmount - 1); Id++) {
						// ���������� ����� � ��������������� (�������� ����) � ���� ��������, ������� ����
						// �������� (������ ������ ���� ���������, � �������� �� ��� ������) � ������� ������ �����
						// LCD_PutValDec (0, 0, 6, APD_ListPlcAdr + APD_List[APD_FrameCur][2] + APD_ListStrId);
						// LCD_PutValDec (0, 7, 6, FOCUS_DataAdr[0]);
						// LCD_PutValDec (1, 7, 6, FOCUS_DataAdr[1]);
						// LCD_PutValDec (2, 7, 6, FOCUS_DataAdr[2]);
						// LCD_PutValDec (3, 7, 6, FOCUS_DataAdr[1]);
						if ((APD_ListPlcAdr + APD_List[APD_FrameCur][2] + APD_ListStrId) == FOCUS_DataAdr[Id]) {
							// LCD_PutSym (2, 0, '*');
							// ���� ������� ����� �������, �� ��������� ��� ������
							IdActual = Id; // ��������� Id
							if (FOCUS_DataStatus[Id]) {
								// ���� �� �������� �� � �������,
								// �� ����� ����� ������ �� �������� ��� ���������� ���������
								APD_ListId = FOCUS_Data[Id];
								DataOk = 1;
							} else {
								DataOk = 0;
							}
						}
					}
					// DataOk = 1;
					// ����� ������ � ����������� �� �������� �������� � �������
					if (((APD_ListId <= APD_ListIdMax) && (APD_ListId >= APD_ListIdMin)) && (DataOk)) {
						for (APD_ListSymbolId = 0; APD_ListSymbolId <= (APD_ListLen - 1); APD_ListSymbolId++) {
							// ���� �� � �������, ������ ����� �� ������� APD_
							LCD_PutSym (APD_ListStr + APD_ListStrId, APD_ListCol + APD_ListSymbolId, APD_PrjFontTable[pgm_read_byte ( APD_PrjData + APD_FrameFieldStartAdr[APD_FrameCur][IdField] + 10 + APD_ListSymbolId + APD_ListId*APD_ListLen )]);
							// LCD_PutSym (2, 0, '*');
						}
					} else {
						if (FOCUS_DataCntNewStatus[IdActual]) {
							for (APD_ListSymbolId = 0; APD_ListSymbolId <= (APD_ListLen - 1); APD_ListSymbolId++) {
								// ������ �������
								LCD_PutSym (APD_ListStr + APD_ListStrId, APD_ListCol + APD_ListSymbolId, 0x20);
							}
						} else {
							for (APD_ListSymbolId = 0; APD_ListSymbolId <= (APD_ListLen - 1); APD_ListSymbolId++) {
								// ������ ����� '*'
								LCD_PutSym (APD_ListStr + APD_ListStrId, APD_ListCol + APD_ListSymbolId, 0xFF);
							}
						}
					}
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

/************************************************************************/
/* -> ������� ����������� ������� � ����		*/
/************************************************************************/
void CLOCK_ShowOn (uint8_t str, uint8_t col) {
	if (TIME_Get ()) {
		CLOCK_Error = 0;
		if ((TIME_GetSeconds & 0x0F)%2) {
			LCD_PutSym (str, col + 2, 0x3A);
			LCD_PutSym (str, col + 5, 0x3A);
		} else {
			LCD_PutSym (str, col + 2, ' ');
			LCD_PutSym (str, col + 5, ' ');	
		}
		LCD_PutValDec (str, col, 2, 10*((TIME_GetHours & 0x30)>>4) + (TIME_GetHours & 0x0F));
		LCD_PutValDec (str, col + 3, 2, 10*((TIME_GetMinutes & 0x70)>>4) + (TIME_GetMinutes & 0x0F));
		LCD_PutValDec (str, col + 6, 2, 10*((TIME_GetSeconds & 0x70)>>4) + (TIME_GetSeconds & 0x0F));
	} else {
		LCD_PutStr (str, col, (uint8_t *)"??:??:??");
		CLOCK_Error = 1;
	}
}

void DATE_ShowOn (uint8_t str, uint8_t col) {
	if (TIME_Get ()) {
		CLOCK_Error = 0;
		LCD_PutSym (str, col + 2, 0x2E);
		LCD_PutSym (str, col + 5, 0x2E);
		LCD_PutValDec (str, col, 2, 10*((TIME_GetDate & 0x30)>>4) + (TIME_GetDate & 0x0F));
		LCD_PutValDec (str, col + 3, 2, 10*((TIME_GetMonth & 0x10)>>4) + (TIME_GetMonth & 0x0F));
		LCD_PutValDec (str, col + 6, 2, 10*((TIME_GetYear & 0xF0)>>4) + (TIME_GetYear & 0x0F));
	} else {
		LCD_PutStr (str, col, (uint8_t *)"??.??.??");
		CLOCK_Error = 1;
	}
}

/************************************************************************/
/* -> ������ �������� ���������											*/
/************************************************************************/
void ADP_TransStart (void) {
	//timeS1 = s_timeout;
	if ((usart_data_register_is_empty (&USARTC0))/* && (ADP_TransMesByteCnt == 0x0000)*/) {	
		ADP_flag = 0;			
		// ������� ������� ����� �������
		//usart_put (&USARTC0, ADP_TransMesBuf[ADP_TransMesByteCnt]);
		//++ ADP_TransMesByteCnt; // ��������� �������� ���� ���������
		// �������� ������ ������� �������� ���������� TX
		//usart_set_tx_interrupt_level (&USARTC0, USART_TXCINTLVL_MED_gc);	
		DMA.CH1.SRCADDR0 = ((volatile)ADP_TransMesBuf+1);
		DMA.CH1.SRCADDR1 = ((volatile)(ADP_TransMesBuf+1))>>8;
		DMA.CH1.SRCADDR2 = 0;
		//DMA.CH0.DESTADDR0 = ((volatile)&(USARTC0.DATA));
		//DMA.CH0.DESTADDR1 = ((volatile)&(USARTC0.DATA))>>8;
		//DMA.CH0.DESTADDR2 = 0;
		//DMA.CH0.ADDRCTRL = DMA_CH_SRCDIR_INC_gc | DMA_CH_DESTDIR_FIXED_gc;
		DMA.CH1.REPCNT = 1;
		DMA.CH1.TRFCNT = ADP_TransMesLen-1;
		DMA.CH1.TRIGSRC = DMA_CH_TRIGSRC_USARTC0_DRE_gc;
		usart_put (&USARTC0, ADP_TransMesBuf[0]);
		DMA.CH1.CTRLA |= DMA_CH_ENABLE_bm;
		while(DMA.CH1.REPCNT) (void)0;
		DMA.CH1.TRIGSRC = 0;
		
		//TIMER_ADP_TIMEOUT_START;
		TIMER_ADP_2MS_START;
			
		//ADP_TransMesByteCnt = 0;
		ADP_TransMesLen = 0;
		ADP_TransMesBody[0] = 0;
		ADP_TransMesBody[1] = 0;
		ADP_MBUS_TransMesBody[0] = 0;
		ADP_MBUS_TransMesBody[1] = 0;
		// ++ ADP_TransMesQueueCnt;
		TIMER_RequestTimeout = 0;
		TIMER_RequestTimeoutEn = 1;
	}
}

/************************************************************************/
/* -> �������� � ������� ������											*/
/************************************************************************/
void ADP_TransContinue (void) {
	/*if ((ADP_TransMesByteCnt < ADP_TransMesLen) && (ADP_TransMesByteCnt)) {
		usart_put (&USARTC0, ADP_TransMesBuf[ADP_TransMesByteCnt]);
		++ ADP_TransMesByteCnt;
	} else if (ADP_TransMesByteCnt >= ADP_TransMesLen) {				
		usart_set_tx_interrupt_level (&USARTC0, USART_TXCINTLVL_OFF_gc);
		ADP_TransMesByteCnt = 0;
		ADP_TransMesLen = 0;
		ADP_TransMesBody[0] = 0;
		ADP_TransMesBody[1] = 0;
		// ++ ADP_TransMesQueueCnt;
		
		TIMER_RequestTimeout = 0;
		TIMER_RequestTimeoutEn = 1;
	}*/
}

/************************************************************************/
/* -> ������ ��������� ��� ��������						*/
/************************************************************************/
uint16_t ADP_TransMesBuild (uint8_t RecAdr, uint8_t TransAdr, uint8_t CmdCode, uint8_t ClarkAdr, uint16_t AmountOfBytes, uint16_t *CmdBody) {
	uint16_t Id = 0;
	ADP_TransMesBuf[0] = RecAdr;
	ADP_TransMesBuf[1] = TransAdr;
	ADP_TransMesBuf[2] = CmdCode;
	ADP_TransMesBuf[3] = ClarkAdr;
	
	ADP_TransMesBuf[4] = AmountOfBytes;
	ADP_TransMesBuf[5] = (AmountOfBytes>>8);
	
	Id = 6;
	while (Id <= (6 + AmountOfBytes - 1)) {
		ADP_TransMesBuf[Id] = (*CmdBody & 0x00FF);
		Id++;
		ADP_TransMesBuf[Id] = ((*CmdBody & 0xFF00)>>8);
		Id++;
		CmdBody++;
	}
	uint16_t MesCrc = 0x0000;
	MesCrc = Crc16 ((uint8_t *)ADP_TransMesBuf, 6 + AmountOfBytes);
	ADP_TransMesBuf[6 + AmountOfBytes] = MesCrc;
	ADP_TransMesBuf[6 + AmountOfBytes + 1] = MesCrc>>8;	
	return (6 + AmountOfBytes + 2);
}

/************************************************************************/
/* -> ������ ��������� ��� ������� ������ �������			*/
/************************************************************************/
uint16_t ADP_TransEventRequest (uint8_t RecAdr, uint8_t TransAdr, uint8_t CmdCode, uint8_t ClarkAdr) {

	ADP_TransMesBuf[0] = RecAdr;
	ADP_TransMesBuf[1] = TransAdr;
	ADP_TransMesBuf[2] = CmdCode;
	ADP_TransMesBuf[3] = ClarkAdr;
	
	ADP_TransMesBuf[4] = 0;
	ADP_TransMesBuf[5] = 0;
	
	uint16_t MesCrc = 0x0000;
	MesCrc = Crc16 ((uint8_t *)ADP_TransMesBuf, 6);
	ADP_TransMesBuf[6] = MesCrc;
	ADP_TransMesBuf[7] = MesCrc>>8;	
	return 8;
}

/************************************************************************/
/* -> ������ ��������� ��� �������� �������� � ��		*/
/************************************************************************/
uint16_t ADP_TransSendToPlcMesBuild (uint8_t RecAdr, uint8_t TransAdr, uint8_t CmdCode, uint8_t ClarkAdr, uint16_t Adress, uint16_t Value) {

	ADP_TransMesBuf[0] = RecAdr;
	ADP_TransMesBuf[1] = TransAdr;
	ADP_TransMesBuf[2] = CmdCode;
	ADP_TransMesBuf[3] = ClarkAdr;
	
	ADP_TransMesBuf[4] = 6;
	ADP_TransMesBuf[5] = 0;

	ADP_TransMesBuf[6] = 2;
	ADP_TransMesBuf[7] = 0;
	
	ADP_TransMesBuf[8] = Adress;
	ADP_TransMesBuf[9] = Adress>>8;
	
	ADP_TransMesBuf[10] = Value;
	ADP_TransMesBuf[11] = Value>>8;
		
	uint16_t MesCrc = 0x0000;
	MesCrc = Crc16 ((uint8_t *)ADP_TransMesBuf, 12);
	ADP_TransMesBuf[12] = MesCrc;
	ADP_TransMesBuf[13] = MesCrc>>8;	
	return 14;
}

/************************************************************************/
/* -> ��������� ��������� �� ���������������			*/
/************************************************************************/
/************************************************************************/
/* -> ������� ����� �� ����������� ���������			*/
/************************************************************************/
void ADP_Function_TransParams (void) {
	uint8_t Id16 = 0;
	uint8_t IdAdr = 0;
	uint16_t RegBuf = 0;
	uint16_t ByteAmount =  (ADP_RecMesBuf[7]<<8) | ADP_RecMesBuf[6];
	uint16_t StartAdr = (ADP_RecMesBuf[9]<<8) | ADP_RecMesBuf[8];
	uint16_t CurAdr = StartAdr;

	uint8_t LCD_RefreshScoreLocal = 0;
	
	if (FOCUS_DataAmount) {
		CurAdr += 1000 * (ADP_RecMesBuf[1] - 0x50);
		for (Id16 = 0; Id16 <= (ByteAmount/2 - 1); ++ Id16) {
			for (IdAdr = 0; IdAdr <=  (FOCUS_DataAmount - 1); IdAdr++) {//APD_MAX_FIELDS
					
				if (FOCUS_DataAdr[IdAdr] == CurAdr) {
					RegBuf = (ADP_RecMesBuf[10 + 2*Id16 + 1]<<8) | (ADP_RecMesBuf[10 + 2*Id16]);
						
					if (RegBuf != FOCUS_Data[IdAdr]) LCD_RefreshScoreLocal++;
						
					FOCUS_Data[IdAdr] = RegBuf;
					FOCUS_DataCntNext[IdAdr] = BROKEN_MES;
				}
			}
			++ CurAdr;
		}
		if (LCD_RefreshScoreLocal) {
			FOCUS_DataMesNew = 1;
			LCD_RefreshScoreLocal = 0;
		}
	}
}

/************************************************************************/
/* -> ������� ����� �� ���������� ���������				*/
/************************************************************************/
void ADP_Function_RecParams (void) {
	uint16_t Adress = 0;
	
	Adress = (ADP_RecMesBuf[9]<<8) | ADP_RecMesBuf[8];

	if (SendToPlc_Amount) {
		for (uint8_t Id = SendToPlc_Amount; Id; -- Id) {
			if (Adress == SendToPlc_Adress[Id - 1]%1000) {
				SendToPlc_Ready[Id - 1] = 0;
			}
		}
	}
	// LCD_PutSym (1, 19, 'A');
}

/************************************************************************/
/* -> ������� ������ �������											*/
/************************************************************************/
void ADP_Function_ReqEvents (void) {
	uint8_t AdrFrom = ADP_RecMesBuf[1] - 0x50;
	uint8_t EventAmount =  ADP_RecMesBuf[6];
	uint8_t EventCode = 0;
		
	// ���� ������� � ������� ���, ������ �� ������
	if ( EVENT_ReqEn ) {
		//LCD_PutValDec(0,0,2,EventAmount);
		for (uint8_t EventId = 0; EventId <= (EventAmount - 1); ++ EventId) {
			EventCode = ADP_RecMesBuf[7 + EventId];
			
			if ( EventStorage[ AdrFrom - 1 ][ EventCode ] > 0 ) {
				
				-- EventStorage[ AdrFrom - 1 ][ EventCode ];
			
				if ( EventStorage[ AdrFrom - 1 ][ EventCode ] == 0) {
					
 					if (EventCode <=  (APD_EventIdAmount - 1)) {/*0x13*/
 						REG_WriteEvent (AdrFrom, EventCode);
 						if ( (Mode == MODE_EVENTS_LIST) || (Mode == MODE_EVENTS_INFO) || (Mode == MODE_EVENTS_SAVE_MENU) ) LCD_Refresh = 1;
 					}

				EventStorage[ AdrFrom - 1 ][ EventCode ] = EventStorage_BOT;
				}
			} else if ( EventStorage[ AdrFrom - 1 ][ EventCode ] < 0 ) {
				
				-- EventStorage[ AdrFrom - 1 ][ EventCode ];
				
// 				if ( EventStorage[ AdrFrom - 1 ][ EventCode ] == 0) {
// 					
// 					EventStorage[ AdrFrom - 1 ][ EventCode ] = EventStorage_TOP;
// 				}
// 			++ EventStorage[ AdrFrom - 1 ][ EventCode ];
// 				
// 			if ( EventStorage[ AdrFrom - 1 ][ EventCode ] == 2 ) {
// 				if (EventCode <= (APD_EventIdAmount - 1)) {
// 					REG_WriteEvent (AdrFrom, EventCode);
// 					if (Mode == MODE_EVENTS_LIST) LCD_Refresh = 1;
// 				}
// 				EventStorage[ AdrFrom - 1 ][ EventCode ] = -1;	
			}
		}
	}
}

/************************************************************************/
/* -> ������ ������� � ������											*/
/************************************************************************/
void REG_WriteEvent (uint8_t From, uint8_t EventCode) {
	// ����� ������ �� EEPROM 1
	uint8_t ReadBuff1[8] = {0, 0, 0, 0, 0, 0, 0, 0};
	// ����� ������ �� EEPROM 2
//	uint8_t ReadBuff2[8] = {0, 0, 0, 0, 0, 0, 0, 0};
	// ����� ������ � EEPROM 1
	uint8_t WriteBuff1[8] = {0, 0, 0, 0, 0, 0, 0, 0};
	// ����� ������ � EEPROM 2
//	uint8_t WriteBuff2[8] = {0, 0, 0, 0, 0, 0, 0, 0};
		
	uint16_t TotMes_ = 0; // ����� ��� �������� �������
	uint16_t NewMes_ = 0; // ����� ��� �������� ����� �������

/************************************************************************/
/* -----> ������ 2�� ���� ��������� �������								*/
/************************************************************************/
	cli();
	EEPROM_AttemptCnt = EEPROM_RW_ATTEMPTS;
	while (EEPROM_AttemptCnt) {
		if (EEPROM_ReadBlock ((REG_HEAD_BEGIN + 1) * REG_BLOCK_SIZE, (uint8_t *) ReadBuff1, REG_BLOCK_SIZE)) break;
		-- EEPROM_AttemptCnt;
	}
	//E_TwiCmdStop ();
	//E_TwiDisable ();
	sei();

/************************************************************************/
/* -----> ������ ������� ������� � �������								*/
/************************************************************************/
	if (!(Crc8 ((uint8_t *) &ReadBuff1, REG_BLOCK_SIZE))) {
		TotMes_ = (ReadBuff1[6]<<8) | ReadBuff1[5];
		NewMes_ = (ReadBuff1[4]<<8) | ReadBuff1[3];
	}

/************************************************************************/
/* -----> ������ ������� �������										*/
/************************************************************************/
	if (TIME_Get ()) {
		WriteBuff1[0] = (10*((TIME_GetHours & 0x30)>>4) + (TIME_GetHours & 0x0F)) | (1<<7);
		WriteBuff1[1] = 10*((TIME_GetMinutes & 0x70)>>4) + (TIME_GetMinutes & 0x0F);
		WriteBuff1[2] = 10*((TIME_GetSeconds & 0x70)>>4) + (TIME_GetSeconds & 0x0F);
		WriteBuff1[3] = 10*((TIME_GetDate & 0x30)>>4) + (TIME_GetDate & 0x0F);
		WriteBuff1[4] = (10*((TIME_GetMonth & 0x10)>>4) + (TIME_GetMonth & 0x0F)) | From<<4;
		WriteBuff1[5] = 10*((TIME_GetYear & 0xF0)>>4) + (TIME_GetYear & 0x0F);
	} else {
		WriteBuff1[0] = 0 | (1<<7);
		WriteBuff1[1] = 0;
		WriteBuff1[2] = 0;
		WriteBuff1[3] = 0;
		WriteBuff1[4] = 0 | From<<4;;
		WriteBuff1[5] = 0;
	}
							
	// ������ �������� �������
	WriteBuff1[6] = EventCode;
	WriteBuff1[7] = Crc8 (WriteBuff1, REG_BLOCK_SIZE - 1);
	
	TotMes_ %= 9999;

/************************************************************************/
/* -----> ���������� ������� � ������ �������							*/
/************************************************************************/
	WP_OFF;
	cli();
	EEPROM_AttemptCnt = EEPROM_RW_ATTEMPTS;
	while (EEPROM_AttemptCnt) {
		if (EEPROM_WriteBlock ((REG_BODY_BEGIN + TotMes_) * REG_BLOCK_SIZE, (uint8_t *) WriteBuff1, REG_BLOCK_SIZE)) break;
		-- EEPROM_AttemptCnt;
	}
	//E_TwiCmdStop ();
	//E_TwiDisable ();
	sei();
	WP_ON;

/************************************************************************/
/* -----> ��������� ��������� �������									*/
/************************************************************************/
/************************************************************************/
/* -----> �������������� ������� ������� � �������						*/
/************************************************************************/
	++ TotMes_;
	++ NewMes_;
/************************************************************************/
/* -----> ������� ���� � ������											*/
/************************************************************************/	
	WriteBuff1[0] = ReadBuff1[0];
	WriteBuff1[1] = ReadBuff1[1];
	WriteBuff1[2] = ReadBuff1[2];
	WriteBuff1[3] = NewMes_; // ������� ����
	WriteBuff1[4] = NewMes_>>8; // ������� ����
	WriteBuff1[5] = TotMes_; // ������� ����
	WriteBuff1[6] = TotMes_>>8; // ������� ����
	WriteBuff1[7] = Crc8 (WriteBuff1, REG_BLOCK_SIZE - 1);

/************************************************************************/
/* -----> ���������� ���������� 2�� ���� ��������� �������				*/
/************************************************************************/	
	WP_OFF;
	cli();
	EEPROM_AttemptCnt = EEPROM_RW_ATTEMPTS;
	while (EEPROM_AttemptCnt) {
		if (EEPROM_WriteBlock ((REG_HEAD_BEGIN + 1) * REG_BLOCK_SIZE, (uint8_t *) WriteBuff1, REG_BLOCK_SIZE)) break;
		-- EEPROM_AttemptCnt;
	}
	//E_TwiCmdStop ();
	//E_TwiDisable ();
	sei();
	WP_ON;
}

/************************************************************************/
/* -> ��������� ������� �������											*/
/************************************************************************/
void REG_Init (void) {
	// ����� ������ �� EEPROM 1
	uint8_t ReadBuff1[8] = {0, 0, 0, 0, 0, 0, 0, 0};
	// ����� ������ �� EEPROM 2
	uint8_t ReadBuff2[8] = {0, 0, 0, 0, 0, 0, 0, 0};

/************************************************************************/
/* -----> ������ ��������� �������										*/
/************************************************************************/
	// ������ ����
	EEPROM_AttemptCnt = EEPROM_RW_ATTEMPTS;
	cli();
	while (EEPROM_AttemptCnt) {
		if (EEPROM_ReadBlock ((REG_HEAD_BEGIN) * REG_BLOCK_SIZE, ReadBuff1, REG_BLOCK_SIZE)) break;
		-- EEPROM_AttemptCnt;
	}
	//E_TwiCmdStop ();
	//E_TwiDisable ();
	// ������ ����
	EEPROM_AttemptCnt = EEPROM_RW_ATTEMPTS;
	while (EEPROM_AttemptCnt) {
		if (EEPROM_ReadBlock ((REG_HEAD_BEGIN + 1) * REG_BLOCK_SIZE, ReadBuff2, REG_BLOCK_SIZE)) break;
		-- EEPROM_AttemptCnt;
	}
	//E_TwiCmdStop ();
	//E_TwiDisable ();
	sei();
	if ((!(Crc8 ((uint8_t *) &ReadBuff1, REG_BLOCK_SIZE))) && (!(Crc8 ((uint8_t *) &ReadBuff2, REG_BLOCK_SIZE)))) {
		
	} else {
		// ���������� ������
		REG_Reset ();
	}
}

/************************************************************************/
/* -> ������ � ���														*/
/************************************************************************/
void DAC_Init (void) {
	DACB.CTRLB = 0b01000000;
	DACB.CTRLC = 0b00001000;
	DACB.CTRLA = 0b00001000;
	// DACB.TIMCTRL = 0b01111100;
	DACB.TIMCTRL = 0b01111100;
	DACB.CTRLA |= (1<<0);

	DACB.CH0DATA = (LCD_Contrast * DAC_per_VOLT) / 10;
	DACB.CH1DATA = (LCD_Contrast * DAC_per_VOLT) / 10;

}
uint8_t g = 0;

void DAC_New (void) {
	DACB.CH0DATA = (LCD_Contrast * DAC_per_VOLT) / 10;
	DACB.CH1DATA = (LCD_Contrast * DAC_per_VOLT) / 10;
}

/************************************************************************/
/*																		*/
/************************************************************************/
/************************************************************************/
/*								MAIN									*/
/************************************************************************/
/************************************************************************/
/*																		*/
/************************************************************************/
//DMA_t DMA_MBUS;
int main (void) {

	uint8_t i=0;
	cli();
/************************************************************************/
/*			��������� DMA		MODBUS_RS485 RMD		*/
/************************************************************************/	
	DMA.CTRL = 0;
	while ( DMA.CTRL & DMA_ENABLE_bm ) (void)0;
	DMA.CTRL = DMA_RESET_bm;
	while ( DMA.CTRL & DMA_RESET_bm ) (void)0;
	DMA.CTRL = DMA_ENABLE_bm;
	DMA.CH3.DESTADDR0 = ((volatile)&(USARTC1.DATA));
	DMA.CH3.DESTADDR1 = ((volatile)&(USARTC1.DATA))>>8;
	DMA.CH3.DESTADDR2 = 0;
	DMA.CH3.ADDRCTRL = DMA_CH_SRCDIR_INC_gc | DMA_CH_DESTDIR_FIXED_gc;
	DMA.CH3.CTRLA = DMA_CH_BURSTLEN_1BYTE_gc | DMA_CH_SINGLE_bm; 
/************************************************************************/
/*			��������� DMA		MODBUS_RS485			*/
/************************************************************************/	
	//DMA.CTRL = 0;
	//while ( DMA.CTRL & DMA_ENABLE_bm ) (void)0;
	//DMA.CTRL = DMA_RESET_bm;
	//while ( DMA.CTRL & DMA_RESET_bm ) (void)0;
	//DMA.CTRL = DMA_ENABLE_bm;
	DMA.CH2.DESTADDR0 = ((volatile)&(USARTE1.DATA));
	DMA.CH2.DESTADDR1 = ((volatile)&(USARTE1.DATA))>>8;
	DMA.CH2.DESTADDR2 = 0;
	DMA.CH2.ADDRCTRL = DMA_CH_SRCDIR_INC_gc | DMA_CH_DESTDIR_FIXED_gc;
	DMA.CH2.CTRLA = DMA_CH_BURSTLEN_1BYTE_gc | DMA_CH_SINGLE_bm;
	//DMA.CH2.CTRLB = DMA_CH_TRNINTLVL0_bm;
/************************************************************************/
/*				��������� DMA		MODBUS_RS232			*/
/************************************************************************/	
	//DMA.CTRL = 0;
	//while ( DMA.CTRL & DMA_ENABLE_bm ) (void)0;
	//DMA.CTRL = DMA_RESET_bm;
	//while ( DMA.CTRL & DMA_RESET_bm ) (void)0;
	//DMA.CTRL = DMA_ENABLE_bm;
	DMA.CH0.DESTADDR0 = ((volatile)&(USARTF0.DATA));
	DMA.CH0.DESTADDR1 = ((volatile)&(USARTF0.DATA))>>8;
	DMA.CH0.DESTADDR2 = 0;
	DMA.CH0.ADDRCTRL = DMA_CH_SRCDIR_INC_gc | DMA_CH_DESTDIR_FIXED_gc;
	DMA.CH0.CTRLA = DMA_CH_BURSTLEN_1BYTE_gc | DMA_CH_SINGLE_bm; 
/************************************************************************/
/*					��������� DMA		ROMBUS					*/
/************************************************************************/	
	DMA.CH1.DESTADDR0 = ((volatile)&(USARTC0.DATA));
	DMA.CH1.DESTADDR1 = ((volatile)&(USARTC0.DATA))>>8;
	DMA.CH1.DESTADDR2 = 0;
	DMA.CH1.ADDRCTRL = DMA_CH_SRCDIR_INC_gc | DMA_CH_DESTDIR_FIXED_gc;
	DMA.CH1.CTRLA = DMA_CH_BURSTLEN_1BYTE_gc | DMA_CH_SINGLE_bm; 
/************************************************************************/
/*				��������� ������� � �������� ������ ������				*/
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
/*						��������� ������������ �������					*/
/************************************************************************/
	sysclk_enable_module (SYSCLK_PORT_C, SYSCLK_USART0);//ROMBUS
	sysclk_enable_module (SYSCLK_PORT_E, SYSCLK_USART0);//RMD
	sysclk_enable_module (SYSCLK_PORT_F, SYSCLK_USART0);//MODBUS RS_232
	sysclk_enable_module (SYSCLK_PORT_C, SYSCLK_USART1);//MODBUS RMD
	sysclk_enable_module (SYSCLK_PORT_E, SYSCLK_USART1);//MODBUS RS_485
	sysclk_enable_module (SYSCLK_PORT_C, SYSCLK_TWI);
	sysclk_enable_module (SYSCLK_PORT_E, SYSCLK_TWI);
	sysclk_enable_module (SYSCLK_PORT_C, SYSCLK_TC0);//������ ��� 1 �� ���������
	sysclk_enable_module (SYSCLK_PORT_C, SYSCLK_TC1);//������ ��� ��������� ��������� ������ ROMBUS
	sysclk_enable_module (SYSCLK_PORT_D, SYSCLK_TC0);//������ ��� MODBUS
	sysclk_enable_module (SYSCLK_PORT_D, SYSCLK_TC1);//������ ��� MODBUS
	sysclk_enable_module (SYSCLK_PORT_E, SYSCLK_TC1);//������ ��� MODBUS		
	sysclk_enable_module (SYSCLK_PORT_E, SYSCLK_TC0);//������ ��� ������������ �������� ������ �� MODBUS
	sysclk_enable_module (SYSCLK_PORT_F, SYSCLK_TC0);//������ ��� �������� ��������� ������ ����� ����������� ROMBUS
	sysclk_enable_module (SYSCLK_PORT_B, SYSCLK_DAC);
/************************************************************************/
/*							������ ������� APD							*/
/************************************************************************/
// �������� ����������� ������� APD_
if ((pgm_read_byte ( APD_PrjData + 0 ) == 0x41) && (pgm_read_byte ( APD_PrjData + 1 ) == 0x44)) {
	PRJ_Error = 0;
} else {
	PRJ_Error = 1;
}

uint8_t Check = 0;

/************************************************************************/
/* >> ����� ������� ������ ������ >>									*/
/************************************************************************/
Check = eeprom_read_byte ((uint8_t *) &SETTINGS_CyckleSaveFlag);
if (Check) {
	APD_FrameRefreshRate = eeprom_read_byte ((uint8_t *) &SETTINGS_CyckleSave)*100;
} else {
	if (pgm_read_byte ( APD_PrjData + 0x000B )) {
		APD_FrameRefreshRate = pgm_read_byte ( APD_PrjData + 0x000B )*100; // ������ ������ ������ (0-����,1-100��,2-200,3...10)
	} else {
		APD_FrameRefreshRate = 2000;
	}
}
Check = 0;
TIMER_DataRequestPer = APD_FrameRefreshRate;
// TIMER_DataRequestPer = 200;

/************************************************************************/
/* >> ����� ��������� ������ ������ ������� >>	*/
/************************************************************************/
Check = eeprom_read_byte ((uint8_t *) &SETTINGS_RateSaveFlag);
if (Check) {
	APD_EventReqRatio = eeprom_read_byte ((uint8_t *) &SETTINGS_RateSave);
} else {
	if ( ( pgm_read_byte ( APD_PrjData + 0x000C ) > 0 ) && ( pgm_read_byte ( APD_PrjData + 0x000C ) < 15 ) ) {
		APD_EventReqRatio = pgm_read_byte ( APD_PrjData + 0x000C ); // ������ ������ ������� (0-����,1-100��,2-200,3...10)
	} else {
		APD_EventReqRatio = 15;
	}
}
Check = 0;
// APD_EventReqRatio %= 20;
ADP_EventReqRatioCnt = 1;
/************************************************************************/
/* >> ����� �������� ������ RS-232 >>						*/
/************************************************************************/
 /*����� �������� ������ � ���
 switch (pgm_read_byte ( APD_PrjData + 0x000A )) {
 	case 0:
 		APD_BaudRateIndex = 9600;
 		break;
 	case 1:
 		APD_BaudRateIndex = 19200;
 		break;
 	case 2:
 		APD_BaudRateIndex = 38400;
 		break;	
 	case 3:
 		APD_BaudRateIndex = 57600;
 		break;
 	default:
 		APD_BaudRateIndex = 9600;
 		break;
 }*/

//Check = eeprom_read_byte ((uint8_t *) &SETTINGS_BaudFlag);
//if (Check) {
	//APD_BaudRateIndex = eeprom_read_byte ((uint8_t *) &SETTINGS_BaudSave);
//} else {
	//APD_BaudRateIndex = pgm_read_byte ( APD_PrjData + 0x000A );
//}
//Check = 0;
//APD_BaudRateIndex = 3;

// ���������� ������ � �������
APD_FrameAmount = pgm_read_byte ( APD_PrjData + 0x0010 );
// ����� ���������� �����
APD_FrameStartNum = pgm_read_byte ( APD_PrjData + 0x0011 );
APD_FrameCur = APD_FrameStartNum; // ������� ���� = ��������� ���� �������
APD_FramesStartAdr = (pgm_read_byte ( APD_PrjData + 0x0014 ) << 8) | (pgm_read_byte ( APD_PrjData + 0x0013 )); // ����� �������� ������
APD_EventsStartAdr = (pgm_read_byte ( APD_PrjData + 0x0016 ) << 8) | (pgm_read_byte ( APD_PrjData + 0x0015 )); // ����� �������� ������ �������
// ���������� �������� ������ �������
APD_EventIdAmount = (pgm_read_byte ( APD_PrjData + APD_EventsStartAdr + 1 )<< 8) | (pgm_read_byte ( APD_PrjData + APD_EventsStartAdr ));
// APD_EventIdAmount = 0;

if (APD_EventIdAmount) {	
	// ����� ������ ������ ������� � ���
	APD_EventAdr = (pgm_read_byte ( APD_PrjData + APD_EventsStartAdr + 3 ) << 8) | (pgm_read_byte ( APD_PrjData + APD_EventsStartAdr + 2 ));
	APD_EventAdr += 1;
	
	if (APD_EventAdr == 1) {
		EVENT_ReqEn = 0;
	} else {
		if (APD_EventAdr < 5000) {
			EVENT_ReqEn = 0;
		} else {
			Check = eeprom_read_byte ((uint8_t *) &SETTINGS_NumSaveFlag);
			if (Check) {
				for (uint8_t AdrId = 0; AdrId <= 3; ++ AdrId) {
					APD_EventAdrArray[AdrId] = eeprom_read_byte ((uint8_t *) SETTINGS_AdrSave + AdrId);
				}
				APD_EventAdrNum = eeprom_read_byte ((uint8_t *) &SETTINGS_NumSave);
				Check = 0;
			} else {
				// ������ ������� ���������,
				// � ������� ����� ������������� ������ �������
				APD_EventAdrArray[0] = (APD_EventAdr % 1000) / 100;		
				AdrTemp = APD_EventAdr % 10;
				APD_EventAdrNum = AdrTemp - APD_EventAdrArray[0] + 1;
				if (APD_EventAdrNum) {
					for (uint8_t AdrId = 1; AdrId <= (APD_EventAdrNum - 1); ++ AdrId) {
						APD_EventAdrArray[AdrId] = APD_EventAdrArray[AdrId - 1] + 1;
					}
				}
			}
			EVENT_ReqEn = 1;
		}
	}
} else {
	EVENT_ReqEn = 0;
}

// ���������� ������� ������� �������� ������:
// ����� ������� ������ - ����� �������� ������. ����� ������ �����������
// �������� � ������������ � ����������� ������ � �������
for (IdFrame = 0; IdFrame <= (APD_FrameAmount - 1); IdFrame++) {
	APD_FrameStartAdr[IdFrame] |= pgm_read_byte ( APD_PrjData + APD_FramesStartAdr + Id8 ); // ������� ����
	Id8++;
	APD_FrameStartAdr[IdFrame] |= (pgm_read_byte ( APD_PrjData + APD_FramesStartAdr + Id8 ) << 8); // ������� ����
	Id8++;
}
Id8 = 0;

// ���������� ������� �������� ����� � �������� � �����:
// ����� ������� ������ - ����� �� ������� ������� �������� ������. ����� ������ �����������
// �������� � ������������ � ����������� ������ � �������.
// � ������� ������ - ����� �������� ����� �����, � ������ - ����� ��������
// �������������� �����.
for (IdFrame = 0; IdFrame <= (APD_FrameAmount - 1); IdFrame++) {
	// ����� �������� ����� �����
	APD_FrameFieldsAndActionsStartAdr[IdFrame][0] |= pgm_read_byte ( APD_PrjData + APD_FrameStartAdr[IdFrame] + Id8 ); // ������� ����
	Id8++;
	APD_FrameFieldsAndActionsStartAdr[IdFrame][0] |= (pgm_read_byte ( APD_PrjData + APD_FrameStartAdr[IdFrame] + Id8 ) << 8); // ������� ����
	Id8++;
	// ����� �������� �������������� �����
	APD_FrameFieldsAndActionsStartAdr[IdFrame][1] |= pgm_read_byte ( APD_PrjData + APD_FrameStartAdr[IdFrame] + Id8 ); // ������� ����
	Id8++;
	APD_FrameFieldsAndActionsStartAdr[IdFrame][1] |= (pgm_read_byte ( APD_PrjData + APD_FrameStartAdr[IdFrame] + Id8 ) << 8); // ������� ����
	Id8++;
	
	Id8 = 0;
}
// ��������� ������ ���������� ����� � �������������� ������� ����� 
for (IdFrame = 0; IdFrame <= (APD_FrameAmount - 1); IdFrame++) {
	APD_FrameFieldsAmount[IdFrame] = pgm_read_byte ( APD_PrjData + APD_FrameFieldsAndActionsStartAdr[IdFrame][0] );
	APD_FrameActionsAmount[IdFrame] = pgm_read_byte ( APD_PrjData + APD_FrameFieldsAndActionsStartAdr[IdFrame][1] );
}	

Id8 = 0;
// ��������� ������ ������� ����� ������� �����
for (IdFrame = 0; IdFrame <= (APD_FrameAmount - 1); IdFrame++) {
	for (IdField = 0; IdField <= APD_FrameFieldsAmount[IdFrame] - 1; IdField++) {
		APD_FrameFieldStartAdr[IdFrame][IdField] |= pgm_read_byte ( APD_PrjData + APD_FrameFieldsAndActionsStartAdr[IdFrame][0] + 1 + Id8 );
		Id8++;
		APD_FrameFieldStartAdr[IdFrame][IdField] |= (pgm_read_byte ( APD_PrjData + APD_FrameFieldsAndActionsStartAdr[IdFrame][0] + 1 + Id8 ) << 8);
		Id8++;
	}
	Id8 = 0;
}	

Id8 = 0;
// ��������� ������ ������� �������������� ������� �����
for (IdFrame = 0; IdFrame <= (APD_FrameAmount - 1); IdFrame++) {
	for (IdAction = 0; IdAction <= APD_FrameActionsAmount[IdFrame] - 1; IdAction++) {
		APD_FrameActionStartAdr[IdFrame][IdAction] |= pgm_read_byte ( APD_PrjData + APD_FrameFieldsAndActionsStartAdr[IdFrame][1] + 1 + Id8 );
		Id8++;
		APD_FrameActionStartAdr[IdFrame][IdAction] |= (pgm_read_byte ( APD_PrjData + APD_FrameFieldsAndActionsStartAdr[IdFrame][1] + 1 + Id8 ) << 8);
		Id8++;
	}
	Id8 = 0;
}

Id8 = 0;
uint8_t APD_FrameDataFromPlcCnt = 0;
// ����� ������ ������������� �� ��� � ������ ����� � ������� ��?
for (IdFrame = 0; IdFrame <= (APD_FrameAmount - 1); IdFrame++) {
	if (APD_FrameFieldsAmount[IdFrame] > 1) {
		for (IdField = 0; IdField <= (APD_FrameFieldsAmount[IdFrame] - 1); IdField++) {
			// ���� ��� ���� - ����� ��� �����:
			if ((pgm_read_byte ( APD_PrjData + APD_FrameFieldStartAdr[IdFrame][IdField] ) == APD_FIELD_TYPE_VALUE) || (pgm_read_byte ( APD_PrjData + APD_FrameFieldStartAdr[IdFrame][IdField] ) == APD_FIELD_TYPE_TEXT)) {
				// ��������� ��� ����� � ��� � ������ 16 ���:
				// ������� ����
				APD_FrameDataFromPlcAdr[IdFrame][APD_FrameDataFromPlcCnt] = pgm_read_byte ( APD_PrjData + APD_FrameFieldStartAdr[IdFrame][IdField] + 4 /* 4 + Id8 */ );
				// ������� ����
				APD_FrameDataFromPlcAdr[IdFrame][APD_FrameDataFromPlcCnt] |= (pgm_read_byte ( APD_PrjData + APD_FrameFieldStartAdr[IdFrame][IdField] + 5 /* 4 + Id8 */ ) << 8);
				// ���������� ����������� ����� � APD_ (��������� 1)
				++ APD_FrameDataFromPlcAdr[IdFrame][APD_FrameDataFromPlcCnt];
				// ���������� ������ ����������� ���������� �� �������
				APD_FrameDataFromPlcStr[IdFrame][APD_FrameDataFromPlcCnt] = pgm_read_byte ( APD_PrjData + APD_FrameFieldStartAdr[IdFrame][IdField] + 1 );
				// ������� ���������� ����� ���� ����� � �����
				++ APD_FrameDataFromPlcCnt;
				//���� ���������� 32-��� ����������, ��������� � � ������ ��� ��� ���� 16-���
				if ((pgm_read_byte ( APD_PrjData + APD_FrameFieldStartAdr[IdFrame][IdField]  + 6) == APD_FIELD_TYPE_32BIT))
				{
					APD_FrameDataFromPlcAdr[IdFrame][APD_FrameDataFromPlcCnt] = APD_FrameDataFromPlcAdr[IdFrame][APD_FrameDataFromPlcCnt-1]+1;

					//++ APD_FrameDataFromPlcAdr[IdFrame][APD_FrameDataFromPlcCnt];
					APD_FrameDataFromPlcStr[IdFrame][APD_FrameDataFromPlcCnt] = APD_FrameDataFromPlcStr[IdFrame][APD_FrameDataFromPlcCnt-1];
					++ APD_FrameDataFromPlcCnt;
				}
			} else if (pgm_read_byte ( APD_PrjData + APD_FrameFieldStartAdr[IdFrame][IdField] ) == APD_FIELD_TYPE_LIST) {
				/************************************************************************/
				/*   ���� ����� ���� ���� ������ ---> ��������� � ����� ������ ������	*/
				/************************************************************************/
				// ���������� �����, ������ ���������� ������ ���� ���� ������ � ������ �����
				APD_List[IdFrame][0]	= APD_FrameDataFromPlcCnt;
				// ���������� ������ ���� ���� ������ (���������� ��, ��� ����� ����� �� �������)
				APD_List[IdFrame][1] = pgm_read_byte ( APD_PrjData + APD_FrameFieldStartAdr[IdFrame][IdField] + 9 );
				// ��������� !���������! ����� ������ � ����������� ������:
				// ������� ����
				APD_ListAdress[IdFrame] = pgm_read_byte ( APD_PrjData + APD_FrameFieldStartAdr[IdFrame][IdField] + 4 );
				// ������� ����
				APD_ListAdress[IdFrame] |= (pgm_read_byte ( APD_PrjData + APD_FrameFieldStartAdr[IdFrame][IdField] + 5 ) << 8);
				// ���������� ����������� ����� � APD_ (��������� 1)
				++ APD_ListAdress[IdFrame];	
								
				/************************************************************************/
				/* -> ���������� ������ ������, ������� ����� ����� �����������			*/
				/************************************************************************/
				for (uint8_t ListAdrId = 0; ListAdrId <= (pgm_read_byte ( APD_PrjData + APD_FrameFieldStartAdr[IdFrame][IdField] + 9 ) - 1); ++ ListAdrId) {
					// ��������� ����� � ������ 16 ���:
					// ������� ����
					APD_FrameDataFromPlcAdr[IdFrame][APD_FrameDataFromPlcCnt] = pgm_read_byte ( APD_PrjData + APD_FrameFieldStartAdr[IdFrame][IdField] + 4 );
					// ������� ����
					APD_FrameDataFromPlcAdr[IdFrame][APD_FrameDataFromPlcCnt] |= (pgm_read_byte ( APD_PrjData + APD_FrameFieldStartAdr[IdFrame][IdField] + 5 ) << 8);
					// ����� = [������� ������ + ���������� ������� � ������ (�� 0 �� [������_������ - 1])					
					// ���������� ����������� ����� � APD_ (��������� 1)
					APD_FrameDataFromPlcAdr[IdFrame][APD_FrameDataFromPlcCnt] += /* APD_List[IdFrame][2] */ + ListAdrId + 1;
					// ���������� ������ ����������� ���������� �� ������� (����� ������ ������� � �����)
					APD_FrameDataFromPlcStr[IdFrame][APD_FrameDataFromPlcCnt] = pgm_read_byte ( APD_PrjData + APD_FrameFieldStartAdr[IdFrame][IdField] + 1 );
									
					++ APD_FrameDataFromPlcCnt;
				}
			}
		}
		// ���������� ��������� ���������� ����� ���� �����, ����� ��� ������
		APD_FrameDataFromPlcAmount[IdFrame] = APD_FrameDataFromPlcCnt;
		APD_FrameDataFromPlcCnt = 0;
		// Id8 = 0;	
	} else {
		// ���� ���� � ����� ������ ����, ������ ��� ���� ���� ��������
		// ��� �� �������
		APD_FrameDataFromPlcAmount[IdFrame] = 0;
	}
}

/************************************************************************/
/*							��������� UARTE0			RMD_keyboard	*/
/************************************************************************/
// ��������� PIN3 �� ����� (��������)
	PORTE.DIR |= TX0;
//	PORTE.OUT |= TX0;
// ��������� PIN2 �� ���� (����)
	PORTE.DIR &= ~RX0;
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
/*						��������� UARTE1			RS485(MODBUS)		*/
/************************************************************************/
// ��������� PIN3 �� ����� (��������)
PORTE.DIR |= TX2;
PORTE.DIR |= TX_EN;	
PORTE.OUT &= ~TX_EN;
// ��������� PIN2 �� ���� (����)
	PORTE.DIR &= ~ RX2;
// ��������� PIN1 �� ����� (TX_enable)
PORTE.OUT |= TX2;
// ��������� �������� ������
	usart_set_baudrate (&USARTE1, /*CONFIG_USART_BAUDRATE */  38400 /* 9600 */, /*BOARD_XOSC_HZ*/ 18432000);
////	USARTE0.BAUDCTRLA = 119;
//// ����� ������ ������ USARTE0
	usart_set_mode (&USARTE1, USART_CMODE_ASYNCHRONOUS_gc);
//// ��������� ������� �������
	usart_format_set (&USARTE1, USART_CHSIZE_8BIT_gc, USART_PMODE_DISABLED_gc, false);
//// ��������� ��������	
	usart_rx_enable (&USARTE1);
//// ��������� �����������
	usart_tx_enable (&USARTE1);
//// ������ ����������
	usart_set_rx_interrupt_level (&USARTE1, USART_RXCINTLVL_MED_gc);
	usart_set_tx_interrupt_level (&USARTE1,USART_TXCINTLVL_MED_gc);
/************************************************************************/
/*						��������� UARTC0			ROMBUS				*/
/************************************************************************/
// ��������� PIN3 �� ����� (��������)
	PORTC.DIR |= TX0;
//	PORTC.OUT |= TX0;
// ��������� PIN2 �� ���� (����)
	PORTC.DIR &= ~ RX0;
// ��������� �������� ������
	usart_set_baudrate (&USARTC0, /*CONFIG_USART_BAUDRATE*/ SETTINGS_BaudValue[APD_BaudRateIndex], /*BOARD_XOSC_HZ*/ 18432000);
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
/*							��������� UARTF0		MODBUS				*/
/************************************************************************/
// ��������� PIN3 �� ����� (��������)
	PORTF.DIR |= TX0;
// ��������� PIN2 �� ���� (����)
	PORTF.DIR &= ~ RX0;
// ��������� �������� ������
	usart_set_baudrate (&USARTF0, 38400/*SETTINGS_BaudValue[APD_BaudRateIndex]*/, /*BOARD_XOSC_HZ*/ 18432000);
// ����� ������ ������ USARTC0	
	usart_set_mode (&USARTF0, USART_CMODE_ASYNCHRONOUS_gc);
// ��������� ������� �������
	usart_format_set (&USARTF0, USART_CHSIZE_8BIT_gc, USART_PMODE_DISABLED_gc, false);
// ��������� ��������	
	usart_rx_enable (&USARTF0);
// ��������� �����������	
	usart_tx_enable (&USARTF0);
// ������ ����������
	usart_set_rx_interrupt_level (&USARTF0, USART_RXCINTLVL_MED_gc);
/************************************************************************/
/*							��������� UARTC1		MODBUS_RMD			*/
/************************************************************************/
// ��������� PIN3 �� ����� (��������)

	PORTC.DIR |= TX2;
// ��������� PIN2 �� ���� (����)
	PORTC.DIR &= ~ RX2;
// ��������� �������� ������
	usart_set_baudrate (&USARTC1, 9600/*SETTINGS_BaudValue[APD_BaudRateIndex]*/, /*BOARD_XOSC_HZ*/ 18432000);
// ����� ������ ������ USARTC0	
	usart_set_mode (&USARTC1, USART_CMODE_ASYNCHRONOUS_gc);
// ��������� ������� �������
	usart_format_set (&USARTC1, USART_CHSIZE_8BIT_gc, USART_PMODE_DISABLED_gc, false);
// ��������� ��������	
	usart_rx_enable (&USARTC1);
// ��������� �����������	
	usart_tx_enable (&USARTC1);
// ������ ����������
	usart_set_rx_interrupt_level (&USARTC1, USART_RXCINTLVL_MED_gc);
/************************************************************************/
/*						�������� ������� �� EEPROM						*/
/************************************************************************/
LCD_Contrast = (uint8_t) eeprom_read_byte ((uint8_t *) &CONTRAST_Save);
TIMER_RequestTimeoutPer = (uint8_t) eeprom_read_byte ((uint8_t *) &SETTINGS_DelaySave);
EventStorage_TOP = (uint8_t) eeprom_read_byte ((uint8_t *) &EventStorage_TOP_Save);
EventStorage_BOT = (- 1) * (uint8_t) eeprom_read_byte ((uint8_t *) &EventStorage_BOT_Save);
/************************************************************************/
/*							��������� �������							*/
/************************************************************************/
	PORTA.DIR |= LED;
	PORTA.OUT |= LED;
	PORTB.DIR |= EN | RS;
	EN_SET;
	LCD_Init ();
	LCD_UserDefinedSymbols ();
	DAC_Init ();
	DAC_New ();
/************************************************************************/
/*							��������� �������							*/
/************************************************************************/
	TIMER_1MS_INIT;
	TIMER_RS232_INIT;
	TIMER_MBUS485_INIT;
	TIMER_MBUS232_INIT;
	TIMER_MBUSRMD_INIT;
	TIMER_ADP_TIMEOUT_INIT;
	TIMER_ADP_TIMEOUT_START;
	TIMER_ADP_2MS_INIT;
/************************************************************************/
/*							��������� I2C (PORTC - RTC)					*/
/************************************************************************/
	C_TwiInit ();
/************************************************************************/
/*						��������� �������						*/
/************************************************************************/
	if (TIME_Get ()) {
		CLOCK_Error = 0;
	} else {
		CLOCK_Error = 1;
	}
/************************************************************************/
/*						��������� I2C (PORTE - EEPROM)				*/
/************************************************************************/	
	E_TwiInit ();
/************************************************************************/
/* -> ��������� ������� �������							*/
/************************************************************************/
	REG_Init ();
/************************************************************************/
/*						������						*/
/************************************************************************/
	MB0_State.lenRx = 0;
	MB0_State.Buf_byte = 0;
	MB0_State.CRC16_OK = 0;
	//MB0_State.Error_count = 0;
	MB0_State.Error_CRC = 0;
	MB0_State.Error_Data = 0;
	MB0_State.Error_MBUS = 0;
	MB0_State.flags = 0;
	MB0_State.MBUS_TimeAns = 0;
	MB0_State.MBUS_TimeAnsMAX = 0;
	MBUS_Init_Reg();
	
	numADPmes = 0;
	nu=0;
	ADP_Rec_CRC_Error=0;
	ADP_flag = 1;
	//MB0_State.timeout = 0;
	FOCUS_Adr ();
	FOCUS_Sort (FOCUS_DataAdr, FOCUS_DataAmount);
	FOCUS_Space ();
	FOCUS_Block ();
	FOCUS_CntReset ();
	
	uint8_t LCD_RefreshScore = 0;
	EventStorageToTOP (EventStorage_TOP);
	sei();
	while (1) {
		if (TIMER_RegSave >= TIMER_RegSavePer) { // ���� �������� ����������� ������
			TIMER_RegSave = 0;
			
			if (REGSAVE_MesCnt <= REGSAVE_MesLim) {

			/************************************************************************/
			/* ������ ������														*/
			/************************************************************************/
				cli();
				EEPROM_AttemptCnt = EEPROM_RW_ATTEMPTS;
				while (EEPROM_AttemptCnt) {
					if (EEPROM_ReadBlock ((REG_BODY_BEGIN + REGSAVE_MesCnt) * REG_BLOCK_SIZE, REGSAVE_Buff, REG_BLOCK_SIZE)) break;
					-- EEPROM_AttemptCnt;
				}
				//E_TwiCmdStop ();
				//E_TwiDisable ();
				sei();
				
				if ((usart_data_register_is_empty (&USARTE0)) && (REGSAVE_MesByteCnt == 0)) {
					// ������� ������� ������� ���������
					usart_put (&USARTE0, * (REGSAVE_Buff + REGSAVE_MesByteCnt/* - 1*/));
					++ REGSAVE_MesByteCnt; // ��������� �������� �������� ��������
					// �������� ������ ������� �������� ���������� TX
					usart_set_tx_interrupt_level (&USARTE0, USART_TXCINTLVL_MED_gc);
					LCD_Refresh = 1;
				}
			} else {
				// ��������� ������ �������� �������
 				TIMER_RegSaveEn = 0;
 				TIMER_RegSave = 0;
				
				// �������� ������� �������
 				TIMER_DataRequestEn = 1;
 				TIMER_DataRequest = 0;
 				TIMER_RequestTimeoutEn = 1;
 				TIMER_RequestTimeout = 0;
// 				DATA_ReqEn = 1;
 				EVENT_ReqEn = 1;
				 
				NET_Mode = NET_MODE_MASTER;
				Mode = MODE_EVENTS_SAVE_COMPLETED;
				LCD_Refresh = 1;
			
			}
		}
		/************************************************************************/
		/*					��������� ������������ ����������� MODBUS			*/
		/************************************************************************/
		if (TIMER_RMDCfgMBUS >= TIMER_RMDCfgPerMBUS) { // ���� �������� ����������� ������
			TIMER_RMDCfgMBUS = 0;
						
			switch (RMD_Config_STATEMBUS) 
			{
	// START - ��������� ������� ������������
				case START:
					/*�������� ��� ���������� ������������ rmd*/
					RMD_Config_STATEMBUS = END;
					RMD_Config_ERRORMBUS = 0;
					break;
					/************************/
					switch (NewBaudRateMBUS) {
						case 38:
							RMD_CfgMBUS [9][5] = '3';
							RMD_CfgMBUS [9][6] = '8';
							RMD_CfgMBUS [10][5] = '3';
							RMD_CfgMBUS [10][6] = '8';
							break;
						
						case 57:
							RMD_CfgMBUS [9][5] = '5';
							RMD_CfgMBUS [9][6] = '7';
							RMD_CfgMBUS [10][5] = '5';
							RMD_CfgMBUS [10][6] = '7';
							break;
							
						case 11:
							RMD_CfgMBUS [9][5] = '1';
							RMD_CfgMBUS [9][6] = '1';
							RMD_CfgMBUS [10][5] = '1';
							RMD_CfgMBUS [10][6] = '1';
							break;
						
						default:
							break;
					}
					RMD_CfgCmdCharCntMBUS = 0;
					RMD_CfgCmdCntMBUS = 0;
					RMD_CfgCmdAmountMBUS = 11;
					RMD_CfgCmdAckMBUS = 1;
					
					RMDMBUS_RESET_ON;
					
					RMD_Config_STATEMBUS = SEND;
					break;
					
// CHECK - �������� ������ �� ������
				case CHECK:
					if (RMD_CfgAckMesReadyMBUS) {
						RMD_CfgAckMesReadyMBUS = 0;
						if ((RMD_CfgAckMesMBUS[0] == 0x0D) && (RMD_CfgAckMesMBUS[1] == 0x0A) && (RMD_CfgAckMesMBUS[2] == 0x4F) && (RMD_CfgAckMesMBUS[3] == 0x4B) && (RMD_CfgAckMesMBUS[4] == 0x0A) && (RMD_CfgAckMesMBUS[5] == 0x0D) && (RMD_CfgAckMesMBUS[6] == 0x3E)) {
							RMD_CfgAckMesMBUS[0] = 0;
							RMD_CfgAckMesMBUS[1] = 0;
							RMD_CfgAckMesMBUS[2] = 0;
							RMD_CfgAckMesMBUS[3] = 0;
							RMD_CfgAckMesMBUS[4] = 0;
							RMD_CfgAckMesMBUS[5] = 0;
							RMD_CfgAckMesMBUS[6] = 0;
							RMD_CfgCmdAckMBUS = 1;	// ���� ������� ����� "��"
						} else {
							RMD_CfgCmdAckMBUS = 0;	// ���� �� �������� ������, ��� ����� �� "��"
						}
					}
					RMD_Config_STATEMBUS = SEND;
					break;

// SEND - ������� ����������������� ���������
				case SEND:
					if (RMD_CfgCmdAckMBUS) {
						if (RMD_CfgCmdCntMBUS <= (RMD_CfgCmdAmountMBUS - 1)) {
							if ((usart_data_register_is_empty (&USARTC1)) && (RMD_CfgCmdCharCntMBUS == 0) && (RMD_CfgCmdAckMBUS)) {
								RMD_CfgCmdAckMBUS = 0;	// ������������ ���� ������������ ��������
								// ������� ������� ������� ��������
								usart_put (&USARTC1, RMD_CfgMBUS[RMD_CfgCmdCntMBUS][RMD_CfgCmdCharCntMBUS + 1]);
								//if (Mode == MODE_INTRO) LCD_PutStrNum (3, 8, RMD_CfgCmdCntMBUS, (uint8_t *) "...................");
								//if (Mode == MODE_TRANSP_PROGRESS) LCD_PutStrNum (3, 0, RMD_CfgCmdCntMBUS, (uint8_t *) "...................");
							
								++ RMD_CfgCmdCharCntMBUS; // ��������� �������� �������� ��������
								// �������� ������ ������� �������� ���������� TX
								usart_set_tx_interrupt_level (&USARTC1, USART_TXCINTLVL_MED_gc);
							}
							RMD_Config_STATEMBUS = CHECK;
						} else {
							RMD_Config_ERRORMBUS = 0;
							RMD_Config_STATEMBUS = END;
						}
					} else {
						RMD_Config_ERRORMBUS = 1;
						RMD_Config_STATEMBUS = END;
					}
					break;

// END - ����� �� ������������	
				case END:
					RMDMBUS_RESET_OFF; // ������ �������� ������ � ���������� ����� ������	

					TIMER_RMDCfgEn_MBUS = 0;
					TIMER_RMDCfgMBUS = 0;
			
					RMD_CfgCmdCntMBUS = 255;
					RMD_CfgCmdCharCntMBUS = 255;
					RMD_CfgCmdAckMBUS = 0;
				
					if (RMD_Config_ERRORMBUS) {
						RMD_Error = 1;
						Mode = MODE_ERROR_RMD /*MODE_DIAGNOSTIC*/; // ����� �������� ������ ��� �����������
						LCD_Refresh = 1; // ���������� �� �����������
					
					} else {
						if (RMD_Config_FIRSTMBUS) {
							RMD_Config_FIRSTMBUS = 0;
						
							NET_Mode = NET_MODE_MASTER;
							RMD_RecEn = 1;
							usart_set_baudrate (&USARTC1, 38400, 18432000);
							LCD_Refresh = 1; // ���������� �� �����������
						} else {
							switch (NewBaudRateMBUS) {
								case 38:
									usart_set_baudrate (&USARTC1, 38400, 18432000);
									break;
								case 57:
									usart_set_baudrate (&USARTC1, 57600, 18432000);
									break;
								case 11:
									usart_set_baudrate (&USARTC1, 115200, 18432000);
									break;
								default:
									break;
							}
							
							//NET_Mode = NET_MODE_TRANSP;
							//Mode = MODE_TRANSP_SCREEN;
							LCD_Refresh = 1;
						}
					}
					break;

				default:
					break;
			}
			//////////////////////////////////////////////////////////////////////////
			//��� �����������
			//TIMER_RMDCfgEn = 0;
			//TIMER_RMDCfg = 0;
			//RMD_CfgCmdCnt = 255;
			//RMD_CfgCmdCharCnt = 255;
			//RMD_CfgCmdAck = 0;
			//RMD_Error = 0;
			//Mode = MODE_NORMAL; // ����� �������� ������ ��� �����������
			//NET_Mode = NET_MODE_MASTER;
			//TIMER_ClockRefreshEn = 1; // ������ ������� ���������� �����
//
			//// ���������� �� ���� �������������� ���������:
			//// �������� ��������� ���������������� ��� ��������������
			//// � ���������� �� RX (��. ���������� ����������)
			//RMD_RecEn = 1;
			//DATA_ReqEn = 1;
			//EVENT_ReqEn = 1;
			//TIMER_DataRequest = 0; // ����� ������� ������ � ��
			//TIMER_DataRequestEn = 1; // ���������� ������ ������� ������ � ��
			//LCD_Refresh = 1; // ���������� �� �����������
			//////////////////////////////////////////////////////////////////////////
		}
/************************************************************************/
/*					��������� ������������ ���������� �����				*/
/************************************************************************/
		if (TIMER_FIRSTSCfg >= TIMER_FIRSTSCCfgPer)
		{
			Q_Test_inc++;
			TIMER_FIRSTSCfg=0;
			/////�������� ����� � 4 �� ��� ����������� ���������� ������
			if (EVENT_FROM_MAX>2){
				ADP_TransMesBody[0] = 0x02;
				ADP_TransMesBody[1] = 0x00;
				if (ADP_TransMesLen = ADP_TransMesBuild (0x53, ADP_OPK, ADP_FUNC_TRANS_PAR, ADP_NULL, 0x0004, (uint16_t *)ADP_TransMesBody)) {
					while (1){
						if (ADP_flag){
							ADP_TransStart ();
							break;
						}
					}
				}
				ADP_TransMesBody[0] = 0x02;
				ADP_TransMesBody[1] = 0x00;
				if (ADP_TransMesLen = ADP_TransMesBuild (0x54, ADP_OPK, ADP_FUNC_TRANS_PAR, ADP_NULL, 0x0004, (uint16_t *)ADP_TransMesBody)) {
					while (1){
						if (ADP_flag){
							ADP_TransStart ();
							break;
						}
					}
				}
			}
			if (Q_Test_inc>Q_TEST_MES){
				TIMER_FIRSTCfgEn=0;
				Mode = /* MODE_INFO */ /*MODE_MENU*/ MODE_NORMAL;
				LCD_Refresh = 1;
				DATA_ReqEn = 1;
				EVENT_ReqEn = 1;
				TIMER_DataRequest = 0; // ����� ������� ������ � ��
				TIMER_DataRequestEn = 1; // ���������� ������ ������� ������ � ��
			}
		}
/************************************************************************/
/*				��������� ������������ �����������						*/
/************************************************************************/
		if (TIMER_RMDCfg >= TIMER_RMDCfgPer) { // ���� �������� ����������� ������
			TIMER_RMDCfg = 0;
						
			switch (RMD_Config_STATE) 
			{
	// START - ��������� ������� ������������
				case START:
					NET_Mode = NET_MODE_CONFIG;
					/*�������� ��� ���������� ������������ rmd*/
					RMD_Config_STATE = END;
					RMD_Config_ERROR = 0;
					break;
					/************************/

					switch (NewBaudRate) {
						case 96:
							RMD_Cfg [9][5] = '9';
							RMD_Cfg [9][6] = '6';
							RMD_Cfg [10][5] = '9';
							RMD_Cfg [10][6] = '6';
							break;
						
						case 57:
							RMD_Cfg [9][5] = '5';
							RMD_Cfg [9][6] = '7';
							RMD_Cfg [10][5] = '5';
							RMD_Cfg [10][6] = '7';
							break;
							
						case 11:
							RMD_Cfg [9][5] = '1';
							RMD_Cfg [9][6] = '1';
							RMD_Cfg [10][5] = '1';
							RMD_Cfg [10][6] = '1';
							break;
						
						default:
							break;
					}
					RMD_CfgCmdCharCnt = 0;
					RMD_CfgCmdCnt = 0;
					RMD_CfgCmdAmount = 10;
					RMD_CfgCmdAck = 1;
					
					RMD_RESET_ON;
					
					RMD_Config_STATE = SEND;
					break;
					
// CHECK - �������� ������ �� ������
				case CHECK:
					if (RMD_CfgAckMesReady) {
						RMD_CfgAckMesReady = 0;
						if ((RMD_CfgAckMes[0] == 0x0D) && (RMD_CfgAckMes[1] == 0x0A) && (RMD_CfgAckMes[2] == 0x4F) && (RMD_CfgAckMes[3] == 0x4B) && (RMD_CfgAckMes[4] == 0x0A) && (RMD_CfgAckMes[5] == 0x0D) && (RMD_CfgAckMes[6] == 0x3E)) {
							RMD_CfgAckMes[0] = 0;
							RMD_CfgAckMes[1] = 0;
							RMD_CfgAckMes[2] = 0;
							RMD_CfgAckMes[3] = 0;
							RMD_CfgAckMes[4] = 0;
							RMD_CfgAckMes[5] = 0;
							RMD_CfgAckMes[6] = 0;
							RMD_CfgCmdAck = 1;	// ���� ������� ����� "��"
						} else {
							RMD_CfgCmdAck = 0;	// ���� �� �������� ������, ��� ����� �� "��"
						}
					}
					RMD_Config_STATE = SEND;
					break;

// SEND - ������� ����������������� ���������
				case SEND:
					if (RMD_CfgCmdAck) {
						if (RMD_CfgCmdCnt <= (RMD_CfgCmdAmount - 1)) {
							if ((usart_data_register_is_empty (&USARTE0)) && (RMD_CfgCmdCharCnt == 0) && (RMD_CfgCmdAck)) {
								RMD_CfgCmdAck = 0;	// ������������ ���� ������������ ��������
								// ������� ������� ������� ��������
								usart_put (&USARTE0, RMD_Cfg[RMD_CfgCmdCnt][RMD_CfgCmdCharCnt + 1]);
								if (Mode == MODE_INTRO) LCD_PutStrNum (3, 8, RMD_CfgCmdCnt, (uint8_t *) "...................");
								if (Mode == MODE_TRANSP_PROGRESS) LCD_PutStrNum (3, 0, RMD_CfgCmdCnt, (uint8_t *) "...................");
							
								++ RMD_CfgCmdCharCnt; // ��������� �������� �������� ��������
								// �������� ������ ������� �������� ���������� TX
								usart_set_tx_interrupt_level (&USARTE0, USART_TXCINTLVL_MED_gc);
							}
							RMD_Config_STATE = CHECK;
						} else {
							RMD_Config_ERROR = 0;
							RMD_Config_STATE = END;
						}
					} else {
						RMD_Config_ERROR = 1;
						RMD_Config_STATE = END;
					}
					break;

// END - ����� �� ������������
				case END:
					TIMER_RMDCfgEn_MBUS = 1; // to config second RMD
					RMD_RESET_OFF; // ������ �������� ������ � ���������� ����� ������	

					TIMER_RMDCfgEn = 0;
					TIMER_RMDCfg = 0;
			
					RMD_CfgCmdCnt = 255;
					RMD_CfgCmdCharCnt = 255;
					RMD_CfgCmdAck = 0;
				
					if (RMD_Config_ERROR) {
						RMD_Error = 1;
						Mode = MODE_ERROR_RMD /*MODE_DIAGNOSTIC*/; // ����� �������� ������ ��� �����������
						LCD_Refresh = 1; // ���������� �� �����������
					
					} else {
						if (RMD_Config_FIRST) {
							RMD_Config_FIRST = 0;

							//NET_Mode = NET_MODE_MASTER;
							if (PRJ_Error) {
								Mode = MODE_ERROR_PRJ;
								TIMER_ClockRefreshEn = 0; // ����� ����

								// ������ �� ���������, ��� ��� ������ APD ������
								RMD_RecEn = 0;
								DATA_ReqEn = 0;
								EVENT_ReqEn = 0;
								TIMER_DataRequest = 0; // ����� ������� ������ � ��
								TIMER_DataRequestEn = 0; // ���������� ������ ������� ������ � ��
							} else {
								//Mode = /* MODE_INFO */ /*MODE_MENU*/ MODE_NORMAL; // ����� �������� ������ ��� �����������
								TIMER_ClockRefreshEn = 1; // ������ ������� ���������� �����

								// ���������� �� ���� �������������� ���������:
								// �������� ��������� ���������������� ��� ��������������
								// � ���������� �� RX (��. ���������� ����������)
								RMD_RecEn = 1;
								//DATA_ReqEn = 1;
								//EVENT_ReqEn = 1;
								//TIMER_DataRequest = 0; // ����� ������� ������ � ��
								//TIMER_DataRequestEn = 1; // ���������� ������ ������� ������ � ��
								TIMER_FIRSTCfgEn=1;
							}
							
							LCD_Refresh = 1; // ���������� �� �����������
							
						} else {
							switch (NewBaudRate) {
								case 96:
									usart_set_baudrate (&USARTE0, 9600, 18432000);
									break;
								case 57:
									usart_set_baudrate (&USARTE0, 57600, 18432000);
									break;								
								case 11:
									usart_set_baudrate (&USARTE0, 115200, 18432000);
									break;
								default:
									break;
							}
							
							NET_Mode = NET_MODE_TRANSP;
							Mode = MODE_TRANSP_SCREEN;
							LCD_Refresh = 1;
						}
					}
					break;

				default:
					break;
			}
			//////////////////////////////////////////////////////////////////////////
			//��� �����������
			//TIMER_RMDCfgEn = 0;
			//TIMER_RMDCfg = 0;
			//RMD_CfgCmdCnt = 255;
			//RMD_CfgCmdCharCnt = 255;
			//RMD_CfgCmdAck = 0;
			//RMD_Error = 0;
			//Mode = MODE_NORMAL; // ����� �������� ������ ��� �����������
			//NET_Mode = NET_MODE_MASTER;
			//TIMER_ClockRefreshEn = 1; // ������ ������� ���������� �����
//
			//// ���������� �� ���� �������������� ���������:
			//// �������� ��������� ���������������� ��� ��������������
			//// � ���������� �� RX (��. ���������� ����������)
			//RMD_RecEn = 1;
			//DATA_ReqEn = 1;
			//EVENT_ReqEn = 1;
			//TIMER_DataRequest = 0; // ����� ������� ������ � ��
			//TIMER_DataRequestEn = 1; // ���������� ������ ������� ������ � ��
			//LCD_Refresh = 1; // ���������� �� �����������
			//////////////////////////////////////////////////////////////////////////
		}
/************************************************************************/
/*               ���� ����������� ������� �������                  */
/************************************************************************/
		if (LCD_Refresh) {
			LCD_Refresh = 0;
			// LCD_Clear ();
			SCREEN_ALL[Mode] ();
/*			
			switch (Mode) {
				case MODE_INTRO:
					LCD_Clear ();
					SCREEN_INTRO ();
					break;
				
				case MODE_NORMAL:
					SCREEN_NORMAL ();
					break;
			
				case MODE_MENU:
					// LCD_Clear ();
					SCREEN_MENU ();
					break;

				case MODE_INFO:
					LCD_Clear ();
					SCREEN_INFO ();
					break;
				
				case MODE_TIME:
					SCREEN_TIME ();
					break;
				
				case MODE_DIAGNOSTIC:
					LCD_Clear ();
					SCREEN_DIAGNOSTIC ();
					break;
				
				case MODE_SETTINGS:
					SCREEN_SETTINGS ();
					break;
					
				case MODE_CONTRAST:
					SCREEN_CONTRAST ();
					break;

				case MODE_EVENTS_CLEAR:
					SCREEN_EVENTS_CLEAR ();
					break;

				case MODE_EVENTS_MENU:
					SCREEN_EVENTS_MENU ();
					break;
					
				case MODE_EVENTS_LIST:
					SCREEN_EVENTS_LIST ();
					break;

				case MODE_EVENTS_INFO:
					SCREEN_EVENTS_INFO ();
					break;
					
				case MODE_EVENTS_SETTINGS:
					SCREEN_EVENTS_SETTINGS ();
					break;					

				case MODE_TRANSP_SELECT:
					SCREEN_TRANSP_SELECT ();
					break;

				case MODE_TRANSP_CONFIRM:
					SCREEN_TRANSP_CONFIRM ();
					break;

				case MODE_TRANSP_PROGRESS:
					SCREEN_TRANSP_PROGRESS ();
					break;					
					
				case MODE_TRANSP_SCREEN:
					SCREEN_TRANSP_SCREEN ();
					break;
					
				case MODE_EVENTS_SAVE_MENU:
					SCREEN_EVENTS_SAVE_MENU ();
					break;
			
				case MODE_EVENTS_SAVE_PROGRESS:
					SCREEN_EVENTS_SAVE_PROGRESS ();
					break;
					
				case MODE_EVENTS_SAVE_COMPLETED:
					SCREEN_EVENTS_SAVE_COMPLETED ();
					break;				
					
				case MODE_EDIT:
					SCREEN_NORMAL ();
					break;
				
				default:
					break;	
			}
*/						
		}

		if (TIMER_EditValBlink >= TIMER_EditValBlinkPer) {
			TIMER_EditValBlink = 0;	
			
			// LCD_PutValDec (0, 19, 1, EDIT_BlinkMarker);
			EDIT_ValBlink ();
		}
/************************************************************************/
/*                      ����� ������� � ��                  	        */
/************************************************************************/			
		// ���� �������� ������ �� ��������
		if (TIMER_DataRequest >= TIMER_DataRequestPer) {
			//LCD_PutValDec(0,0,4,ADP_mis_Error);//������� ������������� ������� ��������� ������ �� ROMBUS
			//LCD_PutValDec(1,0,4,MB0_State.Error_CRC);//������ CRC ����������, �������� �� MODBUS
			//LCD_PutValDec(2,0,4,MB0_State.Error_Data);//������ ��������� ������ ��� ������ ������ �� MODBUS
			//LCD_PutValDec(3,0,4,MB0_State.Error_MBUS);//������, ���� �� ����� �� ���������� ���������� �� MODBUS (��������� ����� �������� 200 ��)
			//LCD_PutValDec(0,4,4,ADP_Rec_CRC_Error);//������ CRC ����������, �������� �� ROMBUS
			//LCD_PutValDec(1,4,4,MB0_State.SubNum);//�� ������ :-)

/************************************************************************/
/*                      ������ ������ ��������                          */
/************************************************************************/
			// ������������� ������
			TIMER_DataRequest = 0;

			if (EVENT_ReqEn) ADP_EventReqReady = 1;
			
			if (ADP_timeout)
			{
				if (MB0_State.Sub_AddressH == 0x50){
					i = EVENT_FROM_MAX;
					MB0_State.SubNum = 1;
				}
				else i=1;
				//i = (MB0_State.Sub_AddressH == 0x50)?(EVENT_FROM_MAX):(1);
				while(i--){
					for(j=0;j<4;j++){
						
						//MBUS_Register[i+MB0_State.SubNum-1][MB0_State.Sub_AddressL] = MBUS_Data;//(MB0_State.UART_RxBuf[5]<<8)+MB0_State.UART_RxBuf[4];
					//--i;
						ADP_MBUS_TransMesBody[0] = 0x0002;	//2 ����� �� ��������� ADP
						ADP_MBUS_TransMesBody[1] = Redef_Reg[1+j];
						ADP_MBUS_TransMesBody[2] = MBUS_Register[i][j+1];
						ADP_TransMesLen = ADP_TransMesBuild (MB0_State.Sub_AddressH+i, ADP_OPK_MBUS, ADP_FUNC_REC_PAR, ADP_NULL, 6, (uint16_t*)ADP_MBUS_TransMesBody);
						while (1){
							if (ADP_flag){
								ADP_TransStart ();
								TIMER_ADP_TIMEOUT_START;
								break;
							}
						}
					}
				}
			}
				
			TRANS_STATUS = DATA;	

			if (DATA_ReqEn) {

/************************************************************************/
/*                              �������� �����                          */
/************************************************************************/
				if (FOCUS_DataAmount) {
						
					LCD_RefreshScore = 0;
				
					for (uint8_t Id = 0; Id <= /*APD_MAX_FIELDS*/ (FOCUS_DataAmount - 1); Id++) {
						/*
						* ���������� �����:
						* ���� �� ������� ����� �����, ��� BROKEN_MES ���,
						* �� ����� �����������, ������ �� ���������
						* ������ �����
						*/
						if (FOCUS_DataCntNext[Id]) { 
							-- FOCUS_DataCntNext[Id];
						}
						// 0 -> 1
						if ((FOCUS_DataCntNext[Id] > 0) && (FOCUS_DataCntPrev[Id] == 0)) {
							FOCUS_DataStatus[Id] = 1;
							++ LCD_RefreshScore;
						// 1 -> 0
						} else if ((FOCUS_DataCntNext[Id] == 0) && (FOCUS_DataCntPrev[Id] > 0)) {
							FOCUS_DataStatus[Id] = 0;
							++ LCD_RefreshScore;
						}	
						/*
							* ����� �� �������� �������?
							*/
						if (FOCUS_DataCntNewNext[Id]) {
							-- FOCUS_DataCntNewNext[Id];
						}
						// 0 -> 1
						if ((FOCUS_DataCntNewNext[Id] > 0) && (FOCUS_DataCntNewPrev[Id] == 0)) {
							// FOCUS_DataCntNewStatus[Id] = 1;
							// ++ LCD_RefreshScore;
							
						// 1 -> 0
						} else if ((FOCUS_DataCntNewNext[Id] == 0) && (FOCUS_DataCntNewPrev[Id] > 0)) {
							FOCUS_DataCntNewStatus[Id] = 0;
							++ LCD_RefreshScore;
							// LCD_Refresh = 1;
 						} //else if ((FOCUS_DataCntNewNext[Id] == 0) && (FOCUS_DataCntNewPrev[Id] == 0)) {
	// 								FOCUS_DataCntNewStatus[Id] = 0;
	// 								++ LCD_RefreshScore;
	// 							} else if ((FOCUS_DataCntNewNext[Id] > 0) && (FOCUS_DataCntNewPrev[Id] > 0)) {
	// 								FOCUS_DataCntNewStatus[Id] = 1;
	// 								++ LCD_RefreshScore;
	// 							}
						if (FOCUS_DataMesNew) {
							FOCUS_DataMesNew = 0;
							++ LCD_RefreshScore;
						}
						FOCUS_DataCntPrev[Id] = FOCUS_DataCntNext[Id];
						FOCUS_DataCntNewPrev[Id] = FOCUS_DataCntNewNext[Id];
					}
				} 	
				// ��������� ���-��, ��������� ���������� ������
				if (LCD_RefreshScore) {
					LCD_RefreshScore = 0;
					if ((Mode == MODE_NORMAL) || (MODE_EDIT) /*|| (Mode == MODE_INFO)*/) {
						LCD_Refresh = 1;						
					}
// 						if ((Mode == MODE_INFO) || (Mode == MODE_EVENTS_LIST) || (Mode == MODE_EVENTS_CLEAR)) {
// 							LCD_Refresh = 0;
// 						}
				}
			}
// 			if (EVENT_ReqEn) {
// 				/************************************************************************/
// 				/* -> ���������� ������������� �������   */
// 				/************************************************************************/
// 				for (uint8_t IdFrom = 0; IdFrom <= (EVENT_FROM_MAX - 1); ++ IdFrom) {
// 					for (uint8_t IdCode = 0; IdCode <= (EVENT_CODES_MAX - 1); ++ IdCode) {
// 				/************************************************************************/
 				/* ��� �������� �ר����?
 				 1. ������� ��������� ���������������� ������ ����������� �����������;
 				 2. ���� ��������� ������ 2 ���� ������ (������� == 2), �� ���
 				    ������������ � ������, � ������� ������������ � -1;
 				 3. ��� ������ �������� ������� ������� ���������������� �� ��������
 				-4. ����� ����� ����� ������������ � 0; */
 				/************************************************************************/
// 						// ���� ��������� ��� �������� � ������, �� ��������� �������
// 						if ( (EventStorage[ IdFrom ][ IdCode ] <= -1) && (EventStorage[ IdFrom ][ IdCode ] > -4) ) {
// 							-- EventStorage[ IdFrom ][ IdCode ];
// 						}
// 						if (EventStorage[ IdFrom ][ IdCode ] == -4) {
// 							EventStorage[ IdFrom ][ IdCode ] = 0;
// 						}
// 					}
// 				}
// 			}
			if (DATA_ReqEn) {							

				if (BLOCK_Amount) ADP_TransMesQueue = BLOCK_Amount;
				if (SendToPlc_Amount) ADP_SendToPlcQueue = SendToPlc_Amount;

				if (ADP_TransMesQueue) {
					// �������� ���� ���������
					ADP_TransMesBody[0] = BLOCK_Request[ADP_TransMesQueue - 1][1]; // ������� ���� ������?
					ADP_TransMesBody[1] = BLOCK_Request[ADP_TransMesQueue - 1][0] % 1000; // � ������ ������?
					// (������� �������������� ������)
					// ���� ����� ��������� �� �������, �������� ��������� ��� ��������
					if (ADP_TransMesLen = ADP_TransMesBuild (0x50 + (BLOCK_Request[ADP_TransMesQueue - 1][0] / 1000) /* ADP_TR1 */, ADP_OPK, ADP_FUNC_TRANS_PAR, ADP_NULL, 0x0004, (uint16_t *)ADP_TransMesBody)) {
						// �������� �������� ���������� ��������� � �������
						while (1)
							{
								if (ADP_flag)
								{
									ADP_TransStart ();
									break;
								}
							}	
						-- ADP_TransMesQueue;
					}
				} else {
					if (ADP_SendToPlcQueue) {
						if ( SendToPlc_Ready[ADP_SendToPlcQueue - 1] ) {
							-- SendToPlc_Ready[ADP_SendToPlcQueue - 1];
												
							if (ADP_TransMesLen = ADP_TransSendToPlcMesBuild (0x50 + (SendToPlc_Adress[ADP_SendToPlcQueue - 1] / 1000) /* ADP_TR1 */, ADP_OPK, ADP_FUNC_REC_PAR, ADP_NULL, SendToPlc_Adress[ADP_SendToPlcQueue - 1]%1000, SendToPlc_Value[ADP_SendToPlcQueue - 1])) {
								// �������� �������� ���������� ��������� � �������
								while (1)
							{
								if (ADP_flag)
								{
									ADP_TransStart ();
									break;
								}
							}	
								-- ADP_SendToPlcQueue;
							}
						}
					}
				}
			}
		}
/************************************************************************/
/*                      ������ ������ ������� �� ��            	      */
/************************************************************************/
		if ( ( TIMER_DataRequest >= (TIMER_DataRequestPer / 2) ) && (ADP_EventReqReady) ) {
			
			ADP_EventReqReady = 0;
// 			LCD_PutValDec (0, 0, 2, EVENT_ReqEn);
// 			LCD_PutValDec (1, 0, 2, ADP_EventReqRatioCnt);
// 			LCD_PutValDec (2, 0, 2, APD_EventReqRatio);
			
			if (EVENT_ReqEn) {
				
				TRANS_STATUS = EVENTS;
				
				++ ADP_EventReqRatioCnt;
				ADP_EventReqRatioCnt %= APD_EventReqRatio;
				
				if (!ADP_EventReqRatioCnt) {	
					/************************************************************************/
					/* -> ���������� ������������� �������                                  */
					/************************************************************************/
					for (uint8_t IdFrom = 0; IdFrom <= (EVENT_FROM_MAX - 1); ++ IdFrom) {
						for (uint8_t IdCode = 0; IdCode <= (EVENT_CODES_MAX - 1); ++ IdCode) {
					/************************************************************************/
					/* ��� �������� �ר����?                                                */
					/* 1. ������� ��������� ���������������� ������ ����������� �����������;*/
					/* 2. ���� ��������� ������ 2 ���� ������ (������� == 2), �� ���        */
					/*    ������������ � ������, � ������� ������������ � -1;               */
					/* 3. ��� ������ �������� ������� ������� ���������������� �� ��������  */
					/*    -4. ����� ����� ����� ������������ � 0;                           */
					/************************************************************************/

							// if ( EventStorage[ IdFrom ][ IdCode ] > 0 ) ++ EventStorage[ IdFrom ][ IdCode ];
							
							if ( EventStorage[ IdFrom ][ IdCode ] < 0 ) {
								
								++ EventStorage[ IdFrom ][ IdCode ];
								
								if ( EventStorage[ IdFrom ][ IdCode ] == 0 ) {
									EventStorage[ IdFrom ][ IdCode ] = EventStorage_TOP;
								}
							}
							// ���� ��������� ��� �������� � ������, �� ��������� �������
// 							if ( (EventStorage[ IdFrom ][ IdCode ] <= -1) && (EventStorage[ IdFrom ][ IdCode ] > -4) ) {
// 								-- EventStorage[ IdFrom ][ IdCode ];
// 							}
// 							if (EventStorage[ IdFrom ][ IdCode ] == -4) {
// 								EventStorage[ IdFrom ][ IdCode ] = 0;
// 							}
						}
					}
					ADP_EventRequestQueue = APD_EventAdrNum;

					if (ADP_EventRequestQueue) {
						
						//LCD_PutSym (ADP_EventRequestQueue - 1, 0, '*');
						//LCD_PutValDec (0, 1, 1, APD_EventAdrNum);
						//LCD_PutValDec (ADP_EventRequestQueue - 1, 1, 1, ADP_EventRequestQueue);
				
						if (ADP_TransMesLen = ADP_TransEventRequest (0x50 + APD_EventAdrArray[ADP_EventRequestQueue - 1], ADP_OPK, ADP_FUNC_EVENT_REQ, ADP_NULL)) {
							-- ADP_EventRequestQueue;
							while (1)
							{
								if (ADP_flag)
								{
									ADP_TransStart ();
									break;
								}
							}
						}
					}
				}
			}
		}
		/************************************************************************/
		/*               �������� ����� ����� ����� �����������*/
		/************************************************************************/
		if (TIMER_RequestTimeout >= TIMER_RequestTimeoutPer) {
			TIMER_RequestTimeout = 0;
			TIMER_RequestTimeoutEn = 0;
			
			switch (TRANS_STATUS) {
				case DATA:
					if (ADP_TransMesQueue) {

						ADP_TransMesBody[0] = BLOCK_Request[ADP_TransMesQueue - 1][1]; // ������� ���� ������?
						ADP_TransMesBody[1] = BLOCK_Request[ADP_TransMesQueue - 1][0] % 1000; // � ������ ������?
						// (������� �������������� ������) 
						// ���� ����� ��������� �� �������, �������� ��������� ��� ��������
						if (ADP_TransMesLen = ADP_TransMesBuild (0x50 + (BLOCK_Request[ADP_TransMesQueue - 1][0] / 1000) /* ADP_TR1 */, ADP_OPK, ADP_FUNC_TRANS_PAR, ADP_NULL, 0x0004, (uint16_t *)ADP_TransMesBody)) {
							// �������� �������� ���������� ��������� � �������
							-- ADP_TransMesQueue;
							while (1)
							{
								if (ADP_flag)
								{
									ADP_TransStart ();
									break;
								}
							}
						}
					} else {
						if (ADP_SendToPlcQueue) {
					
							if ( SendToPlc_Ready[ADP_SendToPlcQueue - 1] ) {
								-- SendToPlc_Ready[ADP_SendToPlcQueue - 1];
														
								if (ADP_TransMesLen = ADP_TransSendToPlcMesBuild (0x50 + (SendToPlc_Adress[ADP_SendToPlcQueue - 1] / 1000) /* ADP_TR1 */, ADP_OPK, ADP_FUNC_REC_PAR, ADP_NULL, SendToPlc_Adress[ADP_SendToPlcQueue - 1] % 1000, SendToPlc_Value[ADP_SendToPlcQueue - 1])) {
									// �������� �������� ���������� ��������� � �������
									-- ADP_SendToPlcQueue;
									while (1){
										if (ADP_flag){
											ADP_TransStart ();
											break;
										}
									}	
								}
							}
						}
					}
					break;
					
				case EVENTS:
					if (ADP_EventRequestQueue) {
					//LCD_PutSym (ADP_EventRequestQueue - 1, 0, '*');
					//LCD_PutValDec (ADP_EventRequestQueue - 1, 1, 1, ADP_EventRequestQueue);
						
						if (ADP_TransMesLen = ADP_TransEventRequest (0x50 + APD_EventAdrArray[ADP_EventRequestQueue - 1], ADP_OPK, ADP_FUNC_EVENT_REQ, ADP_NULL)) {
							-- ADP_EventRequestQueue;
							while (1)
							{
								if (ADP_flag)
								{
									ADP_TransStart ();
									break;
								}
							}
						}
					}
					break;
				default:
					break;
			}
		}
		/*LCD_PutValDecPointMaskNeg_ (0, 0, 4, 0, 1, ADP_RecMesBuf[0]);
	LCD_PutValDecPointMaskNeg_ (1, 0, 4, 0, 1, ADP_RecMesBuf[1]);
	LCD_PutValDecPointMaskNeg_ (2, 0, 4, 0, 1, ADP_RecMesBuf[2]);
	LCD_PutValDecPointMaskNeg_ (3, 0, 4, 0, 1, ADP_RecMesBuf[3]);
	LCD_PutValDecPointMaskNeg_ (0, 4, 5, 0, 1, ADP_RecMesBuf[4]);
	LCD_PutValDecPointMaskNeg_ (1, 4, 5, 0, 1, ADP_RecMesBuf[5]);
	LCD_PutValDecPointMaskNeg_ (2, 4, 5, 0, 1, ADP_RecMesBuf[6]);
	LCD_PutValDecPointMaskNeg_ (3, 4, 5, 0, 1, ADP_RecMesBuf[7]);
	LCD_PutValDecPointMaskNeg_ (0, 9, 5, 0, 1, ADP_RecMesBuf[8]);
	LCD_PutValDecPointMaskNeg_ (1, 9, 5, 0, 1, ADP_RecMesBuf[9]);
	LCD_PutValDecPointMaskNeg_ (2, 9, 5, 0, 1, ADP_RecMesBuf[10]);
	LCD_PutValDecPointMaskNeg_ (3, 9, 5, 0, 1, ADP_RecMesBuf[11]);
	LCD_PutValDecPointMaskNeg_ (0, 14, 5, 0, 1, ADP_RecMesBuf[12]);
	LCD_PutValDecPointMaskNeg_ (1, 14, 5, 0, 1, ADP_RecMesBuf[13]);
	LCD_PutValDecPointMaskNeg_ (2, 14, 5, 0, 1, MB0_State.flags);
	LCD_PutValDecPointMaskNeg_ (3, 14, 5, 0, 1, ADP_flag);*/
/************************************************************************/
/*       ����������� ����� � �������� ������� ������ RTC*/
/************************************************************************/	
// ����������� ����� � �������� ������� (�������� ���������� �� ������, ����������� ������ ClockShowEn)
		if (TIMER_ClockRefresh >= TIMER_ClockRefreshPer) {
			TIMER_ClockRefresh = 0;
			if (ClockShowEn) {
				CLOCK_ShowOn (CLOCK_ShowStr, CLOCK_ShowCol);
			}
			if (DataShowEn) {
				DATE_ShowOn (DATE_ShowStr, DATE_ShowCol);
			}
		}
/************************************************************************/
/*       �������� ��������� ��������� �� ������� ������ (Crc-8)*/
/************************************************************************/
		if (RMD_RecMesReady) {
			RMD_RecMesReady = 0; // ����� ��������� ����������
			if (!(Crc8 ((uint8_t *) &RMD_RecMesBuf, RMD_RecMesBufLen))) {
				RMD_RecMesCrcOk = 1; // ���� ������ Crc-8 �� ����������
				
				// ������� ���� �������� ������
				// LCD_PutValDec (0, 0, 3, RMD_RecMesBuf[1]);
				// LCD_PutValDec (1, 0, 3, RMD_RecMesBuf[2]);
				
				//if (Mode == MODE_INFO) {
					//LCD_Refresh = 1;
				//}
			}
		}
/************************************************************************/
/*�������� ��������� ��������� �� ������� ������ (Crc-16)*/
/************************************************************************/
		if (ADP_RecMesReady) {
			ADP_CRC = Crc16 ((uint8_t *) &ADP_Buf, ADP_RecMesLen);
			ADP_RecMesReady = 0; // ����� ��������� ����������
			ADP_RecMesLen = 0;
			if (!ADP_CRC)
			{
				if (MBUS_num)
				{
					ADP_mis_Error++;
				}
				ADP_RecMesCrcOk = 1; // ���� ������ Crc-16 �� ����������
				MBUS_num = 1;
			}
			else{ADP_Rec_CRC_Error++;
			}
		}
		
		if (MB0_State.TelCompleet) // ���� ���� ������ ��������� ��������
		{
			MB0_State.TelCompleet = 0; 
			//���������� �������� CRC � ���������
			MB0_State.CRC16_OK = Crc16 ((uint8_t *)MBUS_Buf, (MB0_State.lenRx)-2);
			MB0_State.CRC16 = ((MBUS_Buf[MB0_State.lenRx-1]<<8)|MBUS_Buf[MB0_State.lenRx-2]);
			if (MB0_State.CRC16_OK == MB0_State.CRC16) 
			{
				MB0_State.CRC16_OK = 1; // ���� ������ Crc-16 �� ����������
			}	
			else //����� ������� ��������� �� �������� � ����� ����������
			{
				MB0_State.Error_CRC++;
				MB0_State.CRC16_OK = 0;
				//LCD_PutValDec(0,0,4,MB0_State.Error_CRC);
			}
			MB0_State.lenRx = 0;
		}
/************************************************************************/
/*     ���� ��������� ��������� �� ���������������� */
/************************************************************************/	
		if (ADP_RecMesCrcOk) {
			ADP_RecMesCrcOk = 0;
			 //������ �������� ����������
			// ��� ����������?
			if (ADP_Buf[0] == ADP_OPK) {
				MBUS_num = 0;	
				// ��� ����������?
				if ((ADP_Buf[1] == ADP_TR0) || (ADP_Buf[1] == ADP_TR1) || (ADP_Buf[1] == ADP_TR2) || (ADP_Buf[1] == ADP_TR3) || (ADP_RecMesBuf[1] == ADP_TR4)) {
					// ��� �������?
					//if (s_timeout<s_delta){s_timeout = }
					if (ADP_Buf[2] == (ADP_FUNC_TRANS_PAR | (1<<7) )) {// ���� ������ 
						ADP_Function_TransParams ();
						if (Mode == MODE_INFO) {
							LCD_Refresh = 1;
						}
					} else if (ADP_Buf[2] == (ADP_FUNC_EVENT_REQ | (1<<7) )) {// ���� ������ 
						ADP_Function_ReqEvents ();
						if (Mode == MODE_INFO) {
							LCD_Refresh = 1;
						}
					} else if (ADP_Buf[2] == (ADP_FUNC_REC_PAR | (1<<7) )) {// ���� ������ 
						ADP_Function_RecParams ();
					}
					if (((ADP_Buf[1] == ADP_TR4) || (ADP_Buf[1] == ADP_TR3)) && (TIMER_FIRSTCfgEn)){
						NumOfFreqCon=1;
						Mode = /* MODE_INFO */ /*MODE_MENU*/ MODE_NORMAL;
						//LCD_PutValDec(0,0,3,NumOfFreqCon);
						APD_FrameCur=28;
						Mode = MODE_NORMAL;
						LCD_Refresh = 1;
						TIMER_FIRSTCfgEn=0;
						DATA_ReqEn = 1;
						EVENT_ReqEn = 1;
						TIMER_DataRequest = 0; // ����� ������� ������ � ��
						TIMER_DataRequestEn = 1; // ���������� ������ ������� ������ � ��
						FOCUS_Adr ();
						FOCUS_Sort (FOCUS_DataAdr, FOCUS_DataAmount);
						FOCUS_Space ();
						FOCUS_Block ();
						FOCUS_CntReset ();
					}
					//else if (!TIMER_FIRSTCfgEn && !NumOfFreqCon){
						//Mode = /* MODE_INFO */ /*MODE_MENU*/ MODE_NORMAL;
						//LCD_Refresh = 1;
						//DATA_ReqEn = 1;
						//EVENT_ReqEn = 1;
						//TIMER_DataRequest = 0; // ����� ������� ������ � ��
						//TIMER_DataRequestEn = 1; // ���������� ������ ������� ������ � ��
					//}
				}
			}
			else if (ADP_RecMesBuf[0] == ADP_OPK_MBUS)
			{
				if(ADP_Buf[2] == (ADP_FUNC_TRANS_PAR | (1<<7) ))
				{
					MB0_State.SubNum = ADP_Buf[1]-0x50;
					ADP_Rec_MBUS((uint8_t*)ADP_Buf,(uint16_t*)MBUS_Register+(MB0_State.SubNum-1)*50);
					/*ADP_MBUS_Adr = (ADP_Buf[9]<<8) | ADP_Buf[8];
					ADP_MBUS_Q = (ADP_Buf[7]<<8) | ADP_Buf[6];
					MBUS_Register[MB0_State.SubNum-1][1] = ADP_MBUS_Adr;
					for (i=0;i<ADP_MBUS_Q/2;i++)//���� �� ���������� �� ADP ���������
					{
						//for (j=1;j<sizeof(Redef_Reg);j++)
						//{
							//if (Redef_Reg[j]==ADP_MBUS_Adr+i)
							//{
								MBUS_Register[MB0_State.SubNum-1][i] = (ADP_Buf[1 + 2*i]<<8) | (ADP_Buf[0 + 2*i]);
							//}
						//}
					}*/
					MBUS_num = 0;
					if (numADPmes)
					{
						numADPmes--;
					}
					else
					{
						TIMER_ADP_TIMEOUT_STOP;
						//free(MBUS_ADP);
						ADP_flag = 1;
						if(MB0_State.flags&ANSWER)MBUS_Send_04();
					}
					//////////////////////////////////////////////////////////////////////////
					/*ADP_MBUS_Counter++;
					RegAdr = ADP_RecMesBuf[8]|(ADP_RecMesBuf[9]<<8);
					for (Reg2Reg=0;Reg2Reg<50;Reg2Reg++)	/// ����� ������ ������ �� ������ ADP
					{
						if (RegAdr == Redef_Reg[Reg2Reg]){break;}
					}
					
					MBUS_Register[Reg2Reg] = ADP_RecMesBuf[10]|(ADP_RecMesBuf[11]<<8);
					
					if (MB0_State.Quant>ADP_MBUS_Counter)//���� ���� ��� �������� ��� ������, �� �������� ������
					{
						ADP_MBUS_TransMesBody[0] = 0x0002;	//2 ����� �� ��������� ADP
						//ADP_MBUS_TransMesBody[1] = ((Redef_Reg[MB0_State.Sub_AddressL+ADP_MBUS_Counter]<<8)|(Redef_Reg[MB0_State.Sub_AddressL+ADP_MBUS_Counter]>>8));//���� ������ � ������� �������� ��� ����, �� ����� ����� ������������ 
						ADP_MBUS_TransMesBody[1] = Redef_Reg[MB0_State.Sub_AddressL+ADP_MBUS_Counter];
						ADP_TransMesLen = ADP_TransMesBuild (MB0_State.Sub_AddressH, ADP_OPK_MBUS, ADP_FUNC_TRANS_PAR, ADP_NULL, 4, (uint16_t*)ADP_MBUS_TransMesBody);
						while (1)
						{
							if (ADP_flag)
							{
								ADP_TransStart ();
								break;
							}
						}
					}
					else// if (MB0_State.Quant<=2*ADP_MBUS_Counter)//�� ADP �� ���������, ����� ���������� ����� �� MODBUS
					{
						if (MB0_State.Quant-ADP_MBUS_Counter)
						{
							//MB0_State.Error_count++;
						}
						ADP_MBUS_Counter=0;
						TIMER_ADP_TIMEOUT_STOP;
						MBUS_Send_04();
					}*/
				}	
				if(ADP_Buf[2] == (ADP_FUNC_REC_PAR | (1<<7) ))
				{
					ADP_timeout=0;
					MBUS_num = 0;
					//free(MBUS_ADP);
					TIMER_ADP_TIMEOUT_STOP;
					if (MB0_State.command==MBUS_FUNC_SETREGS)MBUS_Send_10();
					else if(MB0_State.flags&ANSWER)MBUS_Send_06();
				}
			}
		}
		//�������� ���������� �������� ������� MODBUS
		if (MB0_State.CRC16_OK)
		{
			MB0_State.CRC16_OK = 0;
			//MB0_State.flags  &= ~ANSWER;	//����� ����� ������
				// ������ �������� ����������
				// ��� ����������?
			MB0_State.address = 	MBUS_Buf[0];
			MB0_State.command =  	MBUS_Buf[1];
			MB0_State.Sub_AddressH = MBUS_Buf[2];//��� ������ ��
			MB0_State.Sub_AddressL = MBUS_Buf[3];
			MB0_State.SubNum = MB0_State.Sub_AddressH - 0x50;
			//MB0_State.Buf_byte = 0;
			if (MB0_State.address == MBUS_OPK) 
			{
				//Redef_Reg[MB0_State.Sub_AddressL] - ����� �������������� ��������
				switch(MB0_State.command)
				{
					//case MBUS_FUNC_GETREG://������ ���� �������
						////��������� ������, ���� ����
						//MB0_State.Quant = MBUS_Buf[5];//((MB0_State.UART_RxBuf[4]<<8)|MB0_State.UART_RxBuf[5]);
						//if (MB0_State.Sub_AddressL>MAX_QUANT_REG)//��� ������ 02 ILLEGAL DATA ADDRESS
						//{
							//MB0_State.Error_Data++;
							//MBUS_Error_Trans(ILLEGAL_ADR);
							//break;
						//}
						//if (MB0_State.Sub_AddressH==SPEED_MBUS)
						//{
							//MBUS_Send_Speed();
							//break;
						//}
						//if (((MB0_State.Sub_AddressH>0x55) || (MB0_State.Sub_AddressH<0x50))&&(MB0_State.Sub_AddressH!=SPEED_MBUS))//��� ������ 02 ILLEGAL DATA ADDRESS
						//{
							//MB0_State.Error_Data++;
							//MBUS_Error_Trans(ILLEGAL_ADR);
							//break;
						//}
						////i = (MB0_State.Sub_AddressH == 0x50)?(EVENT_FROM_MAX):(1);
						////if (MB0_State.Sub_AddressH == 0x50)
						////{
							////i = EVENT_FROM_MAX;
						////}
						////else {i=1;}
						////while(i)
						////{
							////--i;
							//ADP_MBUS_TransMesBody[0] = 0x0002;
							//ADP_MBUS_TransMesBody[1] = Redef_Reg[MB0_State.Sub_AddressL];//MBUS_ADP[0];
							//ADP_TransMesLen = ADP_TransMesBuild (MB0_State.Sub_AddressH, ADP_OPK_MBUS, ADP_FUNC_TRANS_PAR, ADP_NULL, 4, (uint16_t*)ADP_MBUS_TransMesBody);
							//while (1)
							//{
								//if (ADP_flag)
								//{
									//ADP_TransStart ();
									//TIMER_ADP_TIMEOUT_START;
									//break;
								//}
							//}
						////}
					//break;

					case MBUS_FUNC_GETREG:
					case MBUS_FUNC_GETREGS://������ ��������
						MB0_State.Quant = MBUS_Buf[5];//((MB0_State.UART_RxBuf[4]<<8)|MB0_State.UART_RxBuf[5]);
						if ((MB0_State.Sub_AddressL+MB0_State.Quant)>MAX_QUANT_REG)// && MB0_State.Quant)//��� ������ 02 ILLEGAL DATA ADDRESS
						{
							MB0_State.Error_Data++;
							MBUS_Error_Trans(ILLEGAL_ADR);
							break;
						}
						if (((MB0_State.Sub_AddressH>0x55) || (MB0_State.Sub_AddressH<0x50))&&(MB0_State.Sub_AddressH!=SPEED_MBUS))//��� ������ 02 ILLEGAL DATA ADDRESS
						{
							MB0_State.Error_Data++;
							MBUS_Error_Trans(ILLEGAL_ADR);
							break;
						}
						if (MB0_State.Sub_AddressH==SPEED_MBUS)
						{
							MBUS_Send_Speed();
							break;
						}
						//if (MB0_State.Quant>50)
						//{
							//for(numADPmes=0;numADPmes<4;numADPmes++)
							//{
								//ADP_MBUS_TransMesBody[0] = 0x001F;	//31 ���� �� ��������� ADP
								//ADP_MBUS_TransMesBody[1] = Redef_Reg1;
								//ADP_TransMesLen = ADP_TransMesBuild (numADPmes+0x51, ADP_OPK_MBUS, ADP_FUNC_TRANS_PAR, ADP_NULL, 4, (uint16_t*)ADP_MBUS_TransMesBody);
								//while (1)
								//{
									//if (ADP_flag)
									//{
										//ADP_TransStart ();
										//break;
									//}
								//}
							//}
							//TIMER_ADP_TIMEOUT_START;
						//}
						if (MB0_State.Quant>=0x24)
						{
							//i = (MB0_State.Sub_AddressH == 0x50)?(EVENT_FROM_MAX):(1);
							//while(i)
							//{
								//--i;
								ADP_MBUS_TransMesBody[0] = 0x0064;	//50 ���� �� ��������� ADP
								ADP_MBUS_TransMesBody[1] = Redef_Reg1;							
								ADP_TransMesLen = ADP_TransMesBuild (MB0_State.Sub_AddressH, ADP_OPK_MBUS, ADP_FUNC_TRANS_PAR, ADP_NULL, 4, (uint16_t*)ADP_MBUS_TransMesBody);
								while (1)
								{
									if (ADP_flag)
									{
										ADP_TransStart ();
										TIMER_ADP_TIMEOUT_START;
										break;
									}
								}
							//}
						}
						else if (MB0_State.Quant == 1)
						{
							//i = (MB0_State.Sub_AddressH == 0x50)?(EVENT_FROM_MAX):(1);
							//while(i)
							//{
								//--i;
							//ADP_NextStart[0] = MBUS_ADP[0];
								ADP_MBUS_TransMesBody[0] = 0x0002;
								ADP_MBUS_TransMesBody[1] = Redef_Reg[MB0_State.Sub_AddressL];//MBUS_ADP[0];
								ADP_TransMesLen = ADP_TransMesBuild (MB0_State.Sub_AddressH, ADP_OPK_MBUS, ADP_FUNC_TRANS_PAR, ADP_NULL, 4, (uint16_t*)ADP_MBUS_TransMesBody);
								while (1)
								{
									if (ADP_flag)
									{
										ADP_TransStart ();
										TIMER_ADP_TIMEOUT_START;
										break;
									}
								}
							//}
						}
						else
						{
							//////////////////////////////////////////////////////////////////////////
						//���������� �������
						//////
							//MBUS_ADP = (uint16_t*)malloc(MB0_State.Quant);
							//numADPmes = (uint16_t*)malloc(6);
							for(i=0;i<MB0_State.Quant;i++)
							{
								MBUS_ADP[i] = Redef_Reg[i+MB0_State.Sub_AddressL];
							}
							//��� ��������� ������������� ��������, ���������� ����������� ��������	
							//if (MB0_State.Quant == 1)
							//{
								//ADP_NextStart[0] = MBUS_ADP[0];
								//ADP_MBUS_TransMesBody[0] = 0x0002;
								//ADP_MBUS_TransMesBody[1] = MBUS_ADP[0];
								//ADP_TransMesLen = ADP_TransMesBuild (MB0_State.Sub_AddressH, ADP_OPK_MBUS, ADP_FUNC_TRANS_PAR, ADP_NULL, 4, (uint16_t*)ADP_MBUS_TransMesBody);
								//while (1)
								//{
									//if (ADP_flag)
									//{
										//ADP_TransStart ();
										//break;
									//}
								//}
								//TIMER_ADP_TIMEOUT_START;
							//}
							//else
							//{
							//��������� ������ ������ ������� � ��������� ��
								qsort((uint16_t*)MBUS_ADP,MB0_State.Quant,sizeof(uint16_t),(uint16_t(*) (const uint16_t *, const uint16_t *)) comp);
							//� �������������� ������� ���� �������� � ��������� >14 ��� ��������� ��������� ADP
								ADP_NextStart[nu++] = MBUS_ADP[0];//nu ������� ������! ������������!
								ADP_MBUS_Counter[nu-1]+=2;
								for(i=0;i<MB0_State.Quant-1;++i)
								{
									delta = abs(MBUS_ADP[i]-MBUS_ADP[i+1]);
									if (delta>13)//���� ������� ������ ����� ��������, ������� ����� ���������
									{
										numADPmes++;//����������� ���������� ��������� �� ADP
										ADP_NextStart[nu++] = MBUS_ADP[i+1];//���������� ����� ����� ��� ����� �������
										ADP_MBUS_Counter[nu-1]+=2;
									}
									else
									{
										ADP_MBUS_Counter[nu-1]+=delta*2;//����������� ����� ���� � ����� ��������� ADP
									}
								}
								//nu2 = nu-1;
								if (ADP_MBUS_Counter[0])
								{
									for (nu=0;nu<numADPmes+1;++nu)
									{
										//i = (MB0_State.Sub_AddressH == 0x50)?(EVENT_FROM_MAX):(1);
										//while(i)
										//{
											//--i;
											ADP_MBUS_TransMesBody[0] = ADP_MBUS_Counter[nu];
											ADP_MBUS_TransMesBody[1] = ADP_NextStart[nu];
											ADP_TransMesLen = ADP_TransMesBuild (MB0_State.Sub_AddressH, ADP_OPK_MBUS, ADP_FUNC_TRANS_PAR, ADP_NULL, 4, (uint16_t*)ADP_MBUS_TransMesBody);
												ADP_MBUS_Counter[nu] = 0;
											while (1)
											{
												if (ADP_flag)
												{
													ADP_TransStart ();
													TIMER_ADP_TIMEOUT_START;
													break;
												}
											}
										//}
									}
								}
							nu=0;
							//}
						}
							//��� ������ ��������
							//ADP_MBUS_TransMesBody[0] = 0x0002;	//2 ����� �� ��������� ADP
							//ADP_MBUS_TransMesBody[1] = (Redef_Reg[MB0_State.Sub_AddressL]>>8+Redef_Reg[MB0_State.Sub_AddressL]<<8);//���� ������ � ������� �������� ��� ����, �� ����� ����� ������������ 
							//ADP_MBUS_TransMesBody[1] = MBUS_ADP[MB0_State.Sub_AddressL];
							//ADP_TransMesLen = ADP_TransMesBuild (MB0_State.Sub_AddressH, ADP_OPK_MBUS, ADP_FUNC_TRANS_PAR, ADP_NULL, 4, (uint16_t*)ADP_MBUS_TransMesBody);
							//while (1)
							//{
								//if (ADP_flag)
								//{
									//ADP_TransStart ();
									//break;
								//}
							//}	
						//���� ����� ��� ������ �� ADP, ���������� ������ �������� 05
					break;
					case MBUS_FUNC_SETREG://�������� ���� �������
						if ((MB0_State.Sub_AddressL)>MAX_QUANT_REG)//��� ������ 02 ILLEGAL DATA ADDRESS
						{
							MB0_State.Error_Data++;
							MBUS_Error_Trans(ILLEGAL_ADR);
							break;
						}
						if (((MB0_State.Sub_AddressH>0x55) || (MB0_State.Sub_AddressH<0x50)) && (MB0_State.Sub_AddressH!=SPEED_MBUS))//��� ������ 02 ILLEGAL DATA ADDRESS
						{
							MB0_State.Error_Data++;
							MBUS_Error_Trans(ILLEGAL_ADR);
							break;
						}
						if (MB0_State.Sub_AddressH==SPEED_MBUS)
						{
							Time_ans = (MB0_State.UART_RxBuf[4]<<8)+MB0_State.UART_RxBuf[5];
							TIMER_ADP_TIMEOUT_INIT;
							MBUS_Send_Speed();
							break;
						}
						//if (MB0_State.Sub_AddressL>7)//������ ������ � �������� ������ 7-��
						//{
							//MB0_State.Error_Data++;
							//MBUS_Error_Trans(ILLEGAL_DATA);
							//break;
						//}
						//TIMER_ADP_TIMEOUT_START;
						//MBUS_Register[MB0_State.SubNum][MB0_State.Sub_AddressL] = (MB0_State.UART_RxBuf[5]<<8)+MB0_State.UART_RxBuf[4];
						MBUS_Data = (MBUS_Buf[4]<<8)+MBUS_Buf[5];
						if (MB0_State.Sub_AddressH == 0x50)
						{
							i = EVENT_FROM_MAX;
							MB0_State.SubNum = 1;
						}
						else
						{i=1;}
						//i = (MB0_State.Sub_AddressH == 0x50)?(EVENT_FROM_MAX):(1);
						while(i--)
						{
							MBUS_Register[i+MB0_State.SubNum-1][MB0_State.Sub_AddressL] = MBUS_Data;//(MB0_State.UART_RxBuf[5]<<8)+MB0_State.UART_RxBuf[4];
							//--i;
							ADP_MBUS_TransMesBody[0] = 0x0002;	//2 ����� �� ��������� ADP
							ADP_MBUS_TransMesBody[1] = Redef_Reg[MB0_State.Sub_AddressL];
							ADP_MBUS_TransMesBody[2] = MBUS_Data;
							ADP_TransMesLen = ADP_TransMesBuild (MB0_State.Sub_AddressH+i, ADP_OPK_MBUS, ADP_FUNC_REC_PAR, ADP_NULL, 6, (uint16_t*)ADP_MBUS_TransMesBody);
							while (1)
							{
								if (ADP_flag)
								{
									ADP_TransStart ();
									TIMER_ADP_TIMEOUT_START;
									break;
								}
							}
						}
					break;
					case MBUS_FUNC_SETREGS://�������� �������� ���������
						MB0_State.Quant = MBUS_Buf[5];
						if ((MB0_State.Sub_AddressL+MB0_State.Quant)>MAX_QUANT_REG)//��� ������ 02 ILLEGAL DATA ADDRESS
						{
							MB0_State.Error_Data++;
							MBUS_Error_Trans(ILLEGAL_ADR);
							break;
						}
						if (((MB0_State.Sub_AddressH>0x55) || (MB0_State.Sub_AddressH<0x50)) && (MB0_State.Sub_AddressH!=SPEED_MBUS))//��� ������ 02 ILLEGAL DATA ADDRESS
						{
							MB0_State.Error_Data++;
							MBUS_Error_Trans(ILLEGAL_ADR);
							break;
						}
						if (MB0_State.Sub_AddressH==SPEED_MBUS)
						{
							Time_ans = (MB0_State.UART_RxBuf[4]<<8)+MB0_State.UART_RxBuf[5];
							TIMER_ADP_TIMEOUT_INIT;
							MBUS_Send_Speed();
							break;
						}
						//if (MB0_State.Sub_AddressL>7)//������ ������ � �������� ������ 7-��
						//{
							//MB0_State.Error_Data++;
							//MBUS_Error_Trans(ILLEGAL_DATA);
							//break;
						//}
						for (int ii=0;ii<MB0_State.Quant;ii+=2)	MBUS_Datas[ii/2] = (MB0_State.UART_RxBuf[ii+7]<<8)+MB0_State.UART_RxBuf[ii+8];
						if (MB0_State.Sub_AddressH == 0x50)
						{
							i = EVENT_FROM_MAX;
							MB0_State.SubNum = 1;
						}
						else
						{i=1;}
						//i = (MB0_State.Sub_AddressH == 0x50)?(EVENT_FROM_MAX):(1);
						while(i--)
						{
							MBUS_Register[i+MB0_State.SubNum-1][MB0_State.Sub_AddressL] = MBUS_Datas[0];//(MB0_State.UART_RxBuf[5]<<8)+MB0_State.UART_RxBuf[4];
							//--i;
							ADP_MBUS_TransMesBody[0] = 0x0002;	//2 ����� �� ��������� ADP
							ADP_MBUS_TransMesBody[1] = Redef_Reg[MB0_State.Sub_AddressL];
							ADP_MBUS_TransMesBody[2] = MBUS_Datas[0];
							ADP_TransMesLen = ADP_TransMesBuild (MB0_State.Sub_AddressH+i, ADP_OPK_MBUS, ADP_FUNC_REC_PAR, ADP_NULL, 6, (uint16_t*)ADP_MBUS_TransMesBody);
							while (1)
							{
								if (ADP_flag)
								{
									ADP_TransStart ();
									TIMER_ADP_TIMEOUT_START;
									break;
								}
							}
						}
					break;
					case MBUS_FUNC_GETID:
						MBUS_Send_17();
					break;
					default:
						MB0_State.Error_Data++;
						MBUS_Error_Trans(ILLEGAL_FUN);
					break;
				}
			}
		}			// ��������� ������ ��������� ���������
			/*
			for (uint16_t Id = 0; Id <= (ADP_RecMesLen - 1); Id++) {
				ADP_RecMesBuf[Id] = 0;
			}
			ADP_RecMesLen = 0;
			*/
/************************************************************************/
/*       ���� ��������� ���������� � ������ � ������� ������� */
/************************************************************************/
		// ��������� ������� ��� ����������� �� ������ (����� ������� �������� � ��������� �����)
		if (RMD_RecMesCrcOk) 
		{
			RMD_RecMesCrcOk = 0; // ������������� �������������� ��������� ����������
			// ������ � ����������
			if ((RMD_RecMesBuf[1] != BUTTON_First) || (RMD_RecMesBuf[2] != BUTTON_Second)) {
				BUTTON_Release = 1;
			}			
			// �������� ������� ������ � ����������� ���������������� ���������
			if (((RMD_RecMesBuf[1] == BUTTON_First) && (BUTTON_First == 5)) && ((RMD_RecMesBuf[2] == BUTTON_Second) && (BUTTON_Second == 8))) {
				// ������ �������� ��������� ���������� ������
				if (SERVICE_ToggleCnt < SERVICE_ToggleCntLim) {
					++ SERVICE_ToggleCnt;
				} else {
					// ���� ���������� ������ ������������ � ������� �������,
					// ������������� SERVICE_ToggleCntLim,
					// ���������� ����� ������ ������
					SERVICE_ToggleCnt = 0; // ���� ������ ��������, ����� ��������
					if (Mode == MODE_MENU) {

						Mode = MODE_NORMAL;
						BUTTON_Release = 0;
						
						DATA_ReqEn = 1;
						SplashRefreshEn = 1; // ���������� ��������� ��������
						
					} else if (Mode == MODE_NORMAL) {
						ClockShowEn = 1;
						DataShowEn = 0;
						CLOCK_ShowStr = 0;
						CLOCK_ShowCol = 12;

						DATA_ReqEn = 0;
						Mode = MODE_MENU;
						MenuShift = 0;
						
						BUTTON_Release = 0;
					}
					LCD_Refresh = 1; // ���������� �� �����������
				}
			} else {
				SERVICE_ToggleCnt = 0; // ���� ������ �� ��������, ���-����� ����� ��������	
			}
			if (((RMD_RecMesBuf[1] == BUTTON_First) && (BUTTON_First == 5)) && ((RMD_RecMesBuf[2] == BUTTON_Second) && (BUTTON_Second == 7))) {
				// ������ �������� ��������� ���������� ������
				if (TRANSP_ToggleCnt < TRANSP_ToggleCntLim) {
					++ TRANSP_ToggleCnt;
				} else {
					// ���� ���������� ������ ������������ � ������� �������,
					// ������������� TRANSP_ToggleCntLim,
					// ���������� ����� ������ ������
					TRANSP_ToggleCnt = 0; // ���� ������ ��������, ����� ��������
					if (Mode == MODE_MENU) {
						ClockShowEn = 0;
						DataShowEn = 0;

						Mode = MODE_TRANSP_SELECT;
						BUTTON_Release = 0;
					}
					LCD_Refresh = 1; // ���������� �� �����������
				}
			} else {
				TRANSP_ToggleCnt = 0; // ���� ������ �� ��������, ���-����� ����� ��������
			}			
			// ������ ������� �������� ������ � �����
			BUTTON_First = RMD_RecMesBuf[1];
			BUTTON_Second = RMD_RecMesBuf[2];	
			
			// ��������� ������� � ����������� �� ������
			if (BUTTON_Release) {
				BUTTON_Release = 0;
				switch (Mode) {
/************************************************************************/
/* >>> ����� "�������" >>>                                              */
/************************************************************************/
					case MODE_NORMAL: // ������� �����
						APD_FrameButtonAction = APD_FrameButtonActionTable[BUTTON_Second][BUTTON_First];
						SendToPlc_One = 1;
						SCREEN_ACTION (APD_FrameButtonAction);
						APD_FrameButtonAction = 255;
						break;
/************************************************************************/
/* >>> ����� "����"														*/
/************************************************************************/
					case MODE_MENU: // ����
						// (1) TIME	
						if ((BUTTON_First == 9) && (BUTTON_Second == 0)) {
							Mode = MenuAction[0];
							ClockShowEn = 0;
							TIMER_ClockRefreshEn = 0;
							DATA_ReqEn = 0;
							LCD_Refresh = 1;
						// (2) CONTRAST	
						} else if ((BUTTON_First == 10) && (BUTTON_Second == 0)) {
							Mode = MenuAction[1];
							ClockShowEn = 0;
							TIMER_ClockRefreshEn = 0;
							DATA_ReqEn = 0;
							LCD_Refresh = 1;
						// (3) EVENTS	
						} else if ((BUTTON_First == 11) && (BUTTON_Second == 0)) {
							Mode = MenuAction[2];
							ClockShowEn = 0;
							TIMER_ClockRefreshEn = 0;
							DATA_ReqEn = 0;
							LCD_Refresh = 1;
						// (4) SETTINGS
						} else if ((BUTTON_First == 12) && (BUTTON_Second == 0)) {
							Mode = MenuAction[3];
							ClockShowEn = 0;
							TIMER_ClockRefreshEn = 0;
							DATA_ReqEn = 0;
							LCD_Refresh = 1;
						// (5) DIAGNOSTIC
						} else if ((BUTTON_First == 1) && (BUTTON_Second == 0)) {
							Mode = MenuAction[4];
							ClockShowEn = 0;
							TIMER_ClockRefreshEn = 0;
							DATA_ReqEn = 0;
							LCD_Refresh = 1;
						// (6) INFO
						} else if ((BUTTON_First == 2) && (BUTTON_Second == 0)) {
							Mode = MenuAction[5];
							ClockShowEn = 0;
							TIMER_ClockRefreshEn = 0;
							DATA_ReqEn = 0;
							LCD_Refresh = 1;
						// ����
						} else if ((BUTTON_First == 5) && (BUTTON_Second == 7)) {
							if (MenuShift < (MenuMax - 4)) {
								++ MenuShift;
								LCD_Refresh = 1;
							}
						// �����
						} else if ((BUTTON_First == 5) && (BUTTON_Second == 11)) {
							if (MenuShift) {
								-- MenuShift;
								LCD_Refresh = 1;
							}
						}else if ((BUTTON_First == 5)&&(BUTTON_Second == 0)){
							//delta_s=0;
						}							
						
						break;
/************************************************************************/
/* >>> ����� "����������" >>>                                           */
/************************************************************************/
					case MODE_INFO: // ����� ������������
						// (0) BACK
						//delta=0;
						if ((BUTTON_First == 7) && (BUTTON_Second == 0)) {
							Mode = MODE_MENU;
							ClockShowEn = 1;
							TIMER_ClockRefreshEn = 1;
							LCD_Refresh = 1;
						}
						break;
/************************************************************************/
/* >>> ����� "��������� �����" >>>                                      */
/************************************************************************/
					case MODE_TIME: // ����� ��������� �������
						// (0) NO
						if ((BUTTON_First == 7) && (BUTTON_Second == 0)) {
							Action = 0;
							LCD_Refresh = 1;
						// (1) YES	
						} else if ((BUTTON_First == 9) && (BUTTON_Second == 0)) {
							Action = 1;
							LCD_Refresh = 1;
						// (9) DEFAULT	
						} else if ((BUTTON_First == 6) && (BUTTON_Second == 0)) {
							Action = 9;
							LCD_Refresh = 1;
						// (5) �����	
						} else if ((BUTTON_First == 5) && (BUTTON_Second == 11)) {
							Action = 5;
							LCD_Refresh = 1;
						// (6) ����	
						} else if ((BUTTON_First == 5) && (BUTTON_Second == 7)) {
							Action = 6;
							LCD_Refresh = 1;
						// (7) �����	
						} else if ((BUTTON_First == 2) && (BUTTON_Second == 5)) {
							Action = 7;
							LCD_Refresh = 1;
						// (8) ������	
						} else if ((BUTTON_First == 4) && (BUTTON_Second == 5)) {
							Action = 8;
							LCD_Refresh = 1;
						}
						break;
						
/************************************************************************/
/* >>> ����� "�����������" >>>                                          */
/************************************************************************/
					case MODE_DIAGNOSTIC: // ����� ����������� ������
						if ((BUTTON_First == 7) && (BUTTON_Second == 0)) {
							Mode = MODE_MENU;
							ClockShowEn = 1;
							TIMER_ClockRefreshEn = 1;
							LCD_Refresh = 1;
						}
						break;
/************************************************************************/
/* >>> ����� "������: ����" >>>                                         */
/************************************************************************/
					case MODE_EVENTS_MENU: // ����� ����������� ������� �������	
						// (1) LIST
						if ((BUTTON_First == 9) && (BUTTON_Second == 0)) {
							Mode = MODE_EVENTS_LIST;
							LCD_Refresh = 1;
						// (2) INFO	
						} else if ((BUTTON_First == 10) && (BUTTON_Second == 0)) {
							Mode = MODE_EVENTS_INFO;
							LCD_Refresh = 1;
						// (3) CLEAR
						} else if ((BUTTON_First == 11) && (BUTTON_Second == 0)) {
							Mode = MODE_EVENTS_CLEAR;
							LCD_Refresh = 1;
						// (4) SAVE	
						} else if ((BUTTON_First == 12) && (BUTTON_Second == 0)) {
							Mode = MODE_EVENTS_SAVE_MENU;
							LCD_Refresh = 1;
						// (5) SETTINGS
						} else if ((BUTTON_First == 1) && (BUTTON_Second == 0)) {
							Mode = MODE_EVENTS_SETTINGS;
							LCD_Refresh = 1;
						// (0) BACK
						} else if ((BUTTON_First == 7) && (BUTTON_Second == 0)) {
							Mode = MODE_MENU;
							ClockShowEn = 1;
							TIMER_ClockRefreshEn = 1;
							RegMenuShift = 0;
							LCD_Refresh = 1;
						// ����
						} else if ((BUTTON_First == 5) && (BUTTON_Second == 7)) {
							if (RegMenuShift < (RegMenuMax - 4)) {
								++ RegMenuShift;
								LCD_Refresh = 1;
							}
						// �����
						} else if ((BUTTON_First == 5) && (BUTTON_Second == 11)) {
							if (RegMenuShift) {
								-- RegMenuShift;
								LCD_Refresh = 1;
							}
						}
						break;
/************************************************************************/
/* >>> ����� "������: ����������� �������" >>>                          */
/************************************************************************/
					case MODE_EVENTS_LIST: // ����� ����������� ������� �������	
						// (8) ������ - ���Ш� +1
						if ((BUTTON_First == 4) && (BUTTON_Second == 5)) {
							Action = 8;
							LCD_Refresh = 1;
						// (5) ����� - ���Ш� +10	
						} else if ((BUTTON_First == 5) && (BUTTON_Second == 11)) {
							Action = 5;
							LCD_Refresh = 1;
						// (7) ����� - ����� -1
						} else if ((BUTTON_First == 2) && (BUTTON_Second == 5)) {
							Action = 7;
							LCD_Refresh = 1;
						// (6) ���� - ����� -10	
						} else if ((BUTTON_First == 5) && (BUTTON_Second == 7)) {
							Action = 6;
							LCD_Refresh = 1;
						// (0) BACK	
						} else if ((BUTTON_First == 7) && (BUTTON_Second == 0)) {
							Action = 0;
							LCD_Refresh = 1;
						}
						break;
/************************************************************************/
/* >>> ����� "������: ����" >>>                                         */
/************************************************************************/
					case MODE_EVENTS_INFO: // ����� ��������� ���� �������
						// (0) BACK
						if ((BUTTON_First == 7) && (BUTTON_Second == 0)) {
							Mode = MODE_EVENTS_MENU;
							LCD_Refresh = 1;
						}
						break;
/************************************************************************/
/* >>> ����� "������: �����" >>>                                        */
/************************************************************************/
					case MODE_EVENTS_CLEAR: // ����� ��������� ���������
						// (1) YES
						if ((BUTTON_First == 9) && (BUTTON_Second == 0)) {
							Action = 1;
							LCD_Refresh = 1;
						// (0) NO	
						} else if ((BUTTON_First == 7) && (BUTTON_Second == 0)) {
							Action = 0;
							LCD_Refresh = 1;
						}
						break;
/************************************************************************/
/* >>> ����� "��������� ����" >>>                                       */
/************************************************************************/
					case MODE_SETTINGS: // ����� ���������
						// (7) �����
						if ((BUTTON_First == 2) && (BUTTON_Second == 5)) {
							Action = 7;
							LCD_Refresh = 1;
						// (8) ������	
						} else if ((BUTTON_First == 4) && (BUTTON_Second == 5)) {
							Action = 8;
							LCD_Refresh = 1;
						// (5) �����	
						} else if ((BUTTON_First == 5) && (BUTTON_Second == 11)) {
							Action = 5;
							LCD_Refresh = 1;
						// (6) ����	
						} else if ((BUTTON_First == 5) && (BUTTON_Second == 7)) {
							Action = 6;
							LCD_Refresh = 1;
						// (1) YES	
						} else if ((BUTTON_First == 9) && (BUTTON_Second == 0)) {
							Action = 1;
							LCD_Refresh = 1;
						// (0) NO	
						} else if ((BUTTON_First == 7) && (BUTTON_Second == 0)) {
							Action = 0;
							LCD_Refresh = 1;
						// (9) DEFAULT	
						} else if ((BUTTON_First == 6) && (BUTTON_Second == 0)) {
							Action = 9;
							LCD_Refresh = 1;
						}
						break;
/************************************************************************/
/* >>> ����� "��������� �������" >>>                                    */
/************************************************************************/
					case MODE_EVENTS_SETTINGS: // ����� ��������� �������
						// (7) �����
						if ((BUTTON_First == 2) && (BUTTON_Second == 5)) {
							Action = 7;
							LCD_Refresh = 1;
						// (8) ������
						} else if ((BUTTON_First == 4) && (BUTTON_Second == 5)) {
							Action = 8;
							LCD_Refresh = 1;
						// (5) �����
						} else if ((BUTTON_First == 5) && (BUTTON_Second == 11)) {
							Action = 5;
							LCD_Refresh = 1;
						// (6) ����
						} else if ((BUTTON_First == 5) && (BUTTON_Second == 7)) {
							Action = 6;
							LCD_Refresh = 1;
						// (1) YES
						} else if ((BUTTON_First == 9) && (BUTTON_Second == 0)) {
							Action = 1;
							LCD_Refresh = 1;
						// (0) NO
						} else if ((BUTTON_First == 7) && (BUTTON_Second == 0)) {
							Action = 0;
							LCD_Refresh = 1;
						// (9) DEFAULT
						} else if ((BUTTON_First == 6) && (BUTTON_Second == 0)) {
							Action = 9;
							LCD_Refresh = 1;
						}
						break;
/************************************************************************/
/* >>> ����� "��������" >>>                                             */
/************************************************************************/
					case MODE_CONTRAST: // ����� ��������� ���������
						// (7) �����
						if ((BUTTON_First == 2) && (BUTTON_Second == 5)) {
							Action = 7;
							LCD_Refresh = 1;
						// (8) ������
						} else if ((BUTTON_First == 4) && (BUTTON_Second == 5)) {
							Action = 8;
							LCD_Refresh = 1;
						// (5) �����
						} else if ((BUTTON_First == 5) && (BUTTON_Second == 11)) {
							Action = 5;
							LCD_Refresh = 1;
						// (6) ����
						} else if ((BUTTON_First == 5) && (BUTTON_Second == 7)) {
							Action = 6;
							LCD_Refresh = 1;
						// (1) YES
						} else if ((BUTTON_First == 9) && (BUTTON_Second == 0)) {
							Action = 1;
							LCD_Refresh = 1;
						// (0) NO
						} else if ((BUTTON_First == 7) && (BUTTON_Second == 0)) {
							Action = 0;
							LCD_Refresh = 1;
						// (9) DEFAULT
						} else if ((BUTTON_First == 6) && (BUTTON_Second == 0)) {
							Action = 9;
							LCD_Refresh = 1;
						}
						break;
/************************************************************************/
/* >>> ����� "����� �������� ������" >>>                                */
/************************************************************************/
					case MODE_TRANSP_SELECT:
						// (1) 9600
						if ((BUTTON_First == 9) && (BUTTON_Second == 0)) {
							Action = 1;
							LCD_Refresh = 1;
						// (2) 57600	
						} else if ((BUTTON_First == 10) && (BUTTON_Second == 0)) {
							Action = 2;
							LCD_Refresh = 1;
						// (3) 115200
						} else if ((BUTTON_First == 11) && (BUTTON_Second == 0)) {
							Action = 3;
							LCD_Refresh = 1;
						// (0) BACK
						} else if ((BUTTON_First == 7) && (BUTTON_Second == 0)) {
							Action = 0;
							LCD_Refresh = 1;
						}
						break;						
/************************************************************************/
/* >>> ����� "�������������� �������� � ���������� �����"               */
/************************************************************************/
					case MODE_TRANSP_CONFIRM:	
						// (1) YES
						if ((BUTTON_First == 9) && (BUTTON_Second == 0)) {
							Action = 1;
							LCD_Refresh = 1;
						// (0) NO
						} else if ((BUTTON_First == 7) && (BUTTON_Second == 0)) {
							Action = 0;
							LCD_Refresh = 1;
						}
						break;
/************************************************************************/
/* >>> ����� "���������� �������"                                       */
/************************************************************************/
					case MODE_EVENTS_SAVE_MENU:	
						// (1) YES
						if ((BUTTON_First == 9) && (BUTTON_Second == 0)) {
							Action = 1;
							LCD_Refresh = 1;
						// (0) NO
						} else if ((BUTTON_First == 7) && (BUTTON_Second == 0)) {
							Action = 0;
							LCD_Refresh = 1;
						}
						break;
/************************************************************************/
/* >>> ����� "���������� ������� ���������"                             */
/************************************************************************/
					case MODE_EVENTS_SAVE_COMPLETED:
						// (1) OK
						if ((BUTTON_First == 9) && (BUTTON_Second == 0)) {
							Mode = MODE_EVENTS_MENU;
							LCD_Refresh = 1;
						}
						break;
/************************************************************************/
/* >>> ����� "�������������� ����"                                      */
/************************************************************************/
					case MODE_EDIT: // ����� ����� ����
						if ((BUTTON_First == 9) && (BUTTON_Second == 0)) {
						/************************************************************************/
						/* <1> - ���� "1"                                                       */
						/************************************************************************/
							EDIT_ValAdd (1);
						} else if ((BUTTON_First == 10) && (BUTTON_Second == 0)) {
						/************************************************************************/
						/* <2> - ���� "2"                                                       */
						/************************************************************************/
							EDIT_ValAdd (2);
						} else if ((BUTTON_First == 11) && (BUTTON_Second == 0)) {
						/************************************************************************/
						/* <3> - ���� "3"                                                       */
						/************************************************************************/
							EDIT_ValAdd (3);
						} else if ((BUTTON_First == 12) && (BUTTON_Second == 0)) {
						/************************************************************************/
						/* <4> - ���� "4"                                                       */
						/************************************************************************/
							EDIT_ValAdd (4);
						} else if ((BUTTON_First == 1) && (BUTTON_Second == 0)) {
						/************************************************************************/
						/* <5> - ���� "5"                                                       */
						/************************************************************************/
							EDIT_ValAdd (5);
						} else if ((BUTTON_First == 2) && (BUTTON_Second == 0)) {
						/************************************************************************/
						/* <6> - ���� "6"                                                       */
						/************************************************************************/
							EDIT_ValAdd (6);
						} else if ((BUTTON_First == 3) && (BUTTON_Second == 0)) {
						/************************************************************************/
						/* <7> - ���� "7"                                                       */
						/************************************************************************/
							EDIT_ValAdd (7);
						} else if ((BUTTON_First == 4) && (BUTTON_Second == 0)) {
						/************************************************************************/
						/* <8> - ���� "8"                                                       */
						/************************************************************************/
							EDIT_ValAdd (8);
						} else if ((BUTTON_First == 6) && (BUTTON_Second == 0)) {
						/************************************************************************/
						/* <9> - ���� "9"                                                       */
						/************************************************************************/
							EDIT_ValAdd (9);
						} else if ((BUTTON_First == 7) && (BUTTON_Second == 0)) {
						/************************************************************************/
						/* <0> - ���� "0"                                                       */
						/************************************************************************/
							EDIT_ValAdd (0);
						} else if ((BUTTON_First == 8) && (BUTTON_Second == 0)) {
							
						/************************************************************************/
						/* <ENTER> - ���� "ENTER"                                               */
						/************************************************************************/
							if (EDIT_Val > 0xFFFF) {
							} else {
								if (EDIT_ValMultiplier == 0) EDIT_ValMultiplier = 1;
								SendToPlc_Adress[0] = EDIT_ValPlcAdr;
								SendToPlc_Value[0] = ( (EDIT_Val / EDIT_ValMultiplier) * EDIT_ValDivider ) - EDIT_ValAddition;
								SendToPlc_Ready[0] = SEND_ATTEMPTS;
								SendToPlc_Amount = 1;
							}
// ��������������� �������� ��������
// APD_ValTemp = APD_Val * APD_ValMultiplier;

// ����������: (x + div/2) / div =
// APD_ValResult = ((APD_ValTemp + (APD_ValDivider >> 1)) / APD_ValDivider) + APD_ValAddition;
							Mode = MODE_NORMAL;
							TIMER_EditValBlinkEn = 0;
							TIMER_EditValBlink = 0;
	
							LCD_Refresh = 1;
						} else if ((BUTTON_First == 5) && (BUTTON_Second == 8)) {
						/************************************************************************/
						/* <CLEAR> - ���� "CLEAR"                                               */
						/************************************************************************/
							EDIT_Val = 0;
							// EDIT_ValNeg = 0;
						} else if ((BUTTON_First == 5) && (BUTTON_Second == 6)) {
						/************************************************************************/
						/* <-> - ���� "-"                                                       */
						/************************************************************************/
							EDIT_Val *= -1;								
						} else if ((BUTTON_First == 5) && (BUTTON_Second == 12)) {
						/************************************************************************/
						/* <F3> - ���� "�����"                                                  */
						/************************************************************************/
							Mode = MODE_NORMAL;
							TIMER_EditValBlinkEn = 0;
							TIMER_EditValBlink = 0;
							LCD_Refresh = 1;
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
ISR (TCC0_OVF_vect)
{ 
// ������� ������ � �������� 1 ��
	// ����������� �������
	if (TIMER_FIRSTCfgEn)
	{
		++ TIMER_FIRSTSCfg;
	}
	if (TIMER_RMDCfgEn) {
		++ TIMER_RMDCfg;
	}
	if (TIMER_RMDCfgEn_MBUS) {
		++ TIMER_RMDCfgMBUS;
	}
	
	if (TIMER_ClockRefreshEn) {
		++ TIMER_ClockRefresh;
	}

	if (TIMER_DataRequestEn)// && MB0_State.flags)
	{
		++ TIMER_DataRequest;
	}
	
	if (TIMER_RequestTimeoutEn)// && MB0_State.flags)
	{
		++ TIMER_RequestTimeout;
	}
	
	if (TIMER_RegSaveEn) {
		++ TIMER_RegSave;
	}
	
	if (TIMER_EditValBlinkEn) {
		++ TIMER_EditValBlink;
	}
s_timeout++;
	//{
		//s_timeout=0;
		//MB0_State.flags = 1;
		//ADP_flag = 1;
		//numADPmes = 0;
		//nu=0;
		//free(MBUS_ADP);
	//}
}
/************************************************************************/
/*                             ����������                               */
/************************************************************************/
ISR (USARTE0_TXC_vect) {
	switch (NET_Mode) {
		case NET_MODE_CONFIG:
			if ((RMD_CfgCmdCharCnt <= (RMD_Cfg[RMD_CfgCmdCnt][0] - 1)) && (RMD_CfgCmdCharCnt != 0)) {
							
				usart_put (&USARTE0, RMD_Cfg[RMD_CfgCmdCnt][RMD_CfgCmdCharCnt + 1]);

				++ RMD_CfgCmdCharCnt;
				if (RMD_CfgCmdCharCnt > RMD_Cfg[RMD_CfgCmdCnt][0] - 1) {
					RMD_CfgCmdCharCnt = 0x00;
					++ RMD_CfgCmdCnt;
					usart_set_tx_interrupt_level (&USARTE0, USART_TXCINTLVL_OFF_gc);
				}
			}
			break;
		case NET_MODE_MASTER:
			break;
		case NET_MODE_TRANSP:
			
			break;

		case NET_MODE_REGSAVE:
			if ((REGSAVE_MesByteCnt < REG_BLOCK_SIZE) && (REGSAVE_MesByteCnt != 0)) {

				usart_put (&USARTE0, * (REGSAVE_Buff + REGSAVE_MesByteCnt));
				
				++ REGSAVE_MesByteCnt;
				if (REGSAVE_MesByteCnt >= REG_BLOCK_SIZE) {
					REGSAVE_MesByteCnt = 0;
					++ REGSAVE_MesCnt;
					usart_set_tx_interrupt_level (&USARTE0, USART_TXCINTLVL_OFF_gc);
				}
			}
			break;
		default:
			break;
	}
/*
	if (TIMER_RMDCfgEn) {
		if ((RMD_CfgCmdCharCnt <= (RMD_Cfg[RMD_CfgCmdCnt][0] - 1)) && (RMD_CfgCmdCharCnt != 0)) {
			usart_put (&USARTE0, RMD_Cfg[RMD_CfgCmdCnt][RMD_CfgCmdCharCnt + 1]);
			++ RMD_CfgCmdCharCnt;
			if (RMD_CfgCmdCharCnt > RMD_Cfg[RMD_CfgCmdCnt][0] - 1) {	
				RMD_CfgCmdCharCnt = 0x00;
				++ RMD_CfgCmdCnt;
				usart_set_tx_interrupt_level (&USARTE0, USART_TXCINTLVL_OFF_gc);
			}
		}
	}
*/
}
ISR (USARTE0_RXC_vect) {
	// �������� ���� �� ������
	RMD_RecMesCharBuf = usart_get (&USARTE0);
	
	switch (NET_Mode) {
		case NET_MODE_CONFIG:
			if ((RMD_CfgAckMesCharCnt <= (RMD_CfgAckMesLen - 1)) && (RMD_CfgAckMesReady == 0)) {
				RMD_CfgAckMes[RMD_CfgAckMesCharCnt] = RMD_RecMesCharBuf;
				++ RMD_CfgAckMesCharCnt;
				if (RMD_CfgAckMesCharCnt > (RMD_CfgAckMesLen - 1)) {
					RMD_CfgAckMesCharCnt = 0;
					RMD_CfgAckMesReady = 1;
				}
			}
			break;
		case NET_MODE_MASTER:
			// ���� ������ ������ ������� - '$'
			if (RMD_RecMesCharBuf == '$') {
				RMD_RecMesCharCnt = 0;
				RMD_RecMesInProgress = 1;
				RMD_RecMesReady = 0;
			}	
			if ((RMD_RecMesInProgress) && (RMD_RecMesCharCnt <= RMD_RecMesBufLen - 1)) {
				if (RMD_RecMesCharCnt == RMD_RecMesBufLen - 1) {
					RMD_RecMesInProgress = 0;
					RMD_RecMesReady = 1;
				}
				RMD_RecMesBuf[RMD_RecMesCharCnt] = RMD_RecMesCharBuf;
				++ RMD_RecMesCharCnt;
			}
			break;
		case NET_MODE_TRANSP:
			if (usart_data_register_is_empty (&USARTC0)) {
				// �������� ��������� ����� �� ����������� � RS-232
				usart_put (&USARTC0, RMD_RecMesCharBuf);
			}
			break;
		case NET_MODE_REGSAVE:
			break;
		default:
			break;
	}		
/*		
	RMD_RecMesCharBuf = usart_get (&USARTE0);
	// ���� � ������ ������������
	if (TIMER_RMDCfgEn) {		
		if ((RMD_CfgAckMesCharCnt <= (RMD_CfgAckMesLen - 1)) && (RMD_CfgAckMesReady == 0)) {
			RMD_CfgAckMes[RMD_CfgAckMesCharCnt] = RMD_RecMesCharBuf;
			++ RMD_CfgAckMesCharCnt;
			if (RMD_CfgAckMesCharCnt > (RMD_CfgAckMesLen - 1)) {
				RMD_CfgAckMesCharCnt = 0;
				RMD_CfgAckMesReady = 1;
			}
		}
	}
	
	// ���� � ������� ������
	if (RMD_RecEn) {
		// ���� ������ ������ ������� - '$'
		if (RMD_RecMesCharBuf == '$') {
			RMD_RecMesCharCnt = 0;
			RMD_RecMesInProgress = 1;
			RMD_RecMesReady = 0;
		}	
		if ((RMD_RecMesInProgress) && (RMD_RecMesCharCnt <= RMD_RecMesBufLen - 1)) {
			if (RMD_RecMesCharCnt == RMD_RecMesBufLen - 1) {
//				LCD_Refresh = 1;
				RMD_RecMesInProgress = 0;
				RMD_RecMesReady = 1;
			}
			RMD_RecMesBuf[RMD_RecMesCharCnt] = RMD_RecMesCharBuf;
			++ RMD_RecMesCharCnt;
		}
	}	
*/
}
/************************************************************************/
/*                             RS-232 ADP                               */
/************************************************************************/
/*ISR (USARTC0_TXC_vect) {
	switch (NET_Mode) {
		case NET_MODE_CONFIG:
			
			break;
			
		case NET_MODE_MASTER:
			ADP_TransContinue ();
			break;
			
		case NET_MODE_TRANSP:
			
			break;
			
		case NET_MODE_REGSAVE:
			
			break;
			
		default:
			break;
	}
}*/
ISR (USARTC0_RXC_vect) {
	// �������� ���� �� ������
	ADP_RecMesCharBuf = usart_get (&USARTC0);
	switch (NET_Mode) {
		case NET_MODE_CONFIG:
			break;
		case NET_MODE_MASTER:
			TIMER_RS232_START;
			// ���� ��������� ���� ���������� ���������� ���������
			// ����� ���������, ��:
			//if (ADP_RecMesTimeout) {
				//ADP_RecMesTimeout = 0;
				//// �������� ���� ����� �������. ������ ����������
				//ADP_RecMesByteCnt = 0;
		////		ADP_RecMesCharBuf = usart_get (&USARTC0);
				//ADP_RecMesBuf[ADP_RecMesByteCnt] = ADP_RecMesCharBuf;
//
			//} else {
				//// ���������� ���� ������� �������
				//++ ADP_RecMesByteCnt;
		////		ADP_RecMesCharBuf = usart_get (&USARTC0);
				//ADP_RecMesBuf[ADP_RecMesByteCnt] = ADP_RecMesCharBuf;
			//}
			ADP_RecMesBuf[ADP_RecMesLen++] = ADP_RecMesCharBuf;
			break;
		case NET_MODE_TRANSP:
			if (usart_data_register_is_empty (&USARTE0)) {
				// �������� ��������� ����� �� RS-232 � ����������
				usart_put (&USARTE0, ADP_RecMesCharBuf);
			}
			break;
		case NET_MODE_REGSAVE:
			break;
		default:
			break;
	}
/*		
	TIMER_RS232_START;
	// ���� ��������� ���� ���������� ���������� ���������
	// ����� ���������, ��:
	if (ADP_RecMesTimeout) {
		ADP_RecMesTimeout = 0;
		// �������� ���� ����� �������. ������ ����������
		ADP_RecMesByteCnt = 0;
//		ADP_RecMesCharBuf = usart_get (&USARTC0);
		ADP_RecMesBuf[ADP_RecMesByteCnt] = ADP_RecMesCharBuf;

	} else {
		// ���������� ���� ������� �������
		++ ADP_RecMesByteCnt;
//		ADP_RecMesCharBuf = usart_get (&USARTC0);				
		ADP_RecMesBuf[ADP_RecMesByteCnt] = ADP_RecMesCharBuf;
	}*/
}

/************************************************************************/
/*                             MODBUS                                   */
/************************************************************************/
ISR (USARTE1_RXC_vect)
{
	MB0_State.Buf_byte = usart_get (&USARTE1);
	TIMER_MBUS485_START;
	if (MB0_State.lenRx485<=56)
	{
		//MB0_State.flags = 0;
		MB0_State.UART_RxBuf[MB0_State.lenRx485] = MB0_State.Buf_byte;
		++ MB0_State.lenRx485;
	}
	else
	{
		MB0_State.Buf_Ful = 1;
	}
}
ISR (USARTE1_TXC_vect)
{
	//if(usart_tx_is_complete(&USARTE1))
	//{
		PORTE.OUT &= ~ TX_EN;
	//}		
}
/************************************************************************/
/*                             MODBUS   RS232                           */
/************************************************************************/
ISR (USARTF0_RXC_vect)
{
	MB0_State.Buf_byte = usart_get (&USARTF0);
	TIMER_MBUS232_START;
	if (MB0_State.lenRx232<=56)
	{
		MB0_State.UART_RxBuf[MB0_State.lenRx232] = MB0_State.Buf_byte;
		++ MB0_State.lenRx232;
	}
	else
	{
		MB0_State.Buf_Ful = 1;
	}
}
/************************************************************************/
/*                             MODBUS   RS232_RMD                       */
/************************************************************************/
ISR (USARTC1_TXC_vect) {
	switch (NET_Mode) {
		case NET_MODE_CONFIG:
			if ((RMD_CfgCmdCharCntMBUS <= (RMD_CfgMBUS[RMD_CfgCmdCntMBUS][0] - 1)) && (RMD_CfgCmdCharCntMBUS != 0)) {
							
				usart_put (&USARTC1, RMD_CfgMBUS[RMD_CfgCmdCntMBUS][RMD_CfgCmdCharCntMBUS + 1]);

				++ RMD_CfgCmdCharCntMBUS;
				if (RMD_CfgCmdCharCntMBUS > RMD_Cfg[RMD_CfgCmdCntMBUS][0] - 1) {
					RMD_CfgCmdCharCntMBUS = 0x00;
					++ RMD_CfgCmdCntMBUS;
					usart_set_tx_interrupt_level (&USARTC1, USART_TXCINTLVL_OFF_gc);
				}
			}
			break;
		case NET_MODE_MASTER:
			break;
		case NET_MODE_TRANSP:
			
			break;

		//case NET_MODE_REGSAVE:
			//if ((REGSAVE_MesByteCnt < REG_BLOCK_SIZE) && (REGSAVE_MesByteCnt != 0)) {
//
				//usart_put (&USARTE0, * (REGSAVE_Buff + REGSAVE_MesByteCnt));
				//
				//++ REGSAVE_MesByteCnt;
				//if (REGSAVE_MesByteCnt >= REG_BLOCK_SIZE) {
					//REGSAVE_MesByteCnt = 0;
					//++ REGSAVE_MesCnt;
					//usart_set_tx_interrupt_level (&USARTE0, USART_TXCINTLVL_OFF_gc);
				//}
			//}
			//break;
		default:
			break;
	}
/*
	if (TIMER_RMDCfgEn) {
		if ((RMD_CfgCmdCharCnt <= (RMD_Cfg[RMD_CfgCmdCnt][0] - 1)) && (RMD_CfgCmdCharCnt != 0)) {
			usart_put (&USARTE0, RMD_Cfg[RMD_CfgCmdCnt][RMD_CfgCmdCharCnt + 1]);
			++ RMD_CfgCmdCharCnt;
			if (RMD_CfgCmdCharCnt > RMD_Cfg[RMD_CfgCmdCnt][0] - 1) {
				RMD_CfgCmdCharCnt = 0x00;
				++ RMD_CfgCmdCnt;
				usart_set_tx_interrupt_level (&USARTE0, USART_TXCINTLVL_OFF_gc);
			}
		}
	}
*/
}
ISR (USARTC1_RXC_vect)
{
	MB0_State.Buf_byte = usart_get (&USARTC1);
	switch (NET_Mode) {
		case NET_MODE_CONFIG:
			if ((RMD_CfgAckMesCharCntMBUS <= (RMD_CfgAckMesLenMBUS - 1)) && (RMD_CfgAckMesReadyMBUS == 0)) {
				RMD_CfgAckMesMBUS[RMD_CfgAckMesCharCntMBUS] = MB0_State.Buf_byte;
				++ RMD_CfgAckMesCharCntMBUS;
				if (RMD_CfgAckMesCharCntMBUS > (RMD_CfgAckMesLenMBUS - 1)) {
					RMD_CfgAckMesCharCntMBUS = 0;
					RMD_CfgAckMesReadyMBUS = 1;
				}
			}
			break;
		case NET_MODE_MASTER:
			TIMER_MBUSRMD_START;
			if (MB0_State.lenRxrmd<=56)
			{
		//MB0_State.flags = 0;
				MB0_State.UART_RxBuf[MB0_State.lenRxrmd] = MB0_State.Buf_byte;
				++ MB0_State.lenRxrmd;
			}
			else
			{
				MB0_State.Buf_Ful = 1;
			}
			default:
			break;
	}			
}
// ������ ��������� �������� ������� ��������� ADP
ISR (TCD1_OVF_vect) {
	TIMER_RS232_STOP;
	// �������� ������� ����� ��������� ���������:
	// ���������� ���� �������� ������� �
	// ��������� � ����� �����
	//ADP_RecMesTimeout = 1;
	if (ADP_RecMesLen > 4) {
		//ADP_RecMesLen = ADP_RecMesByteCnt + 1;
		//ADP_RecMesByteCnt = 0;
		ADP_RecMesReady = 1;
		if (ADP_RecMesLen!=((ADP_RecMesBuf[4]|ADP_RecMesBuf[5]<<8)+8))
		{
			ADP_RecMesReady = 0;
			ADP_RecMesLen = 0;
		}
		for (i=0;i<ADP_RecMesLen;i++)
		{
			ADP_Buf[i] = ADP_RecMesBuf[i];
		}
	}
}

ISR (TCE1_CCA_vect)
{
	//MB0_State.flags |= 0x01;//���� ��������� ������ 1,5 �������
}

ISR (TCE1_OVF_vect)
{
	TIMER_MBUS485_STOP;
	if (MB0_State.lenRx485 > 1) 
	{
		MB0_State.flags |= ANSWER;
		MB0_State.flags |= MBUS_RS485;
		MB0_State.lenRx = MB0_State.lenRx485;
		MB0_State.lenRx485 = 0;
		MB0_State.Buf_byte = 0;
		MB0_State.TelCompleet = 1;
		for (i=0;i<MB0_State.lenRx;i++)
		{
			MBUS_Buf[i] = MB0_State.UART_RxBuf[i];
		}
	}
}
ISR (TCF0_OVF_vect)
{
	TIMER_MBUS232_STOP;
	if (MB0_State.lenRx232 > 1)
	{
		MB0_State.flags |= ANSWER;
		MB0_State.flags |= MBUS_RS232;
		MB0_State.lenRx = MB0_State.lenRx232;
		MB0_State.lenRx232 = 0;
		MB0_State.Buf_byte = 0;
		MB0_State.TelCompleet = 1;
		for (i=0;i<MB0_State.lenRx;i++)
		{
			MBUS_Buf[i] = MB0_State.UART_RxBuf[i];
		}
	}
}
ISR (TCC1_OVF_vect)
{
	TIMER_MBUSRMD_STOP;
	if (MB0_State.lenRxrmd > 1) 
	{
		MB0_State.flags |= ANSWER;
		MB0_State.flags |= MBUS_RMD;
		MB0_State.lenRx = MB0_State.lenRxrmd;
		MB0_State.lenRxrmd = 0;
		MB0_State.Buf_byte = 0;
		MB0_State.TelCompleet = 1;
		for (i=0;i<MB0_State.lenRx;i++)
		{
			MBUS_Buf[i] = MB0_State.UART_RxBuf[i];
		}
	}
}
ISR (TCE0_OVF_vect)		///200 ms
{
	TIMER_ADP_TIMEOUT_STOP;
	if(MB0_State.flags&ANSWER)
	{
		//PORTE.OUT |= TX2;
		//MBUS_Send_17();
		ADP_flag = 1;
		numADPmes = 0;
		MB0_State.Error_MBUS++;
		//MBUS_Register[0][0] = 0x1234;
		//MBUS_Register[1][0] = 0x4567;
		switch(MB0_State.command)
		//switch(MBUS_Buf[1])
		{
			case MBUS_FUNC_GETREG:
				MBUS_Send_04();
			break;
			case MBUS_FUNC_GETREGS:
				MBUS_Send_04();
			break;
			case MBUS_FUNC_SETREG:
				MBUS_Send_06();
				ADP_timeout = 1;
			break;
			case MBUS_FUNC_SETREGS:
				MBUS_Send_10();
				ADP_timeout = 1;
			break;
			case MBUS_FUNC_GETID:
				MBUS_Send_17();
			break;
			default:
			break;
		}
	}
}

ISR (TCD0_OVF_vect)
{
	TIMER_ADP_2MS_STOP;
	ADP_flag = 1;
}
