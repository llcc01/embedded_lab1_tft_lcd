#include "page.h"

#include <rtc.h>
#include <stdio.h>
#include <string.h>
#include <usart.h>
#include <time.h>

#include "cJSON.h"
#include "lvgl.h"
#include "wifi.h"

struct weather_data_t {
  char* date;
  char* high;
  char* low;
  char* ymd;
  char* week;
  char* sunrise;
  char* sunset;
  int aqi;
  char* fx;
  char* fl;
  char* type;
  char* notice;
  int8_t high_int;
  int8_t low_int;
};

typedef struct _lv_clock {
  lv_obj_t* raw_label;      // 原始标签
  lv_obj_t* time_label;     // 时间标签
  lv_obj_t* date_label;     // 日期标签
  lv_obj_t* weekday_label;  // 星期标签
} lv_clock_t;

const char at_cipstart_weather[] =
"AT+CIPSTART=\"TCP\",\"t.weather.sojson.com\",80\r\n";  // 建立TCP链接
const char at_cipstart_time[] =
"AT+CIPSTART=\"UDP\",\"ntp1.aliyun.com\",123\r\n";  // 建立UDP链接
const char at_cipmux[] = "AT+CIPMUX=0\r\n";             // 设置为单链接
const char at_cipmode[] = "AT+CIPMODE=%d\r\n";          // 透传模式
const char at_cipsend[] = "AT+CIPSEND\r\n";             // 发送数据
const char at_cipsend_time[] = "AT+CIPSEND=48\r\n";     // 发送数据
const char at_cipclose[] = "AT+CIPCLOSE\r\n";           // 关闭链接

const char quit_transparent_transmission[] = "+++";  // 退出透传模式

const char http_get_weather[] =
"GET /api/weather/city/101030100 HTTP/1.1\r\nHost: "
"t.weather.sojson.com\r\n\r\n";  // 获取天气

