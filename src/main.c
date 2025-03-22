/************************************************************************/
/*                       Общие системные настройки                   */
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
	for (i=0;i<Q/2;i++)//цикл по пришедшему по ADP сообщению
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
/*                 Прототипы функций                                    */
/************************************************************************/
// Аппаратные таймеры
void		TIMER_1msInit (void);
void		TIMER_Rs232Init (void);
void		TIMER_Rs232Start (void);
void		TIMER_Rs232Stop (void);
// ЦАП
void		DAC_Init (void);
void		DAC_New (void);
// Работа с микросхемой RTC
uint8_t		TIME_Set (void);
uint8_t		TIME_Get (void);
// Установка времени
uint8_t		TIME_SetManual (uint8_t Hours, uint8_t Minutes, uint8_t Seconds, uint8_t Day, uint8_t Date, uint8_t Month, uint8_t Year);
// Отображение часов и даты на экране
void		CLOCK_ShowOn (uint8_t str, uint8_t col);
void		DATE_ShowOn (uint8_t str, uint8_t col);
// Работа с линией RS-232 по протоколу ADP
void		ADP_TransStart (void);
void		ADP_TransContinue (void);
uint16_t	ADP_TransMesBuild (uint8_t RecAdr, uint8_t TransAdr, uint8_t CmdCode, uint8_t ClarkAdr, uint16_t AmountOfBytes, uint16_t *CmdBody);
uint16_t	ADP_TransEventRequest (uint8_t RecAdr, uint8_t TransAdr, uint8_t CmdCode, uint8_t ClarkAdr);

uint16_t	ADP_TransSendToPlcMesBuild (uint8_t RecAdr, uint8_t TransAdr, uint8_t CmdCode, uint8_t ClarkAdr, uint16_t Adress, uint16_t Value);
uint8_t		ADP_Rec (void);

void		ADP_Function_TransParams (void);
void		ADP_Function_RecParams (void);
void		ADP_Function_ReqEvents (void);

// Подготовка данных для запроса
void		FOCUS_Adr (void); // Подсчёт количества адресов для запроса в текущем фокусе
void		FOCUS_Sort (uint16_t * Array, uint8_t Size); // Сортирвка массива по возрастанию
void		FOCUS_Block (void); // Подготовка к запросу: группировка и выделение блоков
void		FOCUS_Space (void); // Вычисление расстояний между запрашиваемыми адресами
void		FOCUS_CntReset (void);
/************************************************************************/
/* -> ПРОГРАММНЫЕ ТАЙМЕРЫ								*/
/************************************************************************/
// Таймер обмена сообщениями с ПЧ
volatile uint16_t TIMER_DataRequest = 0;
volatile uint16_t TIMER_DataRequestPer = 10000;
volatile uint8_t TIMER_DataRequestEn = 0;

// Таймер обновления часов
volatile uint16_t TIMER_ClockRefresh = 0;
volatile uint16_t TIMER_ClockRefreshPer = 300;
volatile uint8_t TIMER_ClockRefreshEn = 0;

// Таймер отсылки конфигурационных сообщений на радиомодем для клавиатуры
volatile uint16_t TIMER_RMDCfg = 0;
volatile uint16_t TIMER_RMDCfgPer = 100;
volatile uint8_t TIMER_RMDCfgEn = 1;

// Таймер отсылки конфигурационных сообщений на радиомодем для связи по MODBUS
volatile uint16_t TIMER_RMDCfgMBUS = 0;
volatile uint16_t TIMER_RMDCfgPerMBUS = 100;
volatile uint8_t TIMER_RMDCfgEn_MBUS = 0;

// Таймер временной задержки между посылками по протоколу ADP 
volatile uint16_t TIMER_RequestTimeout = 0;
volatile uint16_t TIMER_RequestTimeoutPer = 5;
volatile uint8_t TIMER_RequestTimeoutEn = 0;

// Таймер передачи записей журнала на ПК 
volatile uint16_t TIMER_RegSave = 0;
volatile uint16_t TIMER_RegSavePer = 1000;
volatile uint8_t TIMER_RegSaveEn = 0;

// Таймер передачи записей журнала на ПК 
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
/* -> ПЕРЕМЕННЫЕ И КОНСТАНТЫ ПРОЕКТА APD      */
/************************************************************************/

// Типы полей в проекте
typedef enum APD_field_type {
	APD_FIELD_TYPE_SPLASH = 0, // ЗАСТАВКА
	APD_FIELD_TYPE_VALUE = 1, // ЧИСЛО
	APD_FIELD_TYPE_TEXT = 2, // ТЕКСТ
	APD_FIELD_TYPE_DATE = 3, // ДАТА
	APD_FIELD_TYPE_TIME_ = 4,  // ВРЕМЯ
	APD_FIELD_TYPE_LIST = 5,  // СПИСОК
	APD_FIELD_TYPE_32BIT = 10, // ЧИСЛО
 } APD_field_type_t;

// Форматы вывода числа 
typedef enum APD_field_format {
	APD_FIELD_FORMAT_BIN = 0x00, // Двоичное
	APD_FIELD_FORMAT_HEX = 0x10, // Шестнадцатиричное
	APD_FIELD_FORMAT_DEC_WITHOUT_NULLS = 0x20, // Десятичное знаковое без нулей слева
	APD_FIELD_FORMAT_DEC_WITH_NULLS = 0x30, // Десятичное знаковое с нулями слева
 } APD_field_format_t;
 
// Разные счётчики
uint8_t Id8 = 0; // Счётчик байт
uint8_t IdFrame = 0; // Счётчик кадров
uint8_t IdField = 0; // Счётчик полей
uint8_t IdAction = 0; // Счётчик кнопкодействий
uint16_t IdEvent = 0; // Счётчик событий

// Буфер запрашиваемых событий
uint16_t APD_EventsBuf[ADP_EVENT_REQUEST];

signed int CLOCK_ShowStr = 0; // Номер строки для вывода часов
uint8_t CLOCK_ShowCol = 12; // Номер столбца для вывода часов

signed int DATE_ShowStr = 0; // Номер строки для вывода даты
uint8_t DATE_ShowCol = 0; // Номер столбца для вывода часов

uint16_t APD_FrameRefreshRate = 0; // Период опроса кадров (0 - авто, 1 - 100мс, 2 - 200, 3...10)
uint16_t APD_EventReqRatio = 0; // Период опроса событий (0 - авто, 1 - 100мс, 2 - 200, 3...10)
uint16_t APD_BaudRateIndex = 3; // Скорость связи с ПЛК (0 - 9600, 1 - 19200, 2 - 38400, 3 - 57600)
uint8_t APD_FrameAmount = 0; // Количество кадров в проекте
uint8_t APD_FrameStartNum = 0; // Номер стартового кадра

volatile uint16_t APD_FrameCur = 0; // Текущий кадр проекта
signed int APD_FrameDisplayShift = 0; // Сдвиг кадра на дисплее

uint16_t APD_FramesStartAdr = 0; // Адрес описания кадров
uint16_t APD_EventsStartAdr = 0; // Адрес описания списка событий
uint16_t APD_EventAdr = 0; // Адрес начала полей списка событий
uint16_t APD_EventIdAmount = 0; // Количество индексов событий

uint8_t APD_EventAdrArray[4] = {0, 0, 0, 0}; // Адреса абонентов для запроса списка событий

uint8_t APD_EventAdrNum = 0; // Количество абонентов для запроса СС
uint16_t AdrTemp = 0; // Временный буфер адреса
uint8_t EventEn = 0; // ФЛАГ: Разрешение на запрос списка СС
uint8_t EventTransAct = 0; // ФЛАГ: Можно НАЧИНАТЬ запрос СС
uint8_t EventTransEn = 0; // ФЛАГ: Можно ПРОДОЛЖАТЬ запрос СС

/************************************************************************/
/* -> НАСТРОЙКИ ЖУРНАЛА СОБЫТИЙ                      */
/************************************************************************/

int8_t EventStorage[4][EVENT_CODES_MAX]; // Массив счётчиков принимаемых событий от ПЧ

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

int8_t EventStorage_TOP = 5 /*3*/; // Верхний предел счётчика событий
int8_t EventStorage_BOT = - 6 /*- 4*/; // Нижний предел счётчика событий

uint8_t REG_SET_ShowOnce = 1;
uint8_t REG_SET_TOP = 0;
uint8_t REG_SET_BOT = 0;

uint8_t REG_SET_SetMarker = 0;
/************************************************************************/
/* -> РАЗБОР ПРОЕКТА APD                                 */
/************************************************************************/
uint16_t APD_FrameStartAdr[APD_MAX_FRAMES]; // Массив адресов описаний кадров
uint16_t APD_FrameFieldsAndActionsStartAdr[APD_MAX_FRAMES][2]; // Массив описаний полей и действий в кадре

uint8_t APD_FrameFieldsAmount[APD_MAX_FRAMES]; // Массив количества полей в каждом кадре
uint8_t APD_FrameActionsAmount[APD_MAX_FRAMES]; // Массив количества полей в каждом кадре

uint16_t APD_FrameFieldStartAdr[APD_MAX_FRAMES][APD_MAX_FIELDS]; // Массив адресов описания полей каждого кадра
uint16_t APD_FrameActionStartAdr[APD_MAX_FRAMES][APD_MAX_ACTIONS];// Массив адресов описания кнопкодействий каждого кадра

uint16_t APD_FrameDataFromPlcAdr[APD_MAX_FRAMES][APD_MAX_FIELDS]; // Массив адресов, запрашиваемых из ПЛК в каждом кадре
uint8_t APD_FrameDataFromPlcStr[APD_MAX_FRAMES][APD_MAX_FIELDS]; // Массив строк регистров, запрашиваемых из ПЛК в каждом кадре
uint8_t APD_FrameDataFromPlcAmount[APD_MAX_FRAMES]; // Массив количества запрашиваемых из ПЛК адресов в кадре

/* Информация о СПИСКАХ в проекте:
 * APD_List[N][0] - метка откуда начинаются адреса СПИСКА в APD_FrameDataFromPlcAdr[N][M]
 * APD_List[N][1] - количество индексов (регистров 16-бит)
 * APD_List[N][2] - текущий сдвиг СПИСКА
*/
uint8_t APD_List[APD_MAX_FRAMES][3];
// Стартовые адреса СПИСКОВ в КАДРЕ
uint16_t APD_ListAdress[APD_MAX_FRAMES];

uint16_t FOCUS_DataAdr[APD_MAX_FIELDS]; // Массив адресов переменных в фокусе
int16_t FOCUS_Data[APD_MAX_FIELDS]; // Массив регистров в фокусе
uint8_t FOCUS_DataCntNext[APD_MAX_FIELDS]; // Массив счётчиков запросов (текущее состояние)
uint8_t  FOCUS_DataCntPrev[APD_MAX_FIELDS]; // Массив счётчиков запросов (предыдущее состояние)
uint8_t FOCUS_DataStatus[APD_MAX_FIELDS]; // Массив статусов регистров
uint8_t FOCUS_DataAmount = 0; // Количество адресов в фокусе

uint8_t FOCUS_DataCntNewNext[APD_MAX_FIELDS]; // Массив счётчиков рисования пробелов
uint8_t FOCUS_DataCntNewPrev[APD_MAX_FIELDS]; // Массив счётчиков рисования пробелов
uint8_t FOCUS_DataCntNewStatus[APD_MAX_FIELDS]; // Флаг рисования пробелов
uint16_t FOCUS_SpaceArray[APD_MAX_FIELDS]; // Массив расстояний между адресами в фокусе
uint8_t FOCUS_DataMesNew = 0; // Флаг изменения какого-либо регистра в кадре

 /* Массив адресов в фокусе для запроса:
  * BLOCK_Request[N][0] - начальный адрес блока
  * BLOCK_Request[N][1] - количество байт в блоке для запроса
  */
uint16_t BLOCK_Request[APD_MAX_FIELDS][2];
uint8_t BLOCK_Amount = 0; // Количество адресов в фокусе для запроса 
uint16_t APD_FrameButtonAction = 255; // Код действия по нажатию кнопок

uint8_t FOCUS_New = 1; // Флаг изменения ФОКУСА в кадре (чтобы пересчитать
// запрашиваемые блоки данных)

// Разрешение на отрисовку поля типа ЗАСТАВКА в кадре APD
uint8_t SplashRefreshEn = 1;
// Таблица перевода шрифта проекта APD в вид, пригодный для отображения на LCD дисплея
// Ячейка массива = код символа в APD
// Значение в ячейке = то, что передаётся на вывод в дисплей
/*const */ uint8_t APD_PrjFontTable[] = {
//	00    01    02    03    04    05    06 У  07    08    09    0A    0B    0C    0D    0E    0F
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
//	80 Г  81    82    83    84    85 П  86    87 Ф  88    89    8A Б  8B Д  8C Ж  8D З  8E И  8F Л
	0xA1, 0x20, 0x20, 0x20, 0x20, 0xA8, 0x20, 0xAA, 0x20, 0x20, 0xA0, 0xE0, 0xA3, 0xA4, 0xA5, 0xA7,
//	90 Ц  91 Ч  92 Ш  93 Щ  94 Ъ  95 Ы  96 Ь  97 Э  98 Ю  99 Я  9A Й  9B    9C    9D    9E    9F
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
//	F0    F1    F2    F3    F4    F5    F6    F7 Ё  F8    F9    FA    FB    FC    FD    FE    FF SQ
	0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x45, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0xFF
}; // Вместо буквы Ё буква Е (0x45)

uint8_t RusFont[32] = {
0x41, // 0x00 - А
0xA0, // 0x01 - Б
0x42, // 0x02 - В
0xA1, // 0x03 - Г
0xE0, // 0x04 - Д
0x45, // 0x05 - Е
0xA3, // 0x06 - Ж
0xA4, // 0x07 - З
0xA5, // 0x08 - И
0xA6, // 0x09 - Й
0x4B, // 0x0A - К
0xA7, // 0x0B - Л
0x4D, // 0x0C - М
0x48, // 0x0D - Н
0x4F, // 0x0E - О
0xA8, // 0x0F - П
0x50, // 0x10 - Р
0x43, // 0x11 - С
0x54, // 0x12 - Т
0xA9, // 0x13 - У
0xAA, // 0x14 - Ф
0x58, // 0x15 - Х
0xE1, // 0x16 - Ц
0xAB, // 0x17 - Ч
0xAC, // 0x18 - Ш
0xE2, // 0x19 - Щ
0xAD, // 0x1A - Ъ
0xAE, // 0x1B - Ы
0x62, // 0x1C - Ь
0xAF, // 0x1D - Э
0xB0, // 0x1E - Ю
0xB1, // 0x1F - Я
};

