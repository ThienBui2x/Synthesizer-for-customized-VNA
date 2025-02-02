/*
 * adf4351_hmi.c
 *
 *  Created on: Jan 21, 2025
 *      Author: ADM
 */

#include "adf4351_hmi.h"

volatile hmi_status hmi;

extern char cursor[2];
extern char welcome[3][12];
extern char home[6][24];
extern void (*home_func_ptr[5])(void);

void Clear_Screen(void)
{
	fill_rectangle(0, 0, WIDTH, HEIGHT, COLOR_BG);
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
}

void Welcome(void)
{
	uint8_t col_off = COL_OFFSET + 20;
	uint8_t rows = sizeof(welcome) / sizeof(welcome[0]);
	Clear_Screen();
	for(int idx = 0; idx < rows; idx++)
	{
		draw_string(ROW_OFFSET, col_off+CHAR_HEIGHT_LARGE*idx, COLOR_TITLE, 2, welcome[idx]);
	}

	hmi.curr_state = Home;
	hmi.state_hold	= FALSE;
}

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
//				hmi.prev_state = hmi.curr_state;
				hmi.curr_state = home_func_ptr[hmi.cur_pos-1];
				hmi.state_hold = FALSE;
				break;

			case (FORW):
				hmi.button = NONE;
				break;

			default:
//				hmi.button = NONE;
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
void Plot_VMAG(void)
{
	// TODO
}
void Set_RF_Power(void)
{
	// TODO
}











