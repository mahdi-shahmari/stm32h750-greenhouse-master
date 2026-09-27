#include "App_UI.h"
#include <stdio.h>
#include <string.h>
#include "BT_RTC_PWM.h"
#include "delay.h"
#include "SPI.h"
#include "sys.h"
#include "lcd.h"
#include "GUI.h"
#include "touch.h"
#include "qspi_w25q64.h"
#include <string.h>


#pragma location = "DMA_RAM"
static uint16_t row_buf[480];

#pragma location = "DMA_RAM"
static uint16_t fill_buf[480];   /* one row's worth, reused for any fill */

/* Draws time/date at a given position, only redrawing when the second changes.
   Reusable for main screen or a dedicated clock screen. */
#define TIME_STR_LEN     8    /* "HH:MM:SS" */
#define CHAR_STEP        8    /* fixed step size for ASCII in your Show_Str, size>16 */
u8 redraw=0;
extern RTC_HandleTypeDef hrtc;
ScreenID current_screen=SCREEN_MAIN;
SimpleButton back_button = {10, 10, 110, 50, "Back"};   /* was: static SimpleButton back_button = ... */

/* 6 buttons, 3x2 grid, matches your original LVGL layout */
SimpleButton main_buttons[6] = {
    { 10, 170, 160, 235, "Config 1"}, {165, 170, 315, 235, "Config 2"}, {320, 170, 470, 235, "Config 3"},
    { 10, 245, 160, 310, "Config 4"}, {165, 245, 315, 310, "Config 5"}, {320, 245, 470, 310, "Config 6"},
};

static const char *weekday_name(uint8_t wd)
{
    static const char *names[] = {"", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"};
    return (wd >= 1 && wd <= 7) ? names[wd] : "?";
}

void Draw_Time_Date(u16 y_time, u16 y_date)
{
  if (current_screen != SCREEN_MAIN) return;
  static RTC_TimeTypeDef last = {0xFF, 0xFF, 0xFF};
  RTC_TimeTypeDef sTime = {0};
  RTC_DateTypeDef sDate = {0};
  char buf[16];
  
  HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
  HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);
  if(redraw==1){
    sprintf(buf, "%02d:%02d:%02d", sTime.Hours, sTime.Minutes, sTime.Seconds);
    Gui_StrCenter( y_time, BLACK, WHITE, (u8*)buf, 32, 0);
    sprintf(buf, "%s  20%02d-%02d-%02d", weekday_name(sDate.WeekDay), sDate.Year, sDate.Month, sDate.Date);
    Gui_StrCenter(y_date, BLACK, WHITE, (u8*)buf, 16, 0);
    redraw=0;
  }
  if (sTime.Seconds == last.Seconds) return;
  
  u16 str_x = (lcddev.width - TIME_STR_LEN * CHAR_STEP) / 2;   /* = 208, computed not guessed */
  
  if (sTime.Hours == last.Hours && sTime.Minutes == last.Minutes)
  {
    sprintf(buf, "%02d", sTime.Seconds);
    Show_Str(str_x + 6 * CHAR_STEP, y_time, BLACK, WHITE, (u8*)buf, 32, 0);  /* 6 chars in "HH:MM:" before SS */
    
  }
  else
  {
    sprintf(buf, "%02d:%02d:%02d", sTime.Hours, sTime.Minutes, sTime.Seconds);
    Gui_StrCenter( y_time, BLACK, WHITE, (u8*)buf, 32, 0);
    sprintf(buf, "%s  20%02d-%02d-%02d", weekday_name(sDate.WeekDay), sDate.Year, sDate.Month, sDate.Date);
    Gui_StrCenter(y_date, BLACK, WHITE, (u8*)buf, 16, 0);
  }
  last = sTime;
}

static inline uint16_t Swap16(uint16_t v)
{
    return (uint16_t)((v >> 8) | (v << 8));
}

void LCD_Fill_DMA(u16 sx, u16 sy, u16 ex, u16 ey, u16 color)
{
    u16 width  = ex - sx + 1;
    u16 height = ey - sy + 1;
    uint16_t swapped = Swap16(color);

    for (u16 i = 0; i < width; i++) fill_buf[i] = swapped;   /* swapped once, not per-pixel */

    LCD_SetWindows(sx, sy, ex, ey);
    for (u16 row = 0; row < height; row++)
    {
        SPIv_WriteDataBuffer_DMA(fill_buf, width);
        SPIv_DMA_Wait();
    }
    LCD_SetWindows(0, 0, lcddev.width - 1, lcddev.height - 1);
}

