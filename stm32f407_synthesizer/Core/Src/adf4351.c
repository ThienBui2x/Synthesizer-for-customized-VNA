/*
 * adf4351.c
 *
 *  Created on: Jan 13, 2025
 *      Author: ayano
 */

/* Includes ------------------------------------------------------------------*/
#include "adf4351.h"
#include "adf4351_hmi.h"
#include "tim.h"
#include "spi.h"
#include "adc.h"
#include "math.h"

/* Defines -------------------------------------------------------------------*/

/* Variables -----------------------------------------------------------------*/
ADF4351_TypeDef adf4351 = {0};

extern uint8_t volatile is_scanning;
extern uint16_t volatile vmag_val;

/* Function definition -------------------------------------------------------*/

/*
 *	@brief	A mirco_delay function (small errors)
 */
void ADF4351_MicroDelay(uint16_t us)
{
	__HAL_TIM_SET_COUNTER(&adf4351_htim,0);  /* set the counter value a 0 */
	/* wait for the counter to reach the us input in the parameter */
	while (__HAL_TIM_GET_COUNTER(&adf4351_htim) < us-5);
}

/*
 *	@brief	Pull down to assert the LE-Pin
 */
void ADF4351_Assert_LE(void)
{
	HAL_GPIO_WritePin(ADF4351_LE_GPIO_Port, ADF4351_LE_Pin, GPIO_PIN_RESET);
}

/*
 *	@brief	Pull up to deassert the LE-Pin
 */
void ADF4351_Deassert_LE(void)
{
	HAL_GPIO_WritePin(ADF4351_LE_GPIO_Port, ADF4351_LE_Pin, GPIO_PIN_SET);
}

/*
 *	@brief	Transmit SPI signal function of a register
 *	@param	data	the 32-bit register data
 */
void ADF4351_Write(uint32_t data)
{
	/* Assert LE  for whole period ...
	   Write TxData with MSB order (3->0) */

	ADF4351_Assert_LE();
	ADF4351_MicroDelay(10);
	for (int i = 3; i >= 0; i--)
	{
		uint8_t dataByte = data >> (8*i);
		HAL_SPI_Transmit(&adf4351_hspi, &dataByte, 1, 1000);
	}
	ADF4351_Deassert_LE();
	ADF4351_MicroDelay(5);
	ADF4351_Assert_LE();
}

/*
 *	@brief	Send 6 registers data in LSB format
 *	@param	data	32-bit pointer of registers data array
 */
void ADF4351_Registers_Write(uint32_t* data)
{
	for (int i = 5; i >= 0; i--)
	{
		ADF4351_Write(data[i]);
		ADF4351_MicroDelay(2500);
	}
}

/*
 *	@brief	Initialize the 6 registers of ADF4351
 */
