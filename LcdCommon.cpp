#include "LcdCommon.h"

/* 変数宣言 */
static const char week_day[7][8] = {"(SUN)", "(MON)", "(TUE)", "(WED)", "(THU)", "(FRI)", "(SAT)"};

/* 関数宣言 */
static uint16_t str_position(uint16_t str_length, uint16_t unit_length);
static uint16_t count_char(char *str, uint8_t str_size);

#if defined (ESP32_8BIT)
static MCUFRIEND_kbv lcd;
/* TouchScreen ts(XP, YP, XM, YM, 300); */
// TouchScreen ts(27, 4, 15, 14, 300); /* 使用予定なし */
#elif defined (ESP32)
static Adafruit_ILI9341 lcd = Adafruit_ILI9341(5, 17, 16); /* CS, DC, RESET */
#elif defined (ESP8266)
static Adafruit_SSD1306 lcd(128, 64, &Wire, -1); /* 横解像度, 縦解像度, RESET(無し) */
#endif

/*******************************************************************/
/* 処理内容：LCD初期化処理                                         */
/* 引数　　：なし                                                  */
/* 戻り値　：なし                                                  */
/* 備考　　：ILI9341 SPI/ILI9341 8bit/SSD1306 I2C兼用              */
/*******************************************************************/
void LcdCommon_init(void)
{
#if defined (ESP32_8BIT)
  /* TLP222A経由でILI9341への電源を供給 */
  pinMode(18, OUTPUT);
  digitalWrite(18, HIGH);
  /* 制御開始 */
  lcd.begin(0x9341);
  lcd.setRotation(1);
#elif defined (ESP32)
  /* LCDバックライトのPWM出力 */
  ledcDetach(A4);          /* リセットを繰り返していると点灯しないことがある対策 */
  ledcAttach(A4, 6400, 8); /* IO32(A4)から6.4kHz、8bit分解能でPWM出力設定 */
  ledcWrite(A4, 128);      /* IO32(A4)からduty50%でPWM出力 */
  /* 制御開始 */
  lcd.begin();
  lcd.setRotation(1);
#elif defined (ESP8266)
  /* 制御開始 */
  lcd.begin(SSD1306_EXTERNALVCC, 0x3C);
  lcd.setTextColor(WHITE, BLACK);
  lcd.clearDisplay();
  lcd.display();
#endif
}

/*******************************************************************/
/* 処理内容：マイコン起動失敗時処理                                */
/* 引数　　：なし                                                  */
/* 戻り値　：なし                                                  */
/* 備考　　：ILI9341 SPI/ILI9341 8bit/SSD1306 I2C兼用              */
/*******************************************************************/
void LcdCommon_init_fail(uint8_t flag)
{
  /* LCD初期化処理 */
  LcdCommon_init();
  
#if defined (ESP32)
  lcd.fillScreen(LCD_COMMON_BLACK); /* 画面表示のクリア */
  lcd.setTextSize(3);               /* 表示する文字サイズ */
  lcd.setCursor(0, 0);              /* 文字描画の開始位置 */
  lcd.print("init_error:0x");       /* 文字のデータをセット */
  lcd.println(flag, HEX);
#elif defined (ESP8266)
  lcd.clearDisplay();               /* バッファのクリア */
  lcd.setTextSize(1);             /* 表示する文字サイズ */
  lcd.setTextColor(WHITE, BLACK); /* 表示する文字の色(単色) */
  lcd.setCursor(0, 0);            /* 文字描画の開始位置 */
  lcd.print("init error:0x");     /* 文字のデータをセット */
  lcd.println(flag, HEX);
  lcd.display();
#endif
}

/*******************************************************************/
/* 処理内容：日付/曜日描画処理                                     */
/* 引数　　：日付文字列データ, Timelib算出曜日データ(int型)        */
/* 戻り値　：なし                                                  */
/* 備考　　：ILI9341 SPI/ILI9341 8bit/SSD1306 I2C兼用              */
/*******************************************************************/
void LcdCommon_draw_date(char *day_data, int weekday)
{
#if defined (ESP32)
  lcd.setTextColor(LCD_COMMON_DARKTURQUOISE, LCD_COMMON_BLACK); /* 文字色・文字背景色設定 */
  /* 画面を黒塗り(表示クリア) */
  lcd.fillScreen(LCD_COMMON_BLACK);

  /* 日付を描画 */
  lcd.setTextSize(3);
  lcd.setCursor(27, 24);
  lcd.println(day_data);

  /* 曜日を描画 */
  lcd.setCursor(207, 24);
  lcd.println(week_day[weekday - 1]);
#elif defined (ESP8266)
  /* 画面表示クリア */
  lcd.clearDisplay();

  /* 日付を描画 */
  lcd.setTextSize(1);
  lcd.setCursor(19, 0);
  lcd.println(day_data);

  /* 曜日を描画 */
  lcd.setCursor(79, 0);
  lcd.println(week_day[weekday - 1]);
#endif
}

