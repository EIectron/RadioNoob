#include "gui.h"
#include "crsf.h"

guiMenu_t guiMenu;
guiMenu_t _guiMenu = -1;
uint8_t TrmUpdate = 0;

bool JoystickBorderUpdate = true;

extern TableParams myTableParams;
extern RCValues myRCvalues;
extern uint32_t adc_val[10];


int8_t indexValue = 0, 
			_indexValue = -1,
			selectIndex = -1;
uint32_t itemCount = 0;



extern FILINFO file_info_array[FILE_SIZE];
uint8_t file_size = 0;

extern crsfLinkStatistics_t LinkStatistics;
extern crsf_param_t crsf_params[25];
extern crsf_device_t crsf_devices[CRSF_MAX_DEVICES];
extern uint8_t next_param, next_chunk;
extern uint32_t tickTime;
extern u8 command_stat; //False->Read Param, True->Write Param



uint8_t loadPos = 0;
bool    loadDir = 0;


void DrawBatteryLogo(uint8_t bat_lvl)
{
	/*
	* Battery Level range is 0 to 100.
	* Battery bar range 0 to 30.
	*/
	uint8_t bar_x;
	bat_lvl = constrain(bat_lvl, 0, 100);	// Check bat_lvl is in true range
	bar_x = map(bat_lvl, 0, 100, 0, 30);
	GFXDisplayDrawRect(BatteryPaddingX+bar_x, BatteryPaddingY, BatteryPaddingX+30, BatteryPaddingY+15, WHITE);
}

void DrawRssiLevel(int8_t value)
{
//	value = constrain(value, 0, 120);
	uint8_t rssi_lvl = map(value, -120, 0, 1, 5);
//	printf("bar count: %d\n", rssi_lvl);
	for(uint8_t i = 0; i < rssi_lvl; i++)
	{
		GFXDisplayLineDrawV(((DISP_HOR_RESOLUTION/2) + 15 + (i * 5)), (BatteryPaddingY + 17), ((BatteryPaddingY + 15) - ((i+1) * 3)), WHITE, 2);
		//printf("Bar %d, x:%d, y1:%d , y2:%d\n", i, ((DISP_HOR_RESOLUTION/2) + 15 + (i * 5)), (BatteryPaddingY + 17), ((BatteryPaddingY + 15) - ((i+1) * 3)));
	}
}

void DrawTopBar(void)
{
	GFXDisplayDrawRect(0, 0, DISP_HOR_RESOLUTION, TaskBarHeight, BLACK);
	GFXDisplayPutImage(BatteryPaddingX,BatteryPaddingY, &battery_chrg, true);
	GFXDisplayPutImage(DISP_HOR_RESOLUTION/2,BatteryPaddingY, &antenna, true);
	GFXDisplayPutImage(BatteryPaddingX-55,BatteryPaddingY, &usb_logo, false);
	GFXDisplayPutImage(10,BatteryPaddingY, &user_logo, false);
	
	//DrawBatteryLogo(20);	//usb plug condition
	char *text = (char *)malloc(5);

	if(crsf_devices[1].address == 0x00)
		strlcpy(text, "--", 3);
	else
		sprintf(text, "%d", LinkStatistics.uplink_RSSI_1);
	
	DrawRssiLevel(LinkStatistics.uplink_RSSI_1);
	GFXDisplayPutString(BatteryPaddingX-25, BatteryPaddingY, &fontConsolas24h, "61", WHITE, TRANSPARENT);
	GFXDisplayPutString(((DISP_HOR_RESOLUTION/2)-40), BatteryPaddingY+3, &fontConsolas24h, text, WHITE, TRANSPARENT);
	GFXDisplayPutString(30, BatteryPaddingY, &fontConsolas24h, myTableParams.MainConfig.name, WHITE, TRANSPARENT);
	
	free(text);
}

void DrawVeichle(void)
{
	GFXDisplayPutImage(VeichleLogoX, VeichleLogoY, &multirotor, false);
}

void DrawSticksTrim(void)
{
			int16_t ttrim = map(myTableParams.TrimConfig.trm_throttle, TRM_MIN, TRM_MAX, 204, 40);
			int16_t ytrim = map(myTableParams.TrimConfig.trm_yaw, TRM_MIN, TRM_MAX, 10, 175);
			int16_t ptrim = map(myTableParams.TrimConfig.trm_pitch, TRM_MIN, TRM_MAX, 204, 40);
			int16_t rtrim = map(myTableParams.TrimConfig.trm_roll, TRM_MIN, TRM_MAX, 213, 381);
	
			/*
			// Hangi trim guncellenmisse o arrow'u sil
			*/
			if(TrmUpdate)
			{
				if(TrmUpdate == 1)
					GFXDisplayDrawRect(20, 35, 25, 215, WHITE);
				else if(TrmUpdate == 2)
					GFXDisplayDrawRect(10, 222, 185, 227, WHITE);
				else if(TrmUpdate == 3)
					GFXDisplayDrawRect(378, 35, 383, 215, WHITE);
				else if(TrmUpdate == 4)
					GFXDisplayDrawRect(208, 222, 390, 227, WHITE);
				
				TrmUpdate = 0;
			}
	
//	Throttle up:40, dwn:204
			GFXDisplayPutImage(20, ttrim, &arrow_l, false);

//	Pitch up:40, dwn:204
			GFXDisplayPutImage(378, ptrim, &arrow_r, false);

//	Yaw l:10, r:381
			GFXDisplayPutImage(ytrim, 222, &arrow_d, false);

//	Roll l:10, r:381
			GFXDisplayPutImage(rtrim, 222, &arrow_d, false);
}

