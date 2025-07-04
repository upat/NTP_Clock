#pragma once

/* 環境に合わせて以下設定 */
#define AP_SSID  "" /* 接続するルーターのSSID */
#define AP_PASS  "" /* 接続するルーターのパスワード */
#define HTTP_URL "" /* HTTPサーバー用 */

/* ILI9341 8bitパラレルシールドを使用する場合定義 */
//#define ESP32_8BIT

/* POST受信・画面描画共用バッファサイズ */
#define COMMON_BUFF_SIZE 24