/*******************************************************************/
/* 処理内容：温湿度/気圧描画処理                                   */
/* 引数　　：httpリクエスト用データ, センサー読取データ, データ長  */
/* 戻り値　：なし                                                  */
/* 備考　　：ILI9341 SPI/ILI9341 8bit/SSD1306 I2C兼用              */
/*******************************************************************/
void LcdCommon_draw_weather(char *http_buff, char *sensor_data, uint8_t buff_length)
{
  static uint16_t httpstr_pre = 0;
  static uint16_t ssrstr_pre = 0;

  /* 画面上の文字の長さ */
  uint16_t http_strlen = 0;
  uint16_t sensor_strlen = 0;
  /* 中央寄せに使用するカーソル開始位置 */
  uint16_t http_cursor = 0;
  uint16_t sensor_cursor = 0;

#if defined (ESP32)
  lcd.setTextColor(LCD_COMMON_DARKTURQUOISE, LCD_COMMON_BLACK); /* 文字色・文字背景色設定 */
  /* 画面上の文字の長さ */
  http_strlen = count_char(http_buff, buff_length) * 12;
  sensor_strlen = count_char(sensor_data, buff_length) * 18;
  /* 中央寄せに使用するカーソル開始位置 */
  http_cursor = str_position(http_strlen, 10);
  sensor_cursor = str_position(sensor_strlen, 18);

  /* 文字数が異なる場合のリフレッシュ */
  if ((httpstr_pre != http_strlen) || (ssrstr_pre != sensor_strlen)) {
    lcd.fillRect(0, 70, 320, 50, LCD_COMMON_BLACK); /* 温度情報の画面表示クリア */
    httpstr_pre = http_strlen;                      /* httpリクエスト取得文字数の前回値更新 */
    ssrstr_pre = sensor_strlen;                     /* センサー読み取り文字数の前回値更新 */
  }

  /* httpリクエストで取得した温湿度気圧のデータを描画(文字サイズ:2) */
  lcd.setTextSize(2);
  lcd.setCursor(http_cursor, 70);
  lcd.println(http_buff);

  /* 温度単位をそれっぽく表示(文字サイズ:2) */
  lcd.setCursor(http_cursor + http_strlen + 6, 70);
  lcd.println("C");
  lcd.fillRect(http_cursor + http_strlen, 70, 4, 4, LCD_COMMON_DARKTURQUOISE);

  /* 温湿度センサーのデータを描画(文字サイズ:3) */
  lcd.setTextSize(3);
  lcd.setCursor(sensor_cursor, 96);
  lcd.println(sensor_data);

  /* 温度単位をそれっぽく表示(文字サイズ:3) */
  lcd.setCursor(sensor_cursor + sensor_strlen + 8, 96);
  lcd.println("C");
  lcd.fillRect(sensor_cursor + sensor_strlen, 96, 6, 6, LCD_COMMON_DARKTURQUOISE);

  lcd.setTextColor(LCD_COMMON_DEEPPINK, LCD_COMMON_BLACK); /* 文字色・文字背景色設定 */
#elif defined (ESP8266)
  /* 画面上の文字の長さ */
  http_strlen = count_char(http_buff, buff_length) * 6;
  sensor_strlen = count_char(sensor_data, buff_length) * 12;
  /* 中央寄せに使用するカーソル開始位置 */
  http_cursor = str_position(http_strlen, 10);
  sensor_cursor = str_position(sensor_strlen, 18);

  /* 文字数が異なる場合のリフレッシュ */
  if ((httpstr_pre != http_strlen) || (ssrstr_pre != sensor_strlen)) {
    lcd.fillRect(0, 10, 128, 10, BLACK); /* 温度情報の画面表示クリア */
    httpstr_pre = http_strlen;           /* httpリクエスト取得文字数の前回値更新 */
    ssrstr_pre = sensor_strlen;          /* センサー読み取り文字数の前回値更新 */
  }

  /* httpリクエストで取得した温湿度気圧のデータを描画(文字サイズ:1) */
  lcd.setTextSize(1);
  lcd.setCursor(http_cursor, 10);
  lcd.println(http_buff);

  /* 温度単位をそれっぽく表示(文字サイズ:1) */
  lcd.setCursor(http_cursor + http_strlen + 4, 10);
  lcd.println("C");
  lcd.fillRect(http_cursor + http_strlen + 1, 10, 2, 2, WHITE);

  /* 温湿度のデータの描画(文字サイズ:2) */
  lcd.setTextSize(2);
  lcd.setCursor(sensor_cursor, 24);
  lcd.println(sensor_data);

  /* 温度単位をそれっぽく表示(文字サイズ:2) */
  lcd.setCursor(sensor_cursor + sensor_strlen + 6, 24);
  lcd.println("C");
  lcd.fillRect(sensor_cursor + sensor_strlen, 24, 4, 4, WHITE);
#endif
}

