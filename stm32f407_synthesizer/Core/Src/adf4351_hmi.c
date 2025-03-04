/*
 * adf4351_hmi.c
 *
 *  Created on: Jan 21, 2025
 *      Author: ADM
 */

#include "adf4351_hmi.h"
#include "adf4351.h"
#include "stdlib.h"

volatile hmi_status hmi;

uint16_t volatile vmag[SAMPLES+OFFSET] = {0};
uint16_t volatile vmag_ave = 0;
uint16_t volatile vmag_idx = 0;
uint8_t  volatile vmag_offset = 5;
uint16_t volatile vmag_max_val = 0;
uint16_t volatile vmag_min_val = 0;

uint8_t volatile y_min_val = Y_AXIS_TOP;	// top of y-axis
uint8_t volatile y_max_val = Y_AXIS_BOT-1; // bottom of y-axis
uint8_t volatile y_mean_val = 0;
uint8_t volatile y_curr_val = 0;
uint8_t volatile x_curr_val = 0;
uint8_t volatile y_prev_val = 0;
uint8_t volatile x_prev_val = 0;

extern char cursor[2];
extern char welcome[3][12];
extern char initialize[2][12];
extern char home[6][24];

extern void (*home_func_ptr[5])(void);

void Clear_Screen(void)
{
	fill_rectangle(0, 0, WIDTH, HEIGHT, COLOR_BG);
}

void Clear_Screen_Custom(uint8_t x_s, uint8_t y_s, uint8_t x_e, uint8_t y_e)
{
	fill_rectangle(x_s, y_s, x_e, y_e, COLOR_BG);
}

void Set_Cursor(uint8_t pos)
{
	hmi.cur_pos = pos;
	draw_string(ROW_OFFSET, COL_OFFSET+CHAR_HEIGHT_SMALL*pos, COLOR_SYMBOL, 1, cursor);
}

void Clear_Cursor(uint8_t pos)
{
	draw_string(ROW_OFFSET, COL_OFFSET+CHAR_HEIGHT_SMALL*pos, COLOR_BG, 1, cursor);
}

void HMI_Init(void)
{
	Clear_Screen();
	hmi.state_hold	= TRUE;
	hmi.button 		= NONE;
	hmi.cur_pos 	= 0;
	hmi.cur_start 	= 0;
	hmi.cur_end		= 0;
	hmi.curr_state	= NULL;
	hmi.prev_state	= NULL;
	hmi.while_break = TRUE;
}

void Welcome(void)
{
	/* Welcome screen */
	uint8_t col_off = COL_OFFSET + 20;
	uint8_t rows = sizeof(welcome) / sizeof(welcome[0]);
	Clear_Screen();
	for(int idx = 0; idx < rows; idx++)
	{
		draw_string(ROW_OFFSET, col_off+CHAR_HEIGHT_LARGE*idx, COLOR_TITLE, 2, welcome[idx]);
	}
	HAL_Delay(1000);

	/* ADF4351 Initialization */
	rows = sizeof(initialize) / sizeof(initialize[0]);
	Clear_Screen();
	for(int idx = 0; idx < rows; idx++)
	{
		draw_string(ROW_OFFSET, col_off+CHAR_HEIGHT_LARGE*idx, COLOR_TITLE, 2, initialize[idx]);
	}
	ADF4351_Initialize();
	ADF4351_Set_Out_Freq(920000000);
	HAL_Delay(500);

	/* Switching to next stage */
	hmi.curr_state = Home;
	hmi.state_hold	= FALSE;
}

/*
 * @brief	Main menu for RF Synthesizer
 * 			Current/Up-comming stages:
 * 				- Set_RF_Out	- TODO
 * 				- Scan_RF_Out	- TODO
 * 				- Read_VMAG		- TODO
 * 				- Plot_VMAG		- DONE
 * 				- Set_RF_Power	- TODO
 * @note	BACK-BUT	N/A
 * 			RIGHT-BUT	N/A
 * 			UP-BUT		Move cursor up
 * 			FORW-BUT	Move cursor down
 * 			OK-BUT		Switch to stage at current cursor
 */