void DrawJoystick(void)
{
		if(guiMenu != _guiMenu)
		{
			GFXDisplayAllClear();
			_guiMenu = guiMenu;
		}
		uint16_t x1, x2, y1, y2;
		uint16_t x1_, x2_, y1_, y2_;
		
		if(JoystickBorderUpdate)
		{
			x1 = 50;
			x2 = x1+130;
			y1  = 100;
			y2 = y1+130;
			
			GFXDisplayLineDrawH(x1, x2, y1, BLACK, 2);
			GFXDisplayLineDrawH(x1, x2, y2, BLACK, 2);
			GFXDisplayLineDrawV(x1, y1, y2, BLACK, 2);
			GFXDisplayLineDrawV(x2, y1, y2, BLACK, 2);
			
			
			
			x1 = 220;
			x2 = x1+130;
			y1  = 100;
			y2 = y1+130;
			
			GFXDisplayLineDrawH(x1, x2, y1, BLACK, 2);
			GFXDisplayLineDrawH(x1, x2, y2, BLACK, 2);
			GFXDisplayLineDrawV(x1, y1, y2, BLACK, 2);
			GFXDisplayLineDrawV(x2, y1, y2, BLACK, 2);
			JoystickBorderUpdate = false;
		}
		
		myRCvalues.channelValues[THROTTLE] = mapJoystickValues(adc_val[THROTTLE], myTableParams.channels[THROTTLE].min, 
																																myTableParams.channels[THROTTLE].mid, 
																																myTableParams.channels[THROTTLE].max, 1);
		myRCvalues.channelValues[YAW] = mapJoystickValues(adc_val[YAW], myTableParams.channels[YAW].min, 
																																myTableParams.channels[YAW].mid, 
																																myTableParams.channels[YAW].max, 1);
		myRCvalues.channelValues[PITCH] = mapJoystickValues(adc_val[PITCH], myTableParams.channels[PITCH].min, 
																																myTableParams.channels[PITCH].mid, 
																																myTableParams.channels[PITCH].max, 1);
		myRCvalues.channelValues[ROLL] = mapJoystickValues(adc_val[ROLL], myTableParams.channels[ROLL].min, 
																																myTableParams.channels[ROLL].mid, 
																																myTableParams.channels[ROLL].max, 1);
		
		y1 = map(myRCvalues.channelValues[THROTTLE], 850, 2100, 220, 100);
		y2 = map(myRCvalues.channelValues[PITCH], 850, 2100, 220, 100);
		x1 = map(myRCvalues.channelValues[YAW], 900, 2100, 170, 50);
		x2 = map(myRCvalues.channelValues[ROLL], 900, 2100, 340, 220);
//		printf("x1: %d, y1: %d, \t x2: %d, y2: %d\n", x1, y1, x2, y2);
		
		GFXDisplayDrawRect(52, 102, 178, 228, WHITE);
		GFXDisplayPutText(x1, y1, &abeatbykai24, "o", BLACK);
		GFXDisplayDrawRect(222, 102, 348, 228, WHITE);
		GFXDisplayPutText(x2, y2, &abeatbykai24, "o", BLACK);
		GFXDisplayUpdate();
		
}

