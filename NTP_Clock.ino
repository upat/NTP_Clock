/*** ESP32-DevKitC書き込み時はビルド終了後、転送開始時にBootボタンを押下すること！ ***/
#include "ComCommon.h"
#include "LcdCommon.h"

static HttpPostBuf getjma_buf = {.post_req = HTTP_REQUEST}; /* get_jmaリクエスト用 */
static HttpPostBuf datelist_buf = {.post_req = "datelist"}; /* datelistリクエスト用 1:スリープ不可, 0:スリープ許可 */

static void deepsleep_jdg(void);

void setup()
{
  /* 通信初期化・開始処理 */
  ComCommon_init();

  /* エラー判定処理 */
  if (wifi_connect_error) {
    /* 通信接続失敗時にリセットをかける */
    /* マイコン起動失敗時処理 */
    LcdCommon_init_fail();

    /* 処理を開始せずソフトウェアリセット */
    delay(1000);
    ESP.restart();
    delay(5000); /* 未到達 */
  } else {
    /* NTP設定・取得 */
    timeData.ntp_init();
    /* 休日か判定 */
    ComCommon_post_req(&datelist_buf);
    /* スリープ判定処理 */
    deepsleep_jdg();

    /* LCD初期化処理 */
    LcdCommon_init();
    /* データ取得 */
    ComCommon_post_req(&getjma_buf);
  }
}

void loop()
{
  uint32_t millis_count = millis(); /* 周期管理用 */
  DispBuf dispbuf;                  /* 表示文字列・長さを格納する構造体 */
  DispBuf postbuf;
  static bool isnotfirst = false;

  /* NTPから取得した時刻が設定済み且つ時刻が更新された時 */
  timeData.time_check();
  if (timeData.sec_updflg) {
    /* スリープ判定処理 */
    deepsleep_jdg();

    /* 1日毎タスク */
    if (timeData.day_updflg) {
      /* 日付描画処理 */
      dispbuf.str_len = (uint8_t)snprintf(dispbuf.disp_buf, COMMON_BUFF_SIZE, DAY_FORMAT, timeData.year_d, timeData.month_d, timeData.day_d);
      /* snprintfの戻り値は終端文字を除いた『書こうとした文字数』を返すため */
      /* 異常の場合は-1をuint8_tでキャストした値=255→バッファサイズ未満の時のみ描画処理実施 */
      if (COMMON_BUFF_SIZE > dispbuf.str_len) {
        LcdCommon_draw_date(dispbuf.disp_buf, timeData.weekday_d);
      }
      /* 休日判定結果取得(初回のみsetup()で実施済みのため処理しない) */
      if (isnotfirst) {
        ComCommon_post_req(&datelist_buf);
      }
      isnotfirst = true;
    }
    /* 1分毎タスク処理 */
    if (timeData.min_updflg) {
      /* 温湿度/気圧描画処理 */
      dispbuf.str_len = (uint8_t)snprintf(dispbuf.disp_buf, COMMON_BUFF_SIZE, SENSOR_FORMAT, dht.readHumidity(), dht.readTemperature());
      postbuf.str_len = (uint8_t)snprintf(postbuf.disp_buf, COMMON_BUFF_SIZE, "%s", getjma_buf.recv_buf); /* HttpPostBufのデータをDispBufへコピー */
      if ((COMMON_BUFF_SIZE > dispbuf.str_len) && (COMMON_BUFF_SIZE > dispbuf.str_len)) {
        LcdCommon_draw_weather(&postbuf, &dispbuf);
      }
      /* 毎時一桁目が2分の時、データ取得 */
      if (2 == timeData.min_dig) {
        ComCommon_post_req(&getjma_buf);
      }
    }
    /* 1秒毎タスク 時間描画処理 */
    dispbuf.str_len = (uint8_t)snprintf(dispbuf.disp_buf, COMMON_BUFF_SIZE, TIME_FORMAT, timeData.hour_d, timeData.minute_d, timeData.second_d);
    if (COMMON_BUFF_SIZE > dispbuf.str_len) {
      LcdCommon_draw_time(&dispbuf);
    }
  }

  /* ESP32+ILI9341(8bit)の起動時に最大250～260ms */
  while ((millis() - millis_count) < 500) {
    delayMicroseconds(500); /* loop関数開始から500ms経過するまでループ */
  }
}

/* 平日の日中にdeep-sleepを行う判定処理 */
static void deepsleep_jdg(void)
{
  static bool isnotfirst = false; /* 起動時：false、通常動作中：true */

  if ((8 < timeData.hour_d) && (18 > timeData.hour_d)
   && (1 < timeData.weekday_d) && (7 > timeData.weekday_d)
   && (0 == atoi(datelist_buf.recv_buf))) {
  //if( 10 == minute( now() ) && 0 == second( now() ) && 1 == atoi( datelist_buff ) ) {
    /* begin()前にSPIコマンドを送るとリセットループする対策 */
    if (isnotfirst) {
      LcdCommon_sleep(); /* LCDスリープ処理 */
    }
    ComCommon_sleep();   /* wifi切断 */
#if defined (ESP32)
    /* deep-sleep */
    esp_sleep_enable_timer_wakeup((uint32_t)(1800 * 1000 * 1000 * 1.006));  /* deep-sleepの誤差補正 1.006 */
    esp_deep_sleep_start();
#elif defined (ESP8266)
    /* deep-sleep */
    ESP.deepSleep((uint32_t)(1800 * 1000 * 1000 * 1.006), WAKE_RF_DEFAULT); /* deep-sleepの誤差補正 1.006 */
#endif
    delay(3000); /* 未到達 */
  } else {
    isnotfirst = true;
  }
}
