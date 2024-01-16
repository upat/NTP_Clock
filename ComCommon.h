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
#include "CommonSetting.h"

typedef struct flag_bits {
  uint8_t bit01 : 1;
  uint8_t bit02 : 1;
  uint8_t bit03 : 1;
  uint8_t bit04 : 1;
  uint8_t bit05 : 1;
  uint8_t bit06 : 1;
  uint8_t bit07 : 1;
  uint8_t bit08 : 1;
} _FB;

/* フラグ用共用体 */
typedef union {
  uint8_t all_bits;
  _FB fb;
} _FLAG;

/* 変数定義 */
extern WiFiUDP UDP_NTP; /* NTP通信用 */

// extern _FLAG flag; /* 未使用 */
extern _FLAG err_flag;

#define flag_wifiinit_err ( err_flag.fb.bit08 ) /* wifi接続エラー判定フラグ */
#define flag_udpbegin_err ( err_flag.fb.bit07 ) /* udp接続エラー判定フラグ */

#define TIME_SERVER     "ntp.nict.jp" /* NTPサーバーのドメイン名(IP指定は非推奨) */
#define NTP_PORT        5000          /* NTPサーバーとのUDP通信ポート */
#define NTP_PACKET_SIZE 48            /* NTPサーバーとの通信で使用するパケットサイズ */
#define TIME_ZONE       9             /* タイムゾーンの設定(日本なら9) */
#define UPDATE_MIN_PRE  59            /* 1分経過直前の秒の値 */
#define BUFF_LENGTH     24            /* 受信バッファサイズ */

#if defined ( ESP32_8BIT )
#define SERIAL_SPEED 115200                  /* ESP32用シリアル通信ビットレート */
#define DHT_PIN 19                           /* DHT11 接続端子 */
#define HTTP_REQUEST "get_jma"               /* ESP32用HTTPリクエスト文字列 */
#define HTTP_DEFAULT  "----.-hPa ---% ---.-" /* ESP32用HTTP受信データ初期値 */
#elif defined ( ESP32 )
#define SERIAL_SPEED 115200                  /* ESP32用シリアル通信ビットレート */
#define DHT_PIN 33                           /* DHT11 接続端子 */
#define HTTP_REQUEST "get_jma"               /* ESP32用HTTPリクエスト文字列 */
#define HTTP_DEFAULT  "----.-hPa ---% ---.-" /* ESP32用HTTP受信データ初期値 */
#elif defined ( ESP8266 )
#define SERIAL_SPEED 74880          /* ESP8266用シリアル通信ビットレート */
#define DHT_PIN 14                  /* DHT11 接続端子 */
#define HTTP_REQUEST "get_jma_l"    /* ESP8266用HTTPリクエスト文字列 */
#define HTTP_DEFAULT  "---%  ---.-" /* ESP8266用HTTP受信データ初期値 */
#endif

extern byte     packetBuffer[NTP_PACKET_SIZE];

/* 関数定義 */
extern void     ComCommon_flag_init( void );
extern String   ComCommon_post_req( String request_data );
extern uint8_t  ComCommon_wifi_init( void );
extern void     sendNTPpacket( const char* address );
