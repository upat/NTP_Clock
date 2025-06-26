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

typedef struct flag_bits {
  uint8_t bit1 : 1;
  uint8_t bit2 : 1;
  uint8_t bit3 : 1;
  uint8_t bit4 : 1;
  uint8_t bit5 : 1;
  uint8_t bit6 : 1;
  uint8_t bit7 : 1;
  uint8_t bit8 : 1;
} _FB;

/* フラグ用共用体 */
typedef union {
  uint8_t all_bits;
  _FB fb;
} _FLAG;

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
    uint8_t hour_dig;   /* hourが1桁なら1それ以外0 */
    bool    sec_updflg; /* 秒更新フラグ */
    bool    min_updflg; /* 分更新フラグ 基本false、必要な時のみtrue */
    bool    day_updflg; /* 日更新フラグ 基本false、必要な時のみtrue */

    void ntp_init(void);   /* 初回処理関数 */
    void time_check(void); /* 更新チェック関数 */
  private:
    time_t _now;

    void _time_update(void); /* 更新関数 */
};

#define flag_wifiinit_err (err_flag.fb.bit8) /* wifi接続エラー判定フラグ */
#define flag_udpbegin_err (err_flag.fb.bit7) /* udp接続エラー判定フラグ */

#define TIME_SERVER     "ntp.nict.jp" /* NTPサーバーのドメイン名(IP指定は非推奨) */
#define NTP_PORT        5000          /* NTPサーバーとのUDP通信ポート */
#define NTP_PACKET_SIZE 48            /* NTPサーバーとの通信で使用するパケットサイズ */
#define TIME_ZONE       9             /* タイムゾーンの設定(日本なら9) */
#define BUFF_LENGTH     24            /* 受信バッファサイズ */

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
#define SERIAL_SPEED 74880          /* ESP8266用シリアル通信ビットレート */
#define DHT_PIN 14                  /* DHT11 接続端子 */
#define HTTP_REQUEST "get_jma_l"    /* ESP8266用HTTPリクエスト文字列 */
#define HTTP_DEFAULT  "---%  ---.-" /* ESP8266用HTTP受信データ初期値 */
#endif

/* 変数定義 */
extern _FLAG err_flag;
extern DHT dht;
extern TimeData timeData;

/* 関数定義 */
extern void   ComCommon_init(void);
extern void   ComCommon_post_req(char *response_data, String request_data);
extern time_t getNtpTime(void);
