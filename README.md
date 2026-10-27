# espHA

Home Assistant MQTT discovery library for **ESP-IDF**.

`espHA` is a fork of [Dawid Chyrzynski's arduino-home-assistant](https://github.com/dawidchyrzynski/arduino-home-assistant),
rewritten to drop the Arduino core dependency entirely. It talks to the MQTT broker directly
through the ESP-IDF `mqtt` component (`esp_mqtt_client_*`, see `HAMqtt.cpp`), which runs its own
background FreeRTOS task and reconnects on its own — there's no `Client`/`WiFiClient` to wire up,
and `HAMqtt::loop()` only needs to notice connection-state transitions rather than drive any
blocking I/O.

## Requirements

- ESP-IDF (tested against v5.x)
- The `mqtt` component (part of ESP-IDF) and a working Wi-Fi/network connection
- **No Arduino core** — the library builds as a plain ESP-IDF component. If the rest of your
  project uses `arduino-esp32` as an ESP-IDF component too, that's fine, espHA just doesn't
  require it itself.

## Installation

Drop this folder into your project's `components/` directory (or add it as a git submodule
there). ESP-IDF will pick it up automatically via `idf_component_register()` in
`CMakeLists.txt` — no further registration needed.

## Usage

```cpp
#include <espHA.h>

HADevice device;
HAMqtt mqtt(device);
HAButton buttonA("myButtonA");

static void onButtonCommand(HAButton* sender)
{
    // handle the button press
}

void app_main_init()
{
    // Unique ID is required before HAMqtt::begin() will do anything.
    uint8_t id[] = {1, 2, 3, 4};
    device.setUniqueId(id, sizeof(id));
    device.setName("My Device");

    buttonA.setName("Click me");
    buttonA.setIcon("mdi:gesture-tap-button");
    buttonA.onCommand(onButtonCommand);

    // Wi-Fi must already be connected at this point.
    mqtt.begin("192.168.0.10", 1883, "mqtt-user", "mqtt-pass");
}

void app_main_loop()
{
    mqtt.loop(); // cheap: just checks for connection-state transitions
}
```

`HAMqtt::begin()` accepts a hostname or a dotted-decimal IP address as a plain string — there's
no `IPAddress` overload, since that's an Arduino type this library no longer depends on.

## Supported device types

`HABinarySensor`, `HAButton`, `HACamera`, `HACover`, `HADeviceTracker`, `HADeviceTrigger`,
`HAFan`, `HAHVAC`, `HALight`, `HALock`, `HANumber`, `HAScene`, `HASelect`, `HASensor`,
`HASensorNumber`, `HASwitch`, `HATagScanner`, `HAText`.

Each type can be compiled out to save flash/RAM in `idf.py menuconfig` → Component config →
espHA → Device types (e.g. `CONFIG_ESPHA_DISABLE_CAMERA_FEATURE=y` in `sdkconfig.defaults` if
you don't use the camera entity).

## Debug logging

Debug output goes through the ESP-IDF logging system under the `"AHA"` tag, not through any
Arduino `Serial` object. Enable it in `idf.py menuconfig` → Component config → espHA →
Enable debug logging (`CONFIG_ESPHA_DEBUG=y`), then control verbosity as usual — Component
config → Log output, or at runtime with `esp_log_level_set("AHA", ESP_LOG_DEBUG)`.

## What changed vs. upstream ArduinoHA

- MQTT transport: `PubSubClient`/`Client&` → ESP-IDF's `mqtt` component, used directly.
- No `Arduino.h`, `String`, `IPAddress`, `byte`, `millis()`, `F()`/`PROGMEM`, or
  `__FlashStringHelper` anywhere in `src/`. Every "constant string" the library owns is a plain
  `const char*` — ESP32 has a single unified address space, so there was never a real
  flash-vs-RAM distinction to preserve there.
- Debug printing (`Serial.print()`-style macros) replaced with formatted calls straight onto
  `ESP_LOGD`.
- `examples/` from upstream (Arduino `.ino` sketches) are kept under `examples_old/` for
  reference only — they target the Arduino IDE/PlatformIO and aren't part of this component's
  build.

## License

No LICENSE file is included in this fork yet. Until one is added, check the terms of the
upstream project this was forked from: https://github.com/dawidchyrzynski/arduino-home-assistant

## Author

Sebastian Koczur <themechanos@gmail.com> — https://github.com/TheMechanos/espHA
