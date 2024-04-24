#include "ComCommon.h"

WiFiUDP UDP_NTP;

/* 共用体宣言 */
// _FLAG flag; /* 未使用 */
_FLAG err_flag;

/* 変数宣言 */
byte  packetBuffer[NTP_PACKET_SIZE];

/*******************************************************************/
/* 処理内容：汎用フラグ初期化処理                                  */
/* 引数　　：なし                                                  */
/* 戻り値　：なし                                                  */
/* 備考　　：なし                                                  */
/*******************************************************************/
void ComCommon_flag_init( void )
{
  // flag.all_bits = 0x00; /* 未使用 */
  err_flag.all_bits = 0x00;
}

/*******************************************************************/
/* 処理内容：HTTPリクエスト処理                                    */
/* 引数　　：POSTメソッドで渡す文字列                              */
/* 戻り値　：なし                                                  */
/* 備考　　：なし                                                  */
/*******************************************************************/
String ComCommon_post_req( String request_data )
{
  HTTPClient http;
  WiFiClient wifi; /* ワーニング対策のため宣言のみ */
  String res_str = "";
  int httpCode;

  /* タイムアウト時間の設定(ms) */
  http.setTimeout( 500 );

  http.begin( wifi, HTTP_URL );
  httpCode = http.POST( request_data );
  
  if ( httpCode == HTTP_CODE_OK )
  {
    res_str = http.getString();
  }
  else if( String( "datelist" ) == request_data )
  {
  	/* datelistリクエストが失敗した場合 */
  	res_str = "1";
  }
  else if( ( String( "get_jma" ) == request_data ) || ( String( "get_jma_l" ) == request_data ) )
  {
  	/* リクエストが失敗した場合 */
  	res_str = HTTP_DEFAULT;
  }

  http.end();
  
  return res_str;
}

/*******************************************************************/
/* 処理内容：Wi-Fi初期化処理                                       */
/* 引数　　：なし                                                  */
/* 戻り値　：なし                                                  */
/* 備考　　：なし                                                  */
/*******************************************************************/
uint8_t ComCommon_wifi_init( void )
{
  /* 子機側に設定 */
  WiFi.mode( WIFI_STA );
  
  if( !WiFi.begin( AP_SSID, AP_PASS ) )
  {
    /* 失敗時 */
    return 1;
  }
  
  /* wi-fi出力強度設定(『4dBm ≒ Bluetooth Class2相当』を参考に設定) */
  if( !WiFi.setTxPower( WIFI_POWER_7dBm ) )
  {
    /* 失敗時 */
    return 1;
  }

  /* wi-fiの接続待ち(500ms*10回) */
  for( uint8_t loop = 0; loop < 10; loop++ )
  {
    if( WL_CONNECTED == WiFi.status() )
    {
      /* 成功時 */
      break; /* 接続できたらループ終了 */
    }
    delay( 500 );
  }

  if( WL_CONNECTED != WiFi.status() )
  {
    /* 失敗時 */
    return 1;
  }

  /* 成功時(未到達) */
  return 0;
}

/*** Timeライブラリのサンプルコードから移植 ***/
// send an NTP request to the time server at the given address
void sendNTPpacket( const char* address )
{
  // set all bytes in the buffer to 0
  memset( packetBuffer, 0, NTP_PACKET_SIZE );
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12]  = 49;
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;
  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:                 
  UDP_NTP.beginPacket( address, 123 ); //NTP requests are to port 123
  UDP_NTP.write( packetBuffer, NTP_PACKET_SIZE );
  UDP_NTP.endPacket();
}
