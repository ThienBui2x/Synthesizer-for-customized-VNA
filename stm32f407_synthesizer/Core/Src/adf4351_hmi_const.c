/*
 * adf4351_hmi_const.c
 *
 *  Created on: Jan 21, 2025
 *      Author: ADM
 */

#include "adf4351_hmi.h"

char cursor[2] = ">>";

char welcome[3][12] =
{
	"  ADF4351  \0",
	"Synthesizer\0",
	"  ver 1.0  \0"
};

char initialize[2][12] =
{
	"  ADF4351  \0",
	"Initializes\0"
};

char home[6][24] =
{
	"-------- HOME ---------\0",
    "   Set RF Out Freq     \0",
	"   Scan RF Out Freqs   \0",
	"   Read VMag data      \0",
	"   Plot VMag data      \0",
	"   Set RF Out Power    \0"
};

char set_rf_out[4][24] =
{
	"-----Set RF Range------\0",
	"   StartFreq           \0",
	"   StopFreq            \0",
	"   StepFreq            \0"
};

char set_rf_out1[4][11] =
{
	"          \0",
	"StartFreq \0",
	"StopFreq  \0",
	"StepFreq  \0"
};

char num[10][2] =
{
	"0\0",
	"1\0",
	"2\0",
	"3\0",
	"4\0",
	"5\0",
	"6\0",
	"7\0",
	"8\0",
	"9\0"
};

void (*home_func_ptr[5])(void) =
{
	Set_RF_Out,
	Scan_RF_Out,
	Read_VMAG,
	Plot_VMAG,
	Set_RF_Power
};
