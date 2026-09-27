#include "BT_RTC_PWM.h"
#include <stdio.h>
#include <string.h>
#include "App_UI.h"
#include "qspi_w25q64.h"
#include "main.h"
#include "nrf2401.h"   /* adjust to your actual NRF24 driver header filename */

extern UART_HandleTypeDef huart1;
extern RTC_HandleTypeDef  hrtc;
extern TIM_HandleTypeDef  htim2;

/* ==================================================================
BLUETOOTH RECEIVE - 6 config packets (AA...BB, 60 bytes each)
+ 1 RTC sync packet (7E...7E)
================================================================== */

#define BT_MAX_PAYLOAD     64
#define BT_STORAGE_ADDR    0x7FE000UL   /* one sector below calibration - no collision with image slots */
#define BT_RX_TIMEOUT_MS   300          /* if mid-packet with no new byte for this long, resync - protects against lost bytes */

typedef enum { BT_IDLE, BT_COLLECT_A, BT_COLLECT_RTC } BT_RxState;

static volatile BT_RxState bt_state = BT_IDLE;
static uint8_t   bt_rx_byte;
static uint8_t   bt_payload[BT_MAX_PAYLOAD];
static uint8_t   bt_payload_len = 0;
static volatile uint32_t bt_last_byte_tick = 0;

static uint8_t   bt_packets[6][BT_MAX_PAYLOAD];
static uint8_t   bt_packet_lens[6];
static uint8_t   bt_packet_count = 0;
static volatile uint8_t bt_all_six_ready = 0;

static uint8_t   bt_rtc_payload[BT_MAX_PAYLOAD];
static uint8_t   bt_rtc_len = 0;
static volatile uint8_t bt_rtc_ready = 0;

/* Packet layout (bytes AFTER the leading 0x7E, i.e. bt_rtc_payload[0..]):
[0]=packet ID (0x18)  [1]=year  [2]=month  [3]=date
[4]=weekday (Java Calendar: 1=Sun..7=Sat)  [5]=hour  [6]=min  [7]=sec
[8..17]=padding (0x00) */
#define RTC_IDX_PACKET_ID  0
#define RTC_IDX_YEAR       1
#define RTC_IDX_MONTH      2
#define RTC_IDX_DATE       3
#define RTC_IDX_WEEKDAY_JAVA 4
#define RTC_IDX_HOUR       5
#define RTC_IDX_MIN        6
#define RTC_IDX_SEC        7

#define BT_RTC_PACKET_ID   0x18


PlantConfig active_config;
uint8_t      active_config_loaded = 0;
uint8_t      active_config_id = 0;
uint8_t      current_stage_idx = 0;

uint8_t days_in_current_stage=0;
static uint8_t last_checked_day = 0xFF;
static uint8_t pwm_currently_on = 0;
extern SPI_HandleTypeDef hspi3;
uint8_t schedule_armed = 0;

void Arm_Schedule(void)  { schedule_armed = 1; }
void Disarm_Schedule(void) { schedule_armed = 0; }


void NRF_Send_Start(uint16_t freq, uint8_t power, uint8_t duration_min)
{
    uint8_t data[32] = {0};
    data[0] = 'S';           /* 'S' = start marker */
    data[1] = (uint8_t)(freq >> 8);
    data[2] = (uint8_t)(freq & 0xFF);
    data[3] = power;
    data[4] = duration_min;
    //NRF24_Transmit(&hspi3, data, sizeof(data));
}

void NRF_Send_End(void)
{
    uint8_t data[32] = {0};
    data[0] = 'E';           /* 'E' = end marker */
    //NRF24_Transmit(&hspi3, data, sizeof(data));
}


static void Parse_Config(const uint8_t *payload, PlantConfig *out)
{
    out->id = payload[0];
    memcpy(out->name, payload + 1, 15);
    out->name[15] = '\0';

    for (int i = 0; i < NUM_STAGES; i++)
    {
        const uint8_t *s = payload + 16 + i * 7;   /* was i * 8 */
        out->stages[i].day_repeat   = s[0];
        out->stages[i].start_hour   = s[1];
        out->stages[i].start_min    = s[2];
        out->stages[i].duration_min = s[3];
        out->stages[i].frequency    = ((uint16_t)s[4] << 8) | s[5];   /* was s[5]/s[6] */
        out->stages[i].power        = s[6];                            /* was s[7] */
    }
}

uint8_t Get_PWM_Status(void) { return pwm_currently_on; }
uint8_t Get_Days_In_Current_Stage(void) { return days_in_current_stage; }

