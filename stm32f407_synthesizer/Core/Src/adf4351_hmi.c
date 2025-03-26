/*
 * adf4351_hmi.c
 *
 *  Created on: Jan 21, 2025
 *      Author: ADM
 */

/* Includes ------------------------------------------------------------------*/
#include "adf4351_hmi.h"
#include "adf4351.h"
#include "stdlib.h"
#include "stdio.h"
#include "inttypes.h"
#include "string.h"

/* Variables -----------------------------------------------------------------*/
volatile hmi_status hmi;

uint16_t volatile vmag[1000] = {0};
uint16_t volatile vmag_val = 0;
uint16_t volatile vmag_min = 65535;
uint16_t volatile vmag_ave = 0;
uint16_t volatile vmag_idx = 0;
uint8_t  volatile vmag_offset = 5;
uint16_t volatile vmag_upper_bound = 0;
uint16_t volatile vmag_lower_bound = 0;

uint8_t volatile y_min_val = Y_AXIS_TOP;	/* top of y-axis 	*/
uint8_t volatile y_max_val = Y_AXIS_BOT-1;  /* bottom of y-axis */
uint8_t volatile y_mean_val = 0;
uint8_t volatile y_curr_val = 0;
uint8_t volatile x_curr_val = 0;
uint8_t volatile y_prev_val = 0;
uint8_t volatile x_prev_val = 0;

uint8_t is_default = TRUE;

extern char cursor[2];
extern char welcome[3][12];
extern char initialize[2][12];
extern char home[4][24];
extern char set_rf_out[4][24];
extern char set_rf_out1[4][11];
extern char num[10][2];

extern void (*home_func_ptr[3])(void);

/* Function definition -------------------------------------------------------*/

/*
 * @brief	Initialization of LCD Display
 */
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

/*
 * @brief	Welcome and Initialization of ADF4351 Synthesiszer
 */
void Welcome(void)
{
	/* Welcome screen */
	uint8_t row_off = ROW_OFFSET + 20;
	uint8_t rows = sizeof(welcome) / sizeof(welcome[0]);

	/* NOTE: there is no need for cursor operation in this stage */
	Clear_Screen();
	for(int idx = 0; idx < rows; idx++)
	{
		draw_string(COL_OFFSET, row_off+CHAR_HEIGHT_LARGE*idx, COLOR_TITLE, 2, welcome[idx]);
	}
	HAL_Delay(DISPLAY_TIME);

	/* ADF4351 Initialization */
	rows = sizeof(initialize) / sizeof(initialize[0]);
	Clear_Screen();
	for(int idx = 0; idx < rows; idx++)
	{
		draw_string(COL_OFFSET, row_off+CHAR_HEIGHT_LARGE*idx, COLOR_TITLE, 2, initialize[idx]);
	}
	ADF4351_Initialize();
	ADF4351_Set_Out_Freq(DEFAULT_FREQ);	/* Default RF_Out_Freq */
	HAL_Delay(DISPLAY_TIME);

	/* Switching to next stage */
	hmi.curr_state = Home;
	hmi.state_hold	= FALSE;
}

/*
 * @brief	Main menu for RF Synthesizer
 * 			Current/Up-comming stages:
 * 				- Set_RF_Out
 * 				- Scan_RF_Out
 * 				- Plot_VMAG
 * @note	BACK-BUT	N/A
 * 			RIGHT-BUT	N/A
 * 			UP-BUT		Move cursor up
 * 			FORW-BUT	Move cursor down
 * 			OK-BUT		Switch to stage at current cursor
 */
