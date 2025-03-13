/*
 * adf4351_hmi.h
 *
 *  Created on: Jan 21, 2025
 *      Author: ADM
 */

#ifndef INC_ADF4351_HMI_H_
#define INC_ADF4351_HMI_H_

#ifdef	__cplusplus
extern "C" {
#endif

#include "ILI9225.h"

#define COLOR_BG			COLOR_BLACK
#define COLOR_TITLE			COLOR_GOLD
#define COLOR_TEXT			COLOR_BLUE
#define COLOR_TEXT1			COLOR_GREEN
#define COLOR_SYMBOL		COLOR_ORANGE
#define COLOR_NUM			COLOR_LIGHTGREEN
#define COLOR_ERROR			COLOR_RED
#define COLOR_WARNING		COLOR_YELLOW

#define LCD_WIDTH			WIDTH
#define LCD_HEIGHT			HEIGHT

#define ROW_OFFSET			2
#define COL_OFFSET			2
#define MAX_CHARS_SMALL		24
#define MAX_CHARS_LARGE		MAX_CHARS_SMALL/2
#define CHAR_WIDTH_SMALL	WIDTH/MAX_CHARS_SMALL
#define CHAR_WIDTH_LARGE	CHAR_WIDTH_SMALL*2
#define CHAR_HEIGHT_SMALL	25
#define CHAR_HEIGHT_LARGE	45
#define MAX_LINES_SMALL		((HEIGHT-COL_OFFSET*2)/CHAR_HEIGHT_SMALL)+1
#define MAX_LINES_LARGE		((HEIGHT-COL_OFFSET*2)/CHAR_HEIGHT_LARGE)+1

#define LINE0	0
#define LINE1	1
#define LINE2	2
#define LINE3	3
#define LINE4	4
#define LINE5	5
#define LINE6	6
#define LINE7	7
#define LINE8	8

#define TRUE	1
#define FALSE	0

#define X_AXIS_LEFT		10
#define X_AXIS_RIGHT	210
#define	Y_AXIS_TOP		70
#define Y_AXIS_BOT		170
#define SAMPLES			X_AXIS_RIGHT - X_AXIS_LEFT
#define OFFSET			5

/* RF OUT STAGE MACROs */
#define CURSOR_OFFSET 3
#define SUBTEXT_OFFSET 10
#define START_FREQ	1
#define STOP_FREQ	2
#define STEP_FREQ	3

typedef enum
{
	NONE,
	BACK,
	UP,
	DOWN,
	OK,
	FORW
} button_status;

typedef struct
{
	uint8_t			state_hold;
	button_status 	button;
	uint8_t 		cur_pos;
	uint8_t			cur_start;
	uint8_t			cur_end;
	void			(*curr_state)(void);
	void			(*prev_state)(void);
	uint8_t			while_break;
} hmi_status;

void Clear_Screen(void);
void Clear_Screen_Custom(uint8_t x_s, uint8_t y_s, uint8_t x_e, uint8_t y_e);
void Set_Cursor(uint8_t pos);
void Clear_Cursor(uint8_t pos);
void HMI_Init(void);
void Welcome(void);
void Home(void);
void Scan_RF_Out(void);
void Plot_VMAG(void);
void Set_RF_Out(void);
void Set_RF_Power(void);
void Read_VMAG(void);
void Bresenham_line_algorithm(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2);
void num2str(uint64_t value, char *str, size_t size);
#ifdef	__cplusplus
}
#endif

#endif /* INC_ADF4351_HMI_H_ */