/************************************************************************/
/* -> РАБОТА С РАДИОМОДЕМОМ И ПРОТОКОЛ БЕСПРОВОДНОЙ СВЯЗИ               */
/************************************************************************/
// Список конфигурационных команд радиомодема
uint8_t RMD_Cfg [15][10] = {
// [00] - Установка профиля интерфейса		
	{7, '0', '0', '#', 'R', 'S', '0', 3, 0, 0},
// [01] - Установка параметров слова асинхронного последовательного интерфейса
	{7, '0', '0', '#', '8', 'N', '1', 3, 0, 0},
// [02] - Установка рабочей частоты радиомодема	
	{7, '0', '0', '#', 'F', '0', '0', 3, 0, 0},
// [03] - Установка выходной мощности передатчика
	{6, '0', '0', '#', 'P', 'F', 3, 0, 0, 0},
// [04] - Установка номера сети	
	{6, '0', '0', '#', 'N', '0', 3, 0, 0, 0},
// [05] - Установка адреса приёма
	{9, '0', '0', '#', 'D', 'A', 'A', 'A', 'A', 3},
//	{9, '0', '0', '#', 'D', 'B', 'B', 'B', 'B', 3},
// [06] - Установка адреса передачи	
	{9, '0', '0', '#', 'A', 'B', 'B', 'B', 'B', 3},
//	{9, '0', '0', '#', 'A', 'A', 'A', 'A', 'A', 3},

// [07] - Установка битов управления передачей
	{9, '0', '0', '#', 'C', '0', '0', '0', '0', 3},
// [08] - Установка рабочего режима радиомодема
	{6, '0', '0', '#', 'M', '0', 3, 0, 0, 0},

//	{5, '0', '0', '#', '?', 3, 0, 0, 0, 0},
// [09] - Скорость передачи данных на последовательном интерфейсе
	{7, '0', '0', '#', 'I', '9', '6', 3, 0, 0},
// [10] - Установка скорости передачи информации по радиоканалу	
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
// [00] - Установка профиля интерфейса
	{7, '0', '0', '#', 'R', 'S', '0', 3, 0, 0},
// [01] - Установка параметров слова асинхронного последовательного интерфейса
	{7, '0', '0', '#', '8', 'N', '1', 3, 0, 0},
// [02] - Установка рабочей частоты радиомодема
	{7, '0', '0', '#', 'F', '0', '3', 3, 0, 0},
// [03] - Установка выходной мощности передатчика
	{6, '0', '0', '#', 'P', 'F', 3, 0, 0, 0},
// [04] - Установка номера сети
	{6, '0', '0', '#', 'N', '0', 3, 0, 0, 0},
// [05] - Установка адреса приёма
	{9, '0', '0', '#', 'D', 'B', 'B', 'B', 'B', 3},
//	{9, '0', '0', '#', 'D', 'B', 'B', 'B', 'B', 3},
// [06] - Установка адреса передачи
	{9, '0', '0', '#', 'A', 'A', 'A', 'A', 'A', 3},
//	{9, '0', '0', '#', 'A', 'A', 'A', 'A', 'A', 3},

// [07] - Установка битов управления передачей
	{9, '0', '0', '#', 'C', '0', '0', '0', '0', 3},
// [08] - Установка рабочего режима радиомодема
	{6, '0', '0', '#', 'M', '0', 3, 0, 0, 0},
		
//	{5, '0', '0', '#', '?', 3, 0, 0, 0, 0},
// [09] - Скорость передачи данных на последовательном интерфейсе
	{7, '0', '0', '#', 'I', '9', '6', 3, 0, 0},
// [10] - Установка скорости передачи информации по радиоканалу	
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
// Буфер принятого сообщения с радиомодема
volatile uint8_t RMD_RecMesBuf[RMD_MESSAGE_LENGTH] = {0x00, 0x00, 0x00, 0x00};
// Детектирование посылки осуществляется по принятому символу '$'.
// Выставляется флаг RMD_RecMesInProgress, означающий, что осуществляется
// приём нового сообщения.
// Далее, производится подсчёт принятых символов до значения длины сообщения.
// После этого выставляется флаг принятого сообщения RMD_RecMesReady.
// Флаг RMD_RecMesInProgress сбрасывается, сигнализируя о том, что
// можно принимать новое сообщение.

volatile uint8_t RMD_RecMesCharCnt = 0; // Счётчик принятых символов в сообщении от радиомодема
volatile uint8_t RMD_RecMesBufLen = sizeof (RMD_RecMesBuf) / sizeof (uint8_t); // Длина принятого сообщения
volatile uint8_t RMD_RecMesCharBuf = 0; // Буфер принятого символа
volatile uint8_t RMD_TransMesCharBuf = 0; // Буфер отправленного символа (не используется)
volatile uint8_t RMD_RecMesInProgress = 0; // Флаг процесса приёма нового сообщения
volatile uint8_t RMD_RecEn = 0; // Флаг разрешения приёма сообщений с радиомодема
volatile uint8_t RMD_RecMesReady = 0; // Флаг готовности принятого сообщения
volatile uint8_t RMD_RecMesCrcOk = 0; // Флаг успешной проверки контрольной суммы принятого сообщения 

// Радиоканал
volatile uint8_t RMD_CfgAckMes[7] = {0x0D, 0x0A, 0x4F, 0x4B, 0x0A, 0x0D, 0x3E}; // Сообщение подтверждения успешной записи команды конфигурирования	
volatile uint8_t RMD_CfgAckMesCharCnt = 0; // Счётчик символов ответного сообщения
volatile uint8_t RMD_CfgAckMesLen = 7; // Длина ответного сообщения
volatile uint8_t RMD_CfgAckMesReady = 1; // Флаг готовности ответного сообщения

volatile uint8_t RMD_CfgCmdAmount = 11; // Количество конфигурационных команд
volatile uint8_t RMD_CfgCmdCnt = 0; // Счётчик конфигурационных команд
volatile uint8_t RMD_CfgCmdCharCnt = 0; // Счётчик символов команды
volatile uint8_t RMD_CfgCmdAck = 0; // Флаг успешного квитирования команды
//радиоканал modbus
volatile uint8_t RMD_CfgAckMesMBUS[7] = {0x0D, 0x0A, 0x4F, 0x4B, 0x0A, 0x0D, 0x3E}; // Сообщение подтверждения успешной записи команды конфигурирования	
volatile uint8_t RMD_CfgAckMesCharCntMBUS = 0; // Счётчик символов ответного сообщения
volatile uint8_t RMD_CfgAckMesLenMBUS = 7; // Длина ответного сообщения
volatile uint8_t RMD_CfgAckMesReadyMBUS = 1; // Флаг готовности ответного сообщения

volatile uint8_t RMD_CfgCmdAmountMBUS = 11; // Количество конфигурационных команд
volatile uint8_t RMD_CfgCmdCntMBUS = 0; // Счётчик конфигурационных команд
volatile uint8_t RMD_CfgCmdCharCntMBUS = 0; // Счётчик символов команды
volatile uint8_t RMD_CfgCmdAckMBUS = 0; // Флаг успешного квитирования команды

// Стадии настройки радиомодема
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
/* -> ПРОТОКОЛ СВЯЗИ ПУЛЬТА С ПРЕОБРАЗОВАТЕЛЕМ ЧАСТОТЫ (ADP) */
/************************************************************************/
// Адреса устройств на линии
typedef enum ADP_adress {
	ADP_NULL = 0x00, // НУЛЕВОЙ АДРЕС
	ADP_TR0 = 0x50, // ПЧ 0
	ADP_TR1 = 0x51, // ПЧ 1
	ADP_TR2 = 0x52, // ПЧ 2
	ADP_TR3 = 0x53, // ПЧ 3
	ADP_TR4 = 0x54, // ПЧ 4
	ADP_OPK_MBUS = 0x79,	//Пультик виртуальный для модбас
	ADP_OPK_RES = 0x7B,	//Пультик виртуальный резервный
	ADP_OPK = 0x7D, // Пультик
	ADP_ATOOLS = 0x7F, // ATools
} ADP_adress_t;

// Коды функций протокола ADP
typedef enum ADP_function {
	ADP_FUNC_TRANS_PAR = 0x13, // Передать параметры
	ADP_FUNC_REC_PAR = 0x0B, // Принять параметры
	ADP_FUNC_EVENT_REQ = 0x05, // Запрос списка событий
} ADP_function_t;

// Флаги разрешения приёма\передачи шины UART по протоколу ADP
volatile uint8_t ADP_TransEn = 0x01; // Разрешение передачи
volatile uint8_t ADP_RecEn = 0x01; // Разрешение приёма

uint8_t DATA_ReqEn = 0; // Разрешение запроса данных
uint8_t EVENT_ReqEn = 0; // Разрешение запроса списка событий
/************************************************************************/
/* -> ПРОТОКОЛ RS-232 - ADP												*/
/************************************************************************/
/************************************************************************/
/*								ПЕРЕДАЧА                                */
/************************************************************************/
volatile uint8_t ADP_TransMesBuf[32]; // Буфер передаваемого сообщения
volatile uint16_t ADP_TransMesBody[16]; // Буфер тела функции
volatile uint8_t ADP_TransMesCharBuf=0; // Буфер передаваемого символа
//volatile uint16_t ADP_TransMesByteCnt; // Счётчик переданных байт 
volatile uint16_t ADP_TransMesLen=0; // Длина передаваемой посылки
volatile uint8_t ADP_TransMesQueue=0; // Очередь передаваемых сообщений в запросе на обновление кадра
volatile uint8_t ADP_TransMesQueueCnt=0; // Счётчик очереди сообщений
volatile uint16_t ADP_MBUS_TransMesBody[16]; // Буфер тела функции преобразования протоколов
/************************************************************************/
/*								ПРИЁМ                                   */
/************************************************************************/
volatile uint8_t ADP_RecNewChar=0; // Флаг нового принятого символа
volatile uint8_t ADP_RecMesBuf[128]; // Буфер принимаемого сообщения
volatile uint8_t ADP_RecMesReady=0; // Флаг: приём посылки завершён
volatile uint8_t ADP_RecMesCrcOk=0; // Флаг: посылка прошла проверку контрольной суммы
volatile uint8_t ADP_RecMesCharBuf=0; // Буфер принимаемого символа посылки
volatile uint16_t ADP_RecMesByteCnt = 0; // Счётчик принятых байт
volatile uint16_t ADP_RecMesLen=0; // Длина принимаемой посылки
volatile uint8_t ADP_RecMesTimeout=1; // Флаг таймаута нового сообщения от преобразователя

extern MODBUS_STATE_t MB0_State;
/************************************************************************/
/* -> ВНУТРЕННЕЕ ПРОГРАММНОЕ ОБЕСПЕЧЕНИЕ ПУЛЬТА */
/************************************************************************/
// Флаг задачи на обновление экрана
volatile uint8_t LCD_Refresh = 1;
// "Звёздочки" загрузки
uint8_t LoadBar = 8;
// Флаги обработчика аппаратных ошибок
uint8_t RMD_Error = 0; // Ошибка радиомодема
uint8_t PRJ_Error = 0; // Ошибка проекта APD_
uint8_t CLOCK_Error = 0; // Ошибка часов реального времени
uint8_t EEPROM_Error = 0; // Ошибка микросхемы EEPROM
// Режимы работы сети
typedef enum net_mode_enum {
	NET_MODE_CONFIG = 0, // Конфигурация радиомодема
	NET_MODE_MASTER = 1, // Обычный режим: дисплей мастер, преобразователь слэйв
	NET_MODE_TRANSP = 2, // Прозрачный режим: дисплей - мост между APD и преобразователем
	NET_MODE_REGSAVE = 3, // Режим передачи журнала событий на ПК
} net_mode_t;

uint8_t NET_Mode = NET_MODE_CONFIG; // Текущий режим работы сети
// Список аппаратных ошибок пульта
typedef enum error_enum {
	ERROR_RMD = 0,
	ERROR_APD = 1,
	ERROR_RTC = 2,
	ERROR_EEPROM = 3,
} error_t;

uint8_t APD_ErrorScore = 0;
uint8_t EEPROM_ErrorScore = 0;
// Счётчики-анализатора ошибок работы радиомодема:
uint8_t RMD_ErrorScore = 0; // Счётчик ошибок: ожидание ответа
uint8_t RMD_ErrorScoreLim = 10;
uint8_t RMD_ErrorScoreTry = 0;
uint8_t RMD_ErrorScoreTryLim = 3; // Счётчик ошибок: попытка реконфигурации
uint8_t RMD_ErrorScoreGlobal = 0; // Счётчик ошибок: попытки обнаружения модема
uint8_t RMD_ErrorScoreGlobalLim = 70;

/************************************************************************/
/* -> РАБОТА С КНОПКАМИ КЛАВИАТУРЫ				*/
/************************************************************************/
// Таймеры удержания комбинации клавиш для перехода в спецрежимы:
uint8_t SERVICE_ToggleCnt = 0; // Переход в сервисный режим
uint8_t SERVICE_ToggleCntLim = 40;
uint8_t TRANSP_ToggleCnt = 0; // Переход в прозрачный режим
uint8_t TRANSP_ToggleCntLim = 40;
// Нажатые кнопки:
uint8_t BUTTON_First = 0; // Кнопка 1
uint8_t BUTTON_Second = 0; // Кнопка 2
// Флаг залипания:
// чтобы новое нажатие было обработано, необходимо
// отпустить прежде нажатую кнопку
uint8_t BUTTON_Release = 0;

// Код действия по нажатию для передачи в функцию отображения экранов МЕНЮ
uint8_t Action = 255;
/************************************************************************/
/* -> РАБОТА С ЧАСАМИ РЕАЛЬНОГО ВРЕМЕНИ		*/
/************************************************************************/
uint8_t TIMESET_CurPos = 0; // Позиция курсора на экране установки времени

// Флаг разрешения отображения часов
uint8_t ClockShowEn = 0;
// Флаг разрешения отображения даты
uint8_t DataShowEn = 0;
// Переменные "захвата" времени:
// в них складываются "сырые" значения, полученные из регистров RTC
uint8_t TIME_GetSeconds = 0;
uint8_t TIME_GetMinutes = 0;
uint8_t TIME_GetHours = 0;
uint8_t TIME_GetDay = 0;
uint8_t TIME_GetDate = 0;
uint8_t TIME_GetMonth = 0;
uint8_t TIME_GetYear = 0;

// Переменные предварительной установки времени:
// реальные значения часов, минут и т.д.
uint8_t TIME_SetSeconds = 0;
uint8_t TIME_SetMinutes = 0;
uint8_t TIME_SetHours = 0;
uint8_t TIME_SetDay = 0;
uint8_t TIME_SetDate = 0;
uint8_t TIME_SetMonth = 0;
uint8_t TIME_SetYear = 0;

// Переменные режима УСТАНОВКИ ВРЕМЕНИ
// Флаг, позволяющий сделать "захват" текущего времени на
// весь период работы режима установки времени.
// Сбрасывается при выходе из этого режима
uint8_t TIMESET_ShowOnce = 1;
// В этих переменных передаются реальные значения часов, минут и т.д.
uint8_t TIMESET_ShowSeconds = 0;
uint8_t TIMESET_ShowMinutes = 0;
uint8_t TIMESET_ShowHours = 0;
uint8_t TIMESET_ShowDay = 0;
uint8_t TIMESET_ShowDate = 0;
uint8_t TIMESET_ShowMonth = 0;
uint8_t TIMESET_ShowYear = 0;

// Флаг отказа часов при "захвате" часов в режиме УСТАНОВКИ ВРЕМЕНИ:
// сбрасывается при выходе из ветки обработки ошибки и из режима
uint8_t TIMESET_ClockFailed = 0;

// Переменные режима настройки КОНТРАСТА
uint8_t CONTRAST_ShowOnce = 1;
uint8_t CONTRAST_Contrast = 0;
uint8_t LCD_Contrast = DEFAULT_CONTRAST;
uint8_t EEMEM CONTRAST_Save = DEFAULT_CONTRAST;
// Экраны дисплея
void		SCREEN_INTRO (void); // Заставка при запуске
void		SCREEN_MENU (void); // Экран меню
void		SCREEN_ACTION (uint8_t Action); // Обычный режим (кнопкодействия проекта APD)
void		SCREEN_NORMAL (void); // Обычный режим (отрисовка кадров проекта APD)
void		SCREEN_INFO (void); // Экран информации
uint8_t		SCREEN_TIME (void); // Установка времени
void		SCREEN_DIAGNOSTIC (void); // Диагностика пульта
void		SCREEN_SETTINGS (void); // Настройка связи
void		SCREEN_CONTRAST (void); // Настройка контраста
void		SCREEN_EVENTS_LIST (void); // Журнал событий
void		SCREEN_EVENTS_CLEAR (void); // Экран очистки ЖУРНАЛА
void		SCREEN_EVENTS_MENU (void); // Меню ЖУРНАЛА
void		SCREEN_EVENTS_INFO (void);  // Информация о ЖУРНАЛЕ
void		SCREEN_EVENTS_SETTINGS (void);  // Натсройки ЖУРНАЛА

// Журнал событий
void		REG_WriteEvent (uint8_t From, uint8_t EventCode); // Запись события в журнал
void		REG_Init (void); // Инициализация ЖУРНАЛА
void		REG_Reset (void); // Сброс заголовка ЖУРНАЛА
void		REG_Screen (uint8_t Mode, uint16_t TotMes, uint16_t CurMes, uint16_t NewMes, uint8_t NewFlag, uint8_t *Data); // Рисование экрана ЖУРНАЛА
// Прозрачный режим
void		SCREEN_TRANSP_SELECT (void);
void		SCREEN_TRANSP_CONFIRM (void);
void		SCREEN_TRANSP_PROGRESS (void);
void		SCREEN_TRANSP_SCREEN (void);
// Передача журнала на ПК
void		SCREEN_EVENTS_SAVE_MENU (void);
void		SCREEN_EVENTS_SAVE_PROGRESS (void);
void		SCREEN_EVENTS_SAVE_COMPLETED (void);
// Экраны ошибок
void		SCREEN_ERROR_PRJ (void);
void		SCREEN_ERROR_RMD (void);
// Перечисление режимов работы
typedef enum mode_enum {
	MODE_INTRO =					0, // Стартовый экран
	MODE_NORMAL =					1, // Экран обычного режима
	MODE_MENU =						2, // Экран МЕНЮ
	MODE_INFO =						3, // Экран ИНФОРМАЦИИ
	MODE_TIME =						4, // Экран установки времени и даты
	MODE_DIAGNOSTIC =				5, // Экран диагностики ошибок
	MODE_SETTINGS =					6, // Экран настройки связи
	MODE_CONTRAST =					7, // Настройка контраста

	MODE_EVENTS_MENU =				8, // Меню ЖУРНАЛА СОБЫТИЙ
	MODE_EVENTS_LIST =				9, // Экран отображения журнала событий
	MODE_EVENTS_INFO =				10, // Кадр информации ЖУРНАЛА СОБЫТИЙ
	MODE_EVENTS_CLEAR =				11, // Кадр очистки ЖУРНАЛА СОБЫТИЙ
	MODE_EVENTS_SETTINGS =			12, // Кадр настроек ЖУРНАЛА СОБЫТИЙ
	MODE_EVENTS_SAVE_MENU =			13, // Кадр режима передачи журнала на ПК
	MODE_EVENTS_SAVE_PROGRESS =		14, // Кадр режима передачи журнала: прогресс бар
	MODE_EVENTS_SAVE_COMPLETED =	15, // Кадр режима передачи журнала: завершение

	MODE_TRANSP_SELECT =			16, // Кадр установок режима ПРОЗРАЧНОСТИ
	MODE_TRANSP_CONFIRM =			17, // Кадр входа в режим ПРОЗРАЧНОСТИ
	MODE_TRANSP_PROGRESS =			18, // Кадр входа в режим ПРОЗРАЧНОСТИ
	MODE_TRANSP_SCREEN =			19, // Кадр режима ПРОЗРАЧНОСТИ
	
	MODE_ERROR_PRJ =				20, // Режим ОШИБКА ПРОЕКТА
	MODE_ERROR_RMD =				21, // Режим ОШИБКА МОДЕМА
	
	MODE_EDIT =						22, // Режим редактирования поля
	
	MODE_NORMAL_2 =					23,	//Второй стартовый экран для 4 ПЧ

} mode_t;

uint8_t Mode = MODE_INTRO; // Текущий режим работы
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
{"ОШИБКА! СБОЙ ПРОЕКТА"},
{"ПРОЕКТ ПОВРЕЖДЕН ИЛИ"},
{"    ОТСУТСТВУЕТ!    "},
{"ПЕРЕЗАГРУЗИТЕ ПАНЕЛЬ"},
};

uint8_t EEMEM ErrorRmdText[4][20] = {
{"ОШИБКА! СБОЙ МОДЕМА "},
{"ОТСУТСТВУЕТ СВЯЗЬ С "},
{"   РАДИОМОДЕМОМ!    "},
{"ПЕРЕЗАГРУЗИТЕ ПАНЕЛЬ"},
};

uint8_t EEMEM InfoText[4][20] = {
{"ИНФОРМАЦИЯ:    0-ВЫХ"},
{"- ПАНЕЛЬ ОПЕРАТОРА -"},
{"ВЕРСИЯ ПО:          "},
{"ЗАО 'АСК' (c) 2014  "},
};

 /*uint8_t EEMEM IntroText[4][20] = {
 {"LLC ASC. EKB, RUSSIA"},
 {"OPERATIONAL PANEL   "},
 {"SOFT VERSION:       "},
 {"LOADING             "},
 };*/

uint8_t EEMEM IntroText[4][20] = {

{"---- ЗАО 'АСК' -----"},
{"ЕКАТЕРИНБУРГ, РОССИЯ"},
{"ПАНЕЛЬ ОПЕРАТОРА    "},
{"ЗАГРУЗКА            "},
};

uint8_t MenuShift = 0;
uint8_t MenuMax = MENU_MAX;

// ПОРЯДОК КНОПКОДЕЙСТВИЙ В МЕНЮ
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
	
{"МЕНЮ:               "},
{"1-УСТАН. ВРЕМЕНИ    "},
{"2-УСТАН. КОНТРАСТА  "},
{"3-ЖУРНАЛ СОБЫТИЙ    "},
{"4-НАСТР. СВЯЗИ      "},
{"5-ДИАГНОСТИКА       "},
{"6-ИНФОРМАЦИЯ        "},
	
};

uint8_t EEMEM ContrastText[4][20] = {
// {"CONTRAST:           "},
// {"                    "},
// {"                    "},
// {"1-YES 0-NO 9-DEFAULT"},
	
{"УСТАН. КОНТРАСТА:   "},
{"                    "},
{"                    "},
{"1-ПРИН 0-ВЫХ 9-УМОЛЧ"},	
	
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
	
{"ЖУРН. СОБЫТИЙ: 0-ВЫХ"},
{"1-ПРОСМОТР          "},
{"2-СОСТОЯНИЕ         "},
{"3-ОЧИСТКА           "},
{"4-СОХРАНЕНИЕ        "},
{"5-НАСТРОЙКИ         "},

};

uint8_t EEMEM RegInfoText[4][20] = {
// {"EVENTS INFO:  0-BACK"},
// {"TOT: 0000 NEW: 0000 "},
// {"CLEARED AT:         "},
// {"-> 00:00:00 00.00.00"},

{"СОСТОЯНИЕ:     0-ВЫХ"},
{"ВСЕ: 0000 НОВ: 0000 "},
{"ПОСЛЕДНЯЯ ОЧИСТКА:  "},
{"-> 00:00:00 00.00.00"},

};

// Переменные режима УСТАНОВОК
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

{"НАСТРОЙКИ СВЯЗИ:    "},
{" ЗАДЕРЖКА           "},
{" БОДРЭЙТ            "},
{" ЦИКЛ ОПРОСА        "},
{" КРАТН.ОПРОСА       "},
{" КОЛ-ВО ПЧ:         "},
{" 1-Й АДР -          "},
{" 2-Й АДР -          "},
{" 3-Й АДР -          "},
{" 4-Й АДР -          "},
{"1-ПРИН 0-ВЫХ 9-УМОЛЧ"},

};

uint8_t EEMEM ClockSetText[4][20] = {
{"УСТАНОВКА ВРЕМЕНИ:  "},
{"  :  :     .  .     "},
{"                    "},
{"1-ПРИН 0-ВЫХ 9-УМОЛЧ"},
};

uint8_t EEMEM ClockFailedText[4][20] = {
{"УСТАНОВКА ВРЕМЕНИ:  "},
{"??:??:?? ??.??.??   "},
{"ОШИБКА! СБОЙ ЧАСОВ! "},
{"               0-ВЫХ"},
};

uint8_t EEMEM RegEmptyText[4][20] = {
// {"         (    )/    "},
// {"                    "},
// {"    <NO EVENTS>     "},
// {"              0-BACK"},
	
{"         (    )/    "},
{"                    "},
{"   <ЖУРНАЛ ПУСТ>    "},
{"               0-ВЫХ"},

};

uint8_t EEMEM RegErrorText[4][20] = {
{"         (    )/    "},
{"                    "},
{"  <ОШИБКА ДАННЫХ>   "},
{"               0-ВЫХ"},
};

uint8_t EEMEM RegOutText[4][20] = {
{"         (    )/    "},
{"  :  :        .  .  "},
{" <НЕКОРР. СОБЫТИЕ>  "},
{"               0-ВЫХ"},
};

uint8_t EEMEM RegShowText[4][20] = {
{"         (    )/    "},
{"  :  :        .  .  "},
{"                    "},
{"ПЧ             0-ВЫХ"},
};

uint8_t EEMEM RegResetText[4][20] = {
{"ОЧИСТКА ЖУРНАЛА:    "},
{"СОБЫТИЯ БУДУТ УДАЛЕ-"},
{"НЫ. ПРОДОЛЖИТЬ?     "},
{"1-ПРИН         0-ВЫХ"},
};

uint8_t EEMEM RegSaveText[4][20] = {
{"СОХРАНЕНИЕ ЖУРНАЛА: "},
{"ВСЕ: 0000 НОВ: 0000 "},
{"ПРОДОЛЖИТЬ СОХРАН.? "},
{"1-ПРИН         0-ВЫХ"},
};

uint8_t EEMEM RegSaveProgressText[4][20] = {
{"СОХРАНЕНИЕ ЖУРНАЛА.."},
{"                    "},
{"ВСЕ:       ВЫП:     "},
{"                    "},
};

uint8_t EEMEM RegSaveCompletedText[4][20] = {
{"СОХРАНЕНИЕ ЖУРНАЛА: "},
{" СОХРАНЕНИЕ УСПЕШНО "},
{"     ЗАВЕРШЕНО      "},
{"1-ПРИН              "},
};

uint8_t EEMEM RegSetText[4][20] = {
{"НАСТРОЙКИ ЖУРНАЛА:  "},
{" ЗАХВАТ СОБЫТИЯ     "},
{" СБРОС СОБЫТИЯ      "},
{"1-ПРИН 0-ВЫХ 9-УМОЛЧ"},
};

uint8_t EEMEM TranspSelectText[4][20] = {
{"ВЫБОР СКОР. ОБМЕНА: "},
{"1 9600              "},
{"2 57600             "},
{"3 115200       0-ВЫХ"},
};

uint8_t EEMEM TranspConfirmText[4][20] = {
{"ПРОЗРАЧНЫЙ РЕЖИМ:   "},
{"  ПОДТВЕРДИТЕ ВХОД  "},
{"1-ПРИН              "},
{"0-ВЫХ               "},
};

uint8_t EEMEM TranspProgressText[4][20] = {
{"ПРОЗРАЧНЫЙ РЕЖИМ:   "},
{"КОНФИГУРИРОВАНИЕ    "},
{"ПОДОЖДИТЕ...        "},
{"                    "},
};

uint8_t EEMEM TranspActiveText[4][20] = {
{"ПРОЗРАЧНЫЙ РЕЖИМ:   "},
{"                    "},
{"    АКТИВИРОВАН!    "},
{"                    "},
};

