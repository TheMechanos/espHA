#pragma once

#include "HABaseDeviceType.h"

#ifndef CONFIG_ESPHA_DISABLE_TEXT_FEATURE

#define HAText_CALLBACK(name) void (*name)(const char* value, HAText* sender)

/**
 * HAText represents an editable text field in the Home Assistant panel.
 * The value entered in HA is delivered to the device via the command callback,
 * and the device reports its current value back using HAText::setValue.
 *
 * @note
 * You can find more information about this entity in the Home Assistant documentation:
 * https://www.home-assistant.io/integrations/text.mqtt/
 */
class HAText : public HABaseDeviceType
{
public:
    /**
     * @param uniqueId The unique ID. It needs to be unique in a scope of your device.
     */
    HAText(const char* uniqueId);

    /**
     * Sets icon of the text entity.
     * Any icon from MaterialDesignIcons.com (for example: `mdi:home`).
     *
     * @param icon The icon name.
     */
    inline void setIcon(const char* icon)
        { _icon = icon; }

    /**
     * Sets retain flag for the commands produced by HA.
     * If set to `true` the command will be retained by the broker.
     *
     * @param retain
     */
    inline void setRetain(const bool retain)
        { _retain = retain; }

    /**
     * Publishes the current value on the state topic (retained).
     *
     * @param value The text value.
     * @returns Returns `true` if MQTT message has been published successfully.
     */
    bool setValue(const char* value);

    /**
     * Registers callback that will be called each time the value is changed in the HA panel.
     * The value is a null-terminated copy valid only for the duration of the callback.
     * Please note that it's not possible to register multiple callbacks for the same entity.
     *
     * @param callback
     * @note The new value must be reported back to HA using the HAText::setValue method,
     *       otherwise HA keeps showing the previous one.
     * @note The callback is executed in the esp-mqtt client task.
     */
    inline void onCommand(HAText_CALLBACK(callback))
        { _commandCallback = callback; }

protected:
    virtual void buildSerializer() override;
    virtual void onMqttConnected() override;
    virtual void onMqttMessage(
        const char* topic,
        const uint8_t* payload,
        const uint16_t length
    ) override;

private:
    /**
     * Converts the command payload into a null-terminated string and calls the callback.
     */
    void handleCommand(const uint8_t* cmd, const uint16_t length);

    /// The icon of the text entity. It can be nullptr.
    const char* _icon;

    /// The retain flag for the HA commands.
    bool _retain;

    /// The command callback that will be called when the value is changed in HA.
    HAText_CALLBACK(_commandCallback);
};

#endif