void Get_Next_Job_Time(char *out_buf, uint8_t buf_len)
{
    if (!active_config_loaded) { snprintf(out_buf, buf_len, "No config"); return; }

    StageConfig *s = &active_config.stages[current_stage_idx];
    RTC_TimeTypeDef t = {0};
    RTC_DateTypeDef d = {0};
    HAL_RTC_GetTime(&hrtc, &t, RTC_FORMAT_BIN);
    HAL_RTC_GetDate(&hrtc, &d, RTC_FORMAT_BIN);   /* must follow GetTime on H7 - unlocks shadow regs */

    uint16_t now_min   = t.Hours * 60 + t.Minutes;
    uint16_t start_min = s->start_hour * 60 + s->start_min;
    uint16_t end_min    = start_min + s->duration_min;

    if (now_min >= start_min && now_min < end_min)
    {
        snprintf(out_buf, buf_len, "Running %02d/%02d - ends %02d:%02d",
                 d.Month, d.Date, end_min / 60, end_min % 60);
    }
    else if (now_min < start_min)
    {
        snprintf(out_buf, buf_len, "Next: %02d/%02d %02d:%02d",
                 d.Month, d.Date, s->start_hour, s->start_min);
    }
    else
    {
        /* today's window already passed - next occurrence is tomorrow.
           Avoiding full calendar rollover math here (month/year boundaries) -
           "Tomorrow" is unambiguous and safe without that complexity. */
        snprintf(out_buf, buf_len, "Next: Tomorrow %02d:%02d", s->start_hour, s->start_min);
    }
}

void Load_Config(uint8_t config_id)
{
    uint8_t payload[CONFIG_SIZE_BYTES];
    QSPI_W25Qxx_ReadBuffer(payload, CONFIG_STORAGE_ADDR + (uint32_t)(config_id - 1) * CONFIG_SIZE_BYTES, CONFIG_SIZE_BYTES);

    Parse_Config(payload, &active_config);
    active_config_loaded = 1;
    active_config_id = config_id;
    current_stage_idx = 0;
    days_in_current_stage = 0;
    last_checked_day = 0xFF;
}

