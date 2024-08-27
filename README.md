# NTP Clock

概要
---
ESP系マイコンと画面表示モジュール、温湿度センサーモジュールを組み合わせた多機能時計

機能
---
- 一定時間毎にNTPサーバーと通信し、自動で時刻合わせ
- 消費電力削減のため、日中に自動でスリープ
- [ローカルサーバー](https://github.com/upat/PyWeather-Server)と通信を行い、最新の気象データを取得し表示

開発・動作環境
---
- 使用ボード
    - ESP32-DevKitC V2
    - WeMos D1
- 画面表示モジュール
    - ILI9341(SPI or 8bitパラレル)
    - SSD1306(I2C)
- 温湿度センサー
    - DHT11
- Arduino IDE 2.2.1(使用ライブラリ)
    - ESP32-DevKitC V2 + ILI9341(SPI) + DHT11 使用時
        - Adafruit_GFX_Library
        - Adafruit_ILI9341
        - DHT_sensor_library
        - Time
    - ESP32-DevKitC V2 + ILI9341(8bitパラレル) + DHT11 使用時
        - Adafruit_GFX_Library
        - MCUFRIEND_kbv
        - DHT_sensor_library
        - Time
    - WeMos D1 + SSD1306(I2C) + DHT11 使用時
        - Adafruit_GFX_Library
        - Adafruit_SSD1306
        - DHT_sensor_library
        - Time
- ローカルサーバー
    - [PyWeather-Server](https://github.com/upat/PyWeather-Server)

使い方(配線例)
---
ESP32-DevKitC V2+ILI9341(SPI)+DHT11 使用時
- `CommonSetting.h` の `#define ESP32_8BIT` をコメントアウト**する**  
<img src="https://github.com/upat/NTP_Clock/blob/master/images/pic1.png" width="60%" height="60%">

ESP32-DevKitC V2+ILI9341(8bitパラレル)+DHT11 使用時
- `CommonSetting.h` の `#define ESP32_8BIT` をコメントアウト**しない**  
<img src="https://github.com/upat/NTP_Clock/blob/master/images/pic2.png" width="70%" height="70%">

WeMos D1+SSD1306(I2C)+DHT11 使用時
- `CommonSetting.h` の `#define ESP32_8BIT` をコメントアウト**する**

動作イメージ
---
![pic3](https://github.com/upat/NTP_Clock/blob/master/images/pic3.png)

ファイル構成
---
- LICENCE
    - ライセンスファイル
- README.md
    - このファイル
- BitmapIcon
    - タッチパネル機能のため用意(未使用)
- ComCommon
    - 画面表示モジュール以外の通信関連の処理
- LcdCommon
    - 画面表示モジュール関連の処理
- CommonSetting.h
    - ユーザー設定、コンパイラスイッチの設定
- NTP_Clock.ino
    - メイン処理

使用上の注意
---
- 1周期あたり500msで動作させています。(LCD初期化処理250ms+マージン)
- ILI9341(8bitパラレル)はLED端子の無いものを使用しているため、スリープ時にTLP222Aを使用した回路で5V端子を遮断することで物理的にスリープさせています。

ライセンス
---
MIT Licence
