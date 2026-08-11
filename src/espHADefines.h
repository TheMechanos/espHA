#ifndef AHA_ESPHADEFINES_H
#define AHA_ESPHADEFINES_H

// Turns on debug logging of the espHA core, emitted through the ESP-IDF
// logging system (esp_log.h) under the "AHA" tag. Control the verbosity with
// `idf.py menuconfig` (Component config -> Log output) or at runtime via
// esp_log_level_set("AHA", ESP_LOG_DEBUG).
// #define ESPHA_DEBUG
// Gates a couple of extra-verbose, high-frequency debug prints (e.g. HAMqtt::beginPublish).
// #define ESPHA_DEBUG_LEV 1

// These macros allow to exclude some parts of the library to save more resources.
// #define EX_ARDUINOHA_BINARY_SENSOR
// #define EX_ARDUINOHA_BUTTON
// #define EX_ARDUINOHA_CAMERA
// #define EX_ARDUINOHA_COVER
// #define EX_ARDUINOHA_DEVICE_TRACKER
// #define EX_ARDUINOHA_DEVICE_TRIGGER
// #define EX_ARDUINOHA_FAN
// #define EX_ARDUINOHA_HVAC
// #define EX_ARDUINOHA_LIGHT
// #define EX_ARDUINOHA_LOCK
// #define EX_ARDUINOHA_NUMBER
// #define EX_ARDUINOHA_SCENE
// #define EX_ARDUINOHA_SELECT
// #define EX_ARDUINOHA_SENSOR
// #define EX_ARDUINOHA_SWITCH
// #define EX_ARDUINOHA_TAG_SCANNER

#if defined(ESPHA_DEBUG)
#include "esp_log.h"
#define ESPHA_DEBUG_LOG(fmt, ...) ESP_LOGD("AHA", fmt, ##__VA_ARGS__)

#else
#define ESPHA_DEBUG_LOG(fmt, ...)

#endif


#endif
