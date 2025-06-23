/*** ESP32-DevKitC書き込み時はビルド終了後、転送開始時にBootボタンを押下すること！ ***/
#include "ComCommon.h"
#include "LcdCommon.h"

#include <DHT.h>
#include <TimeLib.h>

DHT dht( DHT_PIN, DHT11 );

/* 温湿度データ */
float humi = 0.0;
float temp = 0.0;
/* 現在時刻(日本時間)を取得 */
time_t now_data = 0;
/* httpリクエスト用 */
char http_buff[BUFF_LENGTH] = {};

/* 初回起動時に0分0秒でNTP更新周期を再設定するフラグ */
bool adjustsync_flg = true;

/* 休日の判定 1:スリープ不可, 0:スリープ許可 */
char holiday_jdg[BUFF_LENGTH] = "1";

void     set_display( uint8_t h_data, uint8_t m_data, uint8_t s_data );
void     read_sensor( void );
void     adjust_syncinterval( void );
void     deepsleep_jdg( uint8_t h_data, uint8_t w_data, bool flag );

void setup()
{
  /* 温湿度センサーの開始 */
  dht.begin();
  /* 通信初期化・開始処理 */
  ComCommon_init();

  /* エラー判定処理 */
  if( 0x00 < err_flag.all_bits )
  {
    /* 通信接続失敗時にリセットをかける */
    /* マイコン起動失敗時処理 */
    LcdCommon_init_fail( err_flag.all_bits );

    /* 処理を開始せずソフトウェアリセット */
    delay( 1000 );
    ESP.restart();
    delay( 5000 ); /* 未到達 */
  }
  else
  {
    setSyncProvider( getNtpTime ); /* 補正に使用する関数設定 */
    setSyncInterval( 3600 );       /* 時刻補正を行う周期設定(秒) 後で調整 */

    /* 休日か判定 */
    ComCommon_post_req( holiday_jdg, "datelist" );
    /* スリープ判定処理 */
    deepsleep_jdg( hour( now() ), weekday( now() ), false );

    /* LCD初期化処理 */
    LcdCommon_init();
    /* 温湿度データの取得(初回) */
    read_sensor();
    /* データ取得 */
    ComCommon_post_req( http_buff, HTTP_REQUEST );
  }
}

void loop()
{
  static uint8_t w_data = 1; /* 曜日 */
  static uint8_t d_data = 0; /* 日   */
  static uint8_t h_data = 0; /* 時   */
  static uint8_t m_data = 0; /* 分   */
  static uint8_t s_data = 0; /* 秒   */
  uint8_t m_div_data = 0;
  uint32_t millis_count = millis();

  /* NTPから取得した時刻が設定済み且つ時刻が更新された時 */
  if( timeNotSet != timeStatus() && now_data != now() )
  {
    now_data = now();
    h_data = hour( now_data );
    m_data = minute( now_data );
    s_data = second( now_data );

    /* スリープ判定処理 */
    deepsleep_jdg( h_data, w_data, true );

    /* 画面表示 */
    set_display( h_data, m_data, s_data );

    /* 1分ごとに温湿度更新 */
    if( UPDATE_MIN_PRE == s_data )
    {
      read_sensor();
    }

    /* 分の一桁目取得用 */
    m_div_data = m_data % 10;

    /* 毎時一桁目が2分の時、データ取得 */
    if( ( 2 == m_div_data ) && ( 0 == s_data ) )
    {
      ComCommon_post_req( http_buff, HTTP_REQUEST );
    }

    /* 日付が変わっていたら休日か判定 */
    if( d_data != day( now_data ) )
    {
      w_data = weekday( now_data );
      d_data = day( now_data );
      
      /* 初回起動時の対策 */
      if( !adjustsync_flg )
      {
        holiday_jdg = ComCommon_post_req( "datelist" );
      }
    }
  }

  /* ESP32+ILI9341(8bit)の起動時に最大250～260ms */
  while( ( millis() - millis_count ) < 500 )
  {
    ; /* loop関数開始から500ms経過するまでループ */
  }
}