uint8_t EEMEM DiagnosticText[4][20] = {
{"ДИАГНОСТИКА:   0-ВЫХ"},
{"РАДИОМОДЕМ.........."},
{"ПРОЕКТ.............."},
{"ЧАСЫ................"},
};
/************************************************************************/
/* -> ЭКРАН: ОШИБКА ПРОЕКТА APD						*/
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
/* -> ЭКРАН: ОШИБКА РАДИОМОДЕМА			*/
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
/* -> ЭКРАН: СТАРТОВОЕ ОКНО							*/
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
/* -> ЭКРАН: МЕНЮ														*/
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
/* -> ЭКРАН: НАСТРОЙКА ВРЕМЕНИ						*/
/************************************************************************/
uint8_t SCREEN_TIME (void) {
	uint8_t DateLim = 0;
	uint8_t TempChar = 0;
	if (TIMESET_ShowOnce) {
		// Запрет нового "захвата"
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
		
		// "Захват" текущего времени
		if (TIME_Get ()) {
			// Сброс флага ошибки часов
			TIMESET_ClockFailed = 0;
			// Захват значений времени и даты
			TIMESET_ShowSeconds = 10*((TIME_GetSeconds & 0x70)>>4) + (TIME_GetSeconds & 0x0F);
			TIMESET_ShowMinutes = 10*((TIME_GetMinutes & 0x70)>>4) + (TIME_GetMinutes & 0x0F);
			TIMESET_ShowHours = 10*((TIME_GetHours & 0x30)>>4) + (TIME_GetHours & 0x0F);
			TIMESET_ShowDay = (TIME_GetDay & 0x07);
			TIMESET_ShowDate = 10*((TIME_GetDate & 0x30)>>4) + (TIME_GetDate & 0x0F);
			TIMESET_ShowMonth = 10*((TIME_GetMonth & 0x10)>>4) + (TIME_GetMonth & 0x0F);
			TIMESET_ShowYear = 10*((TIME_GetYear & 0xF0)>>4) + (TIME_GetYear & 0x0F);
		} else {
			// Установка флага ошибки часов
			TIMESET_ClockFailed = 1;
			// Обнуление значений времени и даты
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
		// Обработка ошибки "Крах часов"
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
			// Возврат в сервисный режим
			Mode = MODE_MENU;
			TIMER_ClockRefreshEn = 1;
			ClockShowEn = 1;
			LCD_Refresh = 1;
			TIMESET_CurPos = 0; // Курсор на начальную позицию
			// Сброс флага ошибки часов
			TIMESET_ClockFailed = 0;
			Action = 255;
			// Разрешение "захвата" текущего времени при новом вызове функции
			TIMESET_ShowOnce = 1;
		}
		return 0;
	} else {
		LCD_Clear ();
		LCD_PutStr (0, 0, (uint8_t *) "TIME SET            ");
		// ОБРАБОТКА НАЖАТИЙ КНОПОК:
		
		// (7) ВЛЕВО - сдвиг курсора влево
		if (Action == 7) {
			if (TIMESET_CurPos == 0) {
				TIMESET_CurPos = 15;
			} else {
				TIMESET_CurPos -= 3;
			}
			Action = 255;
		}
		
		// (8) ВПРАВО - сдвиг курсора вправо
		if (Action == 8) {
			if (TIMESET_CurPos == 15) {
				TIMESET_CurPos = 0;
			} else {
				TIMESET_CurPos += 3;
			}
			Action = 255;
		}
		
		// (5) ВВЕРХ - инкремент значения
		if (Action == 5) {
			// Инкремент величины в зависимости от положения курсора
			// Часы
			if (TIMESET_CurPos == 0) {
				if (TIMESET_ShowHours == 23) {
					TIMESET_ShowHours = 0;
				} else {
					++ TIMESET_ShowHours;
				}
			}
			// Минуты
			if (TIMESET_CurPos == 3) {
				if (TIMESET_ShowMinutes == 59) {
					TIMESET_ShowMinutes = 0;
				} else {
					++ TIMESET_ShowMinutes;
				}
			}
			// Секунды
			if (TIMESET_CurPos == 6) {
				if (TIMESET_ShowSeconds == 59) {
					TIMESET_ShowSeconds = 0;
				} else {
					++ TIMESET_ShowSeconds;
				}
			}
			// Числа
			if (TIMESET_CurPos == 9) {
				if (TIMESET_ShowDate == DateLim) {
					TIMESET_ShowDate = 1;
				} else {
					++ TIMESET_ShowDate;
				}
			}
			// Месяцы
			if (TIMESET_CurPos == 12) {
				if (TIMESET_ShowMonth == 12) {
					TIMESET_ShowMonth = 1;
				} else {
					++ TIMESET_ShowMonth;
				}
			}
			// Года
			if (TIMESET_CurPos == 15) {
				if (TIMESET_ShowYear == 99) {
					TIMESET_ShowYear = 0;
				} else {
					++ TIMESET_ShowYear;
				}
			}
			Action = 255;
		}
		
		// (6) ВНИЗ - декремент значения
		if (Action == 6) {
			// Декремент величины в зависимости от положения курсора
			// Часы
			if (TIMESET_CurPos == 0) {
				if (TIMESET_ShowHours == 0) {
					TIMESET_ShowHours = 23;
				} else {
					-- TIMESET_ShowHours;
				}
			}
			// Минуты
			if (TIMESET_CurPos == 3) {
				if (TIMESET_ShowMinutes == 0) {
					TIMESET_ShowMinutes = 59;
				} else {
					-- TIMESET_ShowMinutes;
				}
			}
			// Секунды
			if (TIMESET_CurPos == 6) {
				if (TIMESET_ShowSeconds == 0) {
					TIMESET_ShowSeconds = 59;
				} else {
					-- TIMESET_ShowSeconds;
				}
			}
			// Числа
			if (TIMESET_CurPos == 9) {
				if (TIMESET_ShowDate == 1) {
					TIMESET_ShowDate = DateLim;
				} else {
					-- TIMESET_ShowDate;
				}
			}
			// Месяцы
			if (TIMESET_CurPos == 12) {
				if (TIMESET_ShowMonth == 1) {
					TIMESET_ShowMonth = 12;
				} else {
					-- TIMESET_ShowMonth;
				}
			}
			// Года
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
			// Применение установок
			if (TIME_SetManual (TIMESET_ShowHours, TIMESET_ShowMinutes, TIMESET_ShowSeconds, TIMESET_ShowDay, TIMESET_ShowDate, TIMESET_ShowMonth, TIMESET_ShowYear)) {
				// Возврат в МЕНЮ
				Mode = MODE_MENU;
				TIMER_ClockRefreshEn = 1;
				ClockShowEn = 1;
				LCD_Refresh = 1;
				TIMESET_CurPos = 0;
				Action = 255;
				// Разрешение "захвата" текущего времени при новом вызове функции
				TIMESET_ShowOnce = 1;
			}
		}
		
		// (0) NO
		if (Action == 0) {
			// Возврат в МЕНЮ
			Mode = MODE_MENU;
			TIMER_ClockRefreshEn = 1;
			ClockShowEn = 1;
			LCD_Refresh = 1;
			TIMESET_CurPos = 0;
			Action = 255;
			// Разрешение "захвата" текущего времени при новом вызове функции
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
				
		// Выбор лимита для даты в зависимости от текущего месяца и года
		switch (TIMESET_ShowMonth) {
			case 1: // Январь
				DateLim = 31;
				break;
			case 2: // Февраль
				if (TIMESET_ShowYear%4) {
					DateLim = 28; // Не високосный
				} else {
					DateLim = 29; // Високосный
				}
				break;
			case 3: // Март
				DateLim = 31;
				break;
			case 4: // Апрель
				DateLim = 30;
				break;
			case 5: // Май
				DateLim = 31;
				break;
			case 6: // Июнь
				DateLim = 30;
				break;
			case 7: // Июль
				DateLim = 31;
				break;
			case 8: // Август
				DateLim = 31;
				break;
			case 9: // Сентябрь
				DateLim = 30;
				break;
			case 10: // Октябрь
				DateLim = 31;
				break;
			case 11: // Ноябрь
				DateLim = 30;
				break;
			case 12: // Декабрь
				DateLim = 31;
				break;
			default:
				DateLim = 31;
				break;
		}
		// Проверка значения числа на текущий лимит
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
/* -> ЭКРАН: НАСТРОЙКА КОНТРАСТА					*/
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
	
	// ОБРАБОТКА ДЕЙСТВИЙ
	switch (Action) {
		
		// (7) ВЛЕВО - УМЕНЬШИТЬ
		case 7:
			if (CONTRAST_Contrast > 7) {
				-- CONTRAST_Contrast;
			}
			LCD_Contrast = CONTRAST_Contrast;
			DAC_New ();
			Action = 255;
			break;
		
		// (8) ВПРАВО - УВЕЛИЧИТЬ
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
			// Применение установок
			if (CONTRAST_ContrastTemp != CONTRAST_Contrast) {
				LCD_Contrast = CONTRAST_Contrast;
				DAC_New ();
				eeprom_write_byte ((uint8_t *) &CONTRAST_Save, (uint8_t) CONTRAST_Contrast);
			}
			
			// Возврат в МЕНЮ
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
			// Возврат в МЕНЮ
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
/* -> ЭКРАН: НАСТРОЙКИ									*/
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
		// Маркер
		LCD_PutSym (SETTINGS_SetMarker, 0, (uint8_t)ARROW_RIGHT);
		if (SettingsShift > 0) LCD_PutSym (1, 19, (uint8_t)ARROW_UP);
		if (SettingsShift < (SettingsMax - 4)) LCD_PutSym (2, 19, (uint8_t)ARROW_DOWN);	
	}
	
	// ОБРАБОТКА НАЖАТИЙ
		switch (Action) {
			// (7) ВЛЕВО - УМЕНЬШИТЬ
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
		
			// (8) ВПРАВО - УВЕЛИЧИТЬ
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

			// (5) ВВЕРХ - МАРКЕР ВВЕРХ
			case 5:
				if (SETTINGS_SetMarker > 1) {
					-- SETTINGS_SetMarker;
				} else {
					SETTINGS_SetMarker = 1;
					if (SettingsShift > 0) -- SettingsShift;
				}
				Action = 255;
				break;

			// (6) ВНИЗ - МАРКЕР ВНИЗ
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
				// Применение установок
				TIMER_RequestTimeoutPer = SETTINGS_Delay;
				eeprom_write_byte ((uint8_t *) &SETTINGS_DelaySave, (uint8_t) SETTINGS_Delay);
				eeprom_write_byte ((uint8_t *) &SETTINGS_DelaySaveFlag, (uint8_t) 1);
			}
			if (SETTINGS_Baud_T != SETTINGS_Baud) {
				APD_BaudRateIndex = SETTINGS_Baud;
				eeprom_write_byte ((uint8_t *) &SETTINGS_BaudSave, (uint8_t) SETTINGS_Baud);
				eeprom_write_byte ((uint8_t *) &SETTINGS_BaudFlag, (uint8_t) 1);
				// Настройка скорости обмена
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
		
			// Возврат в МЕНЮ
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
		
			// Возврат в МЕНЮ
			Mode = MODE_MENU;
			TIMER_ClockRefreshEn = 1;
			ClockShowEn = 1;
			LCD_Refresh = 1;
			Action = 255;
			SETTINGS_ShowOnce = 1;
			break;
		
		// (9) DEFAULT - ВЕРНУТЬ УМОЛЧАНИЯ
		case 9:
			SETTINGS_Delay = DEFAULT_DELAY;
			SETTINGS_Baud = pgm_read_byte ( APD_PrjData + 0x000A );
			SETTINGS_Cyckle = pgm_read_byte ( APD_PrjData + 0x000B );
			SETTINGS_Rate = pgm_read_byte ( APD_PrjData + 0x000C );

			// Разбор адресов абонентов,
			// у которых будут запрашиваться списки событий
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
/* -> ЭКРАН: ДИАГНОСТИКА								*/
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

/* СТРУКТУРА ЗАГОЛОВКА ЖУРНАЛА:
 * 
 * 1ый БАЙТ - 
 * [0] Часы (Реинициализация)
 * [1] Минуты (Реинициализация)
 * [2] Секунды (Реинициализация)
 * [3] Число (Реинициализация)
 * [4] Месяц (Реинициализация)
 * [5] Год (Реинициализация)
 * [6] Код = 0xFF
 * [7] Контрольная сумма
 * 
 * 2ой БАЙТ - 
 * [0] Резерв
 * [1] Резерв
 * [2] Резерв
 * [3] Кол-во прочитанных записей (младший байт)
 * [4] Кол-во прочитанных записей (старший байт)
 * [5] Кол-во записей (младший байт)
 * [6] Кол-во записей (старший байт)
 * [7] Контрольная сумма 
 */

/************************************************************************/
/* -> ЭКРАН: МЕНЮ ЖУРНАЛА							*/
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
/* -> ЭКРАН: ИНФОРМАЦИЯ ЖУРНАЛА						*/
/************************************************************************/
void SCREEN_EVENTS_INFO (void) {
	// uint16_t TotMes = 0; // Количество записей в журнале
	// uint16_t NewMes = 0; // Количество новых записей в журнале
	
	// Буфер чтения из EEPROM 1
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
/* >> ЧТЕНИЕ ЗАГОЛОВКА													*/
/************************************************************************/
	// 2ой байт
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

// 1ый байт
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
/* -> ЭКРАН: СПИСОК СОБЫТИЙ									*/
/************************************************************************/
void SCREEN_EVENTS_LIST (void) {
	uint16_t TotMes = 0; // Количество записей в журнале
	uint16_t CurMes = 0; // Текущая запись
	uint16_t NewMes = 0; // Количество новых записей в журнале
	
	// Буфер чтения из EEPROM 1
	uint8_t ReadBuff1[8] = {0, 0, 0, 0, 0, 0, 0, 0};
	// Буфер чтения из EEPROM 2
	uint8_t ReadBuff2[8] = {0, 0, 0, 0, 0, 0, 0, 0};
	// Буфер записи в EEPROM 1
	uint8_t WriteBuff1[8] = {0, 0, 0, 0, 0, 0, 0, 0};
	// Буфер записи в EEPROM 2
	uint8_t WriteBuff2[8] = {0, 0, 0, 0, 0, 0, 0, 0};
		
	uint8_t NewFlag = 0;
/************************************************************************/
/* >> ЧТЕНИЕ ЗАГОЛОВКА													*/
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
/* >> ЧТЕНИЕ ЗАПИСИ														*/
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
/* >> ВЫВОД ЭКРАНОВ ЖУРНАЛА												*/
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
/* >> ОБНОВЛЕНИЕ ЗАПИСИ													*/
/************************************************************************/
	// Если стоит флаг непрочитанного сообщения
	if (ReadBuff2[0] & 0b10000000) {
		WriteBuff2[0] = ReadBuff2[0] & 0b01111111; // Снимаем флаг
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
/* >> ОБРАБОТКА ДЕЙСТВИЙ												*/
/************************************************************************/
	switch (Action) 
	{
// (8) ВПРАВО - Вперёд (+ 1)
		case 8:
			if (TotMes) {
				if (CurMes < TotMes) {
					++ CurMes;
				}
			}
			Action = 255;
			LCD_Refresh = 1;
			break;

// (5) ВВЕРХ - Вперёд (+ 10)
		case 5:
			if (TotMes) {
				if ((TotMes - CurMes) >= 10) {
					CurMes += 10;
				}
			}
			Action = 255;
			LCD_Refresh = 1;
			break;

// (7) ВЛЕВО - Назад (- 1)
		case 7:
			if (TotMes) {
				if (CurMes > 1) {
					-- CurMes;
				}
			}
			Action = 255;
			LCD_Refresh = 1;
			break;
			
// (6) ВНИЗ - Назад (- 10)	
		case 6:
			if (TotMes) {
				if (CurMes >= 11) {
					CurMes -= 10;
				}
			}
			Action = 255;
			LCD_Refresh = 1;
			break;
			
// (0) BACK - Выход из ЖУРНАЛА
		case 0:
			// Возврат в МЕНЮ ЖУРНАЛА
			Mode = MODE_EVENTS_MENU;
			Action = 255;
			LCD_Refresh = 1;
			return 0;
			break;
		default:
			break;
	}
/************************************************************************/
/* >> ОБНОВЛЕНИЕ ЗАГОЛОВКА												*/
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
/* -> ЭКРАН: ОЧИСТКА ЖУРНАЛА								*/
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
	// Действия в зависимости от нажатых кнопок
	switch (Action) {
		
		// (1) YES
		case 1:
		// Очистка ЖУРНАЛА и возврат в МЕНЮ ЖУРНАЛА
		REG_Reset ();
		Mode = MODE_EVENTS_MENU;
		LCD_Refresh = 1;
		Action = 255;
//		REG_Reset ();
		break;
		
		// (0) NO
		case 0:
		// Возврат в МЕНЮ ЖУРНАЛА без очистки ЖУРНАЛА
		Mode = MODE_EVENTS_MENU;
		LCD_Refresh = 1;
		Action = 255;
		break;

		default:
		break;
	}
}
/************************************************************************/
/* -> РЕИНИЦИАЛИЗАЦИЯ ЖУРНАЛА СОБЫТИЙ        */
/************************************************************************/
void REG_Reset (void) {
	// Буфер записи в EEPROM 1
	uint8_t WriteBuff1[8] = {0, 0, 0, 0, 0, 0, 0, 0};
	
/* СТРУКТУРА ЗАГОЛОВКА ЖУРНАЛА:
 * 
 * 1ый БАЙТ - 
 * [0] Часы (Реинициализация)
 * [1] Минуты (Реинициализация)
 * [2] Секунды (Реинициализация)
 * [3] Число (Реинициализация)
 * [4] Месяц (Реинициализация)
 * [5] Год (Реинициализация)
 * [6] Код = 0xFF
 * [7] Контрольная сумма
 * 
 * 2ой БАЙТ - 
 * [0] Резерв
 * [1] Резерв
 * [2] Резерв
 * [3] Кол-во прочитанных записей (младший байт)
 * [4] Кол-во прочитанных записей (старший байт)
 * [5] Кол-во записей (младший байт)
 * [6] Кол-во записей (старший байт)
 * [7] Контрольная сумма 
 * 
/************************************************************************/
/* >> ПОДГОТОВКА ПЕРВОГО БАЙТА											*/
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
/* >> ЗАПИСЬ ПЕРВОГО БАЙТА												*/
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
/* >> ПОДГОТОВКА ВТОРОГО БАЙТА											*/
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
/* >> ЗАПИСЬ ВТОРОГО БАЙТА												*/
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
/* -> ОБРАБОТКА ЭКРАНОВ ЖУРНАЛА							*/
/************************************************************************/
void REG_Screen (uint8_t Mode, uint16_t TotMes, uint16_t CurMes, uint16_t NewMes, uint8_t NewFlag, uint8_t * Data) {
	uint8_t TempChar = 0;
	switch (Mode) {
/************************************************************************/
/* >> ЖУРНАЛ ПУСТ														*/
/************************************************************************/
		case 0:
			// Информация
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
			// Переменные
			LCD_PutValDec (0, 5, 4, CurMes);
			LCD_PutValDec (0, 10, 4, NewMes);
			LCD_PutValDec (0, 16, 4, TotMes);
			break;
/************************************************************************/
/* >> ОШИБКА CRC ЗАПИСИ													*/
/************************************************************************/
		case 1:
			// Информация
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
			// Переменные
			LCD_PutValDec (0, 5, 4, CurMes);
			LCD_PutValDec (0, 10, 4, NewMes);
			LCD_PutValDec (0, 16, 4, TotMes);
			// Новое сообщение
			if (NewFlag) {
				NewFlag = 0;
				// LCD_PutSym (0, 3, '*');
				LCD_PutStr (0, 0, (uint8_t *) "HOB!");
			}
			break;
/************************************************************************/
/* >> КОД СОБЫТИЯ ВЫХОДИТ ЗА РАМКИ ДОПУСТИМОГО							*/
/************************************************************************/ 
		case 2:
			// Информация
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
			// Время
			LCD_PutValDec (1, 0, 2, *(Data + 0) & 0b01111111);
			LCD_PutValDec (1, 3, 2, *(Data + 1));
			LCD_PutValDec (1, 6, 2, *(Data + 2));
			// Дата
			LCD_PutValDec (1, 12, 2, *(Data + 3));
			LCD_PutValDec (1, 15, 2, *(Data + 4) & 0x0F);
			LCD_PutValDec (1, 18, 2, *(Data + 5));
			// Переменные
			LCD_PutValDec (0, 5, 4, CurMes);
			LCD_PutValDec (0, 10, 4, NewMes);
			LCD_PutValDec (0, 16, 4, TotMes);
			// Номер преобразователя
			LCD_PutValDec (3, 3, 2, (*(Data + 4) & 0xF0)>>4 );
			// Новое сообщение
			if (NewFlag) {
				NewFlag = 0;
				// LCD_PutSym (0, 3, '*');
				LCD_PutStr (0, 0, (uint8_t *) "HOB!");
			}
			break;

/************************************************************************/
/* >> ОТОБРАЖЕНИЕ ТЕКУЩЕЙ ЗАПИСИ										*/
/************************************************************************/
		case 3:
			// Информация
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
			// Время
			LCD_PutValDec (1, 0, 2, *(Data + 0) & 0b01111111);
			LCD_PutValDec (1, 3, 2, *(Data + 1));
			LCD_PutValDec (1, 6, 2, *(Data + 2));
			// Дата
			LCD_PutValDec (1, 12, 2, *(Data + 3));
			LCD_PutValDec (1, 15, 2, *(Data + 4) & 0x0F);
			LCD_PutValDec (1, 18, 2, *(Data + 5));
			// Переменные
			LCD_PutValDec (0, 5, 4, CurMes);
			LCD_PutValDec (0, 10, 4, NewMes);
			LCD_PutValDec (0, 16, 4, TotMes);
			// Номер преобразователя
			LCD_PutValDec (3, 3, 2, (*(Data + 4) & 0xF0)>>4 );
			// Текст
			for (uint8_t IdCol = 0; IdCol <= 19; IdCol++) {
				LCD_PutSym (2, IdCol, APD_PrjFontTable[pgm_read_byte ( APD_PrjData + APD_EventsStartAdr + 8 + *(Data + 6)*20 /*APD_EventStartAdr[*(Data + 6)]*/ + IdCol )]);
			}
			// Новое сообщение
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
/* -> ЭКРАН: СОХРАНЕНИЕ ЖУРНАЛА						*/
/************************************************************************/
void SCREEN_EVENTS_SAVE_MENU (void) {
	uint8_t TempChar = 0;
	// Буфер чтения из EEPROM 1
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
	/* ЧТЕНИЕ ЗАГОЛОВКА *****************************************************/
	/************************************************************************/
	// 2ой байт
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
			// Возврат МЕНЮ журнала
			Mode = MODE_EVENTS_MENU;
			LCD_Refresh = 1;
			Action = 255;
			break;
	
		// (1) YES
		case 1:
			// Отключаем все лишние таймеры обмена и запрещаем запрос данных и
			// списка событий
			TIMER_DataRequestEn = 0;
			TIMER_DataRequest = 0;
			TIMER_RequestTimeoutEn = 0;
			TIMER_RequestTimeout = 0;
			DATA_ReqEn = 0;
			EVENT_ReqEn = 0;

			// Включаем таймер выгрузки журнала
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
/* -> ЭКРАН: ПРОГРЕСС СОХРАНЕНИЯ ЖУРНАЛА		*/
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
/* -> ЭКРАН: СОХРАНЕНИЕ ЖУРНАЛА ВЫПОЛНЕНО		*/
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
/* -> ЭКРАН: НАСТРОЙКИ ЖУРНАЛА								*/
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
		// Маркер
		LCD_PutSym (1 + REG_SET_SetMarker, 0, ARROW_RIGHT);
	}
	// Действия в зависимости от нажатых кнопок
	switch (Action) {
		// (7) ВЛЕВО - УМЕНЬШИТЬ
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
		
		// (8) ВПРАВО - УВЕЛИЧИТЬ
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

		// (5) ВВЕРХ - МАРКЕР ВВЕРХ
		case 5:
		if (REG_SET_SetMarker == 0) {
			REG_SET_SetMarker = 1;
		} else {
			REG_SET_SetMarker = 0;
		}
		Action = 255;
		break;

		// (6) ВНИЗ - МАРКЕР ВНИЗ
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
		// Применение установок
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
		
		// Возврат в сервисный режим
		Mode = MODE_EVENTS_MENU;
		LCD_Refresh = 1;
		Action = 255;
		REG_SET_ShowOnce = 1;
		break;

		// (0) - NO
		case 0:
		REG_SET_SetMarker = 0;
		// Возврат в сервисный режим
		Mode = MODE_EVENTS_MENU;
		LCD_Refresh = 1;
		Action = 255;
		REG_SET_ShowOnce = 1;
		break;
		
		// (9) DEFAULT - ВЕРНУТЬ УМОЛЧАНИЯ
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
	// Маркер
	if (REG_SET_SetMarker == 0) {
		LCD_PutSym (2, 0, 0x20);
	} else {
		LCD_PutSym (1, 0, 0x20);
	}
	LCD_PutSym (1 + REG_SET_SetMarker, 0, ARROW_RIGHT);
}

/************************************************************************/
/* -> ПРОЗРАЧНЫЙ РЕЖИМ: ВЫБОР							*/
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

	// Действия в зависимости от нажатых кнопок
	switch (Action) {
		// (0) BACK
		case 0:
			// Возврат в сервисный режим
			Mode = MODE_MENU;
			TIMER_ClockRefreshEn = 1;
			ClockShowEn = 1;
			LCD_Refresh = 1;
			Action = 255;
			break;
			
		// (1) 9600	
		case 1:
// 			// Отключаем все лишние таймеры обмена и запрещаем запрос данных и 
// 			// списка событий
	
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
/* -> ПРОЗРАЧНЫЙ РЕЖИМ: ПОДТВЕРЖДЕНИЕ				*/
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
	
	// Действия в зависимости от нажатых кнопок
	switch (Action) {
		// (0) NO
		case 0:
			// Возврат в сервисный режим
			Mode = MODE_MENU;
			TIMER_ClockRefreshEn = 1;
			ClockShowEn = 1;
			LCD_Refresh = 1;
			Action = 255;
			break;
			
		// (1) YES	
		case 1:
 			// Отключаем все лишние таймеры обмена и запрещаем запрос данных и 
			// списка событий
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
/* -> ПРОЗРАЧНЫЙ РЕЖИМ: ПРОГРЕСС							*/
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
/* -> ПРОЗРАЧНЫЙ РЕЖИМ: АКТИВНЫЙ							*/
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
/* -> ЭКРАН: ИНФО														*/
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
/*                   ПРИНЯТЫЕ СООБЩЕНИЯ                                 */
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

// Содержимое EEPROM
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
/*				Описание функций										*/
/************************************************************************/
/************************************************************************/
/*			Вспомогательные функции подготовки данных к запросу*/
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

// Подсчёт количества адресов для запроса в текущем фокусе
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

// Сортировка массива по возрастанию
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

// Подготовка к запросу: группировка и выделение блоков
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
	// Начало
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
	// Середина
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
	// Конец
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

// Вычисление расстояний между запрашиваемыми адресами
void FOCUS_Space (void) {
	uint8_t Id8 = 0;
	
	if (FOCUS_DataAmount) {	
		for (Id8 = 1; Id8 <= (FOCUS_DataAmount - 1); Id8++) {
			FOCUS_SpaceArray[Id8] = FOCUS_DataAdr[Id8] - FOCUS_DataAdr[Id8 - 1] - 1;
		}
	}
}

/************************************************************************/
/*               Работа с часами реального времени RTC          */
/************************************************************************/
/************************************************************************/
/* -> РУЧНАЯ УСТАНОВКА ВРЕМЕНИ								*/
/************************************************************************/
uint8_t TIME_SetManual (uint8_t Hours, uint8_t Minutes, uint8_t Seconds, uint8_t Day, uint8_t Date, uint8_t Month, uint8_t Year) {	
	C_TwiEn (); // Включение модуля TWI	
	C_TwiBusSetIdle (); // Перевод шины в состояние IDLE

	if (C_TwiBusWaitForIdle ()) {	// Ожидание состояния шины IDLE
		C_TwiTransactionStart (0); // Старт с флагом записи
	} else {
		C_TwiCmdStop (); // Отправка условия STOP
		C_TwiDisable (); // Выключение модуля TWI
		return 0;} // Функция возвращает 0, если превышен интервал ожидания флага
	if (C_TwiInterruptWaitForWif ()) { // Ожидание флага записи байта WIF
		if (C_TwiCheckAck ()) {	// Проверка, что до этого был получен ACK
			C_TwiDataPut (0); // Запись значения счётчика адресов RTC
		} else {
			// Если получен ответ NACK
			C_TwiCmdStop (); // Отправка условия STOP
			C_TwiDisable (); // Выключение модуля TWI
			// LCD_PutSym (0, 3, '*');
			return 0;} // Функция возвращает 0, если превышен интервал ожидания флага
	} else {
		C_TwiCmdStop (); // Отправка условия STOP
		C_TwiDisable (); // Выключение модуля TWI
		return 0;} // Функция возвращает 0, если превышен интервал ожидания флага
	if (C_TwiInterruptWaitForWif ()) { // Ожидание флага записи байта WIF
		if (C_TwiCheckAck ()) {	// Проверка, что до этого был получен ACK
			C_TwiDataPut (((Seconds/10)<<4) | (Seconds%10)); // Запись байта из регистра (секунды)
		} else {
			// Если получен ответ NACK
			C_TwiCmdStop (); // Отправка условия STOP
			C_TwiDisable (); // Выключение модуля TWI
			// LCD_PutSym (0, 3, '*');
			return 0;} // Функция возвращает 0, если превышен интервал ожидания флага
	} else {
		C_TwiCmdStop (); // Отправка условия STOP
		C_TwiDisable (); // Выключение модуля TWI
		return 0;} // Функция возвращает 0, если превышен интервал ожидания флага
	if (C_TwiInterruptWaitForWif ()) { // Ожидание флага записи байта WIF
		if (C_TwiCheckAck ()) {	// Проверка, что до этого был получен ACK
			C_TwiDataPut (((Minutes/10)<<4) | (Minutes%10)); // Запись байта из регистра (минуты)
		} else {
			// Если получен ответ NACK
			C_TwiCmdStop (); // Отправка условия STOP
			C_TwiDisable (); // Выключение модуля TWI
			// LCD_PutSym (0, 3, '*');
			return 0;} // Функция возвращает 0, если превышен интервал ожидания флага
	} else {
		C_TwiCmdStop (); // Отправка условия STOP
		C_TwiDisable (); // Выключение модуля TWI
		return 0;} // Функция возвращает 0, если превышен интервал ожидания флага
	if (C_TwiInterruptWaitForWif ()) { // Ожидание флага записи байта WIF
		if (C_TwiCheckAck ()) {	// Проверка, что до этого был получен ACK
			C_TwiDataPut ((0x00 | (Hours/10)<<4) | (Hours%10)); // Запись байта из регистра (часы)
		} else {
			// Если получен ответ NACK
			C_TwiCmdStop (); // Отправка условия STOP
			C_TwiDisable (); // Выключение модуля TWI
			// LCD_PutSym (0, 3, '*');
			return 0;} // Функция возвращает 0, если превышен интервал ожидания флага
	} else {
		C_TwiCmdStop (); // Отправка условия STOP
		C_TwiDisable (); // Выключение модуля TWI
		return 0;} // Функция возвращает 0, если превышен интервал ожидания флага
	if (C_TwiInterruptWaitForWif ()) { // Ожидание флага записи байта WIF
		if (C_TwiCheckAck ()) {	// Проверка, что до этого был получен ACK
			C_TwiDataPut (0x00 | Day); // Запись байта из регистра (день)
		} else {
			// Если получен ответ NACK
			C_TwiCmdStop (); // Отправка условия STOP
			C_TwiDisable (); // Выключение модуля TWI
			// LCD_PutSym (0, 3, '*');
			return 0;} // Функция возвращает 0, если превышен интервал ожидания флага
	} else {
		C_TwiCmdStop (); // Отправка условия STOP
		C_TwiDisable (); // Выключение модуля TWI
		return 0;} // Функция возвращает 0, если превышен интервал ожидания флага
	if (C_TwiInterruptWaitForWif ()) { // Ожидание флага записи байта WIF
		if (C_TwiCheckAck ()) {	// Проверка, что до этого был получен ACK
			C_TwiDataPut ((0x00 | (Date/10)<<4) | (Date%10)); // Запись байта из регистра (дата)
		} else {
			// Если получен ответ NACK
			C_TwiCmdStop (); // Отправка условия STOP
			C_TwiDisable (); // Выключение модуля TWI
			// LCD_PutSym (0, 3, '*');
			return 0;} // Функция возвращает 0, если превышен интервал ожидания флага	
	} else {
		C_TwiCmdStop (); // Отправка условия STOP	
		C_TwiDisable (); // Выключение модуля TWI
		return 0;} // Функция возвращает 0, если превышен интервал ожидания флага
	if (C_TwiInterruptWaitForWif ()) { // Ожидание флага записи байта WIF
		if (C_TwiCheckAck ()) {	// Проверка, что до этого был получен ACK
			C_TwiDataPut ((0x00 | (Month/10)<<4) | (Month%10)); // Запись байта из регистра (месяц)
		} else {
			// Если получен ответ NACK
			C_TwiCmdStop (); // Отправка условия STOP
			C_TwiDisable (); // Выключение модуля TWI
			// LCD_PutSym (0, 3, '*');
			return 0;} // Функция возвращает 0, если превышен интервал ожидания флага
	} else {
		C_TwiCmdStop (); // Отправка условия STOP	
		C_TwiDisable (); // Выключение модуля TWI
		return 0;} // Функция возвращает 0, если превышен интервал ожидания флага
	if (C_TwiInterruptWaitForWif ()) { // Ожидание флага записи байта WIF
		if (C_TwiCheckAck ()) {	// Проверка, что до этого был получен ACK
			C_TwiDataPut ((0x00 | (Year/10)<<4) | (Year%10)); // Запись байта из регистра (год)
		} else {
			// Если получен ответ NACK
			C_TwiCmdStop (); // Отправка условия STOP
			C_TwiDisable (); // Выключение модуля TWI
			// LCD_PutSym (0, 3, '*');
			return 0;} // Функция возвращает 0, если превышен интервал ожидания флага
	} else {
		C_TwiCmdStop (); // Отправка условия STOP
		C_TwiDisable (); // Выключение модуля TWI
		return 0;} // Функция возвращает 0, если превышен интервал ожидания флага
	if (C_TwiInterruptWaitForWif ()) { // Ожидание флага записи байта WIF
		if (C_TwiCheckAck ()) {	// Проверка, что до этого был получен ACK
		
		} else {
			// Если получен ответ NACK
			C_TwiCmdStop (); // Отправка условия STOP	
			C_TwiDisable (); // Выключение модуля TWI
			// LCD_PutSym (0, 3, '*');
			return 0;} // Функция возвращает 0, если превышен интервал ожидания флага
	} else {
		C_TwiCmdStop (); // Отправка условия STOP
		C_TwiDisable (); // Выключение модуля TWI
		return 0;} // Функция возвращает 0, если превышен интервал ожидания флага
	C_TwiCmdStop (); // Отправка условия STOP
	C_TwiDisable (); // Выключение модуля TWI
	return 1; // Функция возвращает 1, если запись была завершена успешно 
}

/************************************************************************/
/* -> УСТАНОВКА ВРЕМЕНИ											*/
/************************************************************************/
uint8_t TIME_Set (void) {	
	C_TwiEn (); // Включение модуля TWI	
	C_TwiBusSetIdle (); // Перевод шины в состояние IDLE

	if (C_TwiBusWaitForIdle ()) {	// Ожидание состояния шины IDLE
		C_TwiTransactionStart (0); // Старт с флагом записи
	} else {
		C_TwiCmdStop (); // Отправка условия STOP
		C_TwiDisable (); // Выключение модуля TWI
		return 0;}// Функция возвращает 0, если превышен интервал ожидания флага
	if (C_TwiInterruptWaitForWif ()) { // Ожидание флага записи байта WIF
		if (C_TwiCheckAck ()) {	// Проверка, что до этого был получен ACK
			C_TwiDataPut (0); // Запись значения счётчика адресов RTC
		} else {
			// Если получен ответ NACK
			C_TwiCmdStop (); // Отправка условия STOP
			C_TwiDisable (); // Выключение модуля TWI
			return 0;} // Функция возвращает 0, если превышен интервал ожидания флага
	} else {
		C_TwiCmdStop (); // Отправка условия STOP
		C_TwiDisable (); // Выключение модуля TWI
		return 0;} // Функция возвращает 0, если превышен интервал ожидания флага
	if (C_TwiInterruptWaitForWif ()) { // Ожидание флага записи байта WIF
		if (C_TwiCheckAck ()) {	// Проверка, что до этого был получен ACK
			C_TwiDataPut ((0x00 | (TIME_SetSeconds/10)<<4) | (TIME_SetSeconds%10)); // Запись байта из регистра (секунды)
		} else {
			// Если получен ответ NACK
			C_TwiCmdStop (); // Отправка условия STOP
			C_TwiDisable (); // Выключение модуля TWI
			return 0;} // Функция возвращает 0, если превышен интервал ожидания флага
	} else {
		C_TwiCmdStop (); // Отправка условия STOP
		C_TwiDisable (); // Выключение модуля TWI
		return 0;} // Функция возвращает 0, если превышен интервал ожидания флага
	if (C_TwiInterruptWaitForWif ()) { // Ожидание флага записи байта WIF
		if (C_TwiCheckAck ()) {	// Проверка, что до этого был получен ACK
			C_TwiDataPut ((0x00 | (TIME_SetMinutes/10)<<4) | (TIME_SetMinutes%10)); // Запись байта из регистра (минуты)
		} else {
			// Если получен ответ NACK
			C_TwiCmdStop (); // Отправка условия STOP
			C_TwiDisable (); // Выключение модуля TWI
			return 0;} // Функция возвращает 0, если превышен интервал ожидания флага
	} else {
		C_TwiCmdStop (); // Отправка условия STOP
		C_TwiDisable (); // Выключение модуля TWI
		return 0;} // Функция возвращает 0, если превышен интервал ожидания флага
	if (C_TwiInterruptWaitForWif ()) { // Ожидание флага записи байта WIF
		if (C_TwiCheckAck ()) {	// Проверка, что до этого был получен ACK
			C_TwiDataPut ((0x00 | (TIME_SetHours/10)<<4) | (TIME_SetHours%10)); // Запись байта из регистра (часы)
		} else {
			// Если получен ответ NACK
			C_TwiCmdStop (); // Отправка условия STOP	
			C_TwiDisable (); // Выключение модуля TWI
			return 0;} // Функция возвращает 0, если превышен интервал ожидания флага
	} else {
		C_TwiCmdStop (); // Отправка условия STOP
		C_TwiDisable (); // Выключение модуля TWI
		return 0;} // Функция возвращает 0, если превышен интервал ожидания флага
		if (C_TwiInterruptWaitForWif ()) { // Ожидание флага записи байта WIF
		if (C_TwiCheckAck ()) {	// Проверка, что до этого был получен ACK
			C_TwiDataPut (0x00 | TIME_SetDay); // Запись байта из регистра (день)
		} else {
			// Если получен ответ NACK
			C_TwiCmdStop (); // Отправка условия STOP	
			C_TwiDisable (); // Выключение модуля TWI
			return 0;} // Функция возвращает 0, если превышен интервал ожидания флага
	} else {
		C_TwiCmdStop (); // Отправка условия STOP
		C_TwiDisable (); // Выключение модуля TWI
		return 0;} // Функция возвращает 0, если превышен интервал ожидания флага
	if (C_TwiInterruptWaitForWif ()) { // Ожидание флага записи байта WIF
		if (C_TwiCheckAck ()) {	// Проверка, что до этого был получен ACK
			C_TwiDataPut ((0x00 | (TIME_SetDate/10)<<4) | (TIME_SetDate%10)); // Запись байта из регистра (дата)
		} else {
			// Если получен ответ NACK
			C_TwiCmdStop (); // Отправка условия STOP
			C_TwiDisable (); // Выключение модуля TWI
			return 0;} // Функция возвращает 0, если превышен интервал ожидания флага
	} else {
		C_TwiCmdStop (); // Отправка условия STOP
		C_TwiDisable (); // Выключение модуля TWI
		return 0;} // Функция возвращает 0, если превышен интервал ожидания флага
	if (C_TwiInterruptWaitForWif ()) { // Ожидание флага записи байта WIF
		if (C_TwiCheckAck ()) {	// Проверка, что до этого был получен ACK
			C_TwiDataPut ((0x00 | (TIME_SetMonth/10)<<4) | (TIME_SetMonth%10)); // Запись байта из регистра (месяц)
		} else {
			// Если получен ответ NACK
			C_TwiCmdStop (); // Отправка условия STOP	
			C_TwiDisable (); // Выключение модуля TWI
			return 0;} // Функция возвращает 0, если превышен интервал ожидания флага
	} else {
		C_TwiCmdStop (); // Отправка условия STOP
		C_TwiDisable (); // Выключение модуля TWI
		return 0;} // Функция возвращает 0, если превышен интервал ожидания флага
	if (C_TwiInterruptWaitForWif ()) { // Ожидание флага записи байта WIF
		if (C_TwiCheckAck ()) {	// Проверка, что до этого был получен ACK
			C_TwiDataPut ((0x00 | (TIME_SetYear/10)<<4) | (TIME_SetYear%10)); // Запись байта из регистра (год)
		} else {
			// Если получен ответ NACK
			C_TwiCmdStop (); // Отправка условия STOP
			C_TwiDisable (); // Выключение модуля TWI
			return 0;} // Функция возвращает 0, если превышен интервал ожидания флага
	} else {
		C_TwiCmdStop (); // Отправка условия STOP
		C_TwiDisable (); // Выключение модуля TWI
		return 0;} // Функция возвращает 0, если превышен интервал ожидания флага
	if (C_TwiInterruptWaitForWif ()) { // Ожидание флага записи байта WIF
		if (C_TwiCheckAck ()) {	// Проверка, что до этого был получен ACK
		
		} else {
			// Если получен ответ NACK
			C_TwiCmdStop (); // Отправка условия STOP
			C_TwiDisable (); // Выключение модуля TWI
			return 0;} // Функция возвращает 0, если превышен интервал ожидания флага
	} else {
		C_TwiCmdStop (); // Отправка условия STOP
		C_TwiDisable (); // Выключение модуля TWI
		return 0;} // Функция возвращает 0, если превышен интервал ожидания флага
	C_TwiCmdStop (); // Отправка условия STOP	
	C_TwiDisable (); // Выключение модуля TWI	
	return 1; // Функция возвращает 1, если запись была завершена успешно 
}

/************************************************************************/
/* -> ПОЛУЧЕНИЕ ЗНАЧЕНИЙ ДАТЫ И ВРЕМЕНИ			*/
/************************************************************************/
uint8_t TIME_Get (void) {	
	C_TwiEn (); // Включение модуля TWI
	C_TwiBusSetIdle (); // Перевод шины в состояние IDLE

	if (C_TwiBusWaitForIdle ()) {	// Ожидание состояния шины IDLE
		C_TwiTransactionStart (0); // Старт с флагом записи
	} else {
		C_TwiCmdStop (); // Отправка условия STOP
		C_TwiDisable (); // Выключение модуля TWI
		return 0;} // Функция возвращает 0, если превышен интервал ожидания флага
	if (C_TwiInterruptWaitForWif ()) { // Ожидание флага записи байта WIF
		if (C_TwiCheckAck ()) {	// Проверка, что до этого был получен ACK
			C_TwiDataPut (0);} // Запись значения счётчика адресов RTC
	} else {
		C_TwiCmdStop (); // Отправка условия STOP
		C_TwiDisable (); // Выключение модуля TWI
		return 0;} // Функция возвращает 0, если превышен интервал ожидания флага
	if (C_TwiInterruptWaitForWif ()) { // Ожидание флага записи байта WIF
		if (C_TwiCheckAck ()) {	// Проверка, что до этого был получен ACK
			C_TwiTransactionStart (1);} // Повторный старт с флагом чтения
	} else {
		C_TwiCmdStop (); // Отправка условия STOP
		C_TwiDisable (); // Выключение модуля TWI
		return 0;} // Функция возвращает 0, если превышен интервал ожидания флага
	if (C_TwiInterruptWaitForRif ()) { // Ожидание флага чтения байта RIF
		TIME_GetSeconds = C_TwiDataGet (); // Чтение байта из регистра (секунды)
		C_TwiCmdSendAck (); // Отправка бита подтверждения ACK
	} else {
		C_TwiCmdStop (); // Отправка условия STOP
		C_TwiDisable (); // Выключение модуля TWI
		return 0;} // Функция возвращает 0, если превышен интервал ожидания флага
	if (C_TwiInterruptWaitForRif ()) { // Ожидание флага чтения байта RIF
		TIME_GetMinutes = C_TwiDataGet (); // Чтение байта из регистра (минуты)
		C_TwiCmdSendAck (); // Отправка бита подтверждения ACK
	} else {
		C_TwiCmdStop (); // Отправка условия STOP
		C_TwiDisable (); // Выключение модуля TWI
		return 0;} // Функция возвращает 0, если превышен интервал ожидания флага
	if (C_TwiInterruptWaitForRif ()) { // Ожидание флага чтения байта RIF
		TIME_GetHours = C_TwiDataGet (); // Чтение байта из регистра (часы)
		C_TwiCmdSendNack (); // Отправка бита подтверждения NACK (после последнего байта)
	}  else {
		C_TwiCmdStop (); // Отправка условия STOP
		C_TwiDisable (); // Выключение модуля TWI
		return 0;} // Функция возвращает 0, если превышен интервал ожидания флага
	if (C_TwiInterruptWaitForRif ()) { // Ожидание флага чтения байта RIF
		TIME_GetDay = C_TwiDataGet (); // Чтение байта из регистра (день)
		C_TwiCmdSendAck (); // Отправка бита подтверждения ACK
	} else {
		C_TwiCmdStop (); // Отправка условия STOP
		C_TwiDisable (); // Выключение модуля TWI
		return 0;} // Функция возвращает 0, если превышен интервал ожидания флага
	if (C_TwiInterruptWaitForRif ()) { // Ожидание флага чтения байта RIF
		TIME_GetDate = C_TwiDataGet (); // Чтение байта из регистра (дата)
		C_TwiCmdSendAck (); // Отправка бита подтверждения ACK
	} else {
		C_TwiCmdStop (); // Отправка условия STOP
		C_TwiDisable (); // Выключение модуля TWI
		return 0;} // Функция возвращает 0, если превышен интервал ожидания флага
	if (C_TwiInterruptWaitForRif ()) { // Ожидание флага чтения байта RIF
		TIME_GetMonth = C_TwiDataGet (); // Чтение байта из регистра (месяц)
		C_TwiCmdSendNack (); // Отправка бита подтверждения NACK (после последнего байта)
	}  else {
		C_TwiCmdStop (); // Отправка условия STOP
		C_TwiDisable (); // Выключение модуля TWI
		return 0;} // Функция возвращает 0, если превышен интервал ожидания флага
	if (C_TwiInterruptWaitForRif ()) { // Ожидание флага чтения байта RIF
		TIME_GetYear = C_TwiDataGet (); // Чтение байта из регистра (год)
		C_TwiCmdSendNack (); // Отправка бита подтверждения NACK (после последнего байта)
	}  else {
		C_TwiCmdStop (); // Отправка условия STOP
		C_TwiDisable (); // Выключение модуля TWI
		return 0;} // Функция возвращает 0, если превышен интервал ожидания флага
	C_TwiCmdStop (); // Отправка условия STOP	
	C_TwiDisable (); // Выключение модуля TWI	
	return 1; // Функция возвращает 1, если чтение было завершено успешно 
}

/************************************************************************/
/* -> ФУНКЦИИ ТАЙМЕРОВ												*/
/************************************************************************/
//inline void TIMER_1msInit (void) {	
	//TCC1.PER = 18432;
	//TCC1.INTCTRLA |= (0x01<<0); // Уровень прерывания по переполнению
	//TCC1.CTRLA |= (0x01<<0);		
//}

//inline void TIMER_Rs232Init (void) {	
	//TCD1.PER = 25000;
	//TCD1.INTCTRLA |= (0x02<<0); // Уровень прерывания по переполнению
//}

//inline void TIMER_Rs232Start () {
	//TCD1.CNT = 0x0000; // Обнуление счётного регистра таймера
	//TCD1.CTRLA |= (0x01<<0); // Старт счётчика
//}
//
//inline void TIMER_Rs232Stop () {
	//TCD1.CTRLA &= ~ (0x01<<0); // Стоп счётчика
//}
/************************************************************************/
/*				РАБОТА С КНОПКОДЕЙСТВИЯМИ ПРОЕКТА APD*/
/************************************************************************/
/* Таблица соответствия нажатия кнопок коду кнопкодействия
 * Строка == [Кнопка 1]
 * Столбец == [Кнопка 2]
 * На пересечении == код кнопкодействия
 * Нет нажатия == 0xFF
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
 * Объявление функций кнопкодействий проекта APD
 */
void APD_SendToPlc (uint8_t Id); // Действие "Передать в ПЛК"
void APD_GoToFrame (uint8_t Id); // Действие "Перейти к кадру"
void APD_EditField (uint8_t Id); // Действие "Редактировать поле"
void APD_FrameShiftUp (uint8_t Id); // Действие "Сдвиг кадра вверх"
void APD_FrameShiftDown (uint8_t Id); // Действие "Сдвиг кадра вниз"

void APD_ListShiftUp (uint8_t Id); // Действие "Сдвиг СПИСКА вверх"
void APD_ListShiftDown (uint8_t Id); // Действие "Сдвиг СПИСКА вниз"

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
	DATA = 0, // Запрос данных
	EVENTS = 1, // Запрос списка событий
 } trans_status_t;

/*
 * Описание функций кнопкодействий проекта APD
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

/* КАК ЭТО СДЕЛАНО В APD:
 * 0 - действие = Передать в ПЛК
 * 1 - действие = Переход на кадр
 * 2 - действие = Сдвиг кадра вверх
 * 3 - действие = Сдвиг кадра вниз
 * 4 - действие = Сдвиг вверх поля-списка
 * 5 - действие = Сдвиг вниз поля-списка
 * 6 - действие = Редактировать поле
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

// Обработка нажатий кнопок в проекте APD
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

// Вывод редактируемого поля в процессе редактирования
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
/*					РАБОТА С ЭКРАНАМИ ДИСПЛЕЯ                 */
/************************************************************************/
/************************************************************************/
/* -> НОРМАЛЬНЫЙ РЕЖИМ (ОТОБРАЖАЮТСЯ КАДРЫ ПРОЕКТА APD)  */
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
			/* -> ТИП ПОЛЯ: ЗАСТАВКА												*/
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
			/* -> ТИП ПОЛЯ: ЧИСЛО													*/
			/************************************************************************/
			case APD_FIELD_TYPE_VALUE:

				APD_ValStr = pgm_read_byte ( APD_PrjData + APD_FrameFieldStartAdr[APD_FrameCur][IdField] + 1 ) - APD_FrameDisplayShift;
				if ((APD_ValStr >= 0) && (Abs (APD_ValStr) < (pgm_read_byte ( APD_PrjData + 8 )))) {

					APD_ValStr = Abs (APD_ValStr);
					APD_ValCol = pgm_read_byte ( APD_PrjData + APD_FrameFieldStartAdr[APD_FrameCur][IdField] + 2 );
					// Длина поля
					APD_ValLen = pgm_read_byte ( APD_PrjData + APD_FrameFieldStartAdr[APD_FrameCur][IdField] + 3 );
					// Адрес переменной поля
					APD_ValPlcAdr = (pgm_read_byte ( APD_PrjData + APD_FrameFieldStartAdr[APD_FrameCur][IdField] + 4 )) | (pgm_read_byte ( APD_PrjData + APD_FrameFieldStartAdr[APD_FrameCur][IdField] + 5 ) << 8);
					++ APD_ValPlcAdr;
					// По адресу находим нужный регистр
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
					// Переменная в длинный формат
					APD_Val = (uint64_t) APD_Val_;
					// Формат вывода поля
					APD_ValFormat = pgm_read_byte ( APD_PrjData + APD_FrameFieldStartAdr[APD_FrameCur][IdField] + 7 );
					// Множитель (он же сдвиг вправо)
					APD_ValMultiplier = (int64_t) (pgm_read_byte ( APD_PrjData + APD_FrameFieldStartAdr[APD_FrameCur][IdField] + 9 ) << 8) | (pgm_read_byte ( APD_PrjData + APD_FrameFieldStartAdr[APD_FrameCur][IdField] + 8 ));
					// Делитель
					APD_ValDivider = (int64_t) (pgm_read_byte ( APD_PrjData + APD_FrameFieldStartAdr[APD_FrameCur][IdField] + 11 ) << 8) | (pgm_read_byte ( APD_PrjData + APD_FrameFieldStartAdr[APD_FrameCur][IdField] + 10 ));
					// Добавка
					APD_ValAddition = (int64_t) (pgm_read_byte ( APD_PrjData + APD_FrameFieldStartAdr[APD_FrameCur][IdField] + 13 ) << 8) | (pgm_read_byte ( APD_PrjData + APD_FrameFieldStartAdr[APD_FrameCur][IdField] + 12 ));	

					// Позиция точки
					PointPos = (APD_ValFormat & 0x0F);
					
					// LCD_PutValDecPointMaskNeg (0, 0, 10, 1, 6, 1, 123);
					
					if ( (Mode == MODE_EDIT) &&  (EDIT_IdField == IdField) ) {
						// LCD_PutSym (0, 15, 'M');
						if (EDIT_FieldGet) {
							LCD_PutStrNum (APD_ValStr, APD_ValCol, APD_ValLen, (uint8_t *) StrSpaces);
							// LCD_PutSym (0, 14, 'B');
						}
					} else {
						// Отчистка
						LCD_PutStrNum (APD_ValStr, APD_ValCol, APD_ValLen, (uint8_t *) StrSpaces);
					}

					/************************************************************************/
					/* -> РАБОТА С ЧИСЛОВЫМИ ФОРМАТАМИ                                      */
					/************************************************************************/
					/************************************************************************/
					/* ==>> ФОРМАТ: ДВОИЧНЫЙ                                                */
					/************************************************************************/
					if ((APD_ValFormat & 0xF0) == APD_FIELD_FORMAT_BIN) {
						if (DataOk) {

							// Сдвиг
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
					/* ==>> ФОРМАТ: ШЕСТНАДЦАТИРИЧНЫЙ                                       */
					/************************************************************************/
					} else if ((APD_ValFormat & 0xF0) == APD_FIELD_FORMAT_HEX) {
						if (DataOk) {

							// Сдвиг
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
					/* ==>> ФОРМАТ: ДЕСЯТИЧНЫЙ ЗНАКОВЫЙ БЕЗ НУЛЕЙ СЛЕВА                     */
					/************************************************************************/							
					} else if ((APD_ValFormat & 0xF0) == APD_FIELD_FORMAT_DEC_WITHOUT_NULLS) {
						if (DataOk) {
//							APD_ValResult = ( ( ( APD_Val * ( APD_ValMultiplier << 1 ) ) / APD_ValDivider ) >> 1 ) + APD_ValAddition;
							if((int64_t)APD_Val>0)
								APD_ValResult = (int64_t) ( round ( (double) APD_Val * ( (double) APD_ValMultiplier  / (double) APD_ValDivider ) ) ) + APD_ValAddition;
							else
								APD_ValResult = (int64_t) (((int32_t)APD_ValMultiplier*((int32_t)APD_Val/(int32_t)APD_ValDivider))+APD_ValAddition);// * ( (double) APD_ValMultiplier  / (double) APD_ValDivider ) ) ) + APD_ValAddition;
							

							
// 							if (APD_Val) {
// 								// Масштабирование значения регистра и округление
// 								APD_ValResult = ( ( ( ( ( APD_Val * APD_ValMultiplier ) * 10 ) / APD_ValDivider ) + 5 ) / 10 ) + APD_ValAddition;
// 							} else {	
// 								// Масштабирование значения регистра и округление
// 								APD_ValResult = ( - 1 ) * ( ( ( ( ( Abs (APD_Val) * APD_ValMultiplier ) * 10 ) / APD_ValDivider ) + 5 ) / 10 ) + APD_ValAddition;
// 							}
						
							if ( (Mode == MODE_EDIT) && (EDIT_IdField == IdField) ) {
								// LCD_PutSym (3, 4, 'C');
								if (EDIT_FieldGet) {
									EDIT_FieldGet = 0;
									// забрать параметры поля
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
									// забрать параметры поля
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
					/* ==>> ФОРМАТ: ДЕСЯТИЧНЫЙ ЗНАКОВЫЙ С НУЛЯМИ СЛЕВА                      */
					/************************************************************************/
					} else if ((APD_ValFormat & 0xF0) == APD_FIELD_FORMAT_DEC_WITH_NULLS) {
						if (DataOk) {
//							APD_ValResult = ( ( ( APD_Val * ( APD_ValMultiplier << 1 ) ) / APD_ValDivider ) >> 1 ) + APD_ValAddition;
							APD_ValResult = (int64_t) ( round ( (double) APD_Val * ( (double) APD_ValMultiplier  / (double) APD_ValDivider ) ) ) + APD_ValAddition;
							
// 							if (APD_Val) {
// 								// Масштабирование значения регистра и округление
// 								APD_ValResult = ( ( ( ( ( APD_Val * APD_ValMultiplier ) * 10 ) / APD_ValDivider ) + 5 ) / 10 ) + APD_ValAddition;
// 							} else {	
// 								// Масштабирование значения регистра и округление
// 								APD_ValResult = ( - 1 ) * ( ( ( ( ( Abs (APD_Val) * APD_ValMultiplier ) * 10 ) / APD_ValDivider ) + 5 ) / 10 ) + APD_ValAddition;
// 							}
							
							if ( (Mode == MODE_EDIT) && (EDIT_IdField == IdField) ) {
								// LCD_PutSym (0, 12, 'C');
								if (EDIT_FieldGet) {
									EDIT_FieldGet = 0;
									// забрать параметры поля
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
									// забрать параметры поля
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
			/* -> ТИП ПОЛЯ: ТЕКСТ													*/
			/************************************************************************/
			case APD_FIELD_TYPE_TEXT:
				
				APD_TextStr = pgm_read_byte ( APD_PrjData + APD_FrameFieldStartAdr[APD_FrameCur][IdField] + 1 ) - APD_FrameDisplayShift;
				if ((APD_TextStr >= 0) && (Abs (APD_TextStr) < (pgm_read_byte ( APD_PrjData + 8 )))) {
					
					APD_TextStr = Abs (APD_TextStr);
					APD_TextCol = pgm_read_byte ( APD_PrjData + APD_FrameFieldStartAdr[APD_FrameCur][IdField] + 2 );
					APD_TextLen = pgm_read_byte ( APD_PrjData + APD_FrameFieldStartAdr[APD_FrameCur][IdField] + 3 );	
					
					// Адрес регистра в преобразователе
					APD_TextPlcAdr = (pgm_read_byte ( APD_PrjData + APD_FrameFieldStartAdr[APD_FrameCur][IdField] + 4 )) | (pgm_read_byte ( APD_PrjData + APD_FrameFieldStartAdr[APD_FrameCur][IdField] + 5 ) << 8);
					++ APD_TextPlcAdr; // Добавляем 1, т.к. в APD_ адрес уменьшается на 1
					
					// Берём из APD_ минимальный и максимальный индексы текста
					APD_TextIdMin = pgm_read_byte ( APD_PrjData + APD_FrameFieldStartAdr[APD_FrameCur][IdField] + 7 );
					APD_TextIdMax = pgm_read_byte ( APD_PrjData + APD_FrameFieldStartAdr[APD_FrameCur][IdField] + 8 );
					// По адресу находим нужный регистр
					DataOk = 0;
					//DataOk = 1;
					for (Id = 0; Id <= (FOCUS_DataAmount - 1); Id++) {
						// Сравниваем адрес в преобразователе (текущего поля) с теми адресами, которые были
						// получены (вернее должны быть запрошены, и получены на них ответы) в текущем фокусе кадра
						if ((APD_TextPlcAdr) == FOCUS_DataAdr[Id]) {
							// Если таковой адрес имеется, то проверяем его статус
							IdActual = Id; // Сохранили Id
							if (FOCUS_DataStatus[Id]) {
								// Если со статусом всё в порядке,
								// то можно брать данные из регистра для дальнейшей обработки
								APD_TextId = FOCUS_Data[Id];
								
								DataOk = 1;
							} else {
								DataOk = 0;
							}
						}
					}
					// Вывод текста в зависимости от значения регистра и статуса
					if (((APD_TextId <= APD_TextIdMax) && (APD_TextId >= APD_TextIdMin)) && (DataOk)) {
						for (APD_TextSymbolId = 0; APD_TextSymbolId <= (APD_TextLen - 1); APD_TextSymbolId++) {
							// Если всё в порядке, рисуем текст из проекта APD_
							LCD_PutSym (APD_TextStr, APD_TextCol + APD_TextSymbolId, APD_PrjFontTable[pgm_read_byte ( APD_PrjData + APD_FrameFieldStartAdr[APD_FrameCur][IdField] + 9 + APD_TextSymbolId + APD_TextId*APD_TextLen )]);
						}
					} else {
						if (FOCUS_DataCntNewStatus[IdActual]) {
							for (APD_TextSymbolId = 0; APD_TextSymbolId <= (APD_TextLen - 1); APD_TextSymbolId++) {
								// Рисуем пробелы
								LCD_PutSym (APD_TextStr, APD_TextCol + APD_TextSymbolId, 0x20);
							}
						} else {
							for (APD_TextSymbolId = 0; APD_TextSymbolId <= (APD_TextLen - 1); APD_TextSymbolId++) {
								// Рисуем знаки '*'
								LCD_PutSym (APD_TextStr, APD_TextCol + APD_TextSymbolId, 0xFF);
							}
						}
					}
				}
				break;

			/************************************************************************/
			/* -> ТИП ПОЛЯ: ДАТА													*/
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
			/* -> ТИП ПОЛЯ: ВРЕМЯ													*/
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
			/* -> ТИП ПОЛЯ: СПИСОК													*/
			/************************************************************************/
			case APD_FIELD_TYPE_LIST:
				APD_ListStr = pgm_read_byte ( APD_PrjData + APD_FrameFieldStartAdr[APD_FrameCur][IdField] + 1 ) /* - APD_FrameDisplayShift */;
				// if ((APD_TextStr >= 0) && (Abs (APD_TextStr) < (APD_PrjData[8]))) {
					
				//	APD_TextStr = Abs (APD_TextStr);
				APD_ListCol = pgm_read_byte ( APD_PrjData + APD_FrameFieldStartAdr[APD_FrameCur][IdField] + 2 );
				APD_ListLen = pgm_read_byte ( APD_PrjData + APD_FrameFieldStartAdr[APD_FrameCur][IdField] + 3 );	
					
				// Адрес регистра в преобразователе
				APD_ListPlcAdr = (pgm_read_byte ( APD_PrjData + APD_FrameFieldStartAdr[APD_FrameCur][IdField] + 4 )) | (pgm_read_byte ( APD_PrjData + APD_FrameFieldStartAdr[APD_FrameCur][IdField] + 5 ) << 8);
				++ APD_ListPlcAdr; // Добавляем 1, т.к. в APD_ адрес уменьшается на 1
					
				// Берём из APD_ минимальный и максимальный индексы СПИСКА и высоту
				APD_ListIdMin = pgm_read_byte ( APD_PrjData + APD_FrameFieldStartAdr[APD_FrameCur][IdField] + 7 );
				APD_ListIdMax = pgm_read_byte ( APD_PrjData + APD_FrameFieldStartAdr[APD_FrameCur][IdField] + 8 );
				APD_ListHei = pgm_read_byte ( APD_PrjData + APD_FrameFieldStartAdr[APD_FrameCur][IdField] + 9 );
				
				DataOk = 0;
				for (APD_ListStrId = 0; APD_ListStrId <= (APD_ListHei - 1); APD_ListStrId++) {
					// По адресу находим нужный регистр
					for (Id = 0; Id <= (FOCUS_DataAmount - 1); Id++) {
						// Сравниваем адрес в преобразователе (текущего поля) с теми адресами, которые были
						// получены (вернее должны быть запрошены, и получены на них ответы) в текущем фокусе кадра
						// LCD_PutValDec (0, 0, 6, APD_ListPlcAdr + APD_List[APD_FrameCur][2] + APD_ListStrId);
						// LCD_PutValDec (0, 7, 6, FOCUS_DataAdr[0]);
						// LCD_PutValDec (1, 7, 6, FOCUS_DataAdr[1]);
						// LCD_PutValDec (2, 7, 6, FOCUS_DataAdr[2]);
						// LCD_PutValDec (3, 7, 6, FOCUS_DataAdr[1]);
						if ((APD_ListPlcAdr + APD_List[APD_FrameCur][2] + APD_ListStrId) == FOCUS_DataAdr[Id]) {
							// LCD_PutSym (2, 0, '*');
							// Если таковой адрес имеется, то проверяем его статус
							IdActual = Id; // Сохранили Id
							if (FOCUS_DataStatus[Id]) {
								// Если со статусом всё в порядке,
								// то можно брать данные из регистра для дальнейшей обработки
								APD_ListId = FOCUS_Data[Id];
								DataOk = 1;
							} else {
								DataOk = 0;
							}
						}
					}
					// DataOk = 1;
					// Вывод текста в зависимости от значения регистра и статуса
					if (((APD_ListId <= APD_ListIdMax) && (APD_ListId >= APD_ListIdMin)) && (DataOk)) {
						for (APD_ListSymbolId = 0; APD_ListSymbolId <= (APD_ListLen - 1); APD_ListSymbolId++) {
							// Если всё в порядке, рисуем текст из проекта APD_
							LCD_PutSym (APD_ListStr + APD_ListStrId, APD_ListCol + APD_ListSymbolId, APD_PrjFontTable[pgm_read_byte ( APD_PrjData + APD_FrameFieldStartAdr[APD_FrameCur][IdField] + 10 + APD_ListSymbolId + APD_ListId*APD_ListLen )]);
							// LCD_PutSym (2, 0, '*');
						}
					} else {
						if (FOCUS_DataCntNewStatus[IdActual]) {
							for (APD_ListSymbolId = 0; APD_ListSymbolId <= (APD_ListLen - 1); APD_ListSymbolId++) {
								// Рисуем пробелы
								LCD_PutSym (APD_ListStr + APD_ListStrId, APD_ListCol + APD_ListSymbolId, 0x20);
							}
						} else {
							for (APD_ListSymbolId = 0; APD_ListSymbolId <= (APD_ListLen - 1); APD_ListSymbolId++) {
								// Рисуем знаки '*'
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
/* -> ФУНКЦИИ ОТОБРАЖЕНИЯ ВРЕМЕНИ И ДАТЫ		*/
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
/* -> НАЧАЛО ПЕРЕДАЧИ СООБЩЕНИЯ											*/
/************************************************************************/
void ADP_TransStart (void) {
	//timeS1 = s_timeout;
	if ((usart_data_register_is_empty (&USARTC0))/* && (ADP_TransMesByteCnt == 0x0000)*/) {	
		ADP_flag = 0;			
		// Отсылка первого байта посылки
		//usart_put (&USARTC0, ADP_TransMesBuf[ADP_TransMesByteCnt]);
		//++ ADP_TransMesByteCnt; // Инкремент счётчика байт сообщения
		// Передача задачи отсылки комманды прерыванию TX
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
/* -> ПЕРЕДАЧА В ФОНОВОМ РЕЖИМЕ											*/
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
/* -> СБОРКА СООБЩЕНИЯ ДЛЯ ПЕРЕДАЧИ						*/
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
/* -> СБОРКА СООБЩЕНИЯ ДЛЯ ЗАПРОСА СПИСКА СОБЫТИЙ			*/
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
/* -> СБОРКА СООБЩЕНИЯ ДЛЯ ОТПРАВКИ РЕГИСТРА В ПЧ		*/
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
/* -> ОБРАБОТКА СООБЩЕНИЯ ОТ ПРЕОБРАЗОВАТЕЛЯ			*/
/************************************************************************/
/************************************************************************/
/* -> ПРИНЯТЬ ОТВЕТ НА ЗАПРОШЕННЫЕ ПАРАМЕТРЫ			*/
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
/* -> ПРИНЯТЬ ОТВЕТ НА ПЕРЕДАННЫЕ ПАРАМЕТРЫ				*/
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
/* -> ПРИНЯТЬ СПИСОК СОБЫТИЙ											*/
/************************************************************************/
void ADP_Function_ReqEvents (void) {
	uint8_t AdrFrom = ADP_RecMesBuf[1] - 0x50;
	uint8_t EventAmount =  ADP_RecMesBuf[6];
	uint8_t EventCode = 0;
		
	// Если событий в проекте нет, ничего не делаем
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
/* -> ЗАПИСЬ СОБЫТИЯ В ЖУРНАЛ											*/
/************************************************************************/
void REG_WriteEvent (uint8_t From, uint8_t EventCode) {
	// Буфер чтения из EEPROM 1
	uint8_t ReadBuff1[8] = {0, 0, 0, 0, 0, 0, 0, 0};
	// Буфер чтения из EEPROM 2
//	uint8_t ReadBuff2[8] = {0, 0, 0, 0, 0, 0, 0, 0};
	// Буфер записи в EEPROM 1
	uint8_t WriteBuff1[8] = {0, 0, 0, 0, 0, 0, 0, 0};
	// Буфер записи в EEPROM 2
//	uint8_t WriteBuff2[8] = {0, 0, 0, 0, 0, 0, 0, 0};
		
	uint16_t TotMes_ = 0; // Буфер для счётчика записей
	uint16_t NewMes_ = 0; // Буфер для счётчика новых записей

/************************************************************************/
/* -----> Читаем 2ой байт заголовка ЖУРНАЛА								*/
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
/* -----> Читаем счётчик записей в ЖУРНАЛЕ								*/
/************************************************************************/
	if (!(Crc8 ((uint8_t *) &ReadBuff1, REG_BLOCK_SIZE))) {
		TotMes_ = (ReadBuff1[6]<<8) | ReadBuff1[5];
		NewMes_ = (ReadBuff1[4]<<8) | ReadBuff1[3];
	}

/************************************************************************/
/* -----> ЗАХВАТ ВРЕМЕНИ СОБЫТИЯ										*/
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
							
	// Индекс текущего СОБЫТИЯ
	WriteBuff1[6] = EventCode;
	WriteBuff1[7] = Crc8 (WriteBuff1, REG_BLOCK_SIZE - 1);
	
	TotMes_ %= 9999;

/************************************************************************/
/* -----> ЗАПИСЫВАЕМ СОБЫТИЕ В ПАМЯТЬ ЖУРНАЛА							*/
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
/* -----> ОБНОВЛЯЕМ ЗАГОЛОВОК ЖУРНАЛА									*/
/************************************************************************/
/************************************************************************/
/* -----> Инкрементируем счётчик записей в ЖУРНАЛЕ						*/
/************************************************************************/
	++ TotMes_;
	++ NewMes_;
/************************************************************************/
/* -----> Готовим блок к записи											*/
/************************************************************************/	
	WriteBuff1[0] = ReadBuff1[0];
	WriteBuff1[1] = ReadBuff1[1];
	WriteBuff1[2] = ReadBuff1[2];
	WriteBuff1[3] = NewMes_; // Младший байт
	WriteBuff1[4] = NewMes_>>8; // Старший байт
	WriteBuff1[5] = TotMes_; // Младший байт
	WriteBuff1[6] = TotMes_>>8; // Старший байт
	WriteBuff1[7] = Crc8 (WriteBuff1, REG_BLOCK_SIZE - 1);

/************************************************************************/
/* -----> Записываем обновлённый 2ой байт заголовка ЖУРНАЛА				*/
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
/* -> НАСТРОЙКА ЖУРНАЛА СОБЫТИЙ											*/
/************************************************************************/
void REG_Init (void) {
	// Буфер чтения из EEPROM 1
	uint8_t ReadBuff1[8] = {0, 0, 0, 0, 0, 0, 0, 0};
	// Буфер чтения из EEPROM 2
	uint8_t ReadBuff2[8] = {0, 0, 0, 0, 0, 0, 0, 0};

/************************************************************************/
/* -----> ЧИТАЕМ ЗАГОЛОВОК ЖУРНАЛА										*/
/************************************************************************/
	// ПЕРВЫЙ БАЙТ
	EEPROM_AttemptCnt = EEPROM_RW_ATTEMPTS;
	cli();
	while (EEPROM_AttemptCnt) {
		if (EEPROM_ReadBlock ((REG_HEAD_BEGIN) * REG_BLOCK_SIZE, ReadBuff1, REG_BLOCK_SIZE)) break;
		-- EEPROM_AttemptCnt;
	}
	//E_TwiCmdStop ();
	//E_TwiDisable ();
	// ВТОРОЙ БАЙТ
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
		// СБРАСЫВАЕМ ЖУРНАЛ
		REG_Reset ();
	}
}

/************************************************************************/
/* -> РАБОТА С ЦАП														*/
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
/*			Настройка DMA		MODBUS_RS485 RMD		*/
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
/*			Настройка DMA		MODBUS_RS485			*/
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
/*				Настройка DMA		MODBUS_RS232			*/
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
/*					Настройка DMA		ROMBUS					*/
/************************************************************************/	
	DMA.CH1.DESTADDR0 = ((volatile)&(USARTC0.DATA));
	DMA.CH1.DESTADDR1 = ((volatile)&(USARTC0.DATA))>>8;
	DMA.CH1.DESTADDR2 = 0;
	DMA.CH1.ADDRCTRL = DMA_CH_SRCDIR_INC_gc | DMA_CH_DESTDIR_FIXED_gc;
	DMA.CH1.CTRLA = DMA_CH_BURSTLEN_1BYTE_gc | DMA_CH_SINGLE_bm; 
/************************************************************************/
/*				Настройка частоты и задержки старта кварца				*/
/************************************************************************/	
	OSC.XOSCCTRL |= OSC_FRQRANGE_12TO16_gc | OSC_XOSCSEL_XTAL_16KCLK_gc;
// Разрешение работы кварца
	OSC.CTRL |= OSC_XOSCEN_bm;
// Ожидание готовности кварца
	while ((OSC.STATUS & OSC_XOSCRDY_bm) == 0) {
	}
/*
	CCP = CCP_IOREG_gc;
//// Выбор кварца как источника тактов	
	CLK.CTRL = CLK_SCLKSEL_XOSC_gc;
	CCP = CCP_IOREG_gc;
//// Настройка предделитей
	CLK.PSCTRL = CLK_PSADIV_1_gc | CLK_PSBCDIV_1_1_gc;
	CCP = CCP_IOREG_gc;
//// Блокировка изменения настроек
	CLK.LOCK = CLK_LOCK_bm;
*/
	ccp_write_io ((uint16_t *)&CLK.CTRL, CLK_SCLKSEL_XOSC_gc);
	ccp_write_io ((uint16_t *)&CLK.PSCTRL, CLK_PSADIV_1_gc | CLK_PSBCDIV_1_1_gc);
	ccp_write_io ((uint16_t *)&CLK.LOCK, CLK_LOCK_bm);	
// Отключение всех генераторов, кроме кварца	
	OSC.CTRL &= OSC_XOSCEN_bm;
// Настройка PMIC
	pmic_init ();
	pmic_enable_level (PMIC_LVL_MEDIUM);
/************************************************************************/
/*						Включение тактирования модулей					*/
/************************************************************************/
	sysclk_enable_module (SYSCLK_PORT_C, SYSCLK_USART0);//ROMBUS
	sysclk_enable_module (SYSCLK_PORT_E, SYSCLK_USART0);//RMD
	sysclk_enable_module (SYSCLK_PORT_F, SYSCLK_USART0);//MODBUS RS_232
	sysclk_enable_module (SYSCLK_PORT_C, SYSCLK_USART1);//MODBUS RMD
	sysclk_enable_module (SYSCLK_PORT_E, SYSCLK_USART1);//MODBUS RS_485
	sysclk_enable_module (SYSCLK_PORT_C, SYSCLK_TWI);
	sysclk_enable_module (SYSCLK_PORT_E, SYSCLK_TWI);
	sysclk_enable_module (SYSCLK_PORT_C, SYSCLK_TC0);//таймер для 1 мс интервала
	sysclk_enable_module (SYSCLK_PORT_C, SYSCLK_TC1);//таймер для измерения интервала тишины ROMBUS
	sysclk_enable_module (SYSCLK_PORT_D, SYSCLK_TC0);//таймер для MODBUS
	sysclk_enable_module (SYSCLK_PORT_D, SYSCLK_TC1);//таймер для MODBUS
	sysclk_enable_module (SYSCLK_PORT_E, SYSCLK_TC1);//таймер для MODBUS		
	sysclk_enable_module (SYSCLK_PORT_E, SYSCLK_TC0);//таймер для обязательной отправки ответа по MODBUS
	sysclk_enable_module (SYSCLK_PORT_F, SYSCLK_TC0);//таймер для создания интервала тишины между сообщениями ROMBUS
	sysclk_enable_module (SYSCLK_PORT_B, SYSCLK_DAC);
/************************************************************************/
/*							Разбор проекта APD							*/
/************************************************************************/
// Проверка содержимого проекта APD_
if ((pgm_read_byte ( APD_PrjData + 0 ) == 0x41) && (pgm_read_byte ( APD_PrjData + 1 ) == 0x44)) {
	PRJ_Error = 0;
} else {
	PRJ_Error = 1;
}

uint8_t Check = 0;

/************************************************************************/
/* >> ВЫБОР ПЕРИОДА ОПРОСА КАДРОВ >>									*/
/************************************************************************/
Check = eeprom_read_byte ((uint8_t *) &SETTINGS_CyckleSaveFlag);
if (Check) {
	APD_FrameRefreshRate = eeprom_read_byte ((uint8_t *) &SETTINGS_CyckleSave)*100;
} else {
	if (pgm_read_byte ( APD_PrjData + 0x000B )) {
		APD_FrameRefreshRate = pgm_read_byte ( APD_PrjData + 0x000B )*100; // Период опроса кадров (0-авто,1-100мс,2-200,3...10)
	} else {
		APD_FrameRefreshRate = 2000;
	}
}
Check = 0;
TIMER_DataRequestPer = APD_FrameRefreshRate;
// TIMER_DataRequestPer = 200;

/************************************************************************/
/* >> ВЫБОР КРАТНОСТИ ОПРОСА СПИСКА СОБЫТИЙ >>	*/
/************************************************************************/
Check = eeprom_read_byte ((uint8_t *) &SETTINGS_RateSaveFlag);
if (Check) {
	APD_EventReqRatio = eeprom_read_byte ((uint8_t *) &SETTINGS_RateSave);
} else {
	if ( ( pgm_read_byte ( APD_PrjData + 0x000C ) > 0 ) && ( pgm_read_byte ( APD_PrjData + 0x000C ) < 15 ) ) {
		APD_EventReqRatio = pgm_read_byte ( APD_PrjData + 0x000C ); // Период опроса событий (0-авто,1-100мс,2-200,3...10)
	} else {
		APD_EventReqRatio = 15;
	}
}
Check = 0;
// APD_EventReqRatio %= 20;
ADP_EventReqRatioCnt = 1;
/************************************************************************/
/* >> ВЫБОР СКОРОСТИ РАБОТЫ RS-232 >>						*/
/************************************************************************/
 /*Выбор скорости обмена с ПЛК
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

// Количество кадров в проекте
APD_FrameAmount = pgm_read_byte ( APD_PrjData + 0x0010 );
// Номер стартового кадра
APD_FrameStartNum = pgm_read_byte ( APD_PrjData + 0x0011 );
APD_FrameCur = APD_FrameStartNum; // Текущий кадр = стартовый кадр проекта
APD_FramesStartAdr = (pgm_read_byte ( APD_PrjData + 0x0014 ) << 8) | (pgm_read_byte ( APD_PrjData + 0x0013 )); // Адрес описания кадров
APD_EventsStartAdr = (pgm_read_byte ( APD_PrjData + 0x0016 ) << 8) | (pgm_read_byte ( APD_PrjData + 0x0015 )); // Адрес описания списка событий
// Количество индексов списка событий
APD_EventIdAmount = (pgm_read_byte ( APD_PrjData + APD_EventsStartAdr + 1 )<< 8) | (pgm_read_byte ( APD_PrjData + APD_EventsStartAdr ));
// APD_EventIdAmount = 0;

if (APD_EventIdAmount) {	
	// Адрес начала списка событий в ПЛК
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
				// Разбор адресов абонентов,
				// у которых будут запрашиваться списки событий
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

// Заполнение Массива адресов описаний кадров:
// точка отсчёта адреса - адрес описания кадров. Далее адреса разбираются
// побайтно в соответствии с количеством кадров в проекте
for (IdFrame = 0; IdFrame <= (APD_FrameAmount - 1); IdFrame++) {
	APD_FrameStartAdr[IdFrame] |= pgm_read_byte ( APD_PrjData + APD_FramesStartAdr + Id8 ); // Младший байт
	Id8++;
	APD_FrameStartAdr[IdFrame] |= (pgm_read_byte ( APD_PrjData + APD_FramesStartAdr + Id8 ) << 8); // Старший байт
	Id8++;
}
Id8 = 0;

// Заполнение Массива описаний полей и действий в кадре:
// точка отсчёта адреса - адрес из Массива адресов описаний кадров. Далее адреса разбираются
// побайтно в соответствии с количеством кадров в проекте.
// В нулевой строке - адрес описания полей кадра, в первой - адрес описания
// кнопкодействий кадра.
for (IdFrame = 0; IdFrame <= (APD_FrameAmount - 1); IdFrame++) {
	// Адрес описания полей кадра
	APD_FrameFieldsAndActionsStartAdr[IdFrame][0] |= pgm_read_byte ( APD_PrjData + APD_FrameStartAdr[IdFrame] + Id8 ); // Младший байт
	Id8++;
	APD_FrameFieldsAndActionsStartAdr[IdFrame][0] |= (pgm_read_byte ( APD_PrjData + APD_FrameStartAdr[IdFrame] + Id8 ) << 8); // Старший байт
	Id8++;
	// Адрес описания кнопкодействий кадра
	APD_FrameFieldsAndActionsStartAdr[IdFrame][1] |= pgm_read_byte ( APD_PrjData + APD_FrameStartAdr[IdFrame] + Id8 ); // Младший байт
	Id8++;
	APD_FrameFieldsAndActionsStartAdr[IdFrame][1] |= (pgm_read_byte ( APD_PrjData + APD_FrameStartAdr[IdFrame] + Id8 ) << 8); // Старший байт
	Id8++;
	
	Id8 = 0;
}
// Заполняем массив количества ПОЛЕЙ и КНОПКОДЕЙСТВИЙ каждого кадра 
for (IdFrame = 0; IdFrame <= (APD_FrameAmount - 1); IdFrame++) {
	APD_FrameFieldsAmount[IdFrame] = pgm_read_byte ( APD_PrjData + APD_FrameFieldsAndActionsStartAdr[IdFrame][0] );
	APD_FrameActionsAmount[IdFrame] = pgm_read_byte ( APD_PrjData + APD_FrameFieldsAndActionsStartAdr[IdFrame][1] );
}	

Id8 = 0;
// Заполняем массив адресов ПОЛЕЙ каждого кадра
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
// Заполняем массив адресов КНОПКОДЕЙСТВИЙ каждого кадра
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
// Какие адреса запрашиваются из ПЛК в каждом кадре и сколько их?
for (IdFrame = 0; IdFrame <= (APD_FrameAmount - 1); IdFrame++) {
	if (APD_FrameFieldsAmount[IdFrame] > 1) {
		for (IdField = 0; IdField <= (APD_FrameFieldsAmount[IdFrame] - 1); IdField++) {
			// Если тип поля - ЧИСЛО или ТЕКСТ:
			if ((pgm_read_byte ( APD_PrjData + APD_FrameFieldStartAdr[IdFrame][IdField] ) == APD_FIELD_TYPE_VALUE) || (pgm_read_byte ( APD_PrjData + APD_FrameFieldStartAdr[IdFrame][IdField] ) == APD_FIELD_TYPE_TEXT)) {
				// Сохраняем его адрес в ПЛК в ячейку 16 бит:
				// Младший байт
				APD_FrameDataFromPlcAdr[IdFrame][APD_FrameDataFromPlcCnt] = pgm_read_byte ( APD_PrjData + APD_FrameFieldStartAdr[IdFrame][IdField] + 4 /* 4 + Id8 */ );
				// Старший байт
				APD_FrameDataFromPlcAdr[IdFrame][APD_FrameDataFromPlcCnt] |= (pgm_read_byte ( APD_PrjData + APD_FrameFieldStartAdr[IdFrame][IdField] + 5 /* 4 + Id8 */ ) << 8);
				// Исправляем фактический адрес в APD_ (добавляем 1)
				++ APD_FrameDataFromPlcAdr[IdFrame][APD_FrameDataFromPlcCnt];
				// Записываем строку отображения переменной на дисплее
				APD_FrameDataFromPlcStr[IdFrame][APD_FrameDataFromPlcCnt] = pgm_read_byte ( APD_PrjData + APD_FrameFieldStartAdr[IdFrame][IdField] + 1 );
				// Считаем количество полей типа ЧИСЛО и ТЕКСТ
				++ APD_FrameDataFromPlcCnt;
				//Если встретится 32-бит переменная, добавляем её в список как ещё одну 16-бит
				if ((pgm_read_byte ( APD_PrjData + APD_FrameFieldStartAdr[IdFrame][IdField]  + 6) == APD_FIELD_TYPE_32BIT))
				{
					APD_FrameDataFromPlcAdr[IdFrame][APD_FrameDataFromPlcCnt] = APD_FrameDataFromPlcAdr[IdFrame][APD_FrameDataFromPlcCnt-1]+1;

					//++ APD_FrameDataFromPlcAdr[IdFrame][APD_FrameDataFromPlcCnt];
					APD_FrameDataFromPlcStr[IdFrame][APD_FrameDataFromPlcCnt] = APD_FrameDataFromPlcStr[IdFrame][APD_FrameDataFromPlcCnt-1];
					++ APD_FrameDataFromPlcCnt;
				}
			} else if (pgm_read_byte ( APD_PrjData + APD_FrameFieldStartAdr[IdFrame][IdField] ) == APD_FIELD_TYPE_LIST) {
				/************************************************************************/
				/*   ЕСЛИ НАШЛИ ПОЛЕ ТИПА СПИСОК ---> ДОБАВЛЯЕМ В КОНЦЕ НУЖНЫЕ АДРЕСА	*/
				/************************************************************************/
				// Запоминаем метку, откуда начинаются адреса поля типа СПИСОК в данном КАДРЕ
				APD_List[IdFrame][0]	= APD_FrameDataFromPlcCnt;
				// Записываем высоту поля типа СПИСОК (фактически то, что будет видно на дисплее)
				APD_List[IdFrame][1] = pgm_read_byte ( APD_PrjData + APD_FrameFieldStartAdr[IdFrame][IdField] + 9 );
				// Сохраняем !НАЧАЛЬНЫЙ! адрес СПИСКА в специальный массив:
				// Младший байт
				APD_ListAdress[IdFrame] = pgm_read_byte ( APD_PrjData + APD_FrameFieldStartAdr[IdFrame][IdField] + 4 );
				// Старший байт
				APD_ListAdress[IdFrame] |= (pgm_read_byte ( APD_PrjData + APD_FrameFieldStartAdr[IdFrame][IdField] + 5 ) << 8);
				// Исправляем фактический адрес в APD_ (добавляем 1)
				++ APD_ListAdress[IdFrame];	
								
				/************************************************************************/
				/* -> Записываем адреса СПИСКА, которые будем потом запрашивать			*/
				/************************************************************************/
				for (uint8_t ListAdrId = 0; ListAdrId <= (pgm_read_byte ( APD_PrjData + APD_FrameFieldStartAdr[IdFrame][IdField] + 9 ) - 1); ++ ListAdrId) {
					// Сохраняем адрес в ячейку 16 бит:
					// Младший байт
					APD_FrameDataFromPlcAdr[IdFrame][APD_FrameDataFromPlcCnt] = pgm_read_byte ( APD_PrjData + APD_FrameFieldStartAdr[IdFrame][IdField] + 4 );
					// Старший байт
					APD_FrameDataFromPlcAdr[IdFrame][APD_FrameDataFromPlcCnt] |= (pgm_read_byte ( APD_PrjData + APD_FrameFieldStartAdr[IdFrame][IdField] + 5 ) << 8);
					// Адрес = [Текущая ячейка + Приращение адресов в СПИСКЕ (от 0 до [ВЫСОТА_СПИСКА - 1])					
					// Исправляем фактический адрес в APD_ (добавляем 1)
					APD_FrameDataFromPlcAdr[IdFrame][APD_FrameDataFromPlcCnt] += /* APD_List[IdFrame][2] */ + ListAdrId + 1;
					// Записываем строку отображения переменной на дисплее (чтобы список попадал в ФОКУС)
					APD_FrameDataFromPlcStr[IdFrame][APD_FrameDataFromPlcCnt] = pgm_read_byte ( APD_PrjData + APD_FrameFieldStartAdr[IdFrame][IdField] + 1 );
									
					++ APD_FrameDataFromPlcCnt;
				}
			}
		}
		// Записываем суммарное количество полей типа ЧИСЛО, ТЕКСТ или СПИСОК
		APD_FrameDataFromPlcAmount[IdFrame] = APD_FrameDataFromPlcCnt;
		APD_FrameDataFromPlcCnt = 0;
		// Id8 = 0;	
	} else {
		// Если ПОЛЕ в КАДРЕ только одно, значит это ПОЛЕ типа ЗАСТАВКА
		// Его не считаем
		APD_FrameDataFromPlcAmount[IdFrame] = 0;
	}
}

/************************************************************************/
/*							Настройка UARTE0			RMD_keyboard	*/
/************************************************************************/
// Настройка PIN3 на выход (передача)
	PORTE.DIR |= TX0;
//	PORTE.OUT |= TX0;
// Настройка PIN2 на вход (приём)
	PORTE.DIR &= ~RX0;
// Настройка скорости обмена
	usart_set_baudrate (&USARTE0, /*CONFIG_USART_BAUDRATE*/ 9600, /*BOARD_XOSC_HZ*/ 18432000);
//	USARTE0.BAUDCTRLA = 119;
// Выбор режима работы USARTE0
	usart_set_mode (&USARTE0, USART_CMODE_ASYNCHRONOUS_gc);
// Настройка формата посылки
	usart_format_set (&USARTE0, USART_CHSIZE_8BIT_gc, USART_PMODE_DISABLED_gc, false);
// Включение приёмника	
	usart_rx_enable (&USARTE0);
// Включение передатчика
	usart_tx_enable (&USARTE0);
// Уровни прерываний
	usart_set_rx_interrupt_level (&USARTE0, USART_RXCINTLVL_MED_gc);
/************************************************************************/
/*						Настройка UARTE1			RS485(MODBUS)		*/
/************************************************************************/
// Настройка PIN3 на выход (передача)
PORTE.DIR |= TX2;
PORTE.DIR |= TX_EN;	
PORTE.OUT &= ~TX_EN;
// Настройка PIN2 на вход (приём)
	PORTE.DIR &= ~ RX2;
// Настройка PIN1 на выход (TX_enable)
PORTE.OUT |= TX2;
// Настройка скорости обмена
	usart_set_baudrate (&USARTE1, /*CONFIG_USART_BAUDRATE */  38400 /* 9600 */, /*BOARD_XOSC_HZ*/ 18432000);
////	USARTE0.BAUDCTRLA = 119;
//// Выбор режима работы USARTE0
	usart_set_mode (&USARTE1, USART_CMODE_ASYNCHRONOUS_gc);
//// Настройка формата посылки
	usart_format_set (&USARTE1, USART_CHSIZE_8BIT_gc, USART_PMODE_DISABLED_gc, false);
//// Включение приёмника	
	usart_rx_enable (&USARTE1);
//// Включение передатчика
	usart_tx_enable (&USARTE1);
//// Уровни прерываний
	usart_set_rx_interrupt_level (&USARTE1, USART_RXCINTLVL_MED_gc);
	usart_set_tx_interrupt_level (&USARTE1,USART_TXCINTLVL_MED_gc);
/************************************************************************/
/*						Настройка UARTC0			ROMBUS				*/
/************************************************************************/
// Настройка PIN3 на выход (передача)
	PORTC.DIR |= TX0;
//	PORTC.OUT |= TX0;
// Настройка PIN2 на вход (приём)
	PORTC.DIR &= ~ RX0;
// Настройка скорости обмена
	usart_set_baudrate (&USARTC0, /*CONFIG_USART_BAUDRATE*/ SETTINGS_BaudValue[APD_BaudRateIndex], /*BOARD_XOSC_HZ*/ 18432000);
//	USARTE0.BAUDCTRLA = 119;
// Выбор режима работы USARTC0	
	usart_set_mode (&USARTC0, USART_CMODE_ASYNCHRONOUS_gc);
// Настройка формата посылки
	usart_format_set (&USARTC0, USART_CHSIZE_8BIT_gc, USART_PMODE_DISABLED_gc, false);
// Включение приёмника	
	usart_rx_enable (&USARTC0);
// Включение передатчика	
	usart_tx_enable (&USARTC0);
// Уровни прерываний
	usart_set_rx_interrupt_level (&USARTC0, USART_RXCINTLVL_MED_gc);

/************************************************************************/
/*							Настройка UARTF0		MODBUS				*/
/************************************************************************/
// Настройка PIN3 на выход (передача)
	PORTF.DIR |= TX0;
// Настройка PIN2 на вход (приём)
	PORTF.DIR &= ~ RX0;
// Настройка скорости обмена
	usart_set_baudrate (&USARTF0, 38400/*SETTINGS_BaudValue[APD_BaudRateIndex]*/, /*BOARD_XOSC_HZ*/ 18432000);
// Выбор режима работы USARTC0	
	usart_set_mode (&USARTF0, USART_CMODE_ASYNCHRONOUS_gc);
// Настройка формата посылки
	usart_format_set (&USARTF0, USART_CHSIZE_8BIT_gc, USART_PMODE_DISABLED_gc, false);
// Включение приёмника	
	usart_rx_enable (&USARTF0);
// Включение передатчика	
	usart_tx_enable (&USARTF0);
// Уровни прерываний
	usart_set_rx_interrupt_level (&USARTF0, USART_RXCINTLVL_MED_gc);
/************************************************************************/
/*							Настройка UARTC1		MODBUS_RMD			*/
/************************************************************************/
// Настройка PIN3 на выход (передача)

	PORTC.DIR |= TX2;
// Настройка PIN2 на вход (приём)
	PORTC.DIR &= ~ RX2;
// Настройка скорости обмена
	usart_set_baudrate (&USARTC1, 9600/*SETTINGS_BaudValue[APD_BaudRateIndex]*/, /*BOARD_XOSC_HZ*/ 18432000);
// Выбор режима работы USARTC0	
	usart_set_mode (&USARTC1, USART_CMODE_ASYNCHRONOUS_gc);
// Настройка формата посылки
	usart_format_set (&USARTC1, USART_CHSIZE_8BIT_gc, USART_PMODE_DISABLED_gc, false);
// Включение приёмника	
	usart_rx_enable (&USARTC1);
// Включение передатчика	
	usart_tx_enable (&USARTC1);
// Уровни прерываний
	usart_set_rx_interrupt_level (&USARTC1, USART_RXCINTLVL_MED_gc);
/************************************************************************/
/*						Загрузка уставок из EEPROM						*/
/************************************************************************/
LCD_Contrast = (uint8_t) eeprom_read_byte ((uint8_t *) &CONTRAST_Save);
TIMER_RequestTimeoutPer = (uint8_t) eeprom_read_byte ((uint8_t *) &SETTINGS_DelaySave);
EventStorage_TOP = (uint8_t) eeprom_read_byte ((uint8_t *) &EventStorage_TOP_Save);
EventStorage_BOT = (- 1) * (uint8_t) eeprom_read_byte ((uint8_t *) &EventStorage_BOT_Save);
/************************************************************************/
/*							Настройка дисплея							*/
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
/*							Настройка таймера							*/
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
/*							Настройка I2C (PORTC - RTC)					*/
/************************************************************************/
	C_TwiInit ();
/************************************************************************/
/*						Настройка часиков						*/
/************************************************************************/
	if (TIME_Get ()) {
		CLOCK_Error = 0;
	} else {
		CLOCK_Error = 1;
	}
/************************************************************************/
/*						Настройка I2C (PORTE - EEPROM)				*/
/************************************************************************/	
	E_TwiInit ();
/************************************************************************/
/* -> НАСТРОЙКА ЖУРНАЛА СОБЫТИЙ							*/
/************************************************************************/
	REG_Init ();
/************************************************************************/
/*						Разное						*/
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
		if (TIMER_RegSave >= TIMER_RegSavePer) { // Если сработал программный таймер
			TIMER_RegSave = 0;
			
			if (REGSAVE_MesCnt <= REGSAVE_MesLim) {

			/************************************************************************/
			/* ЧТЕНИЕ ЗАПИСИ														*/
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
					// Отсылка первого символа сообщения
					usart_put (&USARTE0, * (REGSAVE_Buff + REGSAVE_MesByteCnt/* - 1*/));
					++ REGSAVE_MesByteCnt; // Инкремент счётчика символов комманды
					// Передача задачи отсылки комманды прерыванию TX
					usart_set_tx_interrupt_level (&USARTE0, USART_TXCINTLVL_MED_gc);
					LCD_Refresh = 1;
				}
			} else {
				// Выключаем таймер выгрузки журнала
 				TIMER_RegSaveEn = 0;
 				TIMER_RegSave = 0;
				
				// Включаем обратно таймеры
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
		/*					Процедура конфигурации радиомодема MODBUS			*/
		/************************************************************************/
		if (TIMER_RMDCfgMBUS >= TIMER_RMDCfgPerMBUS) { // Если сработал программный таймер
			TIMER_RMDCfgMBUS = 0;
						
			switch (RMD_Config_STATEMBUS) 
			{
	// START - НАЧАЛЬНЫЕ УСЛОВИЯ КОНФИГУРАЦИИ
				case START:
					/*фрагмент для отключения конфигурации rmd*/
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
					
// CHECK - ПРОВЕРКА ОТВЕТА ОТ МОДЕМА
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
							RMD_CfgCmdAckMBUS = 1;	// Если получен ответ "ОК"
						} else {
							RMD_CfgCmdAckMBUS = 0;	// Если не получено ответа, или ответ не "ОК"
						}
					}
					RMD_Config_STATEMBUS = SEND;
					break;

// SEND - ОТСЫЛКА КОНФИГУРАЦИОННОГО СООБЩЕНИЯ
				case SEND:
					if (RMD_CfgCmdAckMBUS) {
						if (RMD_CfgCmdCntMBUS <= (RMD_CfgCmdAmountMBUS - 1)) {
							if ((usart_data_register_is_empty (&USARTC1)) && (RMD_CfgCmdCharCntMBUS == 0) && (RMD_CfgCmdAckMBUS)) {
								RMD_CfgCmdAckMBUS = 0;	// Сбрасывается флаг квитирования комманды
								// Отсылка первого символа комманды
								usart_put (&USARTC1, RMD_CfgMBUS[RMD_CfgCmdCntMBUS][RMD_CfgCmdCharCntMBUS + 1]);
								//if (Mode == MODE_INTRO) LCD_PutStrNum (3, 8, RMD_CfgCmdCntMBUS, (uint8_t *) "...................");
								//if (Mode == MODE_TRANSP_PROGRESS) LCD_PutStrNum (3, 0, RMD_CfgCmdCntMBUS, (uint8_t *) "...................");
							
								++ RMD_CfgCmdCharCntMBUS; // Инкремент счётчика символов комманды
								// Передача задачи отсылки комманды прерыванию TX
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

// END - ВЫХОД ИЗ КОНФИГУРАЦИИ	
				case END:
					RMDMBUS_RESET_OFF; // Макрос перевода модема в нормальный режим работы	

					TIMER_RMDCfgEn_MBUS = 0;
					TIMER_RMDCfgMBUS = 0;
			
					RMD_CfgCmdCntMBUS = 255;
					RMD_CfgCmdCharCntMBUS = 255;
					RMD_CfgCmdAckMBUS = 0;
				
					if (RMD_Config_ERRORMBUS) {
						RMD_Error = 1;
						Mode = MODE_ERROR_RMD /*MODE_DIAGNOSTIC*/; // Выбор текущего экрана для отображения
						LCD_Refresh = 1; // Разрешение на отображение
					
					} else {
						if (RMD_Config_FIRSTMBUS) {
							RMD_Config_FIRSTMBUS = 0;
						
							NET_Mode = NET_MODE_MASTER;
							RMD_RecEn = 1;
							usart_set_baudrate (&USARTC1, 38400, 18432000);
							LCD_Refresh = 1; // Разрешение на отображение
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
			//без радиомодема
			//TIMER_RMDCfgEn = 0;
			//TIMER_RMDCfg = 0;
			//RMD_CfgCmdCnt = 255;
			//RMD_CfgCmdCharCnt = 255;
			//RMD_CfgCmdAck = 0;
			//RMD_Error = 0;
			//Mode = MODE_NORMAL; // Выбор текущего экрана для отображения
			//NET_Mode = NET_MODE_MASTER;
			//TIMER_ClockRefreshEn = 1; // Запуск таймера обновления часов
//
			//// Разрешение на приём информационных сообщений:
			//// принятые сообщения интерпретируются как информационные
			//// в прерывании по RX (см. обработчик прерывания)
			//RMD_RecEn = 1;
			//DATA_ReqEn = 1;
			//EVENT_ReqEn = 1;
			//TIMER_DataRequest = 0; // Сброс таймера обмена с ПЧ
			//TIMER_DataRequestEn = 1; // Разрешение работы таймера обмена с ПЧ
			//LCD_Refresh = 1; // Разрешение на отображение
			//////////////////////////////////////////////////////////////////////////
		}
/************************************************************************/
/*					Процедура конфигурации стартового кадра				*/
/************************************************************************/
		if (TIMER_FIRSTSCfg >= TIMER_FIRSTSCCfgPer)
		{
			Q_Test_inc++;
			TIMER_FIRSTSCfg=0;
			/////проверка связи с 4 ПЧ для определения стартового экрана
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
				TIMER_DataRequest = 0; // Сброс таймера обмена с ПЧ
				TIMER_DataRequestEn = 1; // Разрешение работы таймера обмена с ПЧ
			}
		}
/************************************************************************/
/*				Процедура конфигурации радиомодема						*/
/************************************************************************/
		if (TIMER_RMDCfg >= TIMER_RMDCfgPer) { // Если сработал программный таймер
			TIMER_RMDCfg = 0;
						
			switch (RMD_Config_STATE) 
			{
	// START - НАЧАЛЬНЫЕ УСЛОВИЯ КОНФИГУРАЦИИ
				case START:
					NET_Mode = NET_MODE_CONFIG;
					/*фрагмент для отключения конфигурации rmd*/
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
					
// CHECK - ПРОВЕРКА ОТВЕТА ОТ МОДЕМА
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
							RMD_CfgCmdAck = 1;	// Если получен ответ "ОК"
						} else {
							RMD_CfgCmdAck = 0;	// Если не получено ответа, или ответ не "ОК"
						}
					}
					RMD_Config_STATE = SEND;
					break;

// SEND - ОТСЫЛКА КОНФИГУРАЦИОННОГО СООБЩЕНИЯ
				case SEND:
					if (RMD_CfgCmdAck) {
						if (RMD_CfgCmdCnt <= (RMD_CfgCmdAmount - 1)) {
							if ((usart_data_register_is_empty (&USARTE0)) && (RMD_CfgCmdCharCnt == 0) && (RMD_CfgCmdAck)) {
								RMD_CfgCmdAck = 0;	// Сбрасывается флаг квитирования комманды
								// Отсылка первого символа комманды
								usart_put (&USARTE0, RMD_Cfg[RMD_CfgCmdCnt][RMD_CfgCmdCharCnt + 1]);
								if (Mode == MODE_INTRO) LCD_PutStrNum (3, 8, RMD_CfgCmdCnt, (uint8_t *) "...................");
								if (Mode == MODE_TRANSP_PROGRESS) LCD_PutStrNum (3, 0, RMD_CfgCmdCnt, (uint8_t *) "...................");
							
								++ RMD_CfgCmdCharCnt; // Инкремент счётчика символов комманды
								// Передача задачи отсылки комманды прерыванию TX
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

// END - ВЫХОД ИЗ КОНФИГУРАЦИИ
				case END:
					TIMER_RMDCfgEn_MBUS = 1; // to config second RMD
					RMD_RESET_OFF; // Макрос перевода модема в нормальный режим работы	

					TIMER_RMDCfgEn = 0;
					TIMER_RMDCfg = 0;
			
					RMD_CfgCmdCnt = 255;
					RMD_CfgCmdCharCnt = 255;
					RMD_CfgCmdAck = 0;
				
					if (RMD_Config_ERROR) {
						RMD_Error = 1;
						Mode = MODE_ERROR_RMD /*MODE_DIAGNOSTIC*/; // Выбор текущего экрана для отображения
						LCD_Refresh = 1; // Разрешение на отображение
					
					} else {
						if (RMD_Config_FIRST) {
							RMD_Config_FIRST = 0;

							//NET_Mode = NET_MODE_MASTER;
							if (PRJ_Error) {
								Mode = MODE_ERROR_PRJ;
								TIMER_ClockRefreshEn = 0; // Душим часы

								// Ничего не разрешаем, так как проект APD дохлый
								RMD_RecEn = 0;
								DATA_ReqEn = 0;
								EVENT_ReqEn = 0;
								TIMER_DataRequest = 0; // Сброс таймера обмена с ПЧ
								TIMER_DataRequestEn = 0; // Разрешение работы таймера обмена с ПЧ
							} else {
								//Mode = /* MODE_INFO */ /*MODE_MENU*/ MODE_NORMAL; // Выбор текущего экрана для отображения
								TIMER_ClockRefreshEn = 1; // Запуск таймера обновления часов

								// Разрешение на приём информационных сообщений:
								// принятые сообщения интерпретируются как информационные
								// в прерывании по RX (см. обработчик прерывания)
								RMD_RecEn = 1;
								//DATA_ReqEn = 1;
								//EVENT_ReqEn = 1;
								//TIMER_DataRequest = 0; // Сброс таймера обмена с ПЧ
								//TIMER_DataRequestEn = 1; // Разрешение работы таймера обмена с ПЧ
								TIMER_FIRSTCfgEn=1;
							}
							
							LCD_Refresh = 1; // Разрешение на отображение
							
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
			//без радиомодема
			//TIMER_RMDCfgEn = 0;
			//TIMER_RMDCfg = 0;
			//RMD_CfgCmdCnt = 255;
			//RMD_CfgCmdCharCnt = 255;
			//RMD_CfgCmdAck = 0;
			//RMD_Error = 0;
			//Mode = MODE_NORMAL; // Выбор текущего экрана для отображения
			//NET_Mode = NET_MODE_MASTER;
			//TIMER_ClockRefreshEn = 1; // Запуск таймера обновления часов
//
			//// Разрешение на приём информационных сообщений:
			//// принятые сообщения интерпретируются как информационные
			//// в прерывании по RX (см. обработчик прерывания)
			//RMD_RecEn = 1;
			//DATA_ReqEn = 1;
			//EVENT_ReqEn = 1;
			//TIMER_DataRequest = 0; // Сброс таймера обмена с ПЧ
			//TIMER_DataRequestEn = 1; // Разрешение работы таймера обмена с ПЧ
			//LCD_Refresh = 1; // Разрешение на отображение
			//////////////////////////////////////////////////////////////////////////
		}
/************************************************************************/
/*               Блок отображения экранов дисплея                  */
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
/*                      Обмен данными с ПЧ                  	        */
/************************************************************************/			
		// Если сработал таймер на передачу
		if (TIMER_DataRequest >= TIMER_DataRequestPer) {
			//LCD_PutValDec(0,0,4,ADP_mis_Error);//счетчик необработаных ответов телеграмм чтения по ROMBUS
			//LCD_PutValDec(1,0,4,MB0_State.Error_CRC);//ошибка CRC телеграммы, принятой по MODBUS
			//LCD_PutValDec(2,0,4,MB0_State.Error_Data);//ошибка диапазона данных или адреса данных по MODBUS
			//LCD_PutValDec(3,0,4,MB0_State.Error_MBUS);//ошибка, если во время не отправлена телеграмма по MODBUS (превышено время ожидания 200 мс)
			//LCD_PutValDec(0,4,4,ADP_Rec_CRC_Error);//ошибка CRC телеграммы, принятой по ROMBUS
			//LCD_PutValDec(1,4,4,MB0_State.SubNum);//не ошибка :-)

/************************************************************************/
/*                      НАЧАЛО СЕССИИ ПЕРЕДАЧИ                          */
/************************************************************************/
			// Перезапускаем таймер
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
						ADP_MBUS_TransMesBody[0] = 0x0002;	//2 байта по протоколу ADP
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
/*                              ПРОВЕРКА СВЯЗИ                          */
/************************************************************************/
				if (FOCUS_DataAmount) {
						
					LCD_RefreshScore = 0;
				
					for (uint8_t Id = 0; Id <= /*APD_MAX_FIELDS*/ (FOCUS_DataAmount - 1); Id++) {
						/*
						* Мониторинг связи:
						* если не получен ответ более, чем BROKEN_MES раз,
						* то связь прерывается, данные не актуальны
						* рисуем звёзды
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
							* Можно ли рисовать пробелы?
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
				// Произошло что-то, достойное обновления экрана
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
// 				/* -> МОНИТОРИНГ ЗАПРАШИВАЕМЫХ СОБЫТИЙ   */
// 				/************************************************************************/
// 				for (uint8_t IdFrom = 0; IdFrom <= (EVENT_FROM_MAX - 1); ++ IdFrom) {
// 					for (uint8_t IdCode = 0; IdCode <= (EVENT_CODES_MAX - 1); ++ IdCode) {
// 				/************************************************************************/
 				/* КАК РАБОТАЕТ СЧЁТЧИК?
 				 1. Счётчик постоянно инкрементируется новыми полученными сообщениями;
 				 2. Если сообщение пришло 2 раза подряд (счётчик == 2), то оно
 				    записывается в журнал, и счётчик сбрасывается в -1;
 				 3. При каждой отправке запроса счётчик декрементируется до значения
 				-4. После этого опять сбрасывается в 0; */
 				/************************************************************************/
// 						// Если сообщение уже записано в журнал, то уменьшаем счётчик
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
					// Собираем тело сообщения
					ADP_TransMesBody[0] = BLOCK_Request[ADP_TransMesQueue - 1][1]; // Сколько байт выдать?
					ADP_TransMesBody[1] = BLOCK_Request[ADP_TransMesQueue - 1][0] % 1000; // С какого адреса?
					// (Убираем предварительно тысячи)
					// Если длина сообщения не нулевая, собираем сообщение для отправки
					if (ADP_TransMesLen = ADP_TransMesBuild (0x50 + (BLOCK_Request[ADP_TransMesQueue - 1][0] / 1000) /* ADP_TR1 */, ADP_OPK, ADP_FUNC_TRANS_PAR, ADP_NULL, 0x0004, (uint16_t *)ADP_TransMesBody)) {
						// Начинаем передачу очередного сообщения в очереди
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
								// Начинаем передачу очередного сообщения в очереди
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
/*                      Запрос списка событий из ПЧ            	      */
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
					/* -> МОНИТОРИНГ ЗАПРАШИВАЕМЫХ СОБЫТИЙ                                  */
					/************************************************************************/
					for (uint8_t IdFrom = 0; IdFrom <= (EVENT_FROM_MAX - 1); ++ IdFrom) {
						for (uint8_t IdCode = 0; IdCode <= (EVENT_CODES_MAX - 1); ++ IdCode) {
					/************************************************************************/
					/* КАК РАБОТАЕТ СЧЁТЧИК?                                                */
					/* 1. Счётчик постоянно инкрементируется новыми полученными сообщениями;*/
					/* 2. Если сообщение пришло 2 раза подряд (счётчик == 2), то оно        */
					/*    записывается в журнал, и счётчик сбрасывается в -1;               */
					/* 3. При каждой отправке запроса счётчик декрементируется до значения  */
					/*    -4. После этого опять сбрасывается в 0;                           */
					/************************************************************************/

							// if ( EventStorage[ IdFrom ][ IdCode ] > 0 ) ++ EventStorage[ IdFrom ][ IdCode ];
							
							if ( EventStorage[ IdFrom ][ IdCode ] < 0 ) {
								
								++ EventStorage[ IdFrom ][ IdCode ];
								
								if ( EventStorage[ IdFrom ][ IdCode ] == 0 ) {
									EventStorage[ IdFrom ][ IdCode ] = EventStorage_TOP;
								}
							}
							// Если сообщение уже записано в журнал, то уменьшаем счётчик
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
		/*               ДЕЙСТВИЕ ПОСЛЕ ПАУЗЫ МЕЖДУ СООБЩЕНИЯМИ*/
		/************************************************************************/
		if (TIMER_RequestTimeout >= TIMER_RequestTimeoutPer) {
			TIMER_RequestTimeout = 0;
			TIMER_RequestTimeoutEn = 0;
			
			switch (TRANS_STATUS) {
				case DATA:
					if (ADP_TransMesQueue) {

						ADP_TransMesBody[0] = BLOCK_Request[ADP_TransMesQueue - 1][1]; // Сколько байт выдать?
						ADP_TransMesBody[1] = BLOCK_Request[ADP_TransMesQueue - 1][0] % 1000; // С какого адреса?
						// (Убираем предварительно тысячи) 
						// Если длина сообщения не нулевая, собираем сообщение для отправки
						if (ADP_TransMesLen = ADP_TransMesBuild (0x50 + (BLOCK_Request[ADP_TransMesQueue - 1][0] / 1000) /* ADP_TR1 */, ADP_OPK, ADP_FUNC_TRANS_PAR, ADP_NULL, 0x0004, (uint16_t *)ADP_TransMesBody)) {
							// Начинаем передачу очередного сообщения в очереди
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
									// Начинаем передачу очередного сообщения в очереди
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
/*       Отображение часов с периодом таймера опроса RTC*/
/************************************************************************/	
// Отображение часов с периодом таймера (работает независимо от режима, управляется флагом ClockShowEn)
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
/*       Проверка принятого сообщения на наличие ошибок (Crc-8)*/
/************************************************************************/
		if (RMD_RecMesReady) {
			RMD_RecMesReady = 0; // Новое сообщение обработано
			if (!(Crc8 ((uint8_t *) &RMD_RecMesBuf, RMD_RecMesBufLen))) {
				RMD_RecMesCrcOk = 1; // Если ошибок Crc-8 не обнаружено
				
				// Вывести коды принятых кнопок
				// LCD_PutValDec (0, 0, 3, RMD_RecMesBuf[1]);
				// LCD_PutValDec (1, 0, 3, RMD_RecMesBuf[2]);
				
				//if (Mode == MODE_INFO) {
					//LCD_Refresh = 1;
				//}
			}
		}
/************************************************************************/
/*Проверка принятого сообщения на наличие ошибок (Crc-16)*/
/************************************************************************/
		if (ADP_RecMesReady) {
			ADP_CRC = Crc16 ((uint8_t *) &ADP_Buf, ADP_RecMesLen);
			ADP_RecMesReady = 0; // Новое сообщение обработано
			ADP_RecMesLen = 0;
			if (!ADP_CRC)
			{
				if (MBUS_num)
				{
					ADP_mis_Error++;
				}
				ADP_RecMesCrcOk = 1; // Если ошибок Crc-16 не обнаружено
				MBUS_num = 1;
			}
			else{ADP_Rec_CRC_Error++;
			}
		}
		
		if (MB0_State.TelCompleet) // Если приём нового сообщения завершён
		{
			MB0_State.TelCompleet = 0; 
			//Сравниваем принятый CRC с расчетным
			MB0_State.CRC16_OK = Crc16 ((uint8_t *)MBUS_Buf, (MB0_State.lenRx)-2);
			MB0_State.CRC16 = ((MBUS_Buf[MB0_State.lenRx-1]<<8)|MBUS_Buf[MB0_State.lenRx-2]);
			if (MB0_State.CRC16_OK == MB0_State.CRC16) 
			{
				MB0_State.CRC16_OK = 1; // Если ошибок Crc-16 не обнаружено
			}	
			else //иначе посылка считается не принятой и длина обнуляется
			{
				MB0_State.Error_CRC++;
				MB0_State.CRC16_OK = 0;
				//LCD_PutValDec(0,0,4,MB0_State.Error_CRC);
			}
			MB0_State.lenRx = 0;
		}
/************************************************************************/
/*     Блок обработки сообщений от преобразователей */
/************************************************************************/	
		if (ADP_RecMesCrcOk) {
			ADP_RecMesCrcOk = 0;
			 //Разбор принятой телеграммы
			// Кто получатель?
			if (ADP_Buf[0] == ADP_OPK) {
				MBUS_num = 0;	
				// Кто оправитель?
				if ((ADP_Buf[1] == ADP_TR0) || (ADP_Buf[1] == ADP_TR1) || (ADP_Buf[1] == ADP_TR2) || (ADP_Buf[1] == ADP_TR3) || (ADP_RecMesBuf[1] == ADP_TR4)) {
					// Код команды?
					//if (s_timeout<s_delta){s_timeout = }
					if (ADP_Buf[2] == (ADP_FUNC_TRANS_PAR | (1<<7) )) {// Флаг ответа 
						ADP_Function_TransParams ();
						if (Mode == MODE_INFO) {
							LCD_Refresh = 1;
						}
					} else if (ADP_Buf[2] == (ADP_FUNC_EVENT_REQ | (1<<7) )) {// Флаг ответа 
						ADP_Function_ReqEvents ();
						if (Mode == MODE_INFO) {
							LCD_Refresh = 1;
						}
					} else if (ADP_Buf[2] == (ADP_FUNC_REC_PAR | (1<<7) )) {// Флаг ответа 
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
						TIMER_DataRequest = 0; // Сброс таймера обмена с ПЧ
						TIMER_DataRequestEn = 1; // Разрешение работы таймера обмена с ПЧ
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
						//TIMER_DataRequest = 0; // Сброс таймера обмена с ПЧ
						//TIMER_DataRequestEn = 1; // Разрешение работы таймера обмена с ПЧ
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
					for (i=0;i<ADP_MBUS_Q/2;i++)//цикл по пришедшему по ADP сообщению
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
					for (Reg2Reg=0;Reg2Reg<50;Reg2Reg++)	/// поиск адреса модбас по ответу ADP
					{
						if (RegAdr == Redef_Reg[Reg2Reg]){break;}
					}
					
					MBUS_Register[Reg2Reg] = ADP_RecMesBuf[10]|(ADP_RecMesBuf[11]<<8);
					
					if (MB0_State.Quant>ADP_MBUS_Counter)//если есть ещё регистры для чтения, то посылаем запрос
					{
						ADP_MBUS_TransMesBody[0] = 0x0002;	//2 байта по протоколу ADP
						//ADP_MBUS_TransMesBody[1] = ((Redef_Reg[MB0_State.Sub_AddressL+ADP_MBUS_Counter]<<8)|(Redef_Reg[MB0_State.Sub_AddressL+ADP_MBUS_Counter]>>8));//если адреса в таблице записаны как есть, то нужна такая перестановка 
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
					else// if (MB0_State.Quant<=2*ADP_MBUS_Counter)//по ADP всё прочитали, можно отпрвалять ответ по MODBUS
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
		//начинаем распаковку принятой посылки MODBUS
		if (MB0_State.CRC16_OK)
		{
			MB0_State.CRC16_OK = 0;
			//MB0_State.flags  &= ~ANSWER;	//сброс флага ответа
				// Разбор принятой телеграммы
				// Кто получатель?
			MB0_State.address = 	MBUS_Buf[0];
			MB0_State.command =  	MBUS_Buf[1];
			MB0_State.Sub_AddressH = MBUS_Buf[2];//для выбора ПЧ
			MB0_State.Sub_AddressL = MBUS_Buf[3];
			MB0_State.SubNum = MB0_State.Sub_AddressH - 0x50;
			//MB0_State.Buf_byte = 0;
			if (MB0_State.address == MBUS_OPK) 
			{
				//Redef_Reg[MB0_State.Sub_AddressL] - адрес запрашиваемого регистра
				switch(MB0_State.command)
				{
					//case MBUS_FUNC_GETREG://читать один регистр
						////обработка ошибок, если есть
						//MB0_State.Quant = MBUS_Buf[5];//((MB0_State.UART_RxBuf[4]<<8)|MB0_State.UART_RxBuf[5]);
						//if (MB0_State.Sub_AddressL>MAX_QUANT_REG)//код ошибки 02 ILLEGAL DATA ADDRESS
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
						//if (((MB0_State.Sub_AddressH>0x55) || (MB0_State.Sub_AddressH<0x50))&&(MB0_State.Sub_AddressH!=SPEED_MBUS))//код ошибки 02 ILLEGAL DATA ADDRESS
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
					case MBUS_FUNC_GETREGS://читать регистры
						MB0_State.Quant = MBUS_Buf[5];//((MB0_State.UART_RxBuf[4]<<8)|MB0_State.UART_RxBuf[5]);
						if ((MB0_State.Sub_AddressL+MB0_State.Quant)>MAX_QUANT_REG)// && MB0_State.Quant)//код ошибки 02 ILLEGAL DATA ADDRESS
						{
							MB0_State.Error_Data++;
							MBUS_Error_Trans(ILLEGAL_ADR);
							break;
						}
						if (((MB0_State.Sub_AddressH>0x55) || (MB0_State.Sub_AddressH<0x50))&&(MB0_State.Sub_AddressH!=SPEED_MBUS))//код ошибки 02 ILLEGAL DATA ADDRESS
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
								//ADP_MBUS_TransMesBody[0] = 0x001F;	//31 байт по протоколу ADP
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
								ADP_MBUS_TransMesBody[0] = 0x0064;	//50 байт по протоколу ADP
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
						//Сортировка массива
						//////
							//MBUS_ADP = (uint16_t*)malloc(MB0_State.Quant);
							//numADPmes = (uint16_t*)malloc(6);
							for(i=0;i<MB0_State.Quant;i++)
							{
								MBUS_ADP[i] = Redef_Reg[i+MB0_State.Sub_AddressL];
							}
							//для получения единственного регистра, телеграмма формируется отдельно	
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
							//заполнили массив нужных адресов и сортируем их
								qsort((uint16_t*)MBUS_ADP,MB0_State.Quant,sizeof(uint16_t),(uint16_t(*) (const uint16_t *, const uint16_t *)) comp);
							//в отсортированом массиве ищем пропуски в адресации >14 для разбиения сообщений ADP
								ADP_NextStart[nu++] = MBUS_ADP[0];//nu опасный индекс! Внимательнее!
								ADP_MBUS_Counter[nu-1]+=2;
								for(i=0;i<MB0_State.Quant-1;++i)
								{
									delta = abs(MBUS_ADP[i]-MBUS_ADP[i+1]);
									if (delta>13)//Если большой разрыв между адресами, готовим новое сообщение
									{
										numADPmes++;//Увеличиваем количество сообщений по ADP
										ADP_NextStart[nu++] = MBUS_ADP[i+1];//выставляем новый адрес для новой посылки
										ADP_MBUS_Counter[nu-1]+=2;
									}
									else
									{
										ADP_MBUS_Counter[nu-1]+=delta*2;//увеличиваем число байт в одном сообщении ADP
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
							//для одного регистра
							//ADP_MBUS_TransMesBody[0] = 0x0002;	//2 байта по протоколу ADP
							//ADP_MBUS_TransMesBody[1] = (Redef_Reg[MB0_State.Sub_AddressL]>>8+Redef_Reg[MB0_State.Sub_AddressL]<<8);//если адреса в таблице записаны как есть, то нужна такая перестановка 
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
						//если долго нет ответа по ADP, генерируем ошибку ожидания 05
					break;
					case MBUS_FUNC_SETREG://записать один регистр
						if ((MB0_State.Sub_AddressL)>MAX_QUANT_REG)//код ошибки 02 ILLEGAL DATA ADDRESS
						{
							MB0_State.Error_Data++;
							MBUS_Error_Trans(ILLEGAL_ADR);
							break;
						}
						if (((MB0_State.Sub_AddressH>0x55) || (MB0_State.Sub_AddressH<0x50)) && (MB0_State.Sub_AddressH!=SPEED_MBUS))//код ошибки 02 ILLEGAL DATA ADDRESS
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
						//if (MB0_State.Sub_AddressL>7)//запрет записи в регистры старше 7-го
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
							ADP_MBUS_TransMesBody[0] = 0x0002;	//2 байта по протоколу ADP
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
					case MBUS_FUNC_SETREGS://записать несклько регистров
						MB0_State.Quant = MBUS_Buf[5];
						if ((MB0_State.Sub_AddressL+MB0_State.Quant)>MAX_QUANT_REG)//код ошибки 02 ILLEGAL DATA ADDRESS
						{
							MB0_State.Error_Data++;
							MBUS_Error_Trans(ILLEGAL_ADR);
							break;
						}
						if (((MB0_State.Sub_AddressH>0x55) || (MB0_State.Sub_AddressH<0x50)) && (MB0_State.Sub_AddressH!=SPEED_MBUS))//код ошибки 02 ILLEGAL DATA ADDRESS
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
						//if (MB0_State.Sub_AddressL>7)//запрет записи в регистры старше 7-го
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
							ADP_MBUS_TransMesBody[0] = 0x0002;	//2 байта по протоколу ADP
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
		}			// обнуление буфера принятого сообщения
			/*
			for (uint16_t Id = 0; Id <= (ADP_RecMesLen - 1); Id++) {
				ADP_RecMesBuf[Id] = 0;
			}
			ADP_RecMesLen = 0;
			*/
/************************************************************************/
/*       Блок обработки информации с пульта о нажатых кнопках */
/************************************************************************/
		// Обработка нажатий вне зависимости от режима (поиск условия перехода в сервисный режим)
		if (RMD_RecMesCrcOk) 
		{
			RMD_RecMesCrcOk = 0; // Подтверждение безошибочности сообщения обработано
			// Борьба с залипанием
			if ((RMD_RecMesBuf[1] != BUTTON_First) || (RMD_RecMesBuf[2] != BUTTON_Second)) {
				BUTTON_Release = 1;
			}			
			// Проверка нажатых кнопок с предыдущими зафиксированными нажатиями
			if (((RMD_RecMesBuf[1] == BUTTON_First) && (BUTTON_First == 5)) && ((RMD_RecMesBuf[2] == BUTTON_Second) && (BUTTON_Second == 8))) {
				// Запуск счётчика удержания комбинации кнопок
				if (SERVICE_ToggleCnt < SERVICE_ToggleCntLim) {
					++ SERVICE_ToggleCnt;
				} else {
					// Если комбинация кнопок удерживается в течение периода,
					// определяемого SERVICE_ToggleCntLim,
					// происходит смена режима работы
					SERVICE_ToggleCnt = 0; // Если период выдержан, сброс счётчика
					if (Mode == MODE_MENU) {

						Mode = MODE_NORMAL;
						BUTTON_Release = 0;
						
						DATA_ReqEn = 1;
						SplashRefreshEn = 1; // Разрешение отрисовки ЗАСТАВКИ
						
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
					LCD_Refresh = 1; // Разрешение на отображение
				}
			} else {
				SERVICE_ToggleCnt = 0; // Если период не выдержан, все-равно сброс счётчика	
			}
			if (((RMD_RecMesBuf[1] == BUTTON_First) && (BUTTON_First == 5)) && ((RMD_RecMesBuf[2] == BUTTON_Second) && (BUTTON_Second == 7))) {
				// Запуск счётчика удержания комбинации кнопок
				if (TRANSP_ToggleCnt < TRANSP_ToggleCntLim) {
					++ TRANSP_ToggleCnt;
				} else {
					// Если комбинация кнопок удерживается в течение периода,
					// определяемого TRANSP_ToggleCntLim,
					// происходит смена режима работы
					TRANSP_ToggleCnt = 0; // Если период выдержан, сброс счётчика
					if (Mode == MODE_MENU) {
						ClockShowEn = 0;
						DataShowEn = 0;

						Mode = MODE_TRANSP_SELECT;
						BUTTON_Release = 0;
					}
					LCD_Refresh = 1; // Разрешение на отображение
				}
			} else {
				TRANSP_ToggleCnt = 0; // Если период не выдержан, все-равно сброс счётчика
			}			
			// Запись текущих значений кнопок в буфер
			BUTTON_First = RMD_RecMesBuf[1];
			BUTTON_Second = RMD_RecMesBuf[2];	
			
			// Обработка нажатий в зависимости от режима
			if (BUTTON_Release) {
				BUTTON_Release = 0;
				switch (Mode) {
/************************************************************************/
/* >>> РЕЖИМ "ОБЫЧНЫЙ" >>>                                              */
/************************************************************************/
					case MODE_NORMAL: // Обычный режим
						APD_FrameButtonAction = APD_FrameButtonActionTable[BUTTON_Second][BUTTON_First];
						SendToPlc_One = 1;
						SCREEN_ACTION (APD_FrameButtonAction);
						APD_FrameButtonAction = 255;
						break;
/************************************************************************/
/* >>> РЕЖИМ "МЕНЮ"														*/
/************************************************************************/
					case MODE_MENU: // МЕНЮ
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
						// ВНИЗ
						} else if ((BUTTON_First == 5) && (BUTTON_Second == 7)) {
							if (MenuShift < (MenuMax - 4)) {
								++ MenuShift;
								LCD_Refresh = 1;
							}
						// ВВЕРХ
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
/* >>> РЕЖИМ "РАЗАРБОТКА" >>>                                           */
/************************************************************************/
					case MODE_INFO: // Режим разработчика
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
/* >>> РЕЖИМ "УСТАНОВКА ЧАСОВ" >>>                                      */
/************************************************************************/
					case MODE_TIME: // Режим настройки времени
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
						// (5) ВВЕРХ	
						} else if ((BUTTON_First == 5) && (BUTTON_Second == 11)) {
							Action = 5;
							LCD_Refresh = 1;
						// (6) ВНИЗ	
						} else if ((BUTTON_First == 5) && (BUTTON_Second == 7)) {
							Action = 6;
							LCD_Refresh = 1;
						// (7) ВЛЕВО	
						} else if ((BUTTON_First == 2) && (BUTTON_Second == 5)) {
							Action = 7;
							LCD_Refresh = 1;
						// (8) ВПРАВО	
						} else if ((BUTTON_First == 4) && (BUTTON_Second == 5)) {
							Action = 8;
							LCD_Refresh = 1;
						}
						break;
						
/************************************************************************/
/* >>> РЕЖИМ "ДИАГНОСТИКА" >>>                                          */
/************************************************************************/
					case MODE_DIAGNOSTIC: // Режим диагностики ошибок
						if ((BUTTON_First == 7) && (BUTTON_Second == 0)) {
							Mode = MODE_MENU;
							ClockShowEn = 1;
							TIMER_ClockRefreshEn = 1;
							LCD_Refresh = 1;
						}
						break;
/************************************************************************/
/* >>> РЕЖИМ "ЖУРНАЛ: МЕНЮ" >>>                                         */
/************************************************************************/
					case MODE_EVENTS_MENU: // Режим отображения журнала событий	
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
						// ВНИЗ
						} else if ((BUTTON_First == 5) && (BUTTON_Second == 7)) {
							if (RegMenuShift < (RegMenuMax - 4)) {
								++ RegMenuShift;
								LCD_Refresh = 1;
							}
						// ВВЕРХ
						} else if ((BUTTON_First == 5) && (BUTTON_Second == 11)) {
							if (RegMenuShift) {
								-- RegMenuShift;
								LCD_Refresh = 1;
							}
						}
						break;
/************************************************************************/
/* >>> РЕЖИМ "ЖУРНАЛ: ОТОБРАЖЕНИЕ СОБЫТИЙ" >>>                          */
/************************************************************************/
					case MODE_EVENTS_LIST: // Режим отображения журнала событий	
						// (8) ВПРАВО - ВПЕРЁД +1
						if ((BUTTON_First == 4) && (BUTTON_Second == 5)) {
							Action = 8;
							LCD_Refresh = 1;
						// (5) ВВЕРХ - ВПЕРЁД +10	
						} else if ((BUTTON_First == 5) && (BUTTON_Second == 11)) {
							Action = 5;
							LCD_Refresh = 1;
						// (7) ВЛЕВО - НАЗАД -1
						} else if ((BUTTON_First == 2) && (BUTTON_Second == 5)) {
							Action = 7;
							LCD_Refresh = 1;
						// (6) ВНИЗ - НАЗАД -10	
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
/* >>> РЕЖИМ "ЖУРНАЛ: ИНФО" >>>                                         */
/************************************************************************/
					case MODE_EVENTS_INFO: // Режим настройки ИНФО ЖУРНАЛА
						// (0) BACK
						if ((BUTTON_First == 7) && (BUTTON_Second == 0)) {
							Mode = MODE_EVENTS_MENU;
							LCD_Refresh = 1;
						}
						break;
/************************************************************************/
/* >>> РЕЖИМ "ЖУРНАЛ: СБРОС" >>>                                        */
/************************************************************************/
					case MODE_EVENTS_CLEAR: // Режим настройки КОНТРАСТА
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
/* >>> РЕЖИМ "НАСТРОЙКИ СЕТИ" >>>                                       */
/************************************************************************/
					case MODE_SETTINGS: // Режим настройки
						// (7) ВЛЕВО
						if ((BUTTON_First == 2) && (BUTTON_Second == 5)) {
							Action = 7;
							LCD_Refresh = 1;
						// (8) ВПРАВО	
						} else if ((BUTTON_First == 4) && (BUTTON_Second == 5)) {
							Action = 8;
							LCD_Refresh = 1;
						// (5) ВВЕРХ	
						} else if ((BUTTON_First == 5) && (BUTTON_Second == 11)) {
							Action = 5;
							LCD_Refresh = 1;
						// (6) ВНИЗ	
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
/* >>> РЕЖИМ "НАСТРОЙКИ ЖУРНАЛА" >>>                                    */
/************************************************************************/
					case MODE_EVENTS_SETTINGS: // Режим настройки журнала
						// (7) ВЛЕВО
						if ((BUTTON_First == 2) && (BUTTON_Second == 5)) {
							Action = 7;
							LCD_Refresh = 1;
						// (8) ВПРАВО
						} else if ((BUTTON_First == 4) && (BUTTON_Second == 5)) {
							Action = 8;
							LCD_Refresh = 1;
						// (5) ВВЕРХ
						} else if ((BUTTON_First == 5) && (BUTTON_Second == 11)) {
							Action = 5;
							LCD_Refresh = 1;
						// (6) ВНИЗ
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
/* >>> РЕЖИМ "КОНТРАСТ" >>>                                             */
/************************************************************************/
					case MODE_CONTRAST: // Режим настройки КОНТРАСТА
						// (7) ВЛЕВО
						if ((BUTTON_First == 2) && (BUTTON_Second == 5)) {
							Action = 7;
							LCD_Refresh = 1;
						// (8) ВПРАВО
						} else if ((BUTTON_First == 4) && (BUTTON_Second == 5)) {
							Action = 8;
							LCD_Refresh = 1;
						// (5) ВВЕРХ
						} else if ((BUTTON_First == 5) && (BUTTON_Second == 11)) {
							Action = 5;
							LCD_Refresh = 1;
						// (6) ВНИЗ
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
/* >>> РЕЖИМ "ВЫБОР СКОРОСТИ ОБМЕНА" >>>                                */
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
/* >>> РЕЖИМ "ПОДТВЕРЖЕДЕНИЕ ПЕРЕХОДА В ПРОЗРАЧНЫЙ РЕЖИМ"               */
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
/* >>> РЕЖИМ "СОХРАНЕНИЕ ЖУРНАЛА"                                       */
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
/* >>> РЕЖИМ "СОХРАНЕНИЕ ЖУРНАЛА ЗАВЕРШЕНО"                             */
/************************************************************************/
					case MODE_EVENTS_SAVE_COMPLETED:
						// (1) OK
						if ((BUTTON_First == 9) && (BUTTON_Second == 0)) {
							Mode = MODE_EVENTS_MENU;
							LCD_Refresh = 1;
						}
						break;
/************************************************************************/
/* >>> РЕЖИМ "РЕДАКТИРОВАНИЕ ПОЛЯ"                                      */
/************************************************************************/
					case MODE_EDIT: // Режим ввода поля
						if ((BUTTON_First == 9) && (BUTTON_Second == 0)) {
						/************************************************************************/
						/* <1> - ВВОД "1"                                                       */
						/************************************************************************/
							EDIT_ValAdd (1);
						} else if ((BUTTON_First == 10) && (BUTTON_Second == 0)) {
						/************************************************************************/
						/* <2> - ВВОД "2"                                                       */
						/************************************************************************/
							EDIT_ValAdd (2);
						} else if ((BUTTON_First == 11) && (BUTTON_Second == 0)) {
						/************************************************************************/
						/* <3> - ВВОД "3"                                                       */
						/************************************************************************/
							EDIT_ValAdd (3);
						} else if ((BUTTON_First == 12) && (BUTTON_Second == 0)) {
						/************************************************************************/
						/* <4> - ВВОД "4"                                                       */
						/************************************************************************/
							EDIT_ValAdd (4);
						} else if ((BUTTON_First == 1) && (BUTTON_Second == 0)) {
						/************************************************************************/
						/* <5> - ВВОД "5"                                                       */
						/************************************************************************/
							EDIT_ValAdd (5);
						} else if ((BUTTON_First == 2) && (BUTTON_Second == 0)) {
						/************************************************************************/
						/* <6> - ВВОД "6"                                                       */
						/************************************************************************/
							EDIT_ValAdd (6);
						} else if ((BUTTON_First == 3) && (BUTTON_Second == 0)) {
						/************************************************************************/
						/* <7> - ВВОД "7"                                                       */
						/************************************************************************/
							EDIT_ValAdd (7);
						} else if ((BUTTON_First == 4) && (BUTTON_Second == 0)) {
						/************************************************************************/
						/* <8> - ВВОД "8"                                                       */
						/************************************************************************/
							EDIT_ValAdd (8);
						} else if ((BUTTON_First == 6) && (BUTTON_Second == 0)) {
						/************************************************************************/
						/* <9> - ВВОД "9"                                                       */
						/************************************************************************/
							EDIT_ValAdd (9);
						} else if ((BUTTON_First == 7) && (BUTTON_Second == 0)) {
						/************************************************************************/
						/* <0> - ВВОД "0"                                                       */
						/************************************************************************/
							EDIT_ValAdd (0);
						} else if ((BUTTON_First == 8) && (BUTTON_Second == 0)) {
							
						/************************************************************************/
						/* <ENTER> - ВВОД "ENTER"                                               */
						/************************************************************************/
							if (EDIT_Val > 0xFFFF) {
							} else {
								if (EDIT_ValMultiplier == 0) EDIT_ValMultiplier = 1;
								SendToPlc_Adress[0] = EDIT_ValPlcAdr;
								SendToPlc_Value[0] = ( (EDIT_Val / EDIT_ValMultiplier) * EDIT_ValDivider ) - EDIT_ValAddition;
								SendToPlc_Ready[0] = SEND_ATTEMPTS;
								SendToPlc_Amount = 1;
							}
// Масштабирование значения регистра
// APD_ValTemp = APD_Val * APD_ValMultiplier;

// Округление: (x + div/2) / div =
// APD_ValResult = ((APD_ValTemp + (APD_ValDivider >> 1)) / APD_ValDivider) + APD_ValAddition;
							Mode = MODE_NORMAL;
							TIMER_EditValBlinkEn = 0;
							TIMER_EditValBlink = 0;
	
							LCD_Refresh = 1;
						} else if ((BUTTON_First == 5) && (BUTTON_Second == 8)) {
						/************************************************************************/
						/* <CLEAR> - ВВОД "CLEAR"                                               */
						/************************************************************************/
							EDIT_Val = 0;
							// EDIT_ValNeg = 0;
						} else if ((BUTTON_First == 5) && (BUTTON_Second == 6)) {
						/************************************************************************/
						/* <-> - ВВОД "-"                                                       */
						/************************************************************************/
							EDIT_Val *= -1;								
						} else if ((BUTTON_First == 5) && (BUTTON_Second == 12)) {
						/************************************************************************/
						/* <F3> - ВВОД "ВЫХОД"                                                  */
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
/*                       Программные счётчики                           */
/************************************************************************/
ISR (TCC0_OVF_vect)
{ 
// Главный таймер с периодом 1 мс
	// Программные таймеры
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
/*                             Радиомодем                               */
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
	// Забираем байт из буфера
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
			// Ищем символ начала посылки - '$'
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
				// Передача принятого байта по радиоканалу в RS-232
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
	// Приём в режиме конфигурации
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
	
	// Приём в обычном режиме
	if (RMD_RecEn) {
		// Ищем символ начала посылки - '$'
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
	// Забираем байт из буфера
	ADP_RecMesCharBuf = usart_get (&USARTC0);
	switch (NET_Mode) {
		case NET_MODE_CONFIG:
			break;
		case NET_MODE_MASTER:
			TIMER_RS232_START;
			// Если выставлен флаг превышения временного интервала
			// между посылками, то:
			//if (ADP_RecMesTimeout) {
				//ADP_RecMesTimeout = 0;
				//// Начинаем приём новой посылки. Старая затирается
				//ADP_RecMesByteCnt = 0;
		////		ADP_RecMesCharBuf = usart_get (&USARTC0);
				//ADP_RecMesBuf[ADP_RecMesByteCnt] = ADP_RecMesCharBuf;
//
			//} else {
				//// Продолжаем приём текущей посылки
				//++ ADP_RecMesByteCnt;
		////		ADP_RecMesCharBuf = usart_get (&USARTC0);
				//ADP_RecMesBuf[ADP_RecMesByteCnt] = ADP_RecMesCharBuf;
			//}
			ADP_RecMesBuf[ADP_RecMesLen++] = ADP_RecMesCharBuf;
			break;
		case NET_MODE_TRANSP:
			if (usart_data_register_is_empty (&USARTE0)) {
				// передача принятого байта по RS-232 в радиоканал
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
	// Если выставлен флаг превышения временного интервала
	// между посылками, то:
	if (ADP_RecMesTimeout) {
		ADP_RecMesTimeout = 0;
		// Начинаем приём новой посылки. Старая затирается
		ADP_RecMesByteCnt = 0;
//		ADP_RecMesCharBuf = usart_get (&USARTC0);
		ADP_RecMesBuf[ADP_RecMesByteCnt] = ADP_RecMesCharBuf;

	} else {
		// Продолжаем приём текущей посылки
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
// Таймер временной задержки посылок протокола ADP
ISR (TCD1_OVF_vect) {
	TIMER_RS232_STOP;
	// Превышен таймаут между принятыми символами:
	// выставляем флаг принятой посылки и
	// готовимся к приёму новой
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
	//MB0_State.flags |= 0x01;//Флаг интервала тишины 1,5 символа
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