void LCD_Clear_DMA(u16 color)
{
    LCD_Fill_DMA(0, 0, lcddev.width - 1, lcddev.height - 1, color);
}
void Draw_Button(SimpleButton *b, u16 fill_color, u16 text_color)
{
    LCD_Fill_DMA(b->x1, b->y1, b->x2, b->y2, fill_color);   /* was LCD_DrawFillRectangle */
    POINT_COLOR = BLACK;
    LCD_DrawRectangle(b->x1, b->y1, b->x2, b->y2);          /* border stays as-is - thin lines, cheap either way */
    Show_Str((b->x1+b->x2)/2 - strlen(b->label)*4, (b->y1+b->y2)/2 - 8,
             text_color, fill_color, (u8*)b->label, 16, 0);
}

void Draw_Main_Screen(void)
{
    LCD_Clear_DMA(WHITE);        /* was LCD_Clear(WHITE) */
    for (int i = 0; i < 6; i++) Draw_Button(&main_buttons[i], LIGHTBLUE, BLACK);
    current_screen = SCREEN_MAIN;
    redraw=1;
    Draw_Time_Date(40, 90);
}


void Draw_Image_Screen(uint32_t slot_index)
{
    LCD_Clear_DMA(WHITE);
    Show_Image_QSPI(0, 0, 480, 320, slot_index * 0x50000UL);
    Draw_Button(&back_button, LIGHTBLUE, BLACK);
    current_screen = SCREEN_IMAGE;
}


void Draw_Config_Screen(uint8_t config_id)
{
    char buf[32];
    StageConfig *s = &active_config.stages[current_stage_idx];

    sprintf(buf, "Config %d - Stage %d/6", config_id, current_stage_idx + 1);
    Show_Str(20, 70, BLACK, WHITE, (u8*)buf, 16, 0);

    sprintf(buf, "Days: %d/%d", Get_Days_In_Current_Stage(), s->day_repeat);
    Show_Str(20, 100, BLACK, WHITE, (u8*)buf, 16, 0);

    sprintf(buf, "Start: %02d:%02d  Dur: %dmin", s->start_hour, s->start_min, s->duration_min);
    Show_Str(20, 130, BLACK, WHITE, (u8*)buf, 16, 0);

    sprintf(buf, "Freq: %d  Power: %d", s->frequency, s->power);
    Show_Str(20, 160, BLACK, WHITE, (u8*)buf, 16, 0);

    Get_Next_Job_Time(buf, sizeof(buf));
    Show_Str(20, 220, BLACK, WHITE, (u8*)buf, 16, 0); 
}

void Draw_PWM_Status(void)
{
    static int8_t last_status = -1;   /* -1 forces first draw */
    uint8_t status = Get_PWM_Status();

    if (status == last_status) return;
    last_status = status;

    if (status)
        Show_Str(20, 190, WHITE, RED, (u8*) "Wave is ON ", 16, 0);
    else
        Show_Str(20, 190, WHITE, GRAY, (u8*)"Wave is OFF", 16, 0);
}

/* Hit test in your main loop, using tp_dev.x/y already working from TP_Scan(0) */
int Button_Hit(SimpleButton *b, u16 x, u16 y)
{
  
  return (x >= b->x1 && x <= b->x2 && y >= b->y1 && y <= b->y2);
}

void Show_Image_QSPI(u16 x, u16 y, u16 w, u16 h, uint32_t qspi_addr)
{
    LCD_SetWindows(x, y, x + w - 1, y + h - 1);
    for (u16 row = 0; row < h; row++)
    {
        QSPI_W25Qxx_ReadBuffer((uint8_t*)row_buf, qspi_addr + (uint32_t)row * w * 2, w * 2);
        SPIv_WriteDataBuffer_DMA(row_buf, w);
        SPIv_DMA_Wait();
    }
    LCD_SetWindows(0, 0, lcddev.width - 1, lcddev.height - 1);
}