void ADF4351_Initialize(void)
{
	HAL_TIM_Base_Start(&adf4351_htim);

	adf4351.ref_freq = REF_FREQ_VAL;

	adf4351.R0.INT			= 23;	/* 16-bits INT value (Min = 23) */
	adf4351.R0.FRAC			= 0;	/* 12-bits FRAC value */

	adf4351.R1.PHASE_ADJUST = PHASE_ADJ_OFF;
	adf4351.R1.PRESCALER	= PRESCALER_8_9;
	adf4351.R1.PHASE		= 1;	/* 12-bits Phase value */
	adf4351.R1.MOD			= 0;	/* 12-bits Modulus value */

	adf4351.R2.LN_LS_MODES	= LOW_NOISE_MODE;
	adf4351.R2.MUXOUT		= MUXOUT_THREE_STATE;
	adf4351.R2.REF_DOUBLER	= REF_DOUBLER_DISABLED;
	adf4351.R2.RDIV2		= REF_DIVIDED_BY_2_DISABLED;
	adf4351.R2.R_COUNTER	= 1;	/* 8-bits: Divided by 8 for 1Khz Resolution */
	adf4351.R2.DOUBLE_BUF	= DOUBLE_BUFFER_DISABLED;
	adf4351.R2.CHARGE_PUMP	= CHARGE_PUMP_CURRENT_2_50;
	adf4351.R2.LDF			= LDF_FRAC_N;
	adf4351.R2.LDP			= LDP_10ns;
	adf4351.R2.PD_POLAR		= PD_POLARITY_POSITIVE;
	adf4351.R2.POW_DOWN		= POWER_DOWN_DISABLED;
	adf4351.R2.THREE_STATE	= CP_THREE_STATE_DISABLED;
	adf4351.R2.COUNTER_RST	= COUNTER_RESET_DISABLED;

	adf4351.R3.SEL_CLK_MODE	= BAND_SEL_CLK_MODE_HIGH;
	adf4351.R3.ABP			= ANTIBACKLASH_PULSE_WIDTH_6ns;
	adf4351.R3.CHARGE_CANCEL= CHARGE_CANCELATION_DISABLED;
	adf4351.R3.CSR			= CYCLE_SLIP_REDUCTION_DISABLED;
	adf4351.R3.CLK_DIV_MODE	= CLK_DIVIDER_OFF;
	adf4351.R3.CLK_DIV		= 150;	/* 12-bits CLK Divider value */

	adf4351.R4.FEEDBACK_SEL = FEEDBACK_SEL_FUNDAMENTAL;
	adf4351.R4.RF_DIV_SEL	= 2;	/* Divided by 2^(RF_DIV_SEL) */
	adf4351.R4.SEL_CLK_DIV	= 80;	/* Use value of 200 / RCounter */
	adf4351.R4.VCO_POW_DOWN	= VCO_POWER_DOWN_POWERED_UP;
	adf4351.R4.MTLD			= MUTE_TILL_LOCK_DETECT_DISABLED;
	adf4351.R4.AUX_OUT_SEL	= AUX_OUTPUT_SEL_DIVIDED_OUTPUT;
	adf4351.R4.AUX_OUT_EN	= AUX_OUTPUT_DISABLED;
	adf4351.R4.AUX_OUT_POW	= AUX_OUTPUT_POWER_MINUS_4dBm;
	adf4351.R4.RF_OUT_EN	= RF_OUTPUT_ENABLED;
	adf4351.R4.OUT_POW		= RF_OUTPUT_POWER_MINUS_4dBm;

	adf4351.R5.LD_PIN_MODE	= LOCK_DETECT_PIN_DIGITAL_LOCK;
	adf4351.R5.RESERVED_HIGH= 3;
}

/*
 *	@brief	Write the 6 registers of ADF4351
 */
