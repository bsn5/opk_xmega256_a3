/*
 * apd_project.c
 *
 * Created: 05.10.2012 10:56:31
 *  Author: vshivtsev_d
 */

void ApdPrjProc (void) {
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
}