#include "HAFan.h"
#ifndef CONFIG_ESPHA_DISABLE_FAN_FEATURE

#include <cstring>

#include "../HAMqtt.h"
#include "../utils/HAUtils.h"
#include "../utils/HASerializer.h"

HAFan::HAFan(const char* uniqueId, const uint8_t features) :
    HABaseDeviceType(HAComponentFan, uniqueId),
    _features(features),
    _icon(nullptr),
    _retain(false),
    _optimistic(false),
    _speedRangeMax(),
    _speedRangeMin(),
    _currentState(false),
    _currentSpeed(0),
    _stateCallback(nullptr),
    _speedCallback(nullptr)
{

}

bool HAFan::setState(const bool state, const bool force)
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

bool HAFan::setSpeed(const uint16_t speed, const bool force)
{
    if (!force && speed == _currentSpeed) {
        return true;
    }

    if (publishSpeed(speed)) {
        _currentSpeed = speed;
        return true;
    }

    return false;
}

void HAFan::buildSerializer()
{
    if (_serializer || !uniqueId()) {
        return;
    }

    _serializer = new HASerializer(this, 14); // 14 - max properties nb
    _serializer->set(HANameProperty, _name);
    _serializer->set(HAObjectIdProperty, _objectId);
    _serializer->set(HASerializer::WithUniqueId);
    _serializer->set(HAIconProperty, _icon);

    if (_retain) {
        _serializer->set(
            HARetainProperty,
            &_retain,
            HASerializer::BoolPropertyType
        );
    }

    if (_optimistic) {
        _serializer->set(
            HAOptimisticProperty,
            &_optimistic,
            HASerializer::BoolPropertyType
        );
    }

    if (_features & SpeedsFeature) {
        _serializer->topic(HAPercentageStateTopic);
        _serializer->topic(HAPercentageCommandTopic);

        if (_speedRangeMax.isSet()) {
            _serializer->set(
                HASpeedRangeMaxProperty,
                &_speedRangeMax,
                HASerializer::NumberPropertyType
            );
        }

        if (_speedRangeMin.isSet()) {
            _serializer->set(
                HASpeedRangeMinProperty,
                &_speedRangeMin,
                HASerializer::NumberPropertyType
            );
        }
    }

    _serializer->set(HASerializer::WithDevice);
    _serializer->set(HASerializer::WithAvailability);
    _serializer->topic(HAStateTopic);
    _serializer->topic(HACommandTopic);
}

void HAFan::onMqttConnected()
{
    if (!uniqueId()) {
        return;
    }

    publishConfig();
    publishAvailability();

    if (!_retain) {
        publishState(_currentState);
        publishSpeed(_currentSpeed);
    }

    subscribeTopic(uniqueId(), HACommandTopic);

    if (_features & SpeedsFeature) {
        subscribeTopic(uniqueId(), HAPercentageCommandTopic);
    }
}

void HAFan::onMqttMessage(
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
        handleStateCommand(payload, length);
    } else if (HASerializer::compareDataTopics(
        topic,
        uniqueId(),
        HAPercentageCommandTopic
    )) {
        handleSpeedCommand(payload, length);
    }
}

bool HAFan::publishState(const bool state)
{
    return publishOnDataTopic(
        HAStateTopic,
        state ? HAStateOn : HAStateOff,
        true
    );
}

bool HAFan::publishSpeed(const uint16_t speed)
{
    if (!(_features & SpeedsFeature)) {
        return false;
    }

    char str[5 + 1] = {0}; // uint16_t digits with null terminator
    HANumeric(speed, 0).toStr(str);

    return publishOnDataTopic(HAPercentageStateTopic, str, true);
}

void HAFan::handleStateCommand(const uint8_t* cmd, const uint16_t length)
{
    (void)cmd;

    if (!_stateCallback) {
        return;
    }

    bool state = length == strlen(HAStateOn);
    _stateCallback(state, this);
}

void HAFan::handleSpeedCommand(const uint8_t* cmd, const uint16_t length)
{
    if (!_speedCallback) {
        return;
    }

    const HANumeric& number = HANumeric::fromStr(cmd, length);
    if (number.isUInt16()) {
        _speedCallback(number.toUInt16(), this);
    }
}

#endif