void ADF4351_Registers_Update(void)
{
	adf4351.reg[0]	= (uint32_t)(adf4351.R0.INT) 			<< 15 |
					(uint32_t)(adf4351.R0.FRAC)				<< 3  |
					ADF4351_REG0_CTRL_BIT;
	adf4351.reg[1] 	= (uint32_t)(adf4351.R1.PHASE_ADJUST)	<< 28 |
					(uint32_t)(adf4351.R1.PRESCALER) 		<< 27 |
					(uint32_t)(adf4351.R1.PHASE)			<< 15 |
					(uint32_t)(adf4351.R1.MOD)				<< 3  |
					ADF4351_REG1_CTRL_BIT;
	adf4351.reg[2] 	= (uint32_t)(adf4351.R2.LN_LS_MODES)	<< 29 |
					(uint32_t)(adf4351.R2.MUXOUT)			<< 26 |
					(uint32_t)(adf4351.R2.REF_DOUBLER)		<< 25 |
					(uint32_t)(adf4351.R2.RDIV2)			<< 24 |
					(uint32_t)(adf4351.R2.R_COUNTER)		<< 14 |
					(uint32_t)(adf4351.R2.DOUBLE_BUF)		<< 13 |
					(uint32_t)(adf4351.R2.CHARGE_PUMP)		<< 9  |
					(uint32_t)(adf4351.R2.LDF)				<< 8  |
					(uint32_t)(adf4351.R2.LDP)				<< 7  |
					(uint32_t)(adf4351.R2.PD_POLAR)			<< 6  |
					(uint32_t)(adf4351.R2.POW_DOWN)			<< 5  |
					(uint32_t)(adf4351.R2.THREE_STATE)		<< 4  |
					(uint32_t)(adf4351.R2.COUNTER_RST)		<< 3  |
					ADF4351_REG2_CTRL_BIT;
	adf4351.reg[3] 	= (uint32_t)(adf4351.R3.SEL_CLK_MODE)	<< 23 |
					(uint32_t)(adf4351.R3.ABP)				<< 22 |
					(uint32_t)(adf4351.R3.CHARGE_CANCEL)	<< 21 |
					(uint32_t)(adf4351.R3.CSR)				<< 18 |
					(uint32_t)(adf4351.R3.CLK_DIV_MODE) 	<< 15 |
					(uint32_t)(adf4351.R3.CLK_DIV)			<< 3  |
					ADF4351_REG3_CTRL_BIT;
	adf4351.reg[4] = (uint32_t)(adf4351.R4.FEEDBACK_SEL)	<< 23 |
					(uint32_t)(adf4351.R4.RF_DIV_SEL)		<< 20 |
					(uint32_t)(adf4351.R4.SEL_CLK_DIV)		<< 12 |
					(uint32_t)(adf4351.R4.VCO_POW_DOWN)		<< 11 |
					(uint32_t)(adf4351.R4.MTLD)				<< 10 |
					(uint32_t)(adf4351.R4.AUX_OUT_SEL)		<< 9  |
					(uint32_t)(adf4351.R4.AUX_OUT_EN)		<< 8  |
					(uint32_t)(adf4351.R4.AUX_OUT_POW)		<< 6  |
					(uint32_t)(adf4351.R4.RF_OUT_EN)		<< 5  |
					(uint32_t)(adf4351.R4.OUT_POW)			<< 3  |
					ADF4351_REG4_CTRL_BIT;
	adf4351.reg[5] = (uint32_t)(adf4351.R5.LD_PIN_MODE)		<< 22 |
					(uint32_t)(adf4351.R5.RESERVED_HIGH)	<< 19 |
					ADF4351_REG5_CTRL_BIT;

	ADF4351_Registers_Write(adf4351.reg);
}

/*
 *	@brief	Function to calculate the PFD Frequency
 */
uint8_t ADF4351_Calc_PFD_Freq(void)
{
	float pfd_freq = adf4351.ref_freq * ((float)(1.0 + adf4351.R2.REF_DOUBLER)
			/ (float)(adf4351.R2.R_COUNTER * (1.0 + adf4351.R2.RDIV2)));

	if ((pfd_freq > 32000000.0) || (pfd_freq < 125000.0))
	{
		return 1;	/* Wrong pfd_freq */
	}
	else
	{
		adf4351.pfd_freq = pfd_freq;
		return 0;
	}

}

/*
 *	@brief	Function to set the output frequency
 */
