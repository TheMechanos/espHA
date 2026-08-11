# espHA - Home Assistant MQTT discovery library for ESP-IDF.
# Author: Sebastian Koczur <themechanos@gmail.com>
# https://github.com/TheMechanos/espHA
# Originally based on Dawid Chyrzynski's arduino-home-assistant.

idf_component_register(
    SRCS
        "src/HADevice.cpp"
        "src/HAMqtt.cpp"

        "src/device-types/HABaseDeviceType.cpp"
        "src/device-types/HABinarySensor.cpp"
        "src/device-types/HAButton.cpp"
        "src/device-types/HACamera.cpp"
        "src/device-types/HACover.cpp"
        "src/device-types/HADeviceTracker.cpp"
        "src/device-types/HADeviceTrigger.cpp"
        "src/device-types/HAFan.cpp"
        "src/device-types/HAHVAC.cpp"
        "src/device-types/HALight.cpp"
        "src/device-types/HALock.cpp"
        "src/device-types/HANumber.cpp"
        "src/device-types/HAScene.cpp"
        "src/device-types/HASelect.cpp"
        "src/device-types/HASensor.cpp"
        "src/device-types/HASensorNumber.cpp"
        "src/device-types/HASwitch.cpp"
        "src/device-types/HATagScanner.cpp"
        "src/device-types/HAText.cpp"

        "src/utils/HADictionary.cpp"
        "src/utils/HANumeric.cpp"
        "src/utils/HASerializer.cpp"
        "src/utils/HASerializerArray.cpp"
        "src/utils/HAUtils.cpp"

    INCLUDE_DIRS
        "src"
        "src/device-types"
        "src/utils"

    REQUIRES
        mqtt
        esp_timer
        log
)