/*******************************************************************/
/* 処理内容：時間描画処理                                          */
/* 引数　　：時間文字列データ, 時間桁数フラグ                      */
/* 戻り値　：なし                                                  */
/* 備考　　：ILI9341 SPI/ILI9341 8bit/SSD1306 I2C兼用              */
/*******************************************************************/
void LcdCommon_draw_time(char *time_data, uint8_t hour_digflg)
{
#if defined (ESP32)
  /* 2桁時間→1桁時間のリフレッシュは日時変更時に行っている */
  /* 時間を描画(文字サイズ:6) */
  lcd.setTextSize(6);
  lcd.setCursor(16 + (hour_digflg * 16), 160);
  lcd.println(time_data);

#elif defined (ESP8266)
  /* 2桁時間→1桁時間のリフレッシュは日時変更時に行っている */
  /* 時間を描画(文字サイズ:2) */
  lcd.setTextSize(2);
  lcd.setCursor(16 + (hour_digflg * 6), 48);
  lcd.println(time_data);

  lcd.display();
#endif
}

/*******************************************************************/
/* 処理内容：中央寄せカーソル開始位置算出処理                      */
/* 引数　　：表示する文字列の長さ, 表示する単位文字の幅            */
/* 戻り値　：カーソル開始位置                                      */
/* 備考　　：なし                                                  */
/*******************************************************************/
/* 画面上の文字列を中央寄せに調整する */
static uint16_t str_position(uint16_t str_length, uint16_t unit_length)
{
  uint16_t half_width = 0;
  uint16_t half_strlen = 0;
  
  /* (画面の幅 - 単位(℃)の幅) ÷ 2 */
  half_width = ((uint16_t)LCD_COMMON_WIDTH - unit_length) / 2;
  half_strlen = str_length / 2;

  return (half_width - half_strlen);
}

/*******************************************************************/
/* 処理内容：char型文字列の長さ算出処理                            */
/* 引数　　：char型データ, sizeofで算出したchar型データのデータ長  */
/* 戻り値　：char型文字列の長さ                                    */
/* 備考　　：なし                                                  */
/*******************************************************************/
static uint16_t count_char(char *str, uint8_t str_size)
{
  uint16_t count = 0; /* 文字数カウンタ */
  /* 終端文字を含めずにカウント */
  for (count = 0; count < (uint16_t)str_size; count++) {
    if ('\0' == str[count]) {
      break; /* ループの終了位置=終端文字の配列インデックス=文字数 */
    }
  }

  return count;
}

#if 0
/*******************************************************************/
/* 処理内容：LCDウェイク処理                                       */
/* 引数　　：なし                                                  */
/* 戻り値　：なし                                                  */
/* 備考　　：ILI9341 SPI用(リセットで起こすため未使用)             */
/*******************************************************************/
void LcdCommon_wake(void)
{
  lcd.startWrite();
  
  lcd.writeCommand(ILI9341_SLPOUT);
  delay(120);
  lcd.writeCommand(ILI9341_DISPON);
  delay(20);

  lcd.endWrite();
}
#endif

/*******************************************************************/
/* 処理内容：LCDスリープ処理                                       */
/* 引数　　：なし                                                  */
/* 戻り値　：なし                                                  */
/* 備考　　：ILI9341 SPI/ILI9341 8bit/SSD1306 I2C兼用              */
/*******************************************************************/
void LcdCommon_sleep(void)
{
#if defined (ESP32_8BIT)
  /* TLP222A経由でILI9341への電源を遮断 */
  digitalWrite(18, LOW);
#elif defined (ESP32)
  /* 画面を黒塗り(表示クリア) */
  lcd.fillScreen(LCD_COMMON_BLACK);

  lcd.startWrite();

  lcd.writeCommand(ILI9341_DISPOFF);
  delay(20);
  lcd.writeCommand(ILI9341_SLPIN);
  delay(120);

  lcd.endWrite();
  
  /* バックライト消灯 */
  ledcWrite(0, 0);
#elif defined (ESP8266)
  /* 画面表示をクリア */
  lcd.clearDisplay();
  lcd.display();
#endif
}
