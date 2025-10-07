#include "ComCommon.h"

/* 変数宣言 */
static byte packetBuffer[NTP_PACKET_SIZE] = {};
static WiFiUDP UDP_NTP;
DHT dht(DHT_PIN, DHT11);
TimeData timeData;
bool wifi_connect_error;
/* 関数宣言 */
static void sendNTPpacket(const char *address);

/*******************************************************************/
/* 処理内容：通信初期化・開始処理                                  */
/* 引数　　：なし                                                  */
/* 戻り値　：なし                                                  */
/* 備考　　：なし                                                  */
/*******************************************************************/
void ComCommon_init(void)
{
  //Serial.begin(SERIAL_SPEED);   /* デバッグ用シリアル出力 */
  wifi_connect_error = true;      /* エラーフラグ(クリアは各制御で行う) */

  /* wifi通信の開始 */
  if ((WiFi.mode(WIFI_STA))                           /* 子機側に設定 */
   && (WiFi.begin(AP_SSID, AP_PASS))                  /* wifi接続 */
   && (WiFi.setTxPower(WIFI_POWER_7dBm))) {           /* wifi出力強度設定(『4dBm ≒ Bluetooth Class2相当』を参考に設定) */
    for (uint8_t ms_cnt = 0; ms_cnt < 15; ms_cnt++) { /* wifiの接続待ち(1000ms*15回) */
      delay(1000);
      if (WiFi.status() == WL_CONNECTED) {            /* 平均5～7回で成功 */
        wifi_connect_error = false;                   /* 接続できたらフラグクリアしてループ終了 */
        break;
      } else {
        /* 1s毎に切断後再接続 */
        WiFi.disconnect();
        WiFi.reconnect();
      }
    }
  }

  /* NTP用UDP通信の開始 */
  UDP_NTP.begin(5000);
  /* 温湿度センサとの通信開始 */
  dht.begin();
  
  return;
}

/*******************************************************************/
/* 処理内容：HTTPリクエスト処理                                    */
/* 引数　　：HttpPostBufポインタ                                   */
/* 戻り値　：なし                                                  */
/* 備考　　：なし                                                  */
/*******************************************************************/
void ComCommon_post_req(HttpPostBuf *buf_ptr)
{
  HTTPClient http;
  int httpCode;
  uint8_t byte_count = 0;

  /* リクエストに対応した初期値のセット */
  if (NULL != strstr(buf_ptr->post_req, "date")) {       /* datelistリクエスト */
    snprintf(buf_ptr->recv_buf, COMMON_BUFF_SIZE, "%s", "1");
  } else if (NULL != strstr(buf_ptr->post_req, "jma")) { /* get_jmaリクエスト */
    snprintf(buf_ptr->recv_buf, COMMON_BUFF_SIZE, "%s", HTTP_DEFAULT);
  }

  if (WiFi.status() != WL_CONNECTED) { /* wi-fi接続が切れていた場合再接続 */
      WiFi.disconnect();
      WiFi.reconnect();                /* 次回の通信は10分後になるため待ち処理は未実施 */
  } else {
    /* タイムアウト時間の設定(ms) */
    http.setTimeout(100);
    /* 通信開始 */
    http.begin(HTTP_URL);
    http.addHeader("Content-Type", "application/text");
    httpCode = http.POST(String(buf_ptr->post_req));

    if (HTTP_CODE_OK == httpCode) {
      delay(20);    /* 環境依存でPOSTリクエストの後20ms待ち */
      WiFiClient *stream = http.getStreamPtr();
      
      if (0 < stream->available()) {                    /* 通信失敗チェック(HTTP OKかつ受信データ無し) */
        /* 受信データが1byte以上あり */
        while ((0 < stream->available())
            && ((COMMON_BUFF_SIZE - 2) > byte_count)) { /* 配列インデックス+終端文字の分だけ引く */
          char c_tmp = stream->read();                  /* 1byteずつ読み出す */
          if ((0x20 > c_tmp) || (0x7e < c_tmp)) {       /* ascii文字範囲外の場合はハイフンに置き換え */
            buf_ptr->recv_buf[byte_count] = '-';
          } else {
            buf_ptr->recv_buf[byte_count] = c_tmp;
          }
          byte_count++;                                 /* 受信バイト数をカウント(最大22回想定) */
          delayMicroseconds(500);
        }
        buf_ptr->recv_buf[byte_count] = '\0';           /* 終端文字 */
      }
    }
    /* 通信終了 */
    http.end();
  }

  return;
}

