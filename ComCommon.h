#pragma once

#include <Arduino.h>

#if defined ESP8266
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#elif defined ESP32
#include <WiFi.h>
#include <HTTPClient.h>
#endif
#include <WiFiUDP.h>

#include <DHT.h>
#include <TimeLib.h>

#include "CommonSetting.h"

#define NTP_PACKET_SIZE 48                   /* NTPサーバーとの通信で使用するパケットサイズ */
#define COM_BUFFER_SIZE 24                   /* 通信用バッファサイズ */

#if defined (ESP32_8BIT)
#define SERIAL_SPEED 115200                  /* ESP32用シリアル通信ビットレート */
#define DHT_PIN 19                           /* DHT11 接続端子 */
#define HTTP_REQUEST "get_jma"               /* ESP32用HTTPリクエスト文字列 */
#define HTTP_DEFAULT  "----.-hPa ---% ---.-" /* ESP32用HTTP受信データ初期値 */
#elif defined (ESP32)
#define SERIAL_SPEED 115200                  /* ESP32用シリアル通信ビットレート */
#define DHT_PIN 33                           /* DHT11 接続端子 */
#define HTTP_REQUEST "get_jma"               /* ESP32用HTTPリクエスト文字列 */
#define HTTP_DEFAULT  "----.-hPa ---% ---.-" /* ESP32用HTTP受信データ初期値 */
#elif defined (ESP8266)
#define SERIAL_SPEED 74880                   /* ESP8266用シリアル通信ビットレート */
#define DHT_PIN 14                           /* DHT11 接続端子 */
#define HTTP_REQUEST "get_jma_l"             /* ESP8266用HTTPリクエスト文字列 */
#define HTTP_DEFAULT  "---%  ---.-"          /* ESP8266用HTTP受信データ初期値 */
#endif

/* Timeライブラリから取得したデータを管理するクラス */
class TimeData {
  public:
    uint16_t year_d;
    uint8_t month_d;
    uint8_t day_d;
    uint8_t weekday_d;
    uint8_t hour_d;
    uint8_t minute_d;
    uint8_t second_d;
    uint8_t min_dig;    /* minuteの1桁目の数値 */
    bool    sec_updflg; /* 秒更新フラグ */
    bool    min_updflg; /* 分更新フラグ 基本false、必要な時のみtrue */
    bool    day_updflg; /* 日更新フラグ 基本false、必要な時のみtrue */

    void ntp_init(void);   /* 初回処理関数 */
    void time_check(void); /* 更新チェック関数 */
  private:
    time_t _now;

    void _time_update(void); /* 更新関数 */
};

/* POSTリクエストで使用する構造体 */
typedef struct {
  char post_req[12];              /* POSTリクエスト文字列(最大9文字) */
  char recv_buf[COM_BUFFER_SIZE]; /* 受信バッファ */
} HttpPostBuf;

/* 変数定義 */
extern DHT dht;
extern TimeData timeData;
extern bool wifi_connect_error;

/* 関数定義 */
extern void   ComCommon_init(void);
extern void   ComCommon_post_req(HttpPostBuf *buf_ptr);
extern void   ComCommon_sleep(void);
extern time_t getNtpTime(void);
