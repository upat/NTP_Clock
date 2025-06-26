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

/* 関数定義 */
extern void   ComCommon_post_req(char *response_data, String request_data);
extern void   ComCommon_init(void);
extern time_t getNtpTime(void);
