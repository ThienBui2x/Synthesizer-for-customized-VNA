/*
 * ADF4351.h
 *
 *  Created on: Dec 16, 2024
 *      Author: ayano
 */

#ifndef INC_ADF4351_H_
#define INC_ADF4351_H_

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "math.h"

/* Defines -------------------------------------------------------------------*/
#define adf4351_htim htim1
#define adf4351_hspi hspi1
#define adf4351_hadc hadc1

#define REF_FREQ_VAL	25000000.0		// 25Mhz Crystal Oscillator
#define REF_FREQ_ERR	500.0			// Adjust to Ref Freq Error
#define VCO_FREQ_MIN	2200000000.0	// 2.2 Ghz
#define VCO_FREQ_MAX	4400000000.0	// 4.4 Ghz -> use 64-bits
#define OUT_FREQ_MIN	VCO_FREQ_MIN / 64	// ~34 Mhz
#define OUT_FREQ_MAX	VCO_FREQ_MAX

#define ADF4351_REG0_CTRL_BIT			0x00000000
#define ADF4351_REG1_CTRL_BIT			0x00000001
#define ADF4351_REG2_CTRL_BIT			0x00000002
#define ADF4351_REG3_CTRL_BIT			0x00000003
#define ADF4351_REG4_CTRL_BIT			0x00000004
#define ADF4351_REG5_CTRL_BIT			0x00000005

/* Register 1 */
#define PHASE_ADJ_OFF					0
#define PHASE_ADJ_ON					1

#define PRESCALER_4_5					0	// Pre-scaler = 4/5
#define PRESCALER_8_9					1	// Pre-scaler = 8/9

/* Register 2 */
#define LOW_NOISE_MODE					0
#define LOW_SPUR_MODE					3

#define MUXOUT_THREE_STATE				0
#define MUXOUT_D_VDD					1
#define MUXOUT_D_GND					2
#define MUXOUT_R_COUNTER				3
#define MUXOUT_N_DIVIDER				4
#define MUXOUT_ANALOG_LOCK_DETECT		5
#define MUXOUT_DIGITAL_LOCK_DETECT		6

#define REF_DOUBLER_DISABLED			0
#define REF_DOUBLER_ENABLED				1

#define REF_DIVIDED_BY_2_DISABLED		0
#define REF_DIVIDED_BY_2_ENABLED		1

#define DOUBLE_BUFFER_DISABLED			0
#define DOUBLE_BUFFER_ENABLED			1

#define CHARGE_PUMP_CURRENT_0_31		0	// I(CP) (mA) - 5.1 kOhm
#define CHARGE_PUMP_CURRENT_0_63		1
#define CHARGE_PUMP_CURRENT_0_94		2
#define CHARGE_PUMP_CURRENT_1_25		3
#define CHARGE_PUMP_CURRENT_1_56		4
#define CHARGE_PUMP_CURRENT_1_88		5
#define CHARGE_PUMP_CURRENT_2_19		6
#define CHARGE_PUMP_CURRENT_2_50		7
#define CHARGE_PUMP_CURRENT_2_81		8
#define CHARGE_PUMP_CURRENT_3_13		9
#define CHARGE_PUMP_CURRENT_3_44		10
#define CHARGE_PUMP_CURRENT_3_75		11
#define CHARGE_PUMP_CURRENT_4_06		12
#define CHARGE_PUMP_CURRENT_4_38		13
#define CHARGE_PUMP_CURRENT_4_69		14
#define CHARGE_PUMP_CURRENT_5_00		15

#define LDF_FRAC_N						0
#define LDF_INT_N						1

#define LDP_10ns						0
#define LDP_6ns							1

#define PD_POLARITY_NEGATIVE			0
#define PD_POLARITY_POSITIVE			1

#define POWER_DOWN_DISABLED				0
#define POWER_DOWN_ENABLED				1

#define CP_THREE_STATE_DISABLED			0
#define CP_THREE_STATE_ENABLED			1