void DrawChannels(void)
{
	if(guiMenu != _guiMenu)
	{
		GFXDisplayAllClear();
		_guiMenu = guiMenu;
	}
	
	myRCvalues.channelValues[0] = mapJoystickCRSFValues(adc_val[0], myTableParams.channels[0].min, 
																												myTableParams.channels[0].mid, 
																												myTableParams.channels[0].max, 
																												CHANNEL_MIN,
																												CHANNEL_MID,
																												CHANNEL_MAX,
																												true);

	myRCvalues.channelValues[1] = mapJoystickCRSFValues(adc_val[1], myTableParams.channels[1].min, 
																												myTableParams.channels[1].mid, 
																												myTableParams.channels[1].max, 
																												CHANNEL_MIN,
																												CHANNEL_MID,
																												CHANNEL_MAX,
																												true);

	myRCvalues.channelValues[2] = mapJoystickCRSFValues(adc_val[2], myTableParams.channels[2].min, 
																												myTableParams.channels[2].mid, 
																												myTableParams.channels[2].max, 
																												CHANNEL_MIN,
																												CHANNEL_MID,
																												CHANNEL_MAX,
																												true);
																												
	myRCvalues.channelValues[3] = mapJoystickCRSFValues(adc_val[3], myTableParams.channels[3].min, 
																												myTableParams.channels[3].mid, 
																												myTableParams.channels[3].max, 
																												CHANNEL_MIN,
																												CHANNEL_MID,
																												CHANNEL_MAX,
																												true);
																												
	myRCvalues.channelValues[4] = mapJoystickCRSFValues(adc_val[4], myTableParams.channels[4].min, 
																														myTableParams.channels[4].mid, 
																														myTableParams.channels[4].max, 
																														CHANNEL_MIN,
																														CHANNEL_MID,
																														CHANNEL_MAX,
																														true);

	myRCvalues.channelValues[5] = mapJoystickCRSFValues(adc_val[5], myTableParams.channels[5].min, 
																														myTableParams.channels[5].mid, 
																														myTableParams.channels[5].max, 
																														CHANNEL_MIN,
																														CHANNEL_MID,
																														CHANNEL_MAX,
																														true);
																														
	myRCvalues.channelValues[6] = mapJoystickCRSFValues(adc_val[6], myTableParams.channels[6].min, 
																														myTableParams.channels[6].mid, 
																														myTableParams.channels[6].max, 
																														CHANNEL_MIN,
																														CHANNEL_MID,
																														CHANNEL_MAX,
																														true);					

	myRCvalues.channelValues[7] = mapJoystickCRSFValues(adc_val[7], myTableParams.channels[7].min, 
																														myTableParams.channels[7].mid, 
																														myTableParams.channels[7].max, 
																														CHANNEL_MIN,
																														CHANNEL_MID,
																														CHANNEL_MAX,
																														false);

		
		
		//printf("x1: %d, y1: %d, \t x2: %d, y2: %d\n", myRCvalues.channelValues[0], myRCvalues.channelValues[1], myRCvalues.channelValues[2], myRCvalues.channelValues[3]);
	

		uint16_t x[8];
		x[0] = map(myRCvalues.channelValues[0], CHANNEL_MIN, CHANNEL_MAX, 100, 300);
		x[1] = map(myRCvalues.channelValues[1], CHANNEL_MIN, CHANNEL_MAX, 100, 300);
		x[3] = map(myRCvalues.channelValues[3], CHANNEL_MIN, CHANNEL_MAX, 100, 300);
		x[2] = map(myRCvalues.channelValues[2], CHANNEL_MIN, CHANNEL_MAX, 100, 300);
		x[4] = map(myRCvalues.channelValues[4], CHANNEL_MIN, CHANNEL_MAX, 100, 300);
		x[5] = map(myRCvalues.channelValues[5], CHANNEL_MIN, CHANNEL_MAX, 100, 300);
		x[6] = map(myRCvalues.channelValues[6], CHANNEL_MIN, CHANNEL_MAX, 100, 300);
		x[7] = map(myRCvalues.channelValues[7], CHANNEL_MIN, CHANNEL_MAX, 100, 300);
		
		char *lcd_txt = (char *)malloc(64);
		sprintf(lcd_txt, "c:/Input Values>");	
		GFXDisplayPutString(10, 5, &fontConsolas24h, lcd_txt, BLACK, WHITE);	
	
		
		for(uint8_t i = 0; i<8; i++)
		{
			sprintf(lcd_txt, "%d", myRCvalues.channelValues[i]);	
			GFXDisplayPutString(310, (i*20)+30, &fontConsolas24h, "     ", BLACK, WHITE);	
			GFXDisplayPutString(310, (i*20)+30, &fontConsolas24h, lcd_txt, BLACK, WHITE);	
			if(i<4)
			{
				sprintf(lcd_txt, "CH%d", i);	
				GFXDisplayPutString(10, (i*20)+30, &fontConsolas24h, lcd_txt, BLACK, WHITE);			
			}
			else
			{
				sprintf(lcd_txt, "AUX%d", i);	
				GFXDisplayPutString(10, (i*20)+30, &fontConsolas24h, lcd_txt, BLACK, WHITE);		
			}
			GFXDisplayLineDrawH(100, x[i], (i*20)+30, BLACK, 10);
			GFXDisplayLineDrawH(x[i], 300, (i*20)+30, WHITE, 10);
		}
		
		sprintf(lcd_txt, "SWA[%d] SWB[%d] SWC[%d] SWD[%d]", GetSW(SWA), GetSW(SWB), GetSW(SWC), GetSW(SWD));	
		GFXDisplayPutString(20, (8*20)+30, &fontConsolas24h, lcd_txt, BLACK, WHITE);
		sprintf(lcd_txt, "SWE[%d] SWF[%d] SWG[%d] SWH[%d]", GetSW(SWE), GetSW(SWF), GetSW(SWG), GetSW(SWH));	
		GFXDisplayPutString(20, (9*20)+30, &fontConsolas24h, lcd_txt, BLACK, WHITE);			
		
		GFXDisplayUpdate();
		free(lcd_txt);
}

void DrawMainScreen(void)
{
	if(guiMenu != _guiMenu)
	{
		GFXDisplayAllClear();
		GFXDisplayPutImage(0,0, &main_screen, false);
		
		DrawVeichle();
		_guiMenu = guiMenu;
	}
	
	DrawTopBar();
	DrawSticksTrim();
	uint32_t time = millis() / 1000;
	char *lcd_txt = (char *)malloc(32);
	sprintf(lcd_txt, "%.2d:%.2d", (time / 60), (time % 60));
	GFXDisplayPutString(30, 55, &fontConsolas24h, "T1", BLACK, WHITE);
	GFXDisplayPutText(30, 80, &Arcade36, lcd_txt, BLACK);
	
//	sprintf(lcd_txt, "%.2d:%.2d", (time / 60), (time % 60));
	GFXDisplayPutString(30, 120, &fontConsolas24h, "T2", BLACK, WHITE);
	GFXDisplayPutString(30, 140, &fontConsolas24h, lcd_txt, BLACK, WHITE);
	free(lcd_txt);
	GFXDisplayUpdate();

	//DrawJoystick();
	//DrawChannels();
	
}

void DrawMenuScreen(void)
{
	if(guiMenu != _guiMenu)
	{
		_guiMenu = guiMenu;
		
		itemCount = (sizeof(MenuItems) / sizeof(*MenuItems)) - 1;
		GFXDisplayAllClear();
		GFXDisplayPutText(170, 8, &abeatbykai24, "MENU", BLACK);
		QEI_Stop(QEI0);
		QEI_SET_CNT_VALUE(QEI0, itemCount);
		QEI_SET_CNT_MAX(QEI0, itemCount);
		QEI_Start(QEI0);
		indexValue = 0;
		_indexValue = -1;
	}
	
	if(indexValue != _indexValue)
	{
		for(uint8_t i = 0; i<=itemCount; i++)
		{
			if(indexValue == i)
				GFXDisplayPutString(20, (i*20)+ 30, &fontConsolas24h, MenuItems[i], WHITE, BLACK);
			else
				GFXDisplayPutString(20, (i*20)+ 30, &fontConsolas24h, MenuItems[i], BLACK, WHITE);
		}
		// printf("Index: %d\n", indexValue);: %d\n", indexValue);
		_indexValue = indexValue;
		GFXDisplayUpdate();
	}

	
	indexValue = (itemCount - QEI_GET_CNT_VALUE(QEI0)); // itemCount'tan ?ikarma sebebim direction'in ters olmasi
	//selectIndex = (QEI_GET_INDEX_LATCH_VALUE(QEI0));
	if(ENCODER_SW == 0)
	{
		Delay(50);
		while(ENCODER_SW == 0);
		guiMenu = indexValue + 3;
	}
	
	if(END_SW_PIN == 0)
	{
		Delay(50);
		while(END_SW_PIN == 0);
	  guiMenu = HomeScreen;
	}
}

