#include "ComCommon.h"

WiFiUDP UDP_NTP;

/* 共用体宣言 */
// _FLAG flag; /* 未使用 */
_FLAG err_flag;

/* 変数宣言 */
byte packetBuffer[NTP_PACKET_SIZE];

/*******************************************************************/
/* 処理内容：汎用フラグ初期化処理                                  */
/* 引数　　：なし                                                  */
/* 戻り値　：なし                                                  */
/* 備考　　：なし                                                  */
/*******************************************************************/
void ComCommon_flag_init(void)
{
  // flag.all_bits = 0x00; /* 未使用 */
  err_flag.all_bits = 0xC0; /* フラグクリアは各制御で行う */
}

/*******************************************************************/
/* 処理内容：HTTPリクエスト処理                                    */
/* 引数　　：POSTメソッドで渡す文字列                              */
/* 戻り値　：HTTPレスポンス                                        */
/* 備考　　：なし                                                  */
/*******************************************************************/
String ComCommon_post_req(String request_data)
{
  HTTPClient http;
  String res_str = "";
  int httpCode;
  uint8_t byte_count = 0;

  /* リクエストに対応した初期値のセット */
  if (-1 < request_data.indexOf("date")) {       /* datelistリクエスト */
    res_str = "1";
  } else if (-1 < request_data.indexOf("jma")) { /* get_jmaリクエスト */
    res_str = HTTP_DEFAULT;
  }

  if (WiFi.status() != WL_CONNECTED) { /* wi-fi接続が切れていた場合再接続 */
      WiFi.disconnect();
      WiFi.reconnect(); /* 次回の通信は10分後になるため待ち処理は未実施 */
  } else {
    /* タイムアウト時間の設定(ms) */
    http.setTimeout(100);
    /* 通信開始 */
    http.begin(HTTP_URL);
    http.addHeader("Content-Type", "application/text");
    httpCode = http.POST(request_data);

    if (httpCode == HTTP_CODE_OK) {
      res_str = ""; /* 初期値クリア */
      delay(20);    /* 環境依存でPOSTリクエストの後20ms待ち */
      WiFiClient *stream = http.getStreamPtr();
      
      /* 受信データが1byte以上あり */
      while ((0 < stream->available())
          && (BUFF_LENGTH > byte_count)) {
        char c_temp = stream->read(); /* 1byteずつ読み出し */
        res_str += c_temp;            /* 結合 */
        byte_count++;                 /* 受信バイト数をカウント(最大24回想定) */
        delayMicroseconds(500);
      }
    }
    /* 通信終了 */
    http.end();
  }

  return res_str;
}

/*******************************************************************/
/* 処理内容：Wi-Fi初期化処理                                       */
/* 引数　　：なし                                                  */
/* 戻り値　：なし                                                  */
/* 備考　　：なし                                                  */
/*******************************************************************/
void ComCommon_wifi_init(void)
{
  /* 子機側に設定 */
  WiFi.mode(WIFI_STA);
  
  if ((!WiFi.begin(AP_SSID, AP_PASS))        /* wifi接続 */
   && (!WiFi.setTxPower(WIFI_POWER_7dBm))) { /* wifi出力強度設定(『4dBm ≒ Bluetooth Class2相当』を参考に設定) */
    /* 失敗時フラグクリアをしない */
    return;
  }
  
  /* wifiの接続待ち(1000ms*15回) */
  for (uint8_t ms_cnt = 0; ms_cnt < 15; ms_cnt++) {
    delay(1000);
    if(WiFi.status() == WL_CONNECTED) { /* 平均5～7回で成功 */
      /* 接続できたらフラグクリアしてループ終了 */
      flag_wifiinit_err = 0;
      return;
    } else {
      /* 1s毎に切断後再接続 */
      WiFi.disconnect();
      WiFi.reconnect();
    }
  }
  
  return;
}

/*** Timeライブラリのサンプルコードから移植 ***/
// send an NTP request to the time server at the given address
void sendNTPpacket(const char* address)
{
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
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
  UDP_NTP.beginPacket(address, 123); //NTP requests are to port 123
  UDP_NTP.write(packetBuffer, NTP_PACKET_SIZE);
  UDP_NTP.endPacket();
}
