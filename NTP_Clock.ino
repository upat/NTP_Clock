/*** ESP32-DevKitC書き込み時はビルド終了後、転送開始時にBootボタンを押下すること！ ***/
#include "ComCommon.h"
#include "LcdCommon.h"

#include <TimeLib.h>

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

    /* 初回処理関数 */
    void ntp_init(void) {
      setSyncProvider(getNtpTime); /* 補正に使用する関数設定 */
      setSyncInterval(21600);      /* 時刻補正を行う周期設定(秒) */
      weekday_d = weekday(now());  /* 初回スリープ判定のため */
      hour_d    = hour(now());
    }

    /* 更新チェック関数 */
    void time_check(void) {
      if ((timeStatus() != timeNotSet) && (now() != _now)) {
        sec_updflg = true;
        min_updflg = false;
        day_updflg = false;
        _time_update();
      } else {
        sec_updflg = false;
      }
    }
  private:
    time_t _now;

    /* 更新関数 */
    void _time_update(void) {
      _now = now();
      if (day(_now) != day_d) {
        day_updflg = true;
      }
      if (minute(_now) != minute_d) {
        min_updflg = true;
      }
      year_d    = year(_now);
      month_d   = month(_now);
      day_d     = day(_now);
      weekday_d = weekday(_now);
      hour_d    = hour(_now);
      minute_d  = minute(_now);
      second_d  = second(_now);
      min_dig   = minute_d % 10;

      if (10 > hour_d) {
        hour_dig = 1;
      } else {
        hour_dig = 0;
      }
    }
};
static TimeData timeData;

static char http_buff[BUFF_LENGTH] = {};    /* httpリクエスト用 */
static char holiday_jdg[BUFF_LENGTH] = "1"; /* 休日の判定 1:スリープ不可, 0:スリープ許可 */

static void deepsleep_jdg(void);

void setup()
{
  /* 通信初期化・開始処理 */
  ComCommon_init();

  /* エラー判定処理 */
  if (0x00 < err_flag.all_bits) {
    /* 通信接続失敗時にリセットをかける */
    /* マイコン起動失敗時処理 */
    LcdCommon_init_fail(err_flag.all_bits);

    /* 処理を開始せずソフトウェアリセット */
    delay(1000);
    ESP.restart();
    delay(5000); /* 未到達 */
  } else {
    /* NTP設定・取得 */
    timeData.ntp_init();
    /* 休日か判定 */
    ComCommon_post_req(holiday_jdg, "datelist");
    /* スリープ判定処理 */
    deepsleep_jdg();

    /* LCD初期化処理 */
    LcdCommon_init();
    /* データ取得 */
    ComCommon_post_req(http_buff, HTTP_REQUEST);
  }
}

void loop()
{
  uint32_t millis_count = millis(); /* 周期管理用 */
  char draw_data[BUFF_LENGTH] = {}; /* 表示文字列を格納する配列 */
  static bool isnotfirst = false;

  /* NTPから取得した時刻が設定済み且つ時刻が更新された時 */
  timeData.time_check();
  if (timeData.sec_updflg) {
    /* スリープ判定処理 */
    deepsleep_jdg();

    /* 1日毎タスク */
    if (timeData.day_updflg) {
      /* 日付描画処理 */
      sprintf(draw_data, DAY_FORMAT, timeData.year_d, timeData.month_d, timeData.day_d);
      LcdCommon_draw_date(draw_data, timeData.weekday_d);
      /* 休日判定結果取得(初回のみsetup()で実施済みのため処理しない) */
      if (isnotfirst) {
        ComCommon_post_req(holiday_jdg, "datelist");
      }
      isnotfirst = true;
    }
    /* 1分毎タスク処理 */
    if (timeData.min_updflg) {
      /* 温湿度/気圧描画処理 */
      sprintf(draw_data, SENSOR_FORMAT, dht.readHumidity(), dht.readTemperature());
      LcdCommon_draw_weather(http_buff, draw_data, BUFF_LENGTH);
      /* 毎時一桁目が2分の時、データ取得 */
      if (2 == timeData.min_dig) {
        ComCommon_post_req(http_buff, HTTP_REQUEST);
      }
    }
    /* 1秒毎タスク 時間描画処理 */
    sprintf(draw_data, TIME_FORMAT, timeData.hour_d, timeData.minute_d, timeData.second_d);
    LcdCommon_draw_time(draw_data, timeData.hour_dig);
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
   && (0 == atoi(holiday_jdg))) {
  //if( 10 == minute( now() ) && 0 == second( now() ) && 1 == atoi( holiday_jdg ) ) {
    /* begin()前にSPIコマンドを送るとリセットループする対策 */
    if (isnotfirst) {
      LcdCommon_sleep(); /* LCDスリープ処理 */
    }
#if defined (ESP32)
    /* deep-sleep */
    esp_sleep_enable_timer_wakeup((uint32_t)(1800 * 1000 * 1000 * 1.006)); /* deep-sleepの誤差補正 1.006 */
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