void DrawSDCardScreen(void)
{
	if(guiMenu != _guiMenu)
	{
		GFXDisplayAllClear();
		indexValue = 0;
		_indexValue = -1;
		_guiMenu = guiMenu;
		
		FS_Show_Files("/Sounds", file_info_array);
		itemCount = CheckFileSize()-1;
		
		GFXDisplayAllClear();
		GFXDisplayPutText(150, 8, &abeatbykai24, "SD CARD", BLACK);
		QEI_Stop(QEI0);
		QEI_SET_CNT_VALUE(QEI0, itemCount);
		QEI_SET_CNT_MAX(QEI0, itemCount);
		QEI_Start(QEI0);
	}
	
	if(indexValue != _indexValue)
	{
		for(uint8_t i = 0; i<=itemCount; i++)
		{
			if(indexValue == i)
				GFXDisplayPutString(20, (i*25)+ 30, &fontConsolas24h, file_info_array[i].fname, WHITE, BLACK);
			else
				GFXDisplayPutString(20, (i*25)+ 30, &fontConsolas24h, file_info_array[i].fname, BLACK, WHITE);
		}
		// printf("Index: %d\n", indexValue);: %d\n", indexValue);
		_indexValue = indexValue;
		GFXDisplayUpdate();
	}
	
	indexValue = (itemCount - QEI_GET_CNT_VALUE(QEI0)); // itemCount'tan ?ikarma sebebim direction'in ters olmasi
	
	if(END_SW_PIN == 0)
	{
		Delay(50);
		while(END_SW_PIN == 0);
		guiMenu = MenuScreen;
	}
}

void DrawSettingScreen(void)
{
	if(guiMenu != _guiMenu)
	{
		GFXDisplayAllClear();
		indexValue = 0;
		_indexValue = -1;
		_guiMenu = guiMenu;
		
		itemCount = (sizeof(SettingItems) / sizeof(*SettingItems)) - 1;
		
		GFXDisplayAllClear();
		GFXDisplayPutText(150, 8, &abeatbykai24, "SETTING", BLACK);
		QEI_Stop(QEI0);
		QEI_SET_CNT_VALUE(QEI0, itemCount);
		QEI_SET_CNT_MAX(QEI0, itemCount);
		QEI_Start(QEI0);
	}
	
	
	if(indexValue != _indexValue)
	{
		for(uint8_t i = 0; i<=itemCount; i++)
		{
			if(indexValue == i)
				GFXDisplayPutString(20, (i*25)+ 30, &fontConsolas24h, SettingItems[i], WHITE, BLACK);
			else
				GFXDisplayPutString(20, (i*25)+ 30, &fontConsolas24h, SettingItems[i], BLACK, WHITE);
		}
		// printf("Index: %d\n", indexValue);: %d\n", indexValue);
		_indexValue = indexValue;
		GFXDisplayUpdate();
	}
	
	indexValue = (itemCount - QEI_GET_CNT_VALUE(QEI0)); // itemCount'tan cikarma sebebim direction'in ters olmasi
	
	if(END_SW_PIN == 0)
	{
		Delay(50);
		while(END_SW_PIN == 0);
		guiMenu = MenuScreen;
	}
	
	if(ENCODER_SW == 0)
	{
		Delay(50);
		while(ENCODER_SW == 0);
		if(indexValue == 0)
		{
//			calibrationAnalogJoysticks();
			_guiMenu = -1;
		}
		else if(indexValue == 1)
		{
			guiMenu = QRScreen;
		}
	}
}