void Home(void)
{
	HAL_Delay(DEBOUNCE_TIME);
	hmi.button = NONE;
	hmi.state_hold = TRUE;
	uint8_t rows = sizeof(home) / sizeof(home[0]);
	Clear_Screen();
	Set_Cursor(LINE1);
	hmi.cur_start = hmi.cur_pos;
	hmi.cur_end = rows-1;	/* Skip the first line */
	for(int idx = 0; idx < rows; idx++)
	{
		draw_string(ROW_OFFSET, COL_OFFSET+CHAR_HEIGHT_SMALL*idx, COLOR_TEXT, 1, home[idx]);
	}

	while(hmi.state_hold)
	{
		switch(hmi.button)
		{
			case (BACK):
				HAL_Delay(DEBOUNCE_TIME);
				hmi.button = NONE;
				break;

			case (UP):
				HAL_Delay(DEBOUNCE_TIME);
				hmi.button = NONE;
				if(hmi.cur_pos > hmi.cur_start)
				{
					Clear_Cursor(hmi.cur_pos);
					hmi.cur_pos--;
					Set_Cursor(hmi.cur_pos);
				}
				break;

			case (DOWN):
				HAL_Delay(DEBOUNCE_TIME);
				hmi.button = NONE;
				if(hmi.cur_pos < hmi.cur_end)
				{
					Clear_Cursor(hmi.cur_pos);
					hmi.cur_pos++;
					Set_Cursor(hmi.cur_pos);
				}
				break;

			case (OK):
				HAL_Delay(DEBOUNCE_TIME);
				hmi.button = NONE;
				hmi.curr_state = home_func_ptr[hmi.cur_pos-1];
				hmi.state_hold = FALSE;
				break;

			case (FORW):
				HAL_Delay(DEBOUNCE_TIME);
				hmi.button = NONE;
				break;

			default:
				break;
		}
	}
}

/*
 * @brief	Set Start and Stop RF-Freq for scanning stage
 * @note	RF-Start MIN = 35 	(khz)
 * 			RF-End	 MAX = 4.4	(Ghz)
 *
 * @note	**In case Unselected**
 * 			BACK-BUT	Returns to previous state
 * 			DOWN-BUT	Move Cursor down
 * 			UP-BUT		Move Cursor up
 * 			FORW-BUT	N/A
 * 			OK-BUT		Select the freq to change
 *
 * @note	**In case Selected**
 * 			BACK-BUT	Move digit to the left
 * 			DOWN-BUT	Decrease digit value
 * 			UP-BUT		Increase digit value
 * 			FORW-BUT	Move digit to the right
 * 			OK-BUT		Unselect the freq to change
 */