static void Set_Power_Level(uint8_t level)
{
    if (level < 1 || level > 6) return;

    HAL_GPIO_WritePin(Power_1_GPIO_Port, Power_1_Pin, (level == 1) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(Power_2_GPIO_Port, Power_2_Pin, (level == 2) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(Power_3_GPIO_Port, Power_3_Pin, (level == 3) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(Power_4_GPIO_Port, Power_4_Pin, (level == 4) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(Power_5_GPIO_Port, Power_5_Pin, (level == 5) ? GPIO_PIN_SET : GPIO_PIN_RESET);
    HAL_GPIO_WritePin(Power_6_GPIO_Port, Power_6_Pin, (level == 6) ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

static void Power_Level_Off(void)
{
    HAL_GPIO_WritePin(Power_1_GPIO_Port, Power_1_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(Power_2_GPIO_Port, Power_2_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(Power_3_GPIO_Port, Power_3_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(Power_4_GPIO_Port, Power_4_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(Power_5_GPIO_Port, Power_5_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(Power_6_GPIO_Port, Power_6_Pin, GPIO_PIN_RESET);
}

void Schedule_Task(void)
{
    if (!active_config_loaded || !schedule_armed) return;

    RTC_TimeTypeDef t = {0};
    RTC_DateTypeDef d = {0};
    HAL_RTC_GetTime(&hrtc, &t, RTC_FORMAT_BIN);
    HAL_RTC_GetDate(&hrtc, &d, RTC_FORMAT_BIN);

    if (d.Date != last_checked_day)
    {
        if (last_checked_day != 0xFF)
        {
            days_in_current_stage++;
            if (days_in_current_stage >= active_config.stages[current_stage_idx].day_repeat)
            {
                days_in_current_stage = 0;
                current_stage_idx = (current_stage_idx + 1) % NUM_STAGES;
            }
        }
        last_checked_day = d.Date;
         Save_Schedule_State(); 
    }

    StageConfig *s = &active_config.stages[current_stage_idx];
    uint16_t now_min   = t.Hours * 60 + t.Minutes;
    uint16_t start_min = s->start_hour * 60 + s->start_min;
    uint16_t end_min   = start_min + s->duration_min;

    uint8_t should_run = (now_min >= start_min && now_min < end_min);

    if (should_run && !pwm_currently_on)
    {
        PWM_SetFrequency((uint32_t)s->frequency );
        HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_4);
        Set_Power_Level(s->power);
        NRF_Send_Start(s->frequency, s->power, s->duration_min);
        pwm_currently_on = 1;
    }
    else if (!should_run && pwm_currently_on)
    {
        HAL_TIM_PWM_Stop(&htim2, TIM_CHANNEL_4);
        Power_Level_Off();
        NRF_Send_End();
        pwm_currently_on = 0;
    }
}

static uint8_t Java_Weekday_To_HAL(uint8_t java_wd)
{
  /* Java: 1=Sun,2=Mon,3=Tue,4=Wed,5=Thu,6=Fri,7=Sat
  HAL:  1=Mon,2=Tue,3=Wed,4=Thu,5=Fri,6=Sat,7=Sun */
  return (java_wd == 1) ? 7 : (java_wd - 1);
}

void BT_UART_Init(void)
{
  HAL_NVIC_SetPriority(USART1_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(USART1_IRQn);
  HAL_UART_Receive_IT(&huart1, &bt_rx_byte, 1);
  bt_last_byte_tick = HAL_GetTick();
}

void BT_UART_ByteReceived(void)
{
  uint8_t b = bt_rx_byte;
  bt_last_byte_tick = HAL_GetTick();
  
  switch (bt_state)
  {
  case BT_IDLE:
    if (b == 0xAA)      { bt_payload_len = 0; bt_state = BT_COLLECT_A; }
    else if (b == 0x7E) { bt_payload_len = 0; bt_state = BT_COLLECT_RTC; }
    /* any other byte while idle is ignored - resync-friendly */
    break;
    
  case BT_COLLECT_A:
    if (b == 0xBB)
    {
      if (bt_packet_count < 6)
      {
        memcpy(bt_packets[bt_packet_count], bt_payload, bt_payload_len);
        bt_packet_lens[bt_packet_count] = bt_payload_len;
        bt_packet_count++;
        if (bt_packet_count == 6) { bt_all_six_ready = 1; bt_packet_count = 0; }
      }
      bt_state = BT_IDLE;
    }
    else if (bt_payload_len < BT_MAX_PAYLOAD) bt_payload[bt_payload_len++] = b;
    else bt_state = BT_IDLE;   /* overflow guard - malformed packet, drop and resync */
    break;
    
  case BT_COLLECT_RTC:
    if (b == 0x7E && bt_payload_len > 0)
    {
      memcpy(bt_rtc_payload, bt_payload, bt_payload_len);
      bt_rtc_len = bt_payload_len;
      bt_rtc_ready = 1;
      bt_state = BT_IDLE;
    }
    else if (bt_payload_len < BT_MAX_PAYLOAD) bt_payload[bt_payload_len++] = b;
    else bt_state = BT_IDLE;
    break;
  }
  
  HAL_UART_Receive_IT(&huart1, &bt_rx_byte, 1);   /* re-arm for next byte */
}

/* Safety net: if we're mid-packet and no new byte arrives for BT_RX_TIMEOUT_MS,
something was lost (BT disconnect, noise, partial send) - reset to IDLE so
we don't get permanently stuck waiting for a terminator that will never come. */
void BT_CheckTimeout(void)
{
  if (bt_state != BT_IDLE && (HAL_GetTick() - bt_last_byte_tick) > BT_RX_TIMEOUT_MS)
  {
    bt_state = BT_IDLE;
    bt_payload_len = 0;
  }
}

static uint8_t RTC_Sanity_Check(uint8_t hour, uint8_t min, uint8_t sec,
                                uint8_t weekday, uint8_t date, uint8_t month)
{
  if (hour > 23 || min > 59 || sec > 59) return 0;
  if (weekday < 1 || weekday > 7) return 0;
  if (date < 1 || date > 31) return 0;
  if (month < 1 || month > 12) return 0;
  return 1;
}

void BT_Process(void)
{
  if (bt_all_six_ready)
  {
    bt_all_six_ready = 0;
    
    uint8_t buf[6 * BT_MAX_PAYLOAD];
    uint16_t offset = 0;
    for (int i = 0; i < 6; i++)
    {
      memcpy(buf + offset, bt_packets[i], bt_packet_lens[i]);
      offset += bt_packet_lens[i];
    }
    QSPI_W25Qxx_SectorErase(BT_STORAGE_ADDR);
    QSPI_W25Qxx_WriteBuffer(buf, BT_STORAGE_ADDR, offset);
  }
  
  if (bt_rtc_ready)
  {
    bt_rtc_ready = 0;
    if (bt_rtc_len >= 8 && bt_rtc_payload[RTC_IDX_PACKET_ID] == BT_RTC_PACKET_ID)
    {
      uint8_t y  = bt_rtc_payload[RTC_IDX_YEAR];
      uint8_t mo = bt_rtc_payload[RTC_IDX_MONTH];
      uint8_t d  = bt_rtc_payload[RTC_IDX_DATE];
      uint8_t wd = Java_Weekday_To_HAL(bt_rtc_payload[RTC_IDX_WEEKDAY_JAVA]);
      uint8_t h  = bt_rtc_payload[RTC_IDX_HOUR];
      uint8_t mi = bt_rtc_payload[RTC_IDX_MIN];
      uint8_t s  = bt_rtc_payload[RTC_IDX_SEC];
      
      if (RTC_Sanity_Check(h, mi, s, wd, d, mo))
        RTC_SetDateTime(h, mi, s, wd, d, mo, y);
      /* else: garbled/wrong packet - silently dropped, RTC keeps last good value */
    }
  }
}

/* ==================================================================
RTC
================================================================== */
void RTC_SetDateTime(uint8_t hour, uint8_t min, uint8_t sec,
                     uint8_t weekday, uint8_t date, uint8_t month, uint8_t year)
{
  RTC_TimeTypeDef sTime = {0};
  RTC_DateTypeDef sDate = {0};
  
  sTime.Hours   = hour;
  sTime.Minutes = min;
  sTime.Seconds = sec;
  HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
  
  sDate.WeekDay = weekday;
  sDate.Date    = date;
  sDate.Month   = month;
  sDate.Year    = year;
  HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BIN);
}

/* ==================================================================
PWM - TIM2_CH4, 50% duty, variable frequency 10-50kHz
Prerequisite: CubeMX TIM2 CH4 configured for PWM, Prescaler = 0
================================================================== */
void PWM_Init(void)
{
  HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_4);
  PWM_SetFrequency(10000);
}

#define TIM2_CLK_HZ 200000000UL

void PWM_SetFrequency(uint32_t freq_hz)
{
__HAL_TIM_SET_PRESCALER(&htim2, 0);

uint32_t period = (TIM2_CLK_HZ + freq_hz / 2) / freq_hz;
uint32_t arr = period - 1;
uint32_t ccr = period / 2;

__HAL_TIM_SET_AUTORELOAD(&htim2, arr);
__HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_4, ccr);

TIM2->EGR |= TIM_EGR_UG;
}

void PWM_SetFrequency_1_(uint32_t freq_hz)
{
  uint32_t timer_clk = HAL_RCC_GetPCLK1Freq();
  RCC_ClkInitTypeDef clk; uint32_t flash_latency;
  HAL_RCC_GetClockConfig(&clk, &flash_latency);
  if (clk.APB1CLKDivider != RCC_HCLK_DIV1) timer_clk *= 2;
  
  uint32_t arr = (timer_clk / freq_hz) - 1;
  uint32_t ccr = (arr + 1) / 2;
  
  __HAL_TIM_SET_AUTORELOAD(&htim2, arr);
  __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_4, ccr);
}

void Save_Schedule_State(void)
{
    PersistedState st;
    st.config_id            = active_config_id;
    st.stage_idx             = current_stage_idx;
    st.days_in_stage         = days_in_current_stage;
    st.last_day              = last_checked_day;
    st.armed                 = schedule_armed;
    st.was_on_config_screen  = (current_screen == SCREEN_IMAGE) ? 1 : 0;
    st.magic                 = 0xA5;

    QSPI_W25Qxx_SectorErase(STATE_STORAGE_ADDR);
    QSPI_W25Qxx_WriteBuffer((uint8_t*)&st, STATE_STORAGE_ADDR, sizeof(st));
}

void Set_Schedule_Progress(uint8_t stage, uint8_t days, uint8_t last_day, uint8_t armed)
{
    current_stage_idx     = stage;   /* this one's already a non-static global, fine either way */
    days_in_current_stage = days;    /* static - needs this setter */
    last_checked_day      = last_day; /* static - needs this setter */
    schedule_armed         = armed;   /* already global */
}

uint8_t Load_Schedule_State(uint8_t *out_config_id, uint8_t *out_stage, uint8_t *out_days,
                             uint8_t *out_last_day, uint8_t *out_armed)
{
    PersistedState st;
    QSPI_W25Qxx_ReadBuffer((uint8_t*)&st, STATE_STORAGE_ADDR, sizeof(st));

    if (st.magic == 0xA5 && st.config_id >= 1 && st.config_id <= 6)
    {
        *out_config_id = st.config_id;
        *out_stage      = st.stage_idx;
        *out_days       = st.days_in_stage;
        *out_last_day   = st.last_day;
        *out_armed      = st.armed;
        return st.was_on_config_screen;
    }
    return 0;
}

uint8_t Load_Schedule_State_1_(uint8_t *out_config_id)   /* now reports which config, doesn't load it itself */
{
    PersistedState st;
    QSPI_W25Qxx_ReadBuffer((uint8_t*)&st, STATE_STORAGE_ADDR, sizeof(st));

    if (st.magic == 0xA5 && st.config_id >= 1 && st.config_id <= 6)
    {
        *out_config_id         = st.config_id;
        current_stage_idx      = st.stage_idx;
        days_in_current_stage  = st.days_in_stage;
        last_checked_day       = st.last_day;
        schedule_armed         = st.armed;
        return st.was_on_config_screen;
    }
    return 0;
}