#define COUNTER_RESET_DISABLED			0
#define COUNTER_RESET_ENABLED			1

/* Register 3 */
#define BAND_SEL_CLK_MODE_LOW			0
#define BAND_SEL_CLK_MODE_HIGH			1

#define ANTIBACKLASH_PULSE_WIDTH_6ns 	0	// FRAC-N
#define ANTIBACKLASH_PULSE_WIDTH_3ns 	1	// INT-N

#define CHARGE_CANCELATION_DISABLED		0
#define CHARGE_CANCELATION_ENABLED		1

#define CYCLE_SLIP_REDUCTION_DISABLED 	0
#define CYCLE_SLIP_REDUCTION_ENABLED	1

#define CLK_DIVIDER_OFF					0
#define CLK_DIVIDER__FAST_LOCK_ENABLED	1
#define CLK_DIVIDER_RESYNC_ENABLED		2

/* Register 4 */
#define FEEDBACK_SEL_DIVIDED			0
#define FEEDBACK_SEL_FUNDAMENTAL		1

#define VCO_POWER_DOWN_POWERED_UP		0
#define VCO_POWER_DOWN_POWERED_DOWN		1

#define MUTE_TILL_LOCK_DETECT_DISABLED	0
#define MUTE_TILL_LOCK_DETECT_ENABLED	1

#define AUX_OUTPUT_SEL_DIVIDED_OUTPUT	0
#define AUX_OUTPUT_SEL_FUNDAMENTAL		1

#define AUX_OUTPUT_DISABLED				0
#define AUX_OUTPUT_ENABLED				1

#define AUX_OUTPUT_POWER_MINUS_4dBm		0
#define AUX_OUTPUT_POWER_MINUS_1dBm		1
#define AUX_OUTPUT_POWER_PLUS_2dBm		2
#define AUX_OUTPUT_POWER_PLUS_5dBm		3

#define RF_OUTPUT_DISABLED				0
#define RF_OUTPUT_ENABLED				1

#define RF_OUTPUT_POWER_MINUS_4dBm		0
#define RF_OUTPUT_POWER_MINUS_1dBm		1
#define RF_OUTPUT_POWER_PLUS_2dBm		2
#define RF_OUTPUT_POWER_PLUS_5dBm		3

/* Register 5 */
#define LOCK_DETECT_PIN_LOW				0
#define LOCK_DETECT_PIN_DIGITAL_LOCK	1
//#define LOCK_DETECT_PIN_LOW				2
#define LOCK_DETECT_PIN_HIGH			3

typedef struct
{
	__IO uint16_t FRAC;			// Fractional Value 	Bits[3:14]
	__IO uint16_t INT;			// Integer Value		Bits[15:30]
} ADF4351_REG0_TypeDef;

typedef struct
{
	__IO uint16_t MOD;			// Modulus Value		Bits[3:14]
	__IO uint16_t PHASE;		// Phase Value			Bits[15:26]
	__IO uint8_t  PRESCALER;	// Pre-scaler Bit 		Bits[27]
	__IO uint8_t  PHASE_ADJUST; // Phase Adjust Bit		Bits[28]
} ADF4351_REG1_TypeDef;

typedef struct
{
	__IO uint8_t  COUNTER_RST;	// Counter Reset Bit	Bits[3]
	__IO uint8_t  THREE_STATE;	// CP Three State Bit	Bits[4]
	__IO uint8_t  POW_DOWN;		// Power Down Bit		Bits[5]
	__IO uint8_t  PD_POLAR;		// PD Polarity Bit		Bits[6]
	__IO uint8_t  LDP;			// LDP					Bits[7]
	__IO uint8_t  LDF;			// LDF					Bits[8]
	__IO uint8_t  CHARGE_PUMP;	// Charge Pump Setting  Bits[9:12]
	__IO uint8_t  DOUBLE_BUF;	// Double Buffer Bit	Bits[13]
	__IO uint16_t R_COUNTER;	// R Counter 			Bits[14:23]
	__IO uint8_t  RDIV2;		// RDIV2 Bit			Bits[24]
	__IO uint8_t  REF_DOUBLER;	// Reference Doubler	Bits[25]
	__IO uint8_t  MUXOUT;		// MUXOUT				Bits[26:28]
	__IO uint8_t  LN_LS_MODES;	// Low Noise - Low Spur Bits[29:30]
} ADF4351_REG2_TypeDef;

