#include "HAText.h"
#ifndef CONFIG_ESPHA_DISABLE_TEXT_FEATURE

#include "../HAMqtt.h"
#include "../utils/HASerializer.h"

HAText::HAText(const char* uniqueId) :
    HABaseDeviceType(HAComponentText, uniqueId),
    _icon(nullptr)
{

}

void HAText::buildSerializer()
{
    if (_serializer || !uniqueId()) {
        return;
    }

    _serializer = new HASerializer(this, 9); // 9 - max properties nb
    _serializer->set(HANameProperty, _name);
    _serializer->set(HAObjectIdProperty, _objectId);
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
}


bool HAText::setValue(const char* value)
{
    return publishOnDataTopic(HAStateTopic, value, true);
}

#endif