/* 描画処理関数 */
void set_display( uint8_t h_data, uint8_t m_data, uint8_t s_data )
{
  /* 前回値 */
  static uint8_t day_pre = 0;
  static uint8_t min_pre = 0;

  /* 時刻が10以上の判定フラグ */
  static uint8_t hour_digflg = 0;

  /* 表示文字列を格納する配列 */
  char day_data[BUFF_LENGTH] = {};
  char time_data[BUFF_LENGTH] = {};
  char sensor_data[BUFF_LENGTH] = {};

  /* 1日タスク処理 */
  if( day_pre != day( now_data ) )
  {
    /* 表示文字列の作成(年月日) */
    sprintf( day_data, DAY_FORMAT, year( now_data ), month( now_data ), day( now_data ) );
	/* 日付/曜日描画処理 */
    LcdCommon_draw_date( day_data, weekday( now_data ) );
  }

  /* 1分タスク処理 */
  if( min_pre != m_data )
  {
    /* 時間表示の桁数調整 */
    if( 10 > h_data )
    {
      hour_digflg = 1;
    }
    else
    {
      hour_digflg = 0;
    }

    /* 表示文字列の作成(温湿度) */
    sprintf( sensor_data, SENSOR_FORMAT, humi, temp );
    /* 温湿度/気圧描画処理 */
    LcdCommon_draw_weather( http_buff, sensor_data, BUFF_LENGTH );
  }

  /* 表示文字列の作成(時間) */
  sprintf( time_data, TIME_FORMAT, h_data, m_data, s_data );
  /* 時間描画処理 */
  LcdCommon_draw_time( time_data, hour_digflg );

  /* NTP取得時間の調整(一度だけ) */
  adjust_syncinterval();

  /* 前回値更新 */
  day_pre = day( now_data );
  min_pre = m_data;
}

/* 温湿度データ取得関数の呼び出し */
void read_sensor( void )
{
  humi = dht.readHumidity();
  temp = dht.readTemperature();
}

/* 一度だけNTP取得時間の調整 */
void adjust_syncinterval( void )
{
  /* adjustsync_flgの初期値はtrue */
  if( ( adjustsync_flg ) && ( 0 == minute() ) && ( 0 == second() ) )
  {
    setSyncInterval( 21600 ); /* 時刻補正を行う周期設定(秒) */

    adjustsync_flg = false;
    // Serial.println( "Adjust SyncInterval" );
  }
}

/* 平日の日中にdeep-sleepを行う判定処理 */
void deepsleep_jdg( uint8_t h_data, uint8_t w_data, bool flag )
{
  if( ( 8 < h_data )  &&
      ( 18 > h_data ) &&
      ( 1 < w_data )  &&
      ( 7 > w_data )  &&
      ( 0 == atoi( holiday_jdg ) ) )
  //if( 10 == minute( now() ) && 0 == second( now() ) && 1 == atoi( holiday_jdg ) )
  {
    /* 起動時：false、通常動作中：true */
    /* begin()前にSPIコマンドを送るとリセットループする対策 */
    if( flag )
    {
      /* LCDスリープ処理 */
      LcdCommon_sleep();
    }
#if defined ( ESP32 )
    /* deep-sleep */
    esp_sleep_enable_timer_wakeup( ( uint32_t )( 1800 * 1000 * 1000 * 1.006 ) ); /* deep-sleepの誤差補正 1.006 */
    esp_deep_sleep_start();
#elif defined ( ESP8266 )
    /* deep-sleep */
    ESP.deepSleep( ( uint32_t )( 1800 * 1000 * 1000 * 1.006 ), WAKE_RF_DEFAULT ); /* deep-sleepの誤差補正 1.006 */
#endif
    delay( 3000 ); /* 未到達 */
  }
}