void DrawExpressLRSScreen(void)
{
	uint8_t param_list[ELRS_ParamSize] = {
													Packet_Rate, 
													Telem_Ratio,
													Switch_Mode,
													Model_Match,
													TX_Power,
													VTX_Administrator,
													WiFi_Connectivity,
													BLE_Joystick,
													Bind,
//															Bad_Good,
													Info
													};
	if(guiMenu != _guiMenu)
	{
		indexValue = 0;
		_indexValue = -1;
		_guiMenu = guiMenu;
		
		itemCount = ELRS_ParamSize - 1;
		
		GFXDisplayAllClear();
		GFXDisplayPutText(150, 8, &abeatbykai24, "ExpressLRS", BLACK);
		QEI_Stop(QEI0);
		QEI_SET_CNT_VALUE(QEI0, itemCount);
		QEI_SET_CNT_MAX(QEI0, itemCount);
		QEI_Start(QEI0);
		next_param = ParamsLoaded(CRSF_ADDRESS_CRSF_TRANSMITTER);
		command_stat = false;
	}
	
	if(ParamsLoaded(CRSF_ADDRESS_CRSF_TRANSMITTER) <= crsf_devices[0].number_of_params)
	{
//		printf("Param: %d\n", ParamsLoaded(ELRS_ADDRESS));
//		uint16_t width = map(next_param, 1, crsf_devices[0].number_of_params, 10, 410);
//		GFXDisplayLineDrawH(10, width, 50, BLACK, 20);
		
			char *loaded = (char *)malloc(5);
			GFXDisplayDrawRect(200, 50, 400, 90, WHITE);
			sprintf(loaded, "%c%lu", 25,  map(next_param, 1, crsf_devices[0].number_of_params, 0, 100));
			GFXDisplayPutText(200, 60, &abeatbykai24, loaded, BLACK);
			free(loaded);

			if(loadDir)
			{
				GFXDisplayPutPixel(loadPos+100, 100, BLACK);
				GFXDisplayPutPixel(loadPos+100, 101, BLACK);
				GFXDisplayPutPixel(loadPos+100, 102, BLACK);
				GFXDisplayPutPixel(loadPos+100, 103, BLACK);
				Delay(2);
				loadPos++;
				if(loadPos >= 200)
				{
					loadDir = !loadDir;
					loadPos = 0;
				}
			}
			else
			{
				GFXDisplayPutPixel(loadPos+100, 100, WHITE);
				GFXDisplayPutPixel(loadPos+100, 101, WHITE);
				GFXDisplayPutPixel(loadPos+100, 102, WHITE);
				GFXDisplayPutPixel(loadPos+100, 103, WHITE);
				loadPos++;
				if(loadPos >= 200)
				{
					loadDir = !loadDir;
					loadPos = 0;
				}
			}
		

		GFXDisplayUpdate();
		return;
	}
	else
	{
		if(next_param > 0)
		{
			GFXDisplayLineDrawH(10, 410, 100, WHITE, 20);
			next_param = 0;
		}
	}

	

	if(indexValue != _indexValue)
	{
		uint8_t item = 1;
		uint8_t j = 0;
		

		GFXDisplayLineDrawH(0, 420, 0, BLACK, 30);
		GFXDisplayPutText(10, 5, &abeatbykai24, crsf_devices[0].name, WHITE);	//Device Name

		GFXDisplayPutString(320, 8, &fontConsolas24h, crsf_params[20].value, WHITE, BLACK);	// Bad/Good value
		GFXDisplayPutString(320, 215, &fontConsolas24h, crsf_params[21].value, BLACK, WHITE);	// Version No

															
		if(selectIndex > -1)
		{	
			GFXDisplayPutImage(80, 100, &value_box, false);
			if(param_list[selectIndex] == TX_Power)
			{
				guiMenu = TxPowerScreen;
				return;
			}
			else if(param_list[selectIndex] == VTX_Administrator)
			{
				guiMenu = VTxAdminScreen;
				return;
			}
			else if(param_list[selectIndex] == WiFi_Connectivity)
			{
				guiMenu = WifiConnectivityScreen;
				return;
			} 			
			else if(param_list[selectIndex] == BLE_Joystick)
			{
				GFXDisplayPutString(90, 120, &fontConsolas24h, "INIT BLE...", BLACK, WHITE);
				guiMenu = BleJoystickScreen;
				next_param = BLE_Joystick;
				crsf_params[BLE_Joystick].status = PROGRESS;
				crsf_params[BLE_Joystick].current_value = 1;
			}
			else if(param_list[selectIndex] == Bind) // Try to Bind command but not succesful :/
			{
				GFXDisplayPutString(90, 120, &fontConsolas24h, "Binding...", BLACK, WHITE);
			}

				GFXDisplayPutString(map(strlen(crsf_params[param_list[selectIndex]].options[indexValue]), 13, 20, 120, 85), 120, &fontConsolas24h, crsf_params[param_list[selectIndex]].options[indexValue], BLACK, WHITE);

		}
		else
		{
			for(uint8_t i = 0; i <= ELRS_ParamSize; i++)
			{
				if(i < 4)
				{
					if(indexValue == i)
						GFXDisplayPutString(20, (i*20)+ 35, &fontConsolas24h, crsf_params[param_list[i]].name, WHITE, BLACK);
					else
						GFXDisplayPutString(20, (i*20)+ 35, &fontConsolas24h, crsf_params[param_list[i]].name, BLACK, WHITE);
				
					char *value = (char *)malloc(32);
					sprintf(value, "[%s]", crsf_params[param_list[i]].options[crsf_params[param_list[i]].status]);
					GFXDisplayPutString(180, (i*20)+ 35, &fontConsolas24h, value, BLACK, WHITE);
					free(value);
				}
				else
				{
					char *value = (char *)malloc(32);
					sprintf(value, "> %s", crsf_params[param_list[i]].name);
					
					if(indexValue == i)
						GFXDisplayPutText(20, (i*20)+ 35, &Consolas24B, value, WHITE);
					else
						GFXDisplayPutText(20, (i*20)+ 35, &Consolas24B, value, BLACK);
					
					free(value);
				}
			}
		}

			
		// printf("Index: %d\n", indexValue);
		_indexValue = indexValue;
		GFXDisplayUpdate();
	}
	
	
	indexValue = (itemCount - QEI_GET_CNT_VALUE(QEI0)); // itemCount'tan ?ikarma sebebim direction'in ters olmasi
	
	if(ENCODER_SW == 0)
	{
		Delay(50);
		while(ENCODER_SW == 0);
		if(selectIndex > -1)
		{
			if(crsf_params[param_list[selectIndex]].status != indexValue) 		// if value is change? update new value.
			{
				if(selectIndex < 4)
				{
					CRSF_changeParam(param_list[selectIndex], indexValue);													// CRSF value update
//					printf("[Change param] p: %d, val: %d\n", param_list[selectIndex], indexValue); // For debug
//					memset(crsf_params, 0x00, sizeof(crsf_param_t) * 25);	// Clear for new reading
					crsf_params[param_list[selectIndex]].id = 0x00;
				}
			}
			
			_guiMenu = -1;																					// Print page first time
			selectIndex = -1;																				// Out value select screen
		}
		else
		{
			selectIndex = indexValue;
			itemCount = crsf_params[param_list[selectIndex]].count;
			QEI_Stop(QEI0);
			QEI_SET_CNT_VALUE(QEI0, itemCount);
			QEI_SET_CNT_MAX(QEI0, itemCount);
			QEI_Start(QEI0);
		}
	}
	
	if(END_SW_PIN == 0)
	{
		Delay(50);
		while(END_SW_PIN == 0);
		if(selectIndex >= 0)
		{
			selectIndex = -1;
			_indexValue = -1;
			_guiMenu = -1;
		}
		else
			guiMenu = MenuScreen;
	}
}