void Set_RF_Out(void)
{
	HAL_Delay(DEBOUNCE_TIME);
	hmi.button = NONE;
	hmi.state_hold = TRUE;

	uint8_t rows = sizeof(set_rf_out) / sizeof(set_rf_out[0]);
	uint8_t is_selected = FALSE; /* state-var to lock the selected line */
	uint8_t val_pos = 0;	/* Range [0..9] */
	uint8_t digit = 0;
	char rf_val[3][11];

	if(is_default == TRUE)	/* Default values for freq parameters (ONLY 1 time) */
	{
		is_default = FALSE;
		strcpy(rf_val[START_FREQ-1],"0000035000\0");
		strcpy(rf_val[STOP_FREQ-1],"4400000000\0");
		strcpy(rf_val[STEP_FREQ-1],"0000050000\0");
	}
	else	/* Used the freq values from previous selection */
	{
		num2str_10digits(adf4351.start_freq, rf_val[START_FREQ-1], sizeof(rf_val[START_FREQ-1]));
		num2str_10digits(adf4351.stop_freq, rf_val[STOP_FREQ-1], sizeof(rf_val[STOP_FREQ-1]));
		num2str_10digits(adf4351.step_freq, rf_val[STEP_FREQ-1], sizeof(rf_val[STEP_FREQ-1]));
	}

	Clear_Screen();
	Set_Cursor(LINE1);
	hmi.cur_start = hmi.cur_pos;
	hmi.cur_end = rows-1;	/* Skip the first line */
	for(int idx = 0; idx < rows; idx++)
	{
		draw_string(ROW_OFFSET, COL_OFFSET+CHAR_HEIGHT_SMALL*idx, COLOR_TEXT, 1, set_rf_out[idx]);
	}

	for(int idx = START_FREQ; idx <= STEP_FREQ; idx++)
	{
		draw_string(COL_OFFSET+CHAR_WIDTH_SMALL*(CURSOR_OFFSET+SUBTEXT_OFFSET),
				ROW_OFFSET+CHAR_HEIGHT_SMALL*idx, COLOR_TEXT, 1, rf_val[idx-1]);
	}

	while(hmi.state_hold)
	{
		switch(hmi.button)
		{
			case (BACK):
				HAL_Delay(200);
				hmi.button = NONE;
				if(is_selected == FALSE)
				{
					/* Store start freq and stop freq */
					adf4351.start_freq 	= strtoull(rf_val[START_FREQ-1], NULL, 10);
					adf4351.stop_freq	= strtoull(rf_val[STOP_FREQ-1], NULL, 10);
					adf4351.step_freq 	= strtoull(rf_val[STEP_FREQ-1], NULL, 10);

					hmi.curr_state = Home;
					hmi.state_hold = FALSE;
				}
				else /* is_selected = TRUE */
				{
					/* Examine the digit of current val_pos */
					digit = rf_val[hmi.cur_pos-1][val_pos]-'0';

					/* Draw the digit with COLOR_BG (Clear the digit) */
					draw_string(COL_OFFSET+CHAR_WIDTH_SMALL*(CURSOR_OFFSET+SUBTEXT_OFFSET+val_pos),
							ROW_OFFSET+CHAR_HEIGHT_SMALL*hmi.cur_pos, COLOR_BG, 1, num[digit]);
					/* Re-draw the digit with COLOR_TEXT (unselected) */
					draw_string(COL_OFFSET+CHAR_WIDTH_SMALL*(CURSOR_OFFSET+SUBTEXT_OFFSET+val_pos),
							ROW_OFFSET+CHAR_HEIGHT_SMALL*hmi.cur_pos, COLOR_TEXT, 1, num[digit]);

					if(val_pos > 0) val_pos--;
					else val_pos = 9;

					/* Examine the digit of current val_pos */
					digit = rf_val[hmi.cur_pos-1][val_pos]-'0';

					/* Draw the digit with COLOR_BG (Clear the digit) */
					draw_string(COL_OFFSET+CHAR_WIDTH_SMALL*(CURSOR_OFFSET+SUBTEXT_OFFSET+val_pos),
							ROW_OFFSET+CHAR_HEIGHT_SMALL*hmi.cur_pos, COLOR_BG, 1, num[digit]);

					/* Re-draw the digit with COLOR_TEXT1 (selected) */
					draw_string(COL_OFFSET+CHAR_WIDTH_SMALL*(CURSOR_OFFSET+SUBTEXT_OFFSET+val_pos),
							ROW_OFFSET+CHAR_HEIGHT_SMALL*hmi.cur_pos, COLOR_TEXT1, 1, num[digit]);
				}
				break;

			case (UP):
				HAL_Delay(200);
				hmi.button = NONE;
				if(is_selected == FALSE)
				{
					if(hmi.cur_pos > hmi.cur_start)
					{
						Clear_Cursor(hmi.cur_pos);
						hmi.cur_pos--;
						Set_Cursor(hmi.cur_pos);
					}
				}
				else /* is_selected = TRUE */
				{
					/* Examine the digit of current val_pos */
					digit = rf_val[hmi.cur_pos-1][val_pos]-'0';

					/* Draw the digit with COLOR_BG (Clear the digit) */
					draw_string(COL_OFFSET+CHAR_WIDTH_SMALL*(CURSOR_OFFSET+SUBTEXT_OFFSET+val_pos),
							ROW_OFFSET+CHAR_HEIGHT_SMALL*hmi.cur_pos, COLOR_BG, 1, num[digit]);

					if(digit >= 9) digit = 0;
					else digit++;

					/* Re-draw the digit with COLOR_TEXT1 (selected) and new value */
					draw_string(COL_OFFSET+CHAR_WIDTH_SMALL*(CURSOR_OFFSET+SUBTEXT_OFFSET+val_pos),
							ROW_OFFSET+CHAR_HEIGHT_SMALL*hmi.cur_pos, COLOR_TEXT1, 1, num[digit]);

					/* Update digit in rf_val string */
					rf_val[hmi.cur_pos-1][val_pos] = digit + '0';
				}
				break;

			case (DOWN):
				HAL_Delay(200);
				hmi.button = NONE;
				if(is_selected == FALSE)
				{
					if(hmi.cur_pos < hmi.cur_end)
					{
						Clear_Cursor(hmi.cur_pos);
						hmi.cur_pos++;
						Set_Cursor(hmi.cur_pos);
					}
				}
				else /* is_selected = TRUE */
				{
					/* Examine the digit of current val_pos */
					digit = rf_val[hmi.cur_pos-1][val_pos]-'0';

					/* Draw the digit with COLOR_BG (Clear the digit) */
					draw_string(COL_OFFSET+CHAR_WIDTH_SMALL*(CURSOR_OFFSET+SUBTEXT_OFFSET+val_pos),
							ROW_OFFSET+CHAR_HEIGHT_SMALL*hmi.cur_pos, COLOR_BG, 1, num[digit]);

					if(digit <= 0) digit = 9;
					else digit--;

					/* Re-draw the digit with COLOR_TEXT1 (selected) and new value */
					draw_string(COL_OFFSET+CHAR_WIDTH_SMALL*(CURSOR_OFFSET+SUBTEXT_OFFSET+val_pos),
							ROW_OFFSET+CHAR_HEIGHT_SMALL*hmi.cur_pos, COLOR_TEXT1, 1, num[digit]);

					/* Update digit in rf_val string */
					rf_val[hmi.cur_pos-1][val_pos] = digit + '0';
				}
				break;

			case (OK):
				HAL_Delay(200);
				hmi.button = NONE;
				if(is_selected == FALSE)
				{
					/* Re-draw the text display with COLOR_TEXT1 (selected) */
					is_selected = TRUE;
					draw_string(COL_OFFSET+CHAR_WIDTH_SMALL*CURSOR_OFFSET,
							ROW_OFFSET+CHAR_HEIGHT_SMALL*hmi.cur_pos, COLOR_TEXT1, 1, set_rf_out1[0]);
					draw_string(COL_OFFSET+CHAR_WIDTH_SMALL*CURSOR_OFFSET,
							ROW_OFFSET+CHAR_HEIGHT_SMALL*hmi.cur_pos, COLOR_TEXT1, 1, set_rf_out1[hmi.cur_pos]);
				}
				else /* is_selected = TRUE */
				{
					/* Reset the color and value of digit */
					draw_string(COL_OFFSET+CHAR_WIDTH_SMALL*(CURSOR_OFFSET+SUBTEXT_OFFSET+val_pos),
							ROW_OFFSET+CHAR_HEIGHT_SMALL*hmi.cur_pos, COLOR_BG, 1, num[digit]);
					draw_string(COL_OFFSET+CHAR_WIDTH_SMALL*(CURSOR_OFFSET+SUBTEXT_OFFSET+val_pos),
							ROW_OFFSET+CHAR_HEIGHT_SMALL*hmi.cur_pos, COLOR_TEXT, 1, num[digit]);
					digit = 0;
					val_pos = 0;

					/* Re-draw the text display with COLOR_TEXT (unselected) */
					is_selected = FALSE;
					draw_string(COL_OFFSET+CHAR_WIDTH_SMALL*CURSOR_OFFSET,
							ROW_OFFSET+CHAR_HEIGHT_SMALL*hmi.cur_pos, COLOR_TEXT, 1, set_rf_out1[0]);
					draw_string(COL_OFFSET+CHAR_WIDTH_SMALL*CURSOR_OFFSET,
							ROW_OFFSET+CHAR_HEIGHT_SMALL*hmi.cur_pos, COLOR_TEXT, 1, set_rf_out1[hmi.cur_pos]);
				}
				break;

			case (FORW):
				HAL_Delay(200);
				hmi.button = NONE;
				if(is_selected == FALSE)
				{
					/* DO Nothing */
				}
				else /* is_selected = TRUE */
				{
					/* Examine the digit of current val_pos */
					digit = rf_val[hmi.cur_pos-1][val_pos]-'0';

					/* Draw the digit with COLOR_BG (Clear the digit) */
					draw_string(COL_OFFSET+CHAR_WIDTH_SMALL*(CURSOR_OFFSET+SUBTEXT_OFFSET+val_pos),
							ROW_OFFSET+CHAR_HEIGHT_SMALL*hmi.cur_pos, COLOR_BG, 1, num[digit]);

					/* Re-draw the digit with COLOR_TEXT (unselected) */
					draw_string(COL_OFFSET+CHAR_WIDTH_SMALL*(CURSOR_OFFSET+SUBTEXT_OFFSET+val_pos),
							ROW_OFFSET+CHAR_HEIGHT_SMALL*hmi.cur_pos, COLOR_TEXT, 1, num[digit]);

					if(val_pos < 9) val_pos++;
					else val_pos = 0;

					/* Examine the digit of current val_pos */
					digit = rf_val[hmi.cur_pos-1][val_pos]-'0';

					/* Draw the digit with COLOR_BG (Clear the digit) */
					draw_string(COL_OFFSET+CHAR_WIDTH_SMALL*(CURSOR_OFFSET+SUBTEXT_OFFSET+val_pos),
							ROW_OFFSET+CHAR_HEIGHT_SMALL*hmi.cur_pos, COLOR_BG, 1, num[digit]);

					/* Re-draw the digit with COLOR_TEXT (selected) */
					draw_string(COL_OFFSET+CHAR_WIDTH_SMALL*(CURSOR_OFFSET+SUBTEXT_OFFSET+val_pos),
							ROW_OFFSET+CHAR_HEIGHT_SMALL*hmi.cur_pos, COLOR_TEXT1, 1, num[digit]);
				}
				break;

			default:
				break;
		}
	}
}