/*******************************************************************/
/* 処理内容：スリープ前処理                                        */
/* 引数　　：なし                                                  */
/* 戻り値　：なし                                                  */
/* 備考　　：スリープ前にwifi切断を行う                            */
/*******************************************************************/
void ComCommon_sleep(void)
{
  WiFi.disconnect(true);
}

/*******************************************************************/
/* 処理内容：初回処理関数                                          */
/* 引数　　：なし                                                  */
/* 戻り値　：なし                                                  */
/* 備考　　：TimeDataクラス 公開関数                               */
/*******************************************************************/
void TimeData::ntp_init(void) {
  setSyncProvider(getNtpTime); /* 補正に使用する関数設定 */
  setSyncInterval(21600);      /* 時刻補正を行う周期設定(秒) */
  weekday_d = weekday(now());  /* 初回スリープ判定のため */
  hour_d    = hour(now());

  return;
}

/*******************************************************************/
/* 処理内容：更新チェック関数                                      */
/* 引数　　：なし                                                  */
/* 戻り値　：なし                                                  */
/* 備考　　：TimeDataクラス 公開関数                               */
/*******************************************************************/
void TimeData::time_check(void) {
  if ((timeStatus() != timeNotSet) && (now() != _now)) {
    sec_updflg = true;
    min_updflg = false;
    day_updflg = false;
    _time_update();
  } else {
    sec_updflg = false;
  }

  return;
}

/*******************************************************************/
/* 処理内容：更新関数                                              */
/* 引数　　：なし                                                  */
/* 戻り値　：なし                                                  */
/* 備考　　：TimeDataクラス 非公開関数                             */
/*******************************************************************/
void TimeData::_time_update(void) {
  _now = now();
  if (day(_now) != day_d) {
    day_updflg = true;
  }
  if (minute(_now) != minute_d) {
    min_updflg = true;
  }
  year_d    = (uint16_t)(year(_now));
  month_d   = (uint8_t)(month(_now));
  day_d     = (uint8_t)(day(_now));
  weekday_d = (uint8_t)(weekday(_now));
  hour_d    = (uint8_t)(hour(_now));
  minute_d  = (uint8_t)(minute(_now));
  second_d  = (uint8_t)(second(_now));
  min_dig   = minute_d % 10;

  return;
}

/*** Timeライブラリのサンプルコードから移植 ***/
time_t getNtpTime()
{
  while (UDP_NTP.parsePacket() > 0); // discard any previously received packets
  // Serial.println( "Transmit NTP Request" );
  sendNTPpacket("ntp.nict.jp");                         /* NTPサーバーのドメイン名(IP指定は非推奨) */
  uint32_t beginWait = millis();
  while (millis() - beginWait < 1500) {
    int size = UDP_NTP.parsePacket();
    if (size >= NTP_PACKET_SIZE) {
      // Serial.println( "Receive NTP Response" );
      UDP_NTP.read(packetBuffer, NTP_PACKET_SIZE);  // read packet into the buffer
      unsigned long secsSince1900;
      // convert four bytes starting at location 40 to a long integer
      secsSince1900 =  (unsigned long)packetBuffer[40] << 24;
      secsSince1900 |= (unsigned long)packetBuffer[41] << 16;
      secsSince1900 |= (unsigned long)packetBuffer[42] << 8;
      secsSince1900 |= (unsigned long)packetBuffer[43];
      return secsSince1900 - 2208988800UL + (9 * 3600); /* 9*3600s→日本のタイムゾーンに補正 */
    }
  }
  // Serial.println("No NTP Response :-(");
  return 0; // return 0 if unable to get the time
}

// send an NTP request to the time server at the given address
static void sendNTPpacket(const char *address)
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
