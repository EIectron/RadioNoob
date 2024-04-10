#ifndef MEMORY_LCD_H
#define MEMORY_LCD_H


#include "M480.h"
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include "math.h"
#include "delay.h"
#include "bfcFontMgr.h"
#include "tImage.h"


/**
 * @note  Define any model below and recompile<br>
 *        	LS027B7DH01 = 2.7" Memory LCD with 400*240 pixels<br>
 *        	LS032B7DD02 = 3.16" Memory LCD with 336*536 pixels<br>
 *        	LS044Q7DH01 = 4.4" Memory LCD with 320*240 pixels<br>
 * 					LS006B7DH03 = 0.56" Memory LCD with 64*64 pixels, 3V input voltage<br>
 * 					LS011B7DH03 = 1.08" Memory LCD with 160*68, 3V input voltage(br>)
 */
#define   LS027B7DH01
//#define	LS032B7DD02
//#define 	LS044Q7DH01
//#define 	LS006B7DH03
//#define 	LS011B7DH03

#define SPI_FREQ	2000000 // 2Mhz
//#define SPI_FREQ	1000000 // 1Mhz

// This typedef holds the hardware parameters. For GPIO and SPI
typedef struct {
	SPI_T 			 				 *Bus;
	volatile uint32_t 	 *SS;
	volatile uint32_t		 *EN;
	volatile uint32_t		 *SYNC;
	bool		 vcom;
}lcdConfig;

#ifdef LS027B7DH01
	#define DISP_HOR_RESOLUTION	400
	#define DISP_VER_RESOLUTION	240
#elif defined LS032B7DD02
	#define DISP_HOR_RESOLUTION	336
	#define DISP_VER_RESOLUTION	536
#elif defined LS044Q7DH01
	#define DISP_HOR_RESOLUTION	320
	#define DISP_VER_RESOLUTION	240
#elif defined LS006B7DH03
	#define DISP_HOR_RESOLUTION	64
	#define DISP_VER_RESOLUTION	64
#elif defined LS011B7DH03
	#define DISP_HOR_RESOLUTION	160
	#define DISP_VER_RESOLUTION	68
#else
	#error You need to define the horizontal and vertical resolution for a new model
#endif


//@note Horizontal screen size in byte count
#define GFX_FB_CANVAS_W	((DISP_HOR_RESOLUTION + 7) / 8)
//@note Vertical screen size in line number
#define GFX_FB_CANVAS_H	DISP_VER_RESOLUTION
//@note EXTCOMIN pulse frequency in -> GFXDisplayOn()
#define EXTCOMIN_FREQ 1 


extern uint8_t frameBuffer[GFX_FB_CANVAS_H][GFX_FB_CANVAS_W];
extern uint8_t frameBuffer_[GFX_FB_CANVAS_H][GFX_FB_CANVAS_W];

// Bitmaps
extern const tImage kolamuc_logo;
extern const tImage antenna;
extern const tImage battery_chrg;
extern const tImage battery;
extern const tImage main_screen;
extern const tImage	warning_screen;
extern const tImage multirotor;
extern const tImage anten_logo;
extern const tImage usb_logo;
extern const tImage user_logo;
extern const tImage arrow_l;
extern const tImage arrow_d;
extern const tImage arrow_r;
extern const tImage value_box;
extern const tImage qrcode;

// Fonts
extern const BFC_FONT fontConsolas24h;
extern const tFont Arcade36;
extern const tFont abeatbykai24;
extern const tFont Consolas24B;


typedef enum
{
	BLACK = 0,
	WHITE,
	TRANSPARENT	//means leaving original color
} COLOR;


static inline int32_t MIN(int16_t a, int16_t b);
static inline int32_t MAX(int16_t a, int16_t b);

/**
 * @note	functions to be implemented by individual hardware platform
 */
void 		spi_write_byte(uint8_t value);
void    extcom_start(void);
void    extcom_stop(void);
void	  extcom_toggle(void);
void 		GFXDisplayUpdate(void);


/**
********************************************************************************************************
* @note	API functions
********************************************************************************************************
*/
void GFXDisplayAllClear(void);
void GFXDisplayPowerOn(void);
void GFXDisplayOn(void);	
void GFXDisplayPowerOff(void);
void GFXDisplayOff(void);
void GFXDisplayPutPixel(uint16_t x, uint16_t y, COLOR color);
void GFXDisplayLineDrawH(uint16_t x1, uint16_t x2, uint16_t y, COLOR color, uint8_t thick);
void GFXDisplayLineDrawV(uint16_t x, uint16_t y1, uint16_t y2, COLOR color, uint8_t thick);
void GFXDisplayDrawRect(uint16_t left, uint16_t top, uint16_t right, uint16_t bottom, COLOR color);
//void GFXDisplayPutPicture(uint16_t left, uint16_t top, const uint8_t* data, bool invert);
void GFXDisplayPutImage(uint16_t left, uint16_t top, const tImage* image, bool invert);
uint32_t GFXDisplayTestPattern(uint8_t pattern, void (*pfcn)(void));
uint16_t GFXDisplayPutText(int x, int y, const tFont *font, const char *str, bool invert);	
uint16_t GFXDisplayPutChar(uint16_t x, uint16_t y, const BFC_FONT* pFont, const uint16_t ch, COLOR color, COLOR bg);
uint16_t GFXDisplayPutString(uint16_t x, uint16_t y, const BFC_FONT* pFont, const char *str, COLOR color, COLOR bg);
uint16_t GFXDisplayPutWString(uint16_t x, uint16_t y, const BFC_FONT* pFont, const uint16_t *str, COLOR color, COLOR bg);

uint16_t GFXDisplayGetLCDWidth(void);
uint16_t GFXDisplayGetLCDHeight(void);
uint16_t GFXDisplayGetCharWidth(const BFC_FONT *pFont, const uint16_t ch);
uint16_t GFXDisplayGetFontHeight(const BFC_FONT *pFont);
uint16_t GFXDisplayGetStringWidth(const BFC_FONT *pFont, const char *str);
uint16_t GFXDisplayGetWStringWidth(const BFC_FONT *pFont, const uint16_t *str);


#endif //MEMORY_LCD_H