void DrawTxPowerScreen(void)
{
	uint8_t param_list[3] = {Max_Power, Dynamic, Fan_Threshold};
	if(guiMenu != _guiMenu)
	{
		GFXDisplayAllClear();
		indexValue = 0;
		_indexValue = -1;
		selectIndex = -1;																				// Out value select screen
		_guiMenu = guiMenu;
		
		itemCount = sizeof(param_list) - 1;
		
		GFXDisplayAllClear();
		GFXDisplayPutText(150, 8, &abeatbykai24, "Tx Power", BLACK);
		QEI_Stop(QEI0);
		QEI_SET_CNT_VALUE(QEI0, itemCount);
		QEI_SET_CNT_MAX(QEI0, itemCount);
		QEI_Start(QEI0);
	}
	
	if(crsf_params[TX_Power].id > 0)
	{
		if(indexValue != _indexValue)
		{
			if(selectIndex > -1)
			{	
				GFXDisplayPutImage(80, 100, &value_box, false);
				GFXDisplayPutString(map(strlen(crsf_params[param_list[selectIndex]].options[indexValue]), 13, 20, 120, 85), 120, &fontConsolas24h, crsf_params[param_list[selectIndex]].options[indexValue], BLACK, WHITE);
			}
			else
			{
				for(uint8_t i=0; i<=itemCount; i++)
				{
					if(indexValue == i)
						GFXDisplayPutString(20, (i*20)+ 35, &fontConsolas24h, crsf_params[param_list[i]].name, WHITE, BLACK);
					else
						GFXDisplayPutString(20, (i*20)+ 35, &fontConsolas24h, crsf_params[param_list[i]].name, BLACK, WHITE);
				
					char *value = (char *)malloc(32);
					sprintf(value, "[%s]", crsf_params[param_list[i]].options[crsf_params[param_list[i]].status]);
					GFXDisplayPutString(180, (i*20)+ 35, &fontConsolas24h, value, BLACK, WHITE);
					free(value);
				}
			}
				
			_indexValue = indexValue;
			GFXDisplayUpdate();
			
		}
	}

	
	indexValue = (itemCount - QEI_GET_CNT_VALUE(QEI0)); // itemCount'tan ?ikarma sebebim direction'in ters olmasi
	
	if(ENCODER_SW == 0)
	{
		Delay(50);
		while(ENCODER_SW == 0);
		if(selectIndex > -1)
		{
			if(crsf_params[param_list[selectIndex]].status != indexValue) 		// if value is change? update new value.
			{
				CRSF_changeParam(param_list[selectIndex], indexValue);					// CRSF value update
//				printf("[Change param] p: %d, val: %d\n", param_list[selectIndex], indexValue); // For debug
				crsf_params[param_list[selectIndex]].id = 0x00;
				next_param = 1;	// param start element for new readings background
				next_chunk = 0; // chunk clean element
			}
			_guiMenu = -1;																					// Print page first time
			selectIndex = -1;																				// Out value select screen
		}
		else
		{
			selectIndex = indexValue;
			itemCount = crsf_params[param_list[selectIndex]].count;
			QEI_Stop(QEI0);
			QEI_SET_CNT_VALUE(QEI0, itemCount);
			QEI_SET_CNT_MAX(QEI0, itemCount);
			QEI_Start(QEI0);
		}
	}
	
	if(END_SW_PIN == 0)
	{
		Delay(50);
		while(END_SW_PIN == 0);
		if(selectIndex >= 0)
		{
			selectIndex = -1;
			_indexValue = -1;
			_guiMenu = -1;
		}
		else
			guiMenu = ExpressLRSScreen;
	}
}

void DrawVTxAdminScreen(void)
{
	uint8_t param_list[5] = {Band, Channel, Pwr_Lvl, Pitmode, Send_VTx};
	if(guiMenu != _guiMenu)
	{
		GFXDisplayAllClear();
		indexValue = 0;
		_indexValue = -1;
		selectIndex = -1;																				// Out value select screen
		_guiMenu = guiMenu;
		
		itemCount = sizeof(param_list) - 1;
		
		GFXDisplayAllClear();
		GFXDisplayPutText(100, 8, &abeatbykai24, "VTx Administrator", BLACK);
		QEI_Stop(QEI0);
		QEI_SET_CNT_VALUE(QEI0, itemCount);
		QEI_SET_CNT_MAX(QEI0, itemCount);
		QEI_Start(QEI0);
	}
	
	if(crsf_params[VTX_Administrator].id > 0)
	{
		if(indexValue != _indexValue)
		{
			if(selectIndex > -1)
			{	
				GFXDisplayPutImage(80, 100, &value_box, false);
				GFXDisplayPutString(map(strlen(crsf_params[param_list[selectIndex]].options[indexValue]), 13, 20, 120, 85), 120, &fontConsolas24h, crsf_params[param_list[selectIndex]].options[indexValue], BLACK, WHITE);
			}
			else
			{
				for(uint8_t i=0; i<=itemCount; i++)
				{
					if(crsf_params[param_list[i]].type == 13)
					{
						char *txt = (char *)malloc(32);
						sprintf(txt, "[%s]", crsf_params[param_list[i]].name);
						if(indexValue == i)
							GFXDisplayPutText(20, (i*20)+ 40, &Consolas24B, txt, WHITE);
						else
							GFXDisplayPutText(20, (i*20)+ 40, &Consolas24B, txt, BLACK);
						
						free(txt);
					}
					else
					{
						if(indexValue == i)
							GFXDisplayPutString(20, (i*20)+ 40, &fontConsolas24h, crsf_params[param_list[i]].name, WHITE, BLACK);
						else
							GFXDisplayPutString(20, (i*20)+ 40, &fontConsolas24h, crsf_params[param_list[i]].name, BLACK, WHITE);
						
						char *value = (char *)malloc(32);
						sprintf(value, "[%s]", crsf_params[param_list[i]].options[crsf_params[param_list[i]].status]);
						GFXDisplayPutString(180, (i*20)+ 40, &fontConsolas24h, value, BLACK, WHITE);
						free(value);
					}
				}
			}
				
			_indexValue = indexValue;
			GFXDisplayUpdate();
			
		}
	}

	
	indexValue = (itemCount - QEI_GET_CNT_VALUE(QEI0)); // itemCount'tan ?ikarma sebebim direction'in ters olmasi
	
	if(ENCODER_SW == 0)
	{
		Delay(50);
		while(ENCODER_SW == 0);
		if(selectIndex > -1)
		{
			if(crsf_params[param_list[selectIndex]].status != indexValue) 		// if value is change? update new value.
			{
				CRSF_changeParam(param_list[selectIndex], indexValue);					// CRSF value update
//				printf("[Change param] p: %d, val: %d\n", param_list[selectIndex], indexValue); // For debug
				crsf_params[param_list[selectIndex]].id = 0x00;
				next_param = 1;	// param start element for new readings background
				next_chunk = 0; // chunk clean element
			}
			_guiMenu = -1;																					// Print page first time
			selectIndex = -1;																				// Out value select screen
		}
		else
		{
			selectIndex = indexValue;
			itemCount = crsf_params[param_list[selectIndex]].count;
			QEI_Stop(QEI0);
			QEI_SET_CNT_VALUE(QEI0, itemCount);
			QEI_SET_CNT_MAX(QEI0, itemCount);
			QEI_Start(QEI0);
		}
	}
	
	if(END_SW_PIN == 0)
	{
		Delay(50);
		while(END_SW_PIN == 0);
		if(selectIndex >= 0)
		{
			selectIndex = -1;
			_indexValue = -1;
			_guiMenu = -1;
		}
		else
			guiMenu = ExpressLRSScreen;
	}
}

