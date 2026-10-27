#pragma once

#include "sdkconfig.h"

#ifdef CONFIG_ESPHA_DEBUG
#include "esp_log.h"
#define ESPHA_LOGD(fmt, ...) ESP_LOGD("ESPHA", fmt, ##__VA_ARGS__)
#else
#define ESPHA_LOGD(fmt, ...)
#endif
