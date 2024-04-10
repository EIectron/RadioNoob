#ifndef COMMON_H
#define COMMON_H

#include "M480.h"
#include "stdbool.h"

#define SUPPORT_CRSF_CONFIG 1
//#define HAS_EXTENDED_TELEMETRY
//#define HAS_EXTENDED_TELEMETRY 1

#define bitset(byte,nbit)   ((byte) |=  (1<<(nbit)))
#define bitclear(byte,nbit) ((byte) &= ~(1<<(nbit)))
#define bitflip(byte,nbit)  ((byte) ^=  (1<<(nbit)))
#define bitcheck(byte,nbit) ((byte) &   (1<<(nbit)))

typedef int8_t s8;
typedef int16_t s16;
typedef int32_t s32;
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

// Define ports
#define ANALOG_PORT 	  PB
#define MODE_SW_PORT		PH
#define END_SW_PORT			PH
#define SW_A_PORT       PA
#define SW_B_PORT       PC
#define SW_C_PORT       PC
#define SW_D_PORT       PE
#define SW_E_PORT       PE
#define SW_F_PORT       PA
#define SW_G_PORT       PE
#define SW_H_PORT       PE
#define T_TRM_PORT      PC
#define Y_TRM_PORT      PC
#define P_TRM_PORT      PE
#define R_TRM_PORT      PH
#define VIB_MTR_PORT    PC
#define LED_GREEN_PORT  PF
#define LED_RED_PORT    PC
#define CHRG_PORT       PH
#define BAT_LVL_PORT    PC
#define DISPLAY_EN_PORT PF
#define DISPLAY_EXCOM_PORT PA
#define SPI0_CS_PORT		PF
#define AUDIO_EN_PORT		PB
#define ENCODER_SW_PORT	PA
#define UART1_DIR_PORT	PA


// Define BITs
#define CH1_BIT					BIT2
#define CH2_BIT					BIT3
#define CH3_BIT					BIT4
#define CH4_BIT					BIT5
#define POT_L_BIT				BIT6
#define POT_R_BIT				BIT7
#define VR_D_BIT				BIT8
#define VR_E_BIT				BIT9
#define IBAT_BIT				BIT10
#define BAT_ADC_BIT 		BIT11
#define MODE_SW_BIT			BIT5
#define END_SW_BIT			BIT4
#define SW_A_BIT				BIT2
#define SW_B_BIT    		BIT6
#define SW_C_UP_BIT     BIT7
#define SW_C_DWN_BIT    BIT8
#define SW_D_BIT        BIT11
#define SW_E_UP_BIT     BIT15
#define SW_E_DWN_BIT    BIT14
#define SW_F_BIT        BIT1
#define SW_G_UP_BIT     BIT13
#define SW_G_DWN_BIT    BIT12
#define SW_H_BIT        BIT10  
#define T_TRM_INC_BIT   BIT5
#define T_TRM_DEC_BIT   BIT4
#define Y_TRM_INC_BIT   BIT3
#define Y_TRM_DEC_BIT   BIT2
#define P_TRM_INC_BIT   BIT8
#define P_TRM_DEC_BIT   BIT9
#define R_TRM_INC_BIT   BIT7
#define R_TRM_DEC_BIT   BIT6
#define VIBRATION_MTR_BIT       BIT13
#define LED_GREEN_BIT           BIT11
#define LED_RED_BIT             BIT1
#define CHRG_EN_BIT             BIT8
#define CHRG_STDBY_BIT          BIT9
#define CHRG_STAT_BIT           BIT10
#define BAT_LVL_BIT             BIT10
#define DISPLAY_EN_BIT          BIT10
#define DISPLAY_EXCOM_BIT				BIT11
#define SPI0_CS_BIT							BIT9
#define AUDIO_EN_BIT						BIT14
#define ENCODER_SW_BIT					BIT5
#define UART1_DIR_BIT						BIT0

// Define pins
#define CH1_PIN						PB2
#define CH2_PIN						PB3
#define CH3_PIN						PB4
#define CH4_PIN						PB5
#define POT_L_PIN					PB6
#define POT_R_PIN					PB7
#define VR_D_PIN					PB8
#define VR_E_PIN					PB9
#define IBAT_PIN					PB10
#define BAT_ADC_PIN		    PB11
#define MODE_SW_PIN				PH5
#define END_SW_PIN				PH4
#define SW_A_PIN	        PA2 
#define SW_B_PIN	        PC6
#define SW_C_UP_PIN	     	PC7
#define SW_C_DWN_PIN	    PC8
#define SW_D_PIN	        PE11
#define SW_E_UP_PIN	     	PE15
#define SW_E_DWN_PIN	    PE14
#define SW_F_PIN	        PA1
#define SW_G_UP_PIN	     	PE13
#define SW_G_DWN_PIN	    PE12
#define SW_H_PIN	        PE10
#define T_TRM_INC_PIN	   	PC5
#define T_TRM_DEC_PIN	    PC4
#define Y_TRM_INC_PIN	   	PC3
#define Y_TRM_DEC_PIN	   	PC2
#define P_TRM_INC_PIN	   	PE8
#define P_TRM_DEC_PIN	   	PE9
#define R_TRM_INC_PIN	   	PH7
#define R_TRM_DEC_PIN	   	PH6
#define VIB_MTR_PIN	     	PC13
#define LED_GREEN_PIN	   	PF11
#define LED_RED_PIN	     	PC1
#define CHRG_EN_PIN	     	PH8
#define CHRG_STDBY_PIN	  PH9
#define CHRG_STAT_PIN	   	PH10
#define BAT_LVL_PIN	     	PC10
#define DISPLAY_EN_PIN	  PF10
#define DISPLAY_EXCOM_PIN PA11
#define SPI0_CS_PIN				PF9
#define AUDIO_EN					PB14
#define ENCODER_SW				PA5
#define UART1_DIR_PIN			PA0


int32_t constrain(int32_t value, int32_t minValue, int32_t maxValue);
long map(long x, long in_min, long in_max, long out_min, long out_max);
int16_t mapJoystickValues(int16_t val, int16_t lower, int16_t middle, int16_t upper, bool reverse);
int16_t mapJoystickCRSFValues(int16_t val, int16_t lower, int16_t middle, int16_t upper, int16_t crsf_min, int16_t crsf_mid, int16_t crsf_max, bool reverse);

#endif