void DrawWifiConnectivityScreen(void)
{
	uint8_t param_list[2] = {Enable_Wifi, Enable_Rx_Wifi};
	if(guiMenu != _guiMenu)
	{
		GFXDisplayAllClear();
		indexValue = 0;
		_indexValue = -1;
		selectIndex = -1;																				// Out value select screen
		_guiMenu = guiMenu;
		
		itemCount = sizeof(param_list) - 1;
		
		GFXDisplayAllClear();
		GFXDisplayPutText(100, 8, &abeatbykai24, "WiFi Connectivity", BLACK);
		QEI_Stop(QEI0);
		QEI_SET_CNT_VALUE(QEI0, itemCount);
		QEI_SET_CNT_MAX(QEI0, itemCount);
		QEI_Start(QEI0);
	}
	
	if(crsf_params[VTX_Administrator].id > 0)
	{
		if(indexValue != _indexValue)
		{
			if(selectIndex > -1)
			{	
				GFXDisplayPutImage(80, 100, &value_box, false);
				GFXDisplayPutString(map(strlen(crsf_params[param_list[selectIndex]].options[indexValue]), 13, 20, 120, 85), 120, &fontConsolas24h, crsf_params[param_list[selectIndex]].options[indexValue], BLACK, WHITE);
			}
			else
			{
				for(uint8_t i=0; i<=itemCount; i++)
				{
					if(crsf_params[param_list[i]].type == 13)
					{
						char *txt = (char *)malloc(32);
						sprintf(txt, "[%s]", crsf_params[param_list[i]].name);
						if(indexValue == i)
							GFXDisplayPutText(20, (i*20)+ 40, &Consolas24B, txt, WHITE);
						else
							GFXDisplayPutText(20, (i*20)+ 40, &Consolas24B, txt, BLACK);
						
						free(txt);
					}
					else
					{
						if(indexValue == i)
							GFXDisplayPutString(20, (i*20)+ 40, &fontConsolas24h, crsf_params[param_list[i]].name, WHITE, BLACK);
						else
							GFXDisplayPutString(20, (i*20)+ 40, &fontConsolas24h, crsf_params[param_list[i]].name, BLACK, WHITE);
						
						char *value = (char *)malloc(32);
						sprintf(value, "[%s]", crsf_params[param_list[i]].options[crsf_params[param_list[i]].status]);
						GFXDisplayPutString(180, (i*20)+ 40, &fontConsolas24h, value, BLACK, WHITE);
						free(value);
					}
				}
			}
				
			_indexValue = indexValue;
			GFXDisplayUpdate();
			
		}
	}

	
	indexValue = (itemCount - QEI_GET_CNT_VALUE(QEI0)); // itemCount'tan ?ikarma sebebim direction'in ters olmasi
	
	if(ENCODER_SW == 0)
	{
		Delay(50);
		while(ENCODER_SW == 0);
		if(selectIndex > -1)
		{
			if(crsf_params[param_list[selectIndex]].status != indexValue) 		// if value is change? update new value.
			{
				CRSF_changeParam(param_list[selectIndex], indexValue);					// CRSF value update
//				printf("[Change param] p: %d, val: %d\n", param_list[selectIndex], indexValue); // For debug
				crsf_params[param_list[selectIndex]].id = 0x00;
				next_param = 1;	// param start element for new readings background
				next_chunk = 0; // chunk clean element
			}
			_guiMenu = -1;																					// Print page first time
			selectIndex = -1;																				// Out value select screen
		}
		else
		{
			selectIndex = indexValue;
			itemCount = crsf_params[param_list[selectIndex]].count;
			QEI_Stop(QEI0);
			QEI_SET_CNT_VALUE(QEI0, itemCount);
			QEI_SET_CNT_MAX(QEI0, itemCount);
			QEI_Start(QEI0);
		}
	}
	
	if(END_SW_PIN == 0)
	{
		Delay(50);
		while(END_SW_PIN == 0);
		if(selectIndex >= 0)
		{
			selectIndex = -1;
			_indexValue = -1;
			_guiMenu = -1;
		}
		else
			guiMenu = ExpressLRSScreen;
	}
	
	// Enable Wifi
	// Enable RX-wifi
}

