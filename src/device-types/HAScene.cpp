#include "HAScene.h"
#ifndef EX_ARDUINOHA_SCENE

#include "../HAMqtt.h"
#include "../utils/HASerializer.h"

HAScene::HAScene(const char* uniqueId) :
    HABaseDeviceType(HAComponentScene, uniqueId),
    _icon(nullptr),
    _retain(false),
    _commandCallback(nullptr)
{

}

void HAScene::buildSerializer()
{
    if (_serializer || !uniqueId()) {
        return;
    }

    _serializer = new HASerializer(this, 8); // 8 - max properties nb
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

    // HA 2022.10 throws an exception if this property is not set
    _serializer->set(
        HAPayloadOnProperty,
        HAStateOn,
        HASerializer::ProgmemPropertyValue
    );

    _serializer->set(HASerializer::WithAvailability);
    _serializer->topic(HACommandTopic);
}

void HAScene::onMqttConnected()
{
    if (!uniqueId()) {
        return;
    }

    publishConfig();
    publishAvailability();
    subscribeTopic(uniqueId(), HACommandTopic);
}

void HAScene::onMqttMessage(
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