/*
 * @brief	Scanning RF-Freq in given range [RF-Start, RF-End]
 * @note	RF-Start= 750 	(Mhz)
 * 			RF-End	= 1		(Ghz)
 * 			Step	= 500	(khz)
 *
 * @note	BACK-BUT	Returns to previous state
 * 			DOWN-BUT	N/A
 * 			UP-BUT		N/A
 * 			FORW-BUT	N/A
 * 			OK-BUT		Start plotting
 */
void Scan_RF_Out(void)
{
	HAL_Delay(DEBOUNCE_TIME);
	hmi.button = NONE;
	hmi.state_hold = TRUE;

	/* Reset the index when starting plotting */
	vmag_idx = 0;
	vmag_ave = 0;

	char str_freq[11] = "\0";
	char str2screen[23];

	/* NOTE: there is no need for cursor operation in this stage */
	Clear_Screen();

	/* These texts in 'CHAR_HEIGHT_SMALL' format are optional */
	/* There are maximum 3 lines are available!! - see the comments */
	draw_string(COL_OFFSET, ROW_OFFSET+CHAR_HEIGHT_SMALL*LINE0, COLOR_TEXT, 1, "------RF-SCANNING------\0");
//	draw_string(COL_OFFSET, ROW_OFFSET+CHAR_HEIGHT_SMALL*LINE1, COLOR_TEXT, 1, "   Set RF Out Freq     \0");
//	draw_string(COL_OFFSET, ROW_OFFSET+CHAR_HEIGHT_SMALL*LINE2, COLOR_TEXT, 1, "Hello-Hello\0");

	/* --Plotting xy-axis--
	 * Top of graph (x,y): (10,70)
	 * End of graph (x,y): (210, 170)
	 * --> Total 200 (= 210-10) samples are drawn per time.
	*/
	/* Draw y-axis */
	for (int i = Y_AXIS_TOP; i <= Y_AXIS_BOT; i++)
	{
	  draw_pixel(X_AXIS_LEFT, i, COLOR_TITLE);
	}
	/* Draw x-axis */
	for (int i = X_AXIS_LEFT; i <= X_AXIS_RIGHT; i++)
	{
	  draw_pixel(i, Y_AXIS_BOT, COLOR_TITLE);
	}

	while(hmi.state_hold)
	{
		switch(hmi.button)
		{
			case (BACK):
				HAL_Delay(DEBOUNCE_TIME);
				hmi.button = NONE;
				hmi.curr_state = Home;
				hmi.state_hold = FALSE;
				break;

			case (UP):
				HAL_Delay(DEBOUNCE_TIME);
				hmi.button = NONE;
				break;

			case (DOWN):
				HAL_Delay(DEBOUNCE_TIME);
				hmi.button = NONE;
				break;

			case (OK):
				HAL_Delay(DEBOUNCE_TIME);
				hmi.button = NONE;
				ADF4351_Scanning(adf4351.start_freq,
								 adf4351.stop_freq,
								 adf4351.step_freq);
				ADF4351_Set_Out_Freq(adf4351.scan_freq);
				if(str_freq[0] != '\0')
				{
					/* Reset the screen condition */
					vmag_idx = SAMPLES+OFFSET;

					/* Clear the old scan freq before new measurement */
					snprintf(str2screen, sizeof(str2screen), "freq = %s Hz", str_freq);
					draw_string(COL_OFFSET, ROW_OFFSET+CHAR_HEIGHT_SMALL*LINE1, COLOR_BG, 1, str2screen);
				}
				num2str(adf4351.scan_freq, str_freq, sizeof(str_freq));
				snprintf(str2screen, sizeof(str2screen), "freq = %s Hz", str_freq);
				draw_string(COL_OFFSET, ROW_OFFSET+CHAR_HEIGHT_SMALL*LINE1, COLOR_TEXT, 1, str2screen);
				break;

			case (FORW):
				HAL_Delay(DEBOUNCE_TIME);
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
 * 			DOWN-BUT	N/A
 * 			UP-BUT		N/A
 * 			FORW-BUT	Stop plotting
 * 			OK-BUT		Start plotting
 */
void Plot_VMAG(void)
{
	HAL_Delay(DEBOUNCE_TIME);
	hmi.button = NONE;
	hmi.state_hold = TRUE;

	/* Initialize when there is 'while' loop in the switch case */
	hmi.while_break = TRUE;

	/* Reset the index when starting plotting */
	vmag_idx = 0;
	vmag_ave = 0;

	/* NOTE: there is no need for cursor operation in this stage */
	Clear_Screen();

	/* These texts in 'CHAR_HEIGHT_SMALL' format are optional */
	/* There are maximum 3 lines are available!! - see the comments */
	draw_string(COL_OFFSET, ROW_OFFSET+CHAR_HEIGHT_SMALL*LINE0, COLOR_TEXT, 1, "-----VMAG-PLOTTING-----\0");
//	draw_string(COL_OFFSET, ROW_OFFSET+CHAR_HEIGHT_SMALL*LINE1, COLOR_TEXT, 1, "   Set RF Out Freq     \0");
//	draw_string(COL_OFFSET, ROW_OFFSET+CHAR_HEIGHT_SMALL*LINE2, COLOR_TEXT, 1, "Hello-Hello\0");

	/* --Plotting xy-axis--
	 * Top of graph (x,y): (10,70)
	 * End of graph (x,y): (210, 170)
	 * --> Total 200 (= 210-10) samples are drawn per time.
	*/
	/* Draw y-axis */
	for (int i = Y_AXIS_TOP; i <= Y_AXIS_BOT; i++)
	{
	  draw_pixel(X_AXIS_LEFT, i, COLOR_TITLE);
	}
	/* Draw x-axis */
	for (int i = X_AXIS_LEFT; i <= X_AXIS_RIGHT; i++)
	{
	  draw_pixel(i, Y_AXIS_BOT, COLOR_TITLE);
	}

	while(hmi.state_hold)
	{
		switch(hmi.button)
		{
			case (BACK):
				HAL_Delay(DEBOUNCE_TIME);
				hmi.button = NONE;
				hmi.curr_state = Home;
				hmi.state_hold = FALSE;
				break;

			case (UP):
				HAL_Delay(DEBOUNCE_TIME);
				hmi.button = NONE;
				break;

			case (DOWN):
				HAL_Delay(DEBOUNCE_TIME);
				hmi.button = NONE;
				break;

			case (OK):
				HAL_Delay(DEBOUNCE_TIME);
				hmi.button = NONE;
				while(hmi.while_break == FALSE)
				{
					HAL_GPIO_TogglePin(BLUE_LED_GPIO_Port, BLUE_LED_Pin);
					ADF4351_DataConvert();
					HAL_Delay(100);
				}
				break;

			case (FORW):
				HAL_Delay(DEBOUNCE_TIME);
				hmi.button = NONE;
				break;

			default:
				break;
		}
	}
}

/*
 * @brief	Clear the whole screen
 */
void Clear_Screen(void)
{
	fill_rectangle(0, 0, WIDTH, HEIGHT, COLOR_BG);
}

/*
 * @brief	Clear screen in given rectangular
 * @param	x_s	x-axis value of the first point
 * @param	y_s	y-axis value of the first point
 * @param	x_e	x-axis value of the adjacent point
 * @param	y_e	y-axis value of the adjacent point
 */
void Clear_Screen_Custom(uint8_t x_s, uint8_t y_s, uint8_t x_e, uint8_t y_e)
{
	fill_rectangle(x_s, y_s, x_e, y_e, COLOR_BG);
}

/*
 * @brief	Set cursor at given position
 * @param	pos	see @ref /Cursor position/
 */
void Set_Cursor(uint8_t pos)
{
	hmi.cur_pos = pos;
	draw_string(ROW_OFFSET, COL_OFFSET+CHAR_HEIGHT_SMALL*pos, COLOR_SYMBOL, 1, cursor);
}

/*
 * @brief	Clear cursor at given position
 * @param	pos	see @ref /Cursor position/
 */
void Clear_Cursor(uint8_t pos)
{
	draw_string(ROW_OFFSET, COL_OFFSET+CHAR_HEIGHT_SMALL*pos, COLOR_BG, 1, cursor);
}

/*
 * @brief	Auxiliary function to assist the plotting functionality
 * @param	x1	x-axis value of start point
 * @param	y1	y-axis value of start point
 * @param	x2	x-axis value of end point
 * @param	y2	y-axis value of end point
 */
void Bresenham_line_algorithm(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2)
{
    int dx = abs(x2 - x1), dy = abs(y2 - y1);
    int sx = (x1 < x2) ? 1 : -1;
    int sy = (y1 < y2) ? 1 : -1;
    int err = dx - dy;

    int max_steps = dx + dy + 1;  /* Safety limit (max possible steps) */
    int steps = 0;

    while (steps < max_steps) 	/* Ensures loop does not run forever */
    {
    	draw_pixel(x1, y1, COLOR_SYMBOL);

        if (x1 == x2 && y1 == y2) break;

        int e2 = 2 * err;
        if (e2 > -dy) { err -= dy; x1 += sx; }
        if (e2 < dx) { err += dx; y1 += sy; }

        steps++;
    }
}

/*
 * @brief	Convert a number to corresponding string
 *
 * @param	value	unsigned 64-bit input number
 * @param	str		char pointer of the output string
 * @param	size	the size of the output string
 */
void num2str(uint64_t value, char *str, size_t size)
{
    if (size == 0) return; /* Safety check */

    char temp[21]; /* Max digits for uint64_t + null terminator */
    int i = 0;

    /* Special case for zero */
    if (value == 0)
    {
        temp[i++] = '0';
    }
    else
    {
        while (value > 0 && i < (int)(sizeof(temp) - 1)) {
            temp[i++] = '0' + (value % 10); /* Extract last digit */
            value /= 10; /* Remove last digit */
        }
    }

    temp[i] = '\0'; /* Null-terminate */

    /* Reverse the string into output buffer */
    int len = i;
    if (len >= (int)size) len = (int)size - 1; /* Ensure it fits */

    for (int j = 0; j < len; j++)
    {
        str[j] = temp[len - j - 1]; /* Reverse copy */
    }
    str[len] = '\0'; /* Null-terminate output */
}

/*
 * @brief	Convert a number to corresponding string (10 chars/ digits)
 *
 * @param	value	unsigned 64-bit input number
 * @param	str		char pointer of the output string
 * @param	size	the size of the output string
 */
void num2str_10digits(uint64_t value, char *str, size_t size) {
	/* Convert a number to string first */
    num2str(value, str, size);

    size_t new_len = strlen(str);
    /* Checking whether the new length is less than 10 */
    if (new_len < 10) {
        /* Shift existing characters to the right to make space for zeros */
        memmove(str + (10 - new_len), str, new_len + 1);  /* +1 to move the null terminator */
        /* Fill the start with '0' */
        memset(str, '0', 10 - new_len);
    }
}