void DrawBleJoystickScreen(void)
{
	
	if(guiMenu != _guiMenu)
	{
		_guiMenu = guiMenu;
		if(crsf_params[BLE_Joystick].current_value == READY)
			GFXDisplayPutString(90, 120, &fontConsolas24h, "Joystick Stop!     ", BLACK, WHITE);
		else
			GFXDisplayPutString(90, 120, &fontConsolas24h, crsf_params[BLE_Joystick].s.info, BLACK, WHITE);
		GFXDisplayUpdate();
	}
	
	if(crsf_params[BLE_Joystick].u.status != crsf_params[BLE_Joystick].status)
	{
		next_param = BLE_Joystick;
		next_chunk = crsf_params[BLE_Joystick].current_value;
		command_stat = true;
	}
	else
	{
		if(command_stat)
			_guiMenu = -1;
		command_stat = false;
	}
	
	
	
	if(END_SW_PIN == 0)
	{
		Delay(50);
		while(END_SW_PIN == 0);
		if(crsf_params[BLE_Joystick].u.status == READY)
		{
			if(selectIndex >= 0)
			{
				guiMenu = ExpressLRSScreen;
				selectIndex = -1;
				_indexValue = -1;
				_guiMenu = -1;
			}
			else
				guiMenu = MenuScreen;
		}
		else
		{
			crsf_params[BLE_Joystick].status = READY;
			crsf_params[BLE_Joystick].current_value = READY;
		}
	}
}


void CheckInputsPos(void)
{
		GFXDisplayAllClear();
		Get_ADC(); // For check position
		if((adc_val[THROTTLE] < myTableParams.channels->max - 300) || (GetSW(SW) != SWITCH_HOMES))
		{
			GFXDisplayPutImage(0,60, &warning_screen, false);
			GFXDisplayUpdate();
		}
		while((adc_val[THROTTLE] < myTableParams.channels->max - 300) || (GetSW(SW) != SWITCH_HOMES))
		{
			Get_ADC(); // For check position
//			printf("SW: %d\n", GetSW(SW));
		}
		GFXDisplayAllClear();
		guiMenu = HomeScreen;
}


void GUI_Init(void)
{
		guiMenu = HomeScreen;
		GFXDisplayPowerOn();

//		GFXDisplayPutString(15, 34, &fontConsolas24h, "Welcome to, RadiolinkAT9C", BLACK, WHITE);		
//		GFXDisplayDrawRect(0, 0, 400, 240, BLACK);
		GFXDisplayPutImage(0,50, &kolamuc_logo, false);
		GFXDisplayPutText(35, 10, &abeatbykai24, "Welcome to RadiolinkAT9C", BLACK);
		GFXDisplayPutText(330, 210, &abeatbykai24, VERSION, BLACK);
		GFXDisplayUpdate();
		Delay(1000);
}


void GUI_Update(void)
{
	switch(guiMenu)
	{
		case CheckScreen:
			CheckInputsPos();
		break;
		
		case HomeScreen:
			DrawMainScreen();
			if(MODE_SW_PIN == 0)
			{
				Delay(50);
				guiMenu = MenuScreen;
			}
		break;
			
		case MenuScreen:
			DrawMenuScreen();
		break;
		
		case SDScreen:
			DrawSDCardScreen();
		break;
		
		case SettingScreen:
			DrawSettingScreen();
		break;
		
		case ExpressLRSScreen:
			DrawExpressLRSScreen();
		break;
		
		case ModelSelectScreen:
			if(END_SW_PIN == 0)
			{
				Delay(50);
				while(END_SW_PIN == 0);
				guiMenu = MenuScreen;
			}
		break;
		
		case ModelTypeScreen:
			if(END_SW_PIN == 0)
			{
				Delay(50);
				while(END_SW_PIN == 0);
				guiMenu = MenuScreen;
			}
		break;
		
		case AuxChScreen:
			DrawChannels();
			if(MODE_SW_PIN == 0)
			{
				Delay(50);
				guiMenu = MenuScreen;
			}
		break;
		
		case ReverseScreen:
			DrawJoystick();
			if(MODE_SW_PIN == 0)
			{
				Delay(50);
				guiMenu = MenuScreen;
			}
		break;
		
		case TrainerScreen:
			if(END_SW_PIN == 0)
			{
				Delay(50);
				while(END_SW_PIN == 0);
				guiMenu = MenuScreen;
			}
		break;
		
		case DevOpsScreen:
			if(END_SW_PIN == 0)
			{
				Delay(50);
				while(END_SW_PIN == 0);
				guiMenu = MenuScreen;
			}
		break;
		
		case TxPowerScreen:
			DrawTxPowerScreen();
		break;
		
		case VTxAdminScreen:
			DrawVTxAdminScreen();
		break;
		
		case BleJoystickScreen:
			DrawBleJoystickScreen();
		break;
		
		case BindScreen:
			// Not used
			if(END_SW_PIN == 0)
			{
				Delay(50);
				while(END_SW_PIN == 0);
				guiMenu = MenuScreen;
			}
		break;
		
		case WifiConnectivityScreen:
			DrawWifiConnectivityScreen();
		break;
		
		case QRScreen:
			GFXDisplayDrawRect(0, 0, 400, 240, WHITE);
			GFXDisplayPutImage(120, 0, &qrcode, false);
			GFXDisplayPutText(170, 210, &abeatbykai24, "Scan Me!", BLACK);
			if(END_SW_PIN == 0)
			{
				Delay(50);
				while(END_SW_PIN == 0);
				guiMenu = SettingScreen;
				_guiMenu = -1;
			}
			GFXDisplayUpdate();
		break;
	}
}