const uint8_t udp_time[48] = {
    0xE3, 0x00, 0x06, 0xEC, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x31, 0x4E, 0x31, 0x34, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

char rx_data[2048];
time_t last_time = 0;

lv_obj_t* tv;
char* weather_data;
char* city;
RTC_DateTypeDef rtc_date;  // 获取日期结构体
RTC_TimeTypeDef rtc_time;  // 获取时间结构体

void get_weather() {
  /*********************** 建立TCP链接 *****************************ok*/
  printf("%s", at_cipstart_weather);
  HAL_Delay(500);

  /*********************** 设置为单链接 *****************************ok*/
  printf("%s", at_cipmux);
  HAL_Delay(500);

  /*********************** 透传模式 *****************************ok*/
  printf(at_cipmode, 1);
  HAL_Delay(500);

  /*********************** 发送数据 *****************************ok*/
  printf("%s", at_cipsend);
  HAL_Delay(500);

  /*********************** 发送数据 *****************************ok*/
  printf("%s", http_get_weather);

  /*********************** 接收数据 *****************************ok*/
  while (rx_data[0] != '{') {
    gets(rx_data);
  }

  /*********************** 退出透传模式 *****************************ok*/
  printf("%s", quit_transparent_transmission);
  HAL_Delay(500);

  /*********************** 关闭链接 *****************************ok*/
  printf("%s", at_cipclose);
}

void update_time() {
  printf("%s", at_cipstart_time);
  HAL_Delay(500);

  printf("%s", at_cipmux);
  HAL_Delay(500);

  printf(at_cipmode, 1);
  HAL_Delay(500);

  printf("%s", at_cipsend);
  HAL_Delay(500);

  for (int i = 0; i < 48; i++) {
    putchar(udp_time[i]);
  }

  // while (!(rx_data[0] = getchar()));

  // for (int i = 1; i < 10; i++) {
  //   rx_data[i] = getchar();
  // }

  HAL_Delay(500);

  printf("%s", quit_transparent_transmission);
  HAL_Delay(500);

  printf("%s", at_cipclose);


  struct tm* timeinfo = NULL;
  last_time |= rx_data[40] << 24;
  last_time |= rx_data[41] << 16;
  last_time |= rx_data[42] << 8;
  last_time |= rx_data[43];

  timeinfo = localtime(&last_time);

  rtc_date.Year = timeinfo->tm_year - 2000;
  rtc_date.Month = timeinfo->tm_mon + 1;
  rtc_date.Date = timeinfo->tm_mday;
  rtc_date.WeekDay = timeinfo->tm_wday + 1;

  rtc_time.Hours = timeinfo->tm_hour;
  rtc_time.Minutes = timeinfo->tm_min;
  rtc_time.Seconds = timeinfo->tm_sec;

  // HAL_RTC_SetTime(&hrtc, &rtc_time, RTC_FORMAT_BIN);
  // HAL_RTC_SetDate(&hrtc, &rtc_date, RTC_FORMAT_BIN);
}



struct weather_data_t weather_data_array[15];

void get_weather_data() {
  cJSON* root = cJSON_Parse(rx_data);
  cJSON* data = cJSON_GetObjectItem(root, "data");
  cJSON* forecast = cJSON_GetObjectItem(data, "forecast");
  cJSON* item = NULL;
  int i = 0;
  cJSON_ArrayForEach(item, forecast) {
    cJSON* date = cJSON_GetObjectItem(item, "date");
    cJSON* high = cJSON_GetObjectItem(item, "high");
    cJSON* low = cJSON_GetObjectItem(item, "low");
    cJSON* ymd = cJSON_GetObjectItem(item, "ymd");
    cJSON* week = cJSON_GetObjectItem(item, "week");
    cJSON* sunrise = cJSON_GetObjectItem(item, "sunrise");
    cJSON* sunset = cJSON_GetObjectItem(item, "sunset");
    cJSON* aqi = cJSON_GetObjectItem(item, "aqi");
    cJSON* fx = cJSON_GetObjectItem(item, "fx");
    cJSON* fl = cJSON_GetObjectItem(item, "fl");
    cJSON* type = cJSON_GetObjectItem(item, "type");
    cJSON* notice = cJSON_GetObjectItem(item, "notice");

    weather_data_array[i].date = date->valuestring;
    weather_data_array[i].high = high->valuestring;
    sscanf(weather_data_array[i].high, "高温 %d℃",
      &weather_data_array[i].high_int);
    weather_data_array[i].low = low->valuestring;
    sscanf(weather_data_array[i].low, "低温 %d℃",
      &weather_data_array[i].low_int);
    weather_data_array[i].ymd = ymd->valuestring;
    weather_data_array[i].week = week->valuestring;
    weather_data_array[i].sunrise = sunrise->valuestring;
    weather_data_array[i].sunset = sunset->valuestring;
    weather_data_array[i].aqi = aqi->valueint;
    weather_data_array[i].fx = fx->valuestring;
    weather_data_array[i].fl = fl->valuestring;
    weather_data_array[i].type = type->valuestring;
    weather_data_array[i].notice = notice->valuestring;
    i++;
  }
}

static void clock_date_task_callback(lv_timer_t* timer) {
  static const char* week_day[7] = { "Sunday",    "Monday",   "Tuesday",
                                    "Wednesday", "Thursday", "Friday",
                                    "Saturday" };

  HAL_RTC_GetTime(&hrtc, &rtc_time, RTC_FORMAT_BIN);
  HAL_RTC_GetDate(&hrtc, &rtc_date, RTC_FORMAT_BIN);

  if (timer != NULL && timer->user_data != NULL) {
    lv_clock_t* clock = (lv_clock_t*)(timer->user_data);

    lv_label_set_text_fmt(clock->raw_label, "%x %x %x %x %x %x %x %x %x %x %x",
      rx_data[0], rx_data[1], rx_data[2], rx_data[3],
      rx_data[4], rx_data[5], rx_data[6], rx_data[7],
      rx_data[8], rx_data[9], rx_data[10]);

    if (clock->time_label != NULL) {
      lv_label_set_text_fmt(clock->time_label, "%02d:%02d:%02d", rtc_time.Hours,
        rtc_time.Minutes, rtc_time.Seconds);
      lv_obj_align_to(clock->time_label, lv_obj_get_parent(clock->time_label),
        LV_ALIGN_CENTER, 0, 0);
    }

    if (clock->date_label != NULL) {
      lv_label_set_text_fmt(clock->date_label, "%d-%02d-%02d",
        rtc_date.Year + 2000, rtc_date.Month,
        rtc_date.Date);
      lv_obj_align_to(clock->date_label, lv_obj_get_parent(clock->date_label),
        LV_ALIGN_TOP_MID, 2, 0);
    }

    if (clock->weekday_label != NULL) {
      lv_label_set_text_fmt(clock->weekday_label, "%s",
        week_day[rtc_date.WeekDay]);
      lv_obj_align_to(clock->weekday_label,
        lv_obj_get_parent(clock->weekday_label),
        LV_ALIGN_BOTTOM_MID, -2, 0);
    }
  }
}

static void chart_event_cb(lv_event_t* e) {
  lv_event_code_t code = lv_event_get_code(e);
  lv_obj_t* obj = lv_event_get_target(e);

  if (code == LV_EVENT_DRAW_PART_BEGIN) {
    lv_obj_draw_part_dsc_t* dsc = lv_event_get_param(e);
    if (dsc->part == LV_PART_TICKS && dsc->id == LV_CHART_AXIS_PRIMARY_X) {
      lv_snprintf(dsc->text, dsc->text_length, "%s",
        weather_data_array[dsc->value].date);
    }
  }
}

void weather_create(lv_obj_t* parent) {
  get_weather_data();

  // chart
  lv_obj_t* chart = lv_chart_create(parent);

  lv_obj_set_size(chart, 400, 300);
  lv_obj_set_align(chart, LV_ALIGN_TOP_MID);

  lv_obj_set_grid_cell(chart, LV_GRID_ALIGN_STRETCH, 1, 1,
    LV_GRID_ALIGN_STRETCH, 1, 1);
  lv_chart_set_axis_tick(chart, LV_CHART_AXIS_PRIMARY_X, 0, 0, 15, 1, true, 50);
  lv_chart_set_axis_tick(chart, LV_CHART_AXIS_PRIMARY_Y, 0, 0, 7, 1, true, 50);

  // lv_chart_set_type(chart, LV_CHART_TYPE_LINE);
  lv_chart_set_range(chart, LV_CHART_AXIS_PRIMARY_Y, -20, 40);
  // lv_chart_set_point_count(chart, 10);
  lv_chart_set_div_line_count(chart, 0, 15);
  lv_chart_set_point_count(chart, 15);

  lv_obj_set_style_border_side(chart,
    LV_BORDER_SIDE_LEFT | LV_BORDER_SIDE_BOTTOM, 0);
  // lv_obj_set_style_radius(chart, 0, 0);

  lv_obj_add_event_cb(chart, chart_event_cb, LV_EVENT_ALL, NULL);

  lv_chart_series_t* ser1 = lv_chart_add_series(
    chart, lv_theme_get_color_secondary(chart), LV_CHART_AXIS_PRIMARY_Y);
  lv_chart_series_t* ser2 = lv_chart_add_series(
    chart, lv_theme_get_color_primary(chart), LV_CHART_AXIS_PRIMARY_Y);

  for (int i = 0; i < 15; i++) {
    lv_chart_set_next_value(chart, ser1, weather_data_array[i].high_int);
    lv_chart_set_next_value(chart, ser2, weather_data_array[i].low_int);
  }
}

void time_create(lv_obj_t* parent) {
  /* Time & Date */
  lv_obj_t* time_date_obj =
    lv_obj_create(parent);  // 基于屏幕创建时间日期对象

  lv_obj_set_size(time_date_obj, 320, 100);  // 设置对象大小
  lv_obj_center(time_date_obj);              // 对象居屏幕中间显示

  /*Time display*/
  lv_obj_t* time_obj =
    lv_obj_create(time_date_obj);  // 基于time_date_obj对象创建时间对象

  lv_obj_set_size(time_obj, 158, 100);  // 设置对象大小
  lv_obj_align_to(time_obj, time_date_obj, LV_ALIGN_LEFT_MID, 0,
    0);  // 设置time_obj对象基于time_date_obj对象左边中间对齐

  static lv_clock_t lv_clock = { 0 };

  lv_clock.raw_label = lv_label_create(
    parent);  // 基于time_obj对象创建原始标签对象 lv_clock.raw_label

  lv_clock.time_label = lv_label_create(
    time_obj);  // 基于time_obj对象创建时间显示标签对象 lv_clock.time_label

  /*Date display*/
  lv_obj_t* date_obj =
    lv_obj_create(time_date_obj);  // 基于time_date_obj对象创建date_obj对象
  lv_obj_set_size(date_obj, 158, 100);  // 设置对象大小
  lv_obj_align_to(date_obj, time_date_obj, LV_ALIGN_RIGHT_MID, 0,
    0);  // 设置date_obj对象基于time_date_obj对象右边中部对齐

  lv_clock.date_label = lv_label_create(
    date_obj);  // 基于date_obj对象创建lv_clock.date_label日期显示对象

  /*Week display*/
  lv_clock.weekday_label = lv_label_create(
    date_obj);  // 基于date_obj对象创建星期显示lv_clock.weekday_label对象

  // 设置时间标签lv_clock.time_label对象基于父对象居中对齐
  lv_obj_align_to(lv_clock.time_label, lv_obj_get_parent(lv_clock.time_label),
    LV_ALIGN_CENTER, 0, 0);
  // 设置时间标签lv_clock.date_label对象基于父对象顶部中间对齐
  lv_obj_align_to(lv_clock.date_label, lv_obj_get_parent(lv_clock.date_label),
    LV_ALIGN_TOP_MID, 2, 0);
  // 设置时间标签lv_clock.weekday_label对象基于父对象底部中间对齐
  lv_obj_align_to(lv_clock.weekday_label,
    lv_obj_get_parent(lv_clock.weekday_label),
    LV_ALIGN_BOTTOM_MID, -2, 0);

  lv_timer_t* task_timer =
    lv_timer_create(clock_date_task_callback, 200,
      (void*)&lv_clock);  // 创建定时任务，200ms刷新一次
}

void lv_lc_widgets() {
  // LV_FONT_DECLARE(lv_font_simsun_16_cjk);
  tv = lv_tabview_create(lv_scr_act(), LV_DIR_TOP, 45);
  // lv_obj_set_style_text_font(tv, &lv_font_simsun_16_cjk, 0);

  lv_obj_t* tab1 = lv_tabview_add_tab(tv, "time");
  lv_obj_t* tab2 = lv_tabview_add_tab(tv, "weather");

  time_create(tab1);
  weather_create(tab2);
}