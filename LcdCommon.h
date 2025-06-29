#pragma once

#include <Arduino.h>
//#include "BitmapIcon.h"

#include <SPI.h>
#include <Adafruit_GFX.h>

#if defined (ESP32_8BIT)
  /* Arduino core for the ESP32 v3.*.*でMCUFRIEND_kbv v3.0.0を使用する場合、mcufriend_shield.hの1026行目に以下を追加 */
  /* #include "hal/gpio_ll.h" */
  /* 参考：https://github.com/prenticedavid/MCUFRIEND_kbv/issues/255 */
  #include <MCUFRIEND_kbv.h>
  // #include <TouchScreen.h> /* 使用予定なし */
#elif defined (ESP32)
  #include <Adafruit_ILI9341.h>
#elif defined (ESP8266)
  #include <Wire.h>
  #include <Adafruit_SSD1306.h>
#endif

#define DAY_FORMAT    "%04d/%02d/%02d"  /* 年月日フォーマット */
#define TIME_FORMAT   "%d:%02d:%02d"    /* 時刻フォーマット */
#define SENSOR_FORMAT "%2.0f%% %2.0f"   /* 温湿度フォーマット */

#define LCD_BUFFER_SIZE          24     /* 画面表示用バッファサイズ */

#if defined (ESP32)
#define LCD_COMMON_WIDTH         320    /* ILI9341 横幅 */
#define LCD_COMMON_BLACK         0x0000 /* 黒 */
#define LCD_COMMON_ORANGE        0xFD20 /* オレンジ */
#define LCD_COMMON_DEEPPINK      0xf892 /* ピンクといいつつ紫 */
#define LCD_COMMON_DARKTURQUOISE 0x0679 /* ターコイズといいつつ水色 */
#elif defined (ESP8266)
#define LCD_COMMON_WIDTH         128    /* SSD1306 横幅 */
#endif

/* 画面表示に使用する構造体 */
typedef struct {
  uint8_t str_len;                /* 文字列の長さ≠バッファサイズ */
  char disp_buf[LCD_BUFFER_SIZE]; /* 画面表示用バッファ */
} DispBuf;

/* 関数定義 */
extern void LcdCommon_init(void);
extern void LcdCommon_init_fail(void);
extern void LcdCommon_draw_date(char *day_data, uint8_t weekday);
extern void LcdCommon_draw_weather(DispBuf *postbuf, DispBuf *dispbuf);
extern void LcdCommon_draw_time(DispBuf *dispbuf);
#if 0
extern void LcdCommon_wake(void);
#endif
extern void LcdCommon_sleep(void);
