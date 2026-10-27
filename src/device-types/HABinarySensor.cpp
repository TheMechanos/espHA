#include "HABinarySensor.h"
#ifndef CONFIG_ESPHA_DISABLE_BINARY_SENSOR_FEATURE

#include "../HAMqtt.h"
#include "../utils/HASerializer.h"

HABinarySensor::HABinarySensor(const char* uniqueId) :
    HABaseDeviceType(HAComponentBinarySensor, uniqueId),
    _class(nullptr),
    _icon(nullptr),
    _currentState(false)
{

}

bool HABinarySensor::setState(const bool state, const bool force)
{
    if (!force && state == _currentState) {
        return true;
    }

    if (publishState(state)) {
        _currentState = state;
        return true;
    }

    return false;
}

void HABinarySensor::setExpireAfter(uint16_t expireAfter)
{
    if (expireAfter > 0) {
        _expireAfter.setBaseValue(expireAfter);
    } else {
        _expireAfter.reset();
    }
}

void HABinarySensor::buildSerializer()
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
    _serializer->set(HADeviceClassProperty, _class);
    _serializer->set(HAIconProperty, _icon);

    if (_expireAfter.isSet()) {
        _serializer->set(
            HAExpireAfterProperty,
            &_expireAfter,
            HASerializer::NumberPropertyType
        );
    }

    _serializer->set(HASerializer::WithDevice);
    _serializer->set(HASerializer::WithAvailability);
    _serializer->topic(HAStateTopic);
}

void HABinarySensor::onMqttConnected()
{
    if (!uniqueId()) {
        return;
    }

    publishConfig();
    publishAvailability();
    publishState(_currentState);
}

bool HABinarySensor::publishState(const bool state)
{
    return publishOnDataTopic(
        HAStateTopic,
        state ? HAStateOn : HAStateOff,
        true
    );
}

#endif