/* Call this every loop iteration - replaces LVGL's indev/event system */
void Handle_Touch(void)
{
    if (tp_dev.scan(0))                          /* 0 = screen-mapped coords, calibration-adjusted */
    {
        if (tp_dev.sta & TP_CATH_PRES)            /* just-pressed edge, not held-down repeat */
        {
            tp_dev.sta &= ~(1 << 6);
            u16 x = tp_dev.x, y = tp_dev.y;

            switch (current_screen)
            {
            case SCREEN_MAIN:
                for (int i = 0; i < 6; i++)
                {
                  if (Button_Hit(&main_buttons[i], x, y))
                  {
                    //Arm_Schedule();
                    //Load_Config(i + 1);
                    //Draw_Image_Screen(i);   /* shows that slot's image; add param text overlay if you want config details visible too */
                    //Draw_Config_Screen(i + 1);
                    //Save_Schedule_State();
                    Enter_Config_Screen(i + 1);
                    break;
                  }
                }
                break;

            case SCREEN_SECOND:
            case SCREEN_IMAGE:
                if (Button_Hit(&back_button, x, y)) 
                {
                  Disarm_Schedule();
                  Draw_Main_Screen();
                  Save_Schedule_State();
                }
                break;
            }
        }
    }
}

void Draw_Time_Date_Compact(u16 x, u16 y)
{
    static RTC_TimeTypeDef last = {0xFF, 0xFF, 0xFF};
    RTC_TimeTypeDef sTime = {0};
    RTC_DateTypeDef sDate = {0};
    char buf[24];

    HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
    HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);

    if (sTime.Seconds == last.Seconds) return;
    last = sTime;

    sprintf(buf, "%02d:%02d:%02d %02d/%02d", sTime.Hours, sTime.Minutes, sTime.Seconds, sDate.Month, sDate.Date);
    Show_Str(x, y, BLACK, WHITE, (u8*)buf, 12, 0);
}

static uint8_t cfg_last_stage = 0xFF;
static uint8_t cfg_last_days = 0xFF;
static int8_t  cfg_last_pwm = -1;
static char    cfg_last_next[32] = {0};

void Enter_Config_Screen(uint8_t config_id)
{
  Arm_Schedule();
  Load_Config(config_id);
  Draw_Image_Screen(config_id-1);   /* shows that slot's image; add param text overlay if you want config details visible too */
  Draw_Config_Screen(config_id);
  Save_Schedule_State();
  current_screen = SCREEN_IMAGE;
  Update_Config_Screen();     /* draw all fields immediately, don't wait for next loop */
}

void Update_Config_Screen(void)
{
    char buf[32];
    StageConfig *s = &active_config.stages[current_stage_idx];
    uint8_t days = Get_Days_In_Current_Stage();

    if (current_stage_idx != cfg_last_stage || days != cfg_last_days)
    {
        sprintf(buf, "Config %d - Stage %d/6", active_config_id, current_stage_idx + 1);
        Show_Str(20, 70, BLACK, WHITE, (u8*)buf, 16, 0);

        sprintf(buf, "Days: %d/%d", days, s->day_repeat);
        Show_Str(20, 100, BLACK, WHITE, (u8*)buf, 16, 0);

        sprintf(buf, "Start: %02d:%02d  Dur: %dmin", s->start_hour, s->start_min, s->duration_min);
        Show_Str(20, 130, BLACK, WHITE, (u8*)buf, 16, 0);

        sprintf(buf, "Freq: %d  Power: %d", s->frequency, s->power);
        Show_Str(20, 160, BLACK, WHITE, (u8*)buf, 16, 0);

        cfg_last_stage = current_stage_idx;
        cfg_last_days  = days;
    }

    int8_t pwm = (int8_t)Get_PWM_Status();
    if (pwm != cfg_last_pwm)
    {
        if (pwm) Show_Str(20, 190, WHITE, RED, (u8*)"PWM: ON ", 16, 0);
        else     Show_Str(20, 190, WHITE, GRAY, (u8*)"PWM: OFF", 16, 0);
        cfg_last_pwm = pwm;
    }

    Get_Next_Job_Time(buf, sizeof(buf));
    if (strcmp(buf, cfg_last_next) != 0)
    {
        Show_Str(20, 220, BLACK, WHITE, (u8*)"                    ", 16, 0);   /* blank first - clears old, possibly longer/shorter text */
        Show_Str(20, 220, BLACK, WHITE, (u8*)buf, 16, 0);
        strcpy(cfg_last_next, buf);
    }
}