uint8_t ADF4351_Set_Out_Freq(double freq)
{
	uint64_t vco_freq = 0;
	uint8_t DIV_val = 1;
	uint8_t RF_DIV_val = 0;	/* log2(DIV_val) */
	uint32_t localosc_ratio = VCO_FREQ_MIN/freq;
	uint16_t INT_val = 0;
	uint16_t FRAC_val = 0;
	uint16_t MOD_val = 0;
	uint64_t calc_freq = 0;
	uint32_t gcd_val = 0;

	if(ADF4351_Calc_PFD_Freq() != 0)
	{
		return 1;	/* Wrong pfd_freq */
	}

	if ((freq > OUT_FREQ_MAX) || (freq < OUT_FREQ_MIN))
	{
		return 1; /* Out of Range */
	}

	/* select the output divider */
	while ( (DIV_val <= localosc_ratio) && (DIV_val <= 64) ) {
		DIV_val *= 2 ;
		RF_DIV_val++  ;
	}
	vco_freq = DIV_val * freq;
	INT_val = (uint16_t)(vco_freq / adf4351.pfd_freq);
	gcd_val = gcd(vco_freq,adf4351.pfd_freq);
	FRAC_val = (vco_freq % adf4351.pfd_freq) / gcd_val;
	MOD_val = adf4351.pfd_freq / gcd_val;

	calc_freq = (INT_val*MOD_val+FRAC_val)*gcd_val/DIV_val;

	adf4351.R0.INT = INT_val;
	adf4351.R0.FRAC = FRAC_val;
	adf4351.R1.MOD = MOD_val;

	/* If operating (freq) above 3,6 Ghz -> PRESCALER_8_9 */
	if (freq > 3600000000/DIV_val)
	{
		adf4351.R1.PRESCALER = PRESCALER_8_9;
	}
	else
	{
		adf4351.R1.PRESCALER = PRESCALER_4_5;
	}

	if (FRAC_val == 0)
	{
		/* 0 < FRAC < (MOD - 1) < 4095
		   0 < MOD < 4095 */
		adf4351.R1.MOD = 2500;

		/* Operate in INT-N mode */
		adf4351.R2.LDP = LDP_6ns;
		adf4351.R2.LDF = LDF_INT_N;
		adf4351.R3.CHARGE_CANCEL = CHARGE_CANCELATION_ENABLED;
		adf4351.R3.ABP = ANTIBACKLASH_PULSE_WIDTH_3ns;
	}
	else
	{
		/* Operate in FRAC-N mode */
		adf4351.R2.LDP = LDP_10ns;
		adf4351.R2.LDF = LDF_FRAC_N;
		adf4351.R3.CHARGE_CANCEL = CHARGE_CANCELATION_DISABLED;
		adf4351.R3.ABP = ANTIBACKLASH_PULSE_WIDTH_6ns;
	}
	adf4351.R4.RF_DIV_SEL = RF_DIV_val;
	adf4351.vco_freq = vco_freq;
	adf4351.calc_freq = calc_freq;

	ADF4351_Registers_Update();

	/* Verifying the calculated RF frequency */
	if(calc_freq == freq) return 0;
	else return 1;
}

/*
 *	@brief	Function to set the RF Power
 *	@param	power see @ref /Register 4/
 */
void ADF4351_Set_RF_Power(uint8_t power)
{
	/* Ignore invalid power values */
	if (power > 3)
		return;
	adf4351.R4.OUT_POW = power;
}

/*
 *	@brief	Function to scan the RF frequency given start and stop frequencies
 *	@param	start	start frequency (not less than 35 KHz)
 *	@param	stop	stop frequency (not more than 4.4 GHz)
 *	@param	step	step frequency
 */
uint8_t ADF4351_Scanning(uint64_t start, uint64_t stop, uint64_t step)
{
	/* wrong scanning period */
	if (stop <= start) return 1;
	else
	{
		uint64_t new_freq = start;
		uint16_t idx = 0;
		is_scanning = TRUE;
		while((new_freq <= stop) && (is_scanning))
		{
			HAL_GPIO_TogglePin(BLUE_LED_GPIO_Port, BLUE_LED_Pin);
			HAL_Delay(50);
			if ( ADF4351_Set_Out_Freq(new_freq) == 1)
			{
				HAL_GPIO_TogglePin(RED_LED_GPIO_Port, RED_LED_Pin);
			}

			ADF4351_DataConvert();

			new_freq = new_freq + step;
			idx++;
		}
		return 1;
	}
}

/*
 *	@brief	Function to convert ADF4351 data using ADC
 */
void ADF4351_DataConvert(void)
{
	HAL_ADC_Start_IT(&adf4351_hadc);
}

/*
 *	@brief	Function to calculate GCD of 2 numbers
 *	@param	a	first number
 *	@param	b	second number
 */
uint64_t gcd(uint64_t a, uint64_t b)
{
    while (b != 0) {
    	uint64_t temp = b;
        b = a % b;
        a = temp;
    }
    return a;
}
