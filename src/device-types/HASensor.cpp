#include "HASensor.h"
#ifndef EX_ARDUINOHA_SENSOR

#include "../HAMqtt.h"
#include "../utils/HASerializer.h"

HASensor::HASensor(const char* uniqueId, const uint16_t features) :
    HABaseDeviceType(HAComponentSensor, uniqueId),
    _features(features),
    _deviceClass(nullptr),
    _stateClass(nullptr),
    _forceUpdate(false),
    _icon(nullptr),
    _unitOfMeasurement(nullptr),
    _expireAfter()
{

}

bool HASensor::setValue(const char* value)
{
    if (!value) {
        return publishOnDataTopic(HAStateTopic, HAStateNone, true);
    }

    return publishOnDataTopic(HAStateTopic, value, true);
}

bool HASensor::setJsonAttributes(const char* json)
{
    return publishOnDataTopic(HAJsonAttributesTopic, json, true);
}

void HASensor::setExpireAfter(uint16_t expireAfter)
{
    if (expireAfter > 0) {
        _expireAfter.setBaseValue(expireAfter);
    } else {
        _expireAfter.reset();
    }
}

void HASensor::buildSerializer()
{
    if (_serializer || !uniqueId()) {
        return;
    }

    _serializer = new HASerializer(this, 13); // 13 - max properties nb
    _serializer->set(HANameProperty, _name);
    _serializer->set(HAObjectIdProperty, _objectId);
    _serializer->set(HASerializer::WithUniqueId);
    _serializer->set(HADeviceClassProperty, _deviceClass);
    _serializer->set(HAStateClassProperty, _stateClass);
    _serializer->set(HAIconProperty, _icon);
    _serializer->set(HAUnitOfMeasurementProperty, _unitOfMeasurement);

    if (_forceUpdate) {
        _serializer->set(
            HAForceUpdateProperty,
            &_forceUpdate,
            HASerializer::BoolPropertyType
        );
    }

    if (_expireAfter.isSet()) {
        _serializer->set(
            HAExpireAfterProperty,
            &_expireAfter,
            HASerializer::NumberPropertyType
        );
    }

    if (_features & JsonAttributesFeature) {
        _serializer->topic(HAJsonAttributesTopic);
    }

    _serializer->set(HASerializer::WithDevice);
    _serializer->set(HASerializer::WithAvailability);
    _serializer->topic(HAStateTopic);
}

void HASensor::onMqttConnected()
{
    if (!uniqueId()) {
        return;
    }

    publishConfig();
    publishAvailability();
}

#endif
