#ifndef GUI_H
#define GUI_H

#include "m480.h"
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include "memory_lcd.h"
#include "delay.h"
#include "inputs.h"
#include "sdcard.h"

#define FPS 10

#define TaskBarHeight	27

#define BatteryPaddingX	365
#define BatteryPaddingY	3

#define VeichleLogoX	230
#define VeichleLogoY	55

#define ELRS_ParamSize	10

#define CHANNEL_MIN	1000
#define CHANNEL_MID 1500
#define CHANNEL_MAX	2000

typedef enum { // <-- the use of typedef is optional
	CheckScreen = 0,
	HomeScreen,
	MenuScreen,
	SDScreen,
	SettingScreen,
	ExpressLRSScreen,
	ModelSelectScreen,
	ModelTypeScreen,
	AuxChScreen,
	ReverseScreen,
	TrainerScreen,
	DevOpsScreen,
	TxPowerScreen,
	VTxAdminScreen,
	BleJoystickScreen,
	BindScreen,
	WifiConnectivityScreen,
	QRScreen
} guiMenu_t;


static const char *MenuItems[] = {
	"1- SD CARD",
	"2- SETTING",
	"3- ExpressLRS",
	"4- MODEL SELECT",
	"5- MODEL TYPE",
	"6- AUX/CH",
	"7- REVERSE",
	"8- TRAINER",
	"9- DEV OPS"
};		

static const char *SettingItems[] = {
	"1- Joystick Calibration",
	"2- About Me"
};


//void DrawMainScreen(void);
//void DrawSettingScreen(void);
void GUI_Init(void);
void GUI_Update(void);


#endif