#ifndef AHA_ESPHA_H
#define AHA_ESPHA_H

/**
 * espHA - a Home Assistant MQTT discovery library for ESP-IDF.
 *
 * This is an ESP-IDF-native fork of the ArduinoHA library. It has no dependency on the
 * Arduino core: MQTT transport is handled directly through the ESP-IDF "mqtt" component
 * (esp_mqtt_client_*, see HAMqtt.cpp) and its own background FreeRTOS task, so HAMqtt::loop()
 * only needs to notice connection-state transitions rather than drive any blocking I/O.
 *
 * Include this single header to pull in every device type (HASwitch, HASensor, HALight, ...).
 * Individual device types can be compiled out via the EX_ARDUINOHA_* defines documented in
 * espHADefines.h, and ESPHA_DEBUG there enables verbose logging under the "AHA" tag.
 */

#include "HADevice.h"
#include "HAMqtt.h"
#include "device-types/HABinarySensor.h"
#include "device-types/HAButton.h"
#include "device-types/HACamera.h"
#include "device-types/HACover.h"
#include "device-types/HADeviceTracker.h"
#include "device-types/HADeviceTrigger.h"
#include "device-types/HAFan.h"
#include "device-types/HAHVAC.h"
#include "device-types/HALight.h"
#include "device-types/HALock.h"
#include "device-types/HANumber.h"
#include "device-types/HAScene.h"
#include "device-types/HASelect.h"
#include "device-types/HASensor.h"
#include "device-types/HASensorNumber.h"
#include "device-types/HASwitch.h"
#include "device-types/HATagScanner.h"
#include "device-types/HAText.h"
#include "utils/HAUtils.h"
#include "utils/HANumeric.h"

#endif
