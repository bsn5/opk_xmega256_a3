/*
 * apd_project.c
 *
 * Created: 05.10.2012 10:56:31
 *  Author: vshivtsev_d
 */

void ApdPrjProc (void) {
	/************************************************************************/
	/*                         Разбор проекта APD                           */
	/************************************************************************/
	// Проверка содержимого проекта APD
	if ((ApdPrjData[0] == 0x41) && (ApdPrjData[1] == 0x44)) {
		OpkErrorPrj = 0;
	} else {
		OpkErrorPrj = 1;
	}
	// Выбор периода опроса кадров
	if (ApdPrjData[0x000B]) {
		ApdFrameRefreshRate = ApdPrjData[0x000B]*100; // Период опроса кадров (0-авто,1-100мс,2-200,3...10)
	} else {
		ApdFrameRefreshRate = 2000;
	}
	TcAdpTransPer = ApdFrameRefreshRate;

	// Выбор периода опроса событий
	if (ApdPrjData[0x000C]) {
		ApdEventRefreshRate = ApdPrjData[0x000C]*100; // Период опроса событий (0-авто,1-100мс,2-200,3...10)
	} else {
		ApdEventRefreshRate = 2000;
	}
	TcAdpEventsPer = ApdEventRefreshRate;
	TcAdpTransFaultPer = 3 * TcAdpTransPer; // Период таймера обработки ошибок передачи

	// Выбор скорости обмена с ПЛК
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

	ApdFrameAmount = ApdPrjData[0x0010]; // Количество кадров в проекте
	ApdFrameStartNumber = ApdPrjData[0x0011]; // Номер стартового кадра

	ApdFrameCur = ApdFrameStartNumber; // Текущий кадр = стартовый кадр проекта

	ApdFramesStartAdr = (ApdPrjData[0x0014]<<8) | (ApdPrjData[0x0013]); // Адрес описания кадров
	ApdEventsStartAdr = (ApdPrjData[0x0016]<<8) | (ApdPrjData[0x0015]); // Адрес описания списка событий

	// Количество индексов списка событий
	ApdEventIdAmount = (ApdPrjData[ApdEventsStartAdr + 1]<<8) | (ApdPrjData[ApdEventsStartAdr]);
	if (ApdEventIdAmount) {	
		// Адрес начала списка событий в ПЛК
		ApdEventAdr = (ApdPrjData[ApdEventsStartAdr + 3]<<8) | (ApdPrjData[ApdEventsStartAdr + 2]);
		ApdEventAdr += 1;
		// Заполнение массива адресов названий событий
		for (IdEvent = 0; IdEvent <= (ApdEventIdAmount - 1); IdEvent++) {
			ApdEventStartAdr[IdEvent] = ApdEventsStartAdr + 8 + IdEvent*20;
		}
	}	

	// Заполнение Массива адресов описаний кадров:
	// точка отсчёта адреса - адрес описания кадров. Далее адреса разбираются
	// побайтно в соответствии с количеством кадров в проекте
	for (IdFrame = 0; IdFrame <= (ApdFrameAmount - 1); IdFrame++) {
		ApdFrameStartAdr[IdFrame] |= ApdPrjData[ApdFramesStartAdr + Id8]; // Младший байт
		Id8++;
		ApdFrameStartAdr[IdFrame] |= (ApdPrjData[ApdFramesStartAdr + Id8]<<8); // Старший байт
		Id8++;
	}

	Id8 = 0;

	// Заполнение Массива описаний полей и действий в кадре:
	// точка отсчёта адреса - адрес из Массива адресов описаний кадров. Далее адреса разбираются
	// побайтно в соответствии с количеством кадров в проекте.
	// В нулевой строке - адрес описания полей кадра, в первой - адрес описания
	// кнопкодействий кадра.
	for (IdFrame = 0; IdFrame <= (ApdFrameAmount - 1); IdFrame++) {
		// Адрес описания полей кадра
		ApdFrameFieldsAndActionsStartAdr[IdFrame][0] |= ApdPrjData[ApdFrameStartAdr[IdFrame] + Id8]; // Младший байт
		Id8++;
		ApdFrameFieldsAndActionsStartAdr[IdFrame][0] |= (ApdPrjData[ApdFrameStartAdr[IdFrame] + Id8]<<8); // Старший байт
		Id8++;
		// Адрес описания кнопкодействий кадра
		ApdFrameFieldsAndActionsStartAdr[IdFrame][1] |= ApdPrjData[ApdFrameStartAdr[IdFrame] + Id8]; // Младший байт
		Id8++;
		ApdFrameFieldsAndActionsStartAdr[IdFrame][1] |= (ApdPrjData[ApdFrameStartAdr[IdFrame] + Id8]<<8); // Старший байт
		Id8++;
	
		Id8 = 0;	
	}

	// Заполняем массив количества ПОЛЕЙ и КНОПКОДЕЙСТВИЙ каждого кадра 
	for (IdFrame = 0; IdFrame <= (ApdFrameAmount - 1); IdFrame++) {
		ApdFrameFieldsAmount[IdFrame] = ApdPrjData[ApdFrameFieldsAndActionsStartAdr[IdFrame][0]];
		ApdFrameActionsAmount[IdFrame] = ApdPrjData[ApdFrameFieldsAndActionsStartAdr[IdFrame][1]];
	}	

	Id8 = 0;

	// Заполняем массив адресов ПОЛЕЙ каждого кадра
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

	// Заполняем массив адресов КНОПКОДЕЙСТВИЙ каждого кадра
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

	// Какие адреса запрашиваются из ПЛК в каждом кадре и сколько их?
	for (IdFrame = 0; IdFrame <= (ApdFrameAmount - 1); IdFrame++) {
		if (ApdFrameFieldsAmount[IdFrame] > 1) {
			for (IdField = 0; IdField <= (ApdFrameFieldsAmount[IdFrame] - 1); IdField++) {
				// Если тип поля - ЧИСЛО или ТЕКСТ:
				if ((ApdPrjData[ApdFrameFieldStartAdr[IdFrame][IdField]] == APD_FIELD_TYPE_VALUE) || (ApdPrjData[ApdFrameFieldStartAdr[IdFrame][IdField]] == APD_FIELD_TYPE_TEXT)) {
					// Сохраняем его адрес в ПЛК в ячейку 16 бит:
					// Младший байт
					ApdFrameDataFromPlcAdr[IdFrame][ApdFrameDataFromPlcCnt] = ApdPrjData[ApdFrameFieldStartAdr[IdFrame][IdField] + 4 + Id8];
					Id8++;
					// Старший байт
					ApdFrameDataFromPlcAdr[IdFrame][ApdFrameDataFromPlcCnt] |= (ApdPrjData[ApdFrameFieldStartAdr[IdFrame][IdField] + 4 + Id8]<<8);
					ApdFrameDataFromPlcAdr[IdFrame][ApdFrameDataFromPlcCnt] += 1;
					Id8++;
					// Записываем строку отображения переменной на дисплее
					ApdFrameDataFromPlcString[IdFrame][ApdFrameDataFromPlcCnt] = ApdPrjData[ApdFrameFieldStartAdr[IdFrame][IdField] + 1];
					// Считаем количество полей типа ЧИСЛО
					ApdFrameDataFromPlcCnt++;		
				}
				Id8 = 0;
			}
			// Записываем суммарное количество полей типа ЧИСЛО в ячейку массива
			ApdFrameDataFromPlcAmount[IdFrame] = ApdFrameDataFromPlcCnt;
			ApdFrameDataFromPlcCnt = 0;
			Id8 = 0;	
		} else {
			ApdFrameDataFromPlcAmount[IdFrame] = 0;		
		}		
	}	
}