#include "HAButton.h"
#ifndef CONFIG_ESPHA_DISABLE_BUTTON_FEATURE

#include "../HAMqtt.h"
#include "../utils/HASerializer.h"

HAButton::HAButton(const char* uniqueId) :
    HABaseDeviceType(HAComponentButton, uniqueId),
    _class(nullptr),
    _icon(nullptr),
    _retain(false),
    _commandCallback(nullptr)
{

}

void HAButton::buildSerializer()
{
    if (_serializer || !uniqueId()) {
        return;
    }

    _serializer = new HASerializer(this, 9); // 9 - max properties nb
    _serializer->set(HANameProperty, _name);
    _serializer->set(HAObjectIdProperty, _objectId);
    _serializer->set(HASerializer::WithUniqueId);
    _serializer->set(HADeviceClassProperty, _class);
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
}

void HAButton::onMqttConnected()
{
    if (!uniqueId()) {
        return;
    }

    publishConfig();
    publishAvailability();
    subscribeTopic(uniqueId(), HACommandTopic);
}

void HAButton::onMqttMessage(
    const char* topic,
    const uint8_t* payload,
    const uint16_t length
)
{
    (void)payload;
    (void)length;

    if (_commandCallback && HASerializer::compareDataTopics(
        topic,
        uniqueId(),
        HACommandTopic
    )) {
        _commandCallback(this);
    }
}

#endif
