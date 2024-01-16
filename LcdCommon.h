#pragma once

#include <Arduino.h>
#include "BitmapIcon.h"

#include <SPI.h>
#include <Adafruit_GFX.h>

#if defined ( ESP32_8BIT )
  #include <MCUFRIEND_kbv.h>
  // #include <TouchScreen.h> /* 使用予定なし */
#elif defined ( ESP32 )
  #include <Adafruit_ILI9341.h>
#elif defined ( ESP8266 )
  #include <Wire.h>
  #include <Adafruit_SSD1306.h>
#endif

#define DAY_FORMAT    "%04d/%02d/%02d" /* 年月日フォーマット */
#define TIME_FORMAT   "%d:%02d:%02d"   /* 時刻フォーマット */
#define SENSOR_FORMAT "%2.0f%% %2.0f"  /* 温湿度フォーマット */

#if defined ( ESP32 )
#define LCD_COMMON_WIDTH  320          /* ILI9341 横幅 */
#define LCD_COMMON_BLACK  0x0000       /* 黒 */
#define LCD_COMMON_ORANGE 0xFD20       /* オレンジ */
#elif defined ( ESP8266 )
#define LCD_COMMON_WIDTH  128          /* SSD1306 横幅 */
#endif

/* 関数定義 */
extern void     LcdCommon_init( void );
extern void     LcdCommon_init_fail( uint8_t flag );
extern void     LcdCommon_draw_date( char *day_data, int weekday );
extern void     LcdCommon_draw_weather( char *http_buff, char *sensor_data, uint8_t buff_length );
extern void     LcdCommon_draw_time( char *time_data, uint8_t hour_digflg );
#if 0
extern void     LcdCommon_wake( void );
#endif
extern void     LcdCommon_sleep( void );