typedef struct
{
	__IO uint16_t CLK_DIV;		// Clock Divider Value	Bits[3:14]
	__IO uint8_t  CLK_DIV_MODE;	// Clock Divider Mode	Bits[15:16]
	__IO uint8_t  CSR;			// CSR Bit				Bits[18]
	__IO uint8_t  CHARGE_CANCEL;// Charge Cancel Bit	Bits[21]
	__IO uint8_t  ABP;			// ABP Bit				Bits[22]
	__IO uint8_t  SEL_CLK_MODE;	// Band Select Clock	Bits[23]
} ADF4351_REG3_TypeDef;

typedef struct
{
	__IO uint8_t  OUT_POW;		// Output Power			Bits[3:4]
	__IO uint8_t  RF_OUT_EN;	// RF Output Enable Bit	Bits[5]
	__IO uint8_t  AUX_OUT_POW;	// Aux Output Power     Bits[6:7]
	__IO uint8_t  AUX_OUT_EN;	// Aux Output Enable 	Bits[8]
	__IO uint8_t  AUX_OUT_SEL;	// Aux Output Select	Bits[9]
	__IO uint8_t  MTLD;			// MTLD Bit				Bits[10]
	__IO uint8_t  VCO_POW_DOWN;	// VCO Power Down		Bits[11]
	__IO uint8_t  SEL_CLK_DIV;	// Band Select Clk Div	Bits[12:19]
	__IO uint8_t  RF_DIV_SEL;	// RF Divider Select	Bits[20:22]
	__IO uint8_t  FEEDBACK_SEL; // Feedback Select		Bits[23]
} ADF4351_REG4_TypeDef;

typedef struct
{
	__IO uint8_t RESERVED_HIGH;	// Reserved High bits	Bits[19:20]
	__IO uint8_t LD_PIN_MODE;	// LD Pin Mode 			Bits[22:23]
} ADF4351_REG5_TypeDef;

typedef struct
{
	ADF4351_REG0_TypeDef R0;
	ADF4351_REG1_TypeDef R1;
	ADF4351_REG2_TypeDef R2;
	ADF4351_REG3_TypeDef R3;
	ADF4351_REG4_TypeDef R4;
	ADF4351_REG5_TypeDef R5;
	uint32_t reg[6];
	uint32_t 	ref_freq;
	uint32_t 	pfd_freq;
	uint32_t	vco_freq;
	uint64_t	calc_freq;
	uint64_t	scan_freq;
} ADF4351_TypeDef;

/* Variables -----------------------------------------------------------------*/
extern ADF4351_TypeDef adf4351;

/* Function declaration ------------------------------------------------------*/
void ADF4351_MicroDelay(uint16_t us);
void ADF4351_Assert_LE(void);
void ADF4351_Deassert_LE(void);
void ADF4351_Write(uint32_t data);
void ADF4351_Registers_Write(uint32_t* data);
void ADF4351_Initialize(void);
void ADF4351_Registers_Update(void);
uint8_t ADF4351_Calc_PFD_Freq(void);
uint8_t ADF4351_Set_Out_Freq(double freq);
void ADF4351_Set_RF_Power(uint8_t power);
uint8_t ADF4351_Scanning(uint32_t start, uint32_t stop, uint32_t step);
void ADF4351_DataConvert(void);
uint64_t gcd(uint64_t a, uint64_t b);
uint16_t denominator(double number);
uint16_t closest_power_of_2(uint16_t n);

#ifdef __cplusplus
}
#endif

#endif /* INC_ADF4351_H_ */
