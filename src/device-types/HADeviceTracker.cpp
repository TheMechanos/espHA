#include "HADeviceTracker.h"
#ifndef CONFIG_ESPHA_DISABLE_DEVICE_TRACKER_FEATURE

#include "../HAMqtt.h"
#include "../utils/HASerializer.h"

HADeviceTracker::HADeviceTracker(const char* uniqueId) :
    HABaseDeviceType(HAComponentDeviceTracker, uniqueId),
    _icon(nullptr),
    _sourceType(SourceTypeUnknown),
    _currentState(StateUnknown)
{

}

bool HADeviceTracker::setState(const TrackerState state, const bool force)
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

void HADeviceTracker::buildSerializer()
{
    if (_serializer || !uniqueId()) {
        return;
    }

    _serializer = new HASerializer(this, 10); // 10 - max properties nb
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
    _serializer->set(
        HASourceTypeProperty,
        getSourceTypeProperty(),
        HASerializer::ProgmemPropertyValue
    );
    _serializer->set(HASerializer::WithDevice);
    _serializer->set(HASerializer::WithAvailability);
    _serializer->topic(HAStateTopic);
}

void HADeviceTracker::onMqttConnected()
{
    if (!uniqueId()) {
        return;
    }

    publishConfig();
    publishAvailability();
    publishState(_currentState);
}

bool HADeviceTracker::publishState(const TrackerState state)
{
    const char *stateStr = nullptr;
    switch (state) {
    case StateHome:
        stateStr = HAHome;
        break;

    case StateNotHome:
        stateStr = HANotHome;
        break;

    case StateNotAvailable:
        stateStr = HAOffline;
        break;

    default:
        return false;
    }

    return publishOnDataTopic(HAStateTopic, stateStr, true);
}

const char* HADeviceTracker::getSourceTypeProperty() const
{
    switch (_sourceType) {
    case SourceTypeGPS:
        return HAGPSType;

    case SourceTypeRouter:
        return HARouterType;

    case SourceTypeBluetooth:
        return HABluetoothType;

    case SourceTypeBluetoothLE:
        return HABluetoothLEType;

    default:
        return nullptr;
    }
}

#endif
