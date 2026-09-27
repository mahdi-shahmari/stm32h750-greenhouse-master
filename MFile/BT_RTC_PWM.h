#ifndef __BT_RTC_PWM_H__
#define __BT_RTC_PWM_H__

#include "sys.h"
#include "stm32h7xx_hal.h"


#define NUM_STAGES            6
#define NUM_CONFIGS           6
#define CONFIG_SIZE_BYTES     58
#define CONFIG_STORAGE_ADDR   0x7FE000UL
#define STATE_STORAGE_ADDR    0x7FD000UL

extern uint8_t days_in_current_stage;
extern uint8_t schedule_armed;




typedef struct {
    uint8_t  day_repeat;
    uint8_t  start_hour;
    uint8_t  start_min;
    uint8_t  duration_min;
    uint16_t frequency;    /* real Hz value, big-endian on the wire */
    uint8_t  power;        /* 1-6, selects which power pin is HIGH */
} StageConfig;

typedef struct {
    uint8_t     id;
    char        name[16];
    StageConfig stages[NUM_STAGES];
} PlantConfig;

extern PlantConfig active_config;
extern uint8_t      active_config_loaded;
extern uint8_t      active_config_id;
extern uint8_t      current_stage_idx;

typedef struct {
    uint8_t config_id;
    uint8_t stage_idx;
    uint8_t days_in_stage;
    uint8_t last_day;
    uint8_t armed;
    uint8_t was_on_config_screen;
    uint8_t magic;
} PersistedState;

extern uint8_t schedule_armed;

void Save_Schedule_State(void);
uint8_t Load_Schedule_State(uint8_t *out_config_id, uint8_t *out_stage, uint8_t *out_days,
                             uint8_t *out_last_day, uint8_t *out_armed);
void Set_Schedule_Progress(uint8_t stage, uint8_t days, uint8_t last_day, uint8_t armed);
uint8_t Get_Days_In_Current_Stage(void);
uint8_t Get_PWM_Status(void);

void Load_Config(uint8_t config_id);
void Schedule_Task(void);

/* ---------------- Bluetooth / UART4 ---------------- */
void BT_UART_Init(void);
void BT_UART_ByteReceived(void);   /* call from HAL_UART_RxCpltCallback */
void BT_Process(void);             /* call every loop iteration in main() */
void BT_CheckTimeout(void);        /* call every loop iteration in main() - safety net for lost/partial packets */

/* ---------------- RTC ---------------- */
void RTC_SetDateTime(uint8_t hour, uint8_t min, uint8_t sec,
                      uint8_t weekday, uint8_t date, uint8_t month, uint8_t year);

/* ---------------- PWM (TIM2_CH4) ---------------- */
void PWM_Init(void);
void PWM_SetFrequency(uint32_t freq_hz);   /* clamped 10000-50000, always 50% duty */
uint8_t Get_Days_In_Current_Stage(void);

void Get_Next_Job_Time(char *out_buf, uint8_t buf_len);
void NRF_Send_Start(uint16_t freq, uint8_t power, uint8_t duration_min);
void NRF_Send_End(void);

void Arm_Schedule(void);
void Disarm_Schedule(void);
//uint8_t Load_Schedule_State(uint8_t *out_config_id);
uint8_t Load_Schedule_State(uint8_t *out_config_id, uint8_t *out_stage, uint8_t *out_days,
                             uint8_t *out_last_day, uint8_t *out_armed);
void Save_Schedule_State(void);
void Set_Schedule_Progress(uint8_t stage, uint8_t days, uint8_t last_day, uint8_t armed);
#endif