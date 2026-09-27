#ifndef __APP_UI_H__
#define __APP_UI_H__

#include "sys.h"
#include "GUI.h"
#include "lcd.h"

/* One entry per screen. Add more if you need a 3rd/4th screen later. */

typedef enum { SCREEN_MAIN, SCREEN_SECOND, SCREEN_IMAGE } ScreenID;
extern ScreenID current_screen;

/* One button = one rectangle you remember and hit-test */
typedef struct {
  u16 x1, y1, x2, y2;
  const char *label;
} SimpleButton;

extern SimpleButton main_buttons[];
extern SimpleButton back_button;
/* Call once at startup, after LCD_Init() and TP hardware init are done. */
void AppUI_Init(void);

/* Call repeatedly in your main while(1) loop. Non-blocking, cheap. */
void AppUI_Task(void);

/* Screen switches - call these yourself from wherever makes sense
(e.g. from AppUI_Task's button handlers, or from your own sensor logic). */
void AppUI_ShowMain(void);
void AppUI_ShowSecond(void);
void AppUI_ShowImage(uint32_t qspi_offset);

void Show_Image_QSPI(u16 x, u16 y, u16 w, u16 h, uint32_t qspi_offset);
void Draw_Button(SimpleButton *b, u16 fill_color, u16 text_color);
int Button_Hit(SimpleButton *b, u16 x, u16 y);
void Handle_Touch(void);
void Draw_Main_Screen(void);
void Draw_Time_Date(u16 y_time, u16 y_date);
void Draw_PWM_Status(void);
void Draw_Time_Date_Compact(u16 x, u16 y);
void Update_Config_Screen(void);
void Enter_Config_Screen(uint8_t config_id);
void Draw_Config_Screen(uint8_t config_id);
#endif