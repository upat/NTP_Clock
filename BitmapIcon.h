#pragma once

#include "CommonSetting.h"

// #if defined ( ESP32_8BIT )
#if 0 /* 使用予定なし */
#include <Arduino.h>
#include <pgmspace.h>

extern const PROGMEM uint8_t bitmap_sun[];    /* 晴れ(昼) */
extern const PROGMEM uint8_t bitmap_moon[];   /* 晴れ(夜) */
extern const PROGMEM uint8_t bitmap_cloudy[]; /* 曇り */
extern const PROGMEM uint8_t bitmap_rain[];   /* 雨 */
#endif

