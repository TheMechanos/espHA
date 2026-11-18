#include "HAText.h"
#ifndef CONFIG_ESPHA_DISABLE_TEXT_FEATURE

#include <cstring>

#include "../HAMqtt.h"
#include "../utils/HASerializer.h"

HAText::HAText(const char* uniqueId) :
    HABaseDeviceType(HAComponentText, uniqueId),
    _icon(nullptr),
    _retain(false),
    _commandCallback(nullptr)
{

}

void HAText::buildSerializer()
{
    if (_serializer || !uniqueId()) {
        return;
    }

    _serializer = new HASerializer(this, 11); // 11 - max properties nb
    _serializer->set(HANameProperty, _name);
    _serializer->set(HAObjectIdProperty, _objectId);
    _serializer->set(HAEntityCategoryProperty, entityCategoryStr());
    _serializer->set(
        HAEnabledByDefaultProperty,
        disabledByDefaultValue(),
        HASerializer::BoolPropertyType
    );
    _serializer->set(HASerializer::WithUniqueId);
    _serializer->set(HAIconProperty, _icon);

    // optional property
    if (_retain) {
        _serializer->set(
            HARetainProperty,
            &_retain,
            HASerializer::BoolPropertyType
        );
    }

    _serializer->set(HASerializer::WithDevice);
    _serializer->set(HASerializer::WithAvailability);
    _serializer->topic(HACommandTopic);
    _serializer->topic(HAStateTopic);
}

void HAText::onMqttConnected()
{
    if (!uniqueId()) {
        return;
    }

    publishConfig();
    publishAvailability();
    subscribeTopic(uniqueId(), HACommandTopic);
}

void HAText::onMqttMessage(
    const char* topic,
    const uint8_t* payload,
    const uint16_t length
)
{
    if (HASerializer::compareDataTopics(
        topic,
        uniqueId(),
        HACommandTopic
    )) {
        handleCommand(payload, length);
    }
}

bool HAText::setValue(const char* value)
{
    return publishOnDataTopic(HAStateTopic, value, true);
}

void HAText::handleCommand(const uint8_t* cmd, const uint16_t length)
{
    if (!_commandCallback) {
        return;
    }

    // the MQTT payload isn't null-terminated, an empty text is a valid value
    char value[length + 1];
    if (cmd && length > 0) {
        memcpy(value, cmd, length);
    }
    value[length] = '\0';

    _commandCallback(value, this);
}

#endif
