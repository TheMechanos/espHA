#include "HANumber.h"
#ifndef EX_ARDUINOHA_NUMBER

#include <cstring>

#include "../HAMqtt.h"
#include "../utils/HASerializer.h"

HANumber::HANumber(const char* uniqueId, const NumberPrecision precision) :
    HABaseDeviceType(HAComponentNumber, uniqueId),
    _precision(precision),
    _class(nullptr),
    _icon(nullptr),
    _retain(false),
    _optimistic(false),
    _mode(ModeAuto),
    _unitOfMeasurement(nullptr),
    _minValue(),
    _maxValue(),
    _step(),
    _currentState(),
    _commandCallback(nullptr)
{

}

bool HANumber::setState(const HANumeric& state, const bool force)
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

void HANumber::buildSerializer()
{
    if (_serializer || !uniqueId()) {
        return;
    }

    _serializer = new HASerializer(this, 16); // 16 - max properties nb
    _serializer->set(HANameProperty, _name);
    _serializer->set(HAObjectIdProperty, _objectId);
    _serializer->set(HASerializer::WithUniqueId);
    _serializer->set(HADeviceClassProperty, _class);
    _serializer->set(HAIconProperty, _icon);
    _serializer->set(HAUnitOfMeasurementProperty, _unitOfMeasurement);
    _serializer->set(
        HAModeProperty,
        getModeProperty(),
        HASerializer::ProgmemPropertyValue
    );
    _serializer->set(
        HACommandTemplateProperty,
        getCommandTemplate(),
        HASerializer::ProgmemPropertyValue
    );

    if (_minValue.isSet()) {
        _serializer->set(
            HAMinProperty,
            &_minValue,
            HASerializer::NumberPropertyType
        );
    }

    if (_maxValue.isSet()) {
        _serializer->set(
            HAMaxProperty,
            &_maxValue,
            HASerializer::NumberPropertyType
        );
    }

    if (_step.isSet()) {
        _serializer->set(
            HAStepProperty,
            &_step,
            HASerializer::NumberPropertyType
        );
    }

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

    _serializer->set(HASerializer::WithDevice);
    _serializer->set(HASerializer::WithAvailability);
    _serializer->topic(HAStateTopic);
    _serializer->topic(HACommandTopic);
}

void HANumber::onMqttConnected()
{
    if (!uniqueId()) {
        return;
    }

    publishConfig();
    publishAvailability();

    if (!_retain) {
        publishState(_currentState);
    }

    subscribeTopic(uniqueId(), HACommandTopic);
}

void HANumber::onMqttMessage(
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

bool HANumber::publishState(const HANumeric& state)
{
    if (!state.isSet()) {
        return publishOnDataTopic(
            HAStateTopic,
            HAStateNone,
            true
        );
    }

    const uint8_t size = state.calculateSize();
    if (size == 0) {
        return false;
    }

    char str[size + 1]; // with null terminator
    str[size] = 0;
    state.toStr(str);

    return publishOnDataTopic(
        HAStateTopic,
        str,
        true
    );
}

void HANumber::handleCommand(const uint8_t* cmd, const uint16_t length)
{
    if (!_commandCallback) {
        return;
    }

    if (memcmp(cmd, HAStateNone, length) == 0) {
        _commandCallback(HANumeric(), this);
    } else {
        HANumeric number = HANumeric::fromStr(cmd, length);
        if (number.isSet()) {
            number.setPrecision(_precision);
            _commandCallback(number, this);
        }
    }
}

const char* HANumber::getModeProperty() const
{
    switch (_mode) {
    case ModeBox:
        return HAModeBox;

    case ModeSlider:
        return HAModeSlider;

    default:
        return nullptr;
    }
}

const char* HANumber::getCommandTemplate()
{
    switch (_precision) {
    case PrecisionP1:
        return HAValueTemplateFloatP1;

    case PrecisionP2:
        return HAValueTemplateFloatP2;

    case PrecisionP3:
        return HAValueTemplateFloatP3;

    default:
        return nullptr;
    }
}

#endif