void Home(void)
{
	hmi.button = NONE;
	hmi.state_hold = TRUE;
	uint8_t rows = sizeof(home) / sizeof(home[0]);
	Clear_Screen();
	Set_Cursor(LINE1);
	hmi.cur_start = hmi.cur_pos;
	hmi.cur_end = rows-1;	// Skip the first line
	for(int idx = 0; idx < rows; idx++)
	{
		draw_string(ROW_OFFSET, COL_OFFSET+CHAR_HEIGHT_SMALL*idx, COLOR_TEXT, 1, home[idx]);
	}

	while(hmi.state_hold)
	{
		switch(hmi.button)
		{
			case (BACK):
				hmi.button = NONE;
				break;

			case (UP):
				hmi.button = NONE;
				if(hmi.cur_pos > hmi.cur_start)
				{
					Clear_Cursor(hmi.cur_pos);
					hmi.cur_pos--;
					Set_Cursor(hmi.cur_pos);
				}
				break;

			case (DOWN):
				hmi.button = NONE;
				if(hmi.cur_pos < hmi.cur_end)
				{
					Clear_Cursor(hmi.cur_pos);
					hmi.cur_pos++;
					Set_Cursor(hmi.cur_pos);
				}
				break;

			case (OK):
				hmi.button = NONE;
				hmi.curr_state = home_func_ptr[hmi.cur_pos-1];
				hmi.state_hold = FALSE;
				break;

			case (FORW):
				hmi.button = NONE;
				break;

			default:
				break;
		}
	}
}

/*
 * @brief	Plotting VMAG values in time-domain using ADC
 * 			based on the scanned frequency.
 * @note	BACK-BUT	Returns to previous state
 * 			RIGHT-BUT	N/A
 * 			UP-BUT		N/A
 * 			FORW-BUT	Stop plotting
 * 			OK-BUT		Start plotting
 */
void Plot_VMAG(void)
{
	hmi.button = NONE;
	hmi.state_hold = TRUE;

	/* Reset the index when starting plotting */
	vmag_idx = 0;
	vmag_ave = 0;

	/* NOTE: there is no need for cursor operation in this stage */
	Clear_Screen();

	/* These texts in 'CHAR_HEIGHT_SMALL' format are optional */
	/* There are maximum 3 lines are available!! - see the comments */
	draw_string(2, 2+CHAR_HEIGHT_SMALL*0, COLOR_TEXT, 1, "     VMAG-PLOTTING     \0");
//	draw_string(2, 2+CHAR_HEIGHT_SMALL*1, COLOR_TEXT, 1, "   Set RF Out Freq     \0");
//	draw_string(2, 2+CHAR_HEIGHT_SMALL*2, COLOR_TEXT, 1, "Hello-Hello\0");

	/* --Plotting xy-axis--
	 * Top of graph (x,y): (10,70)
	 * End of graph (x,y): (210, 170)
	 * --> Total 200 (= 210-10) samples are drawn per time.
	*/
	// Draw y-axis
	for (int i = Y_AXIS_TOP; i <= Y_AXIS_BOT; i++)
	{
	  draw_pixel(X_AXIS_LEFT, i, COLOR_TITLE);
	}
	// Draw x-axis
	for (int i = X_AXIS_LEFT; i <= X_AXIS_RIGHT; i++)
	{
	  draw_pixel(i, Y_AXIS_BOT, COLOR_TITLE);
	}

	while(hmi.state_hold)
	{
		switch(hmi.button)
		{
			case (BACK):
				hmi.button = NONE;
				hmi.curr_state = Home;
				hmi.state_hold = FALSE;
				break;

			case (UP):
				hmi.button = NONE;
				break;

			case (DOWN):
				hmi.button = NONE;
				break;

			case (OK):
				hmi.button = NONE;
				while(hmi.while_break == FALSE)
				{
					HAL_GPIO_TogglePin(BLUE_LED_GPIO_Port, BLUE_LED_Pin);
					ADF4351_DataConvert();
					HAL_Delay(100);
				}
				break;

			case (FORW):
				hmi.button = NONE;
				break;

			default:
				break;
		}
	}
}
void Set_RF_Out(void)
{
	// TODO
}
void Scan_RF_Out(void)
{
	// TODO
}

void Read_VMAG(void)
{
	// TODO
}

/*
 * @brief	Auxiliary function to assist the plotting functionality
 */
void Bresenham_line_algorithm(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2)
{
    int dx = abs(x2 - x1), dy = abs(y2 - y1);
    int sx = (x1 < x2) ? 1 : -1;
    int sy = (y1 < y2) ? 1 : -1;
    int err = dx - dy;

    int max_steps = dx + dy + 1;  // Safety limit (max possible steps)
    int steps = 0;

    while (steps < max_steps) {  // Ensures loop does not run forever
    	draw_pixel(x1, y1, COLOR_SYMBOL);

        if (x1 == x2 && y1 == y2) break;

        int e2 = 2 * err;
        if (e2 > -dy) { err -= dy; x1 += sx; }
        if (e2 < dx) { err += dx; y1 += sy; }

        steps++;
    }
}

void Set_RF_Power(void)
{
	// TODO
}











