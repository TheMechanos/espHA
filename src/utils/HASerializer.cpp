
#include <cassert>
#include <cstring>

#include "HASerializer.h"
#include "../espHADefines.h"
#include "../HADevice.h"
#include "../HAMqtt.h"
#include "../utils/HAUtils.h"
#include "../utils/HANumeric.h"
#include "../device-types/HABaseDeviceType.h"

uint16_t HASerializer::calculateConfigTopicLength(
    const char* componentName,
    const char* objectId
)
{
    const HAMqtt* mqtt = HAMqtt::instance();
    if (
        !componentName ||
        !objectId ||
        !mqtt ||
        !mqtt->getDiscoveryPrefix() ||
        !mqtt->getDevice()
    ) {
        return 0;
    }

    return
        strlen(mqtt->getDiscoveryPrefix()) + 1 + // prefix with slash
        strlen(componentName) + 1 + // component name with slash
        strlen(mqtt->getDevice()->getUniqueId()) + 1 + // device ID with slash
        strlen(objectId) + 1 + // object ID with slash
        strlen(HAConfigTopic) + 1; // including null terminator
}

bool HASerializer::generateConfigTopic(
    char* output,
    const char* componentName,
    const char* objectId
)
{
    const HAMqtt* mqtt = HAMqtt::instance();
    if (
        !output ||
        !componentName ||
        !objectId ||
        !mqtt ||
        !mqtt->getDiscoveryPrefix() ||
        !mqtt->getDevice()
    ) {
        return false;
    }

    strcpy(output, mqtt->getDiscoveryPrefix());
    strcat(output, HASerializerSlash);

    strcat(output, componentName);
    strcat(output, HASerializerSlash);

    strcat(output, mqtt->getDevice()->getUniqueId());
    strcat(output, HASerializerSlash);

    strcat(output, objectId);
    strcat(output, HASerializerSlash);

    strcat(output, HAConfigTopic);
    return true;
}

uint16_t HASerializer::calculateDataTopicLength(
    const char* objectId,
    const char* topic
)
{
    const HAMqtt* mqtt = HAMqtt::instance();
    if (
        !topic ||
        !mqtt ||
        !mqtt->getDataPrefix() ||
        !mqtt->getDevice()
    ) {
        return 0;
    }

    uint16_t size =
        strlen(mqtt->getDataPrefix()) + 1 + // prefix with slash
        strlen(mqtt->getDevice()->getUniqueId()) + 1 + // device ID with slash
        strlen(topic);

    if (objectId) {
        size += strlen(objectId) + 1; // object ID with slash;
    }

    return size + 1; // including null terminator
}

bool HASerializer::generateDataTopic(
    char* output,
    const char* objectId,
    const char* topic
)
{
    const HAMqtt* mqtt = HAMqtt::instance();
    if (
        !output ||
        !topic ||
        !mqtt ||
        !mqtt->getDataPrefix() ||
        !mqtt->getDevice()
    ) {
        return false;
    }

    strcpy(output, mqtt->getDataPrefix());
    strcat(output, HASerializerSlash);

    strcat(output, mqtt->getDevice()->getUniqueId());
    strcat(output, HASerializerSlash);

    if (objectId) {
        strcat(output, objectId);
        strcat(output, HASerializerSlash);
    }

    strcat(output, topic);
    return true;
}

bool HASerializer::compareDataTopics(
    const char* actualTopic,
    const char* objectId,
    const char* topic
)
{
    if (!actualTopic) {
        return false;
    }

    const uint16_t topicLength = calculateDataTopicLength(objectId, topic);
    if (topicLength == 0) {
        return false;
    }

    char expectedTopic[topicLength];
    if (!generateDataTopic(expectedTopic, objectId, topic)) {
        return false;
    }

    return memcmp(actualTopic, expectedTopic, topicLength) == 0;
}

HASerializer::HASerializer(
    HABaseDeviceType* deviceType,
    const uint8_t maxEntriesNb
) :
    _deviceType(deviceType),
    _entriesNb(0),
    _maxEntriesNb(maxEntriesNb),
    _entries(new SerializerEntry[maxEntriesNb])
{

}

HASerializer::~HASerializer()
{
    delete[] _entries;
}

void HASerializer::set(
    const char* property,
    const void* value,
    PropertyValueType valueType
)
{
    if (!property || !value) {
        return;
    }

    SerializerEntry* entry = addEntry();
    entry->type = PropertyEntryType;
    entry->subtype = static_cast<uint8_t>(valueType);
    entry->property = property;
    entry->value = value;
}

void HASerializer::set(const FlagType flag)
{
    if (flag == WithDevice || flag == WithUniqueId) {
        SerializerEntry* entry = addEntry();
        entry->type = FlagEntryType;
        entry->subtype = static_cast<uint8_t>(flag);
        entry->property = nullptr;
        entry->value = nullptr;
    } else if (flag == WithAvailability) {
        HAMqtt* mqtt = HAMqtt::instance();
        const bool isSharedAvailability = mqtt->getDevice()->isSharedAvailabilityEnabled();
        const bool isAvailabilityConfigured = _deviceType->isAvailabilityConfigured();

        if (!isSharedAvailability && !isAvailabilityConfigured) {
            return; // not configured
        }

        SerializerEntry* entry = addEntry();
        entry->type = TopicEntryType;
        entry->property = HAAvailabilityTopic;
        entry->value = isSharedAvailability
            ? mqtt->getDevice()->getAvailabilityTopic()
            : nullptr;
    }
}

void HASerializer::topic(const char* topic)
{
    if (!_deviceType || !topic) {
        return;
    }

    SerializerEntry* entry = addEntry();
    entry->type = TopicEntryType;
    entry->property = topic;
}

HASerializer::SerializerEntry* HASerializer::addEntry()
{
    // catches a too small maxEntriesNb passed by the device type (no check in NDEBUG builds)
    assert(_entriesNb < _maxEntriesNb);
    return &_entries[_entriesNb++];
}

uint16_t HASerializer::calculateSize() const
{
    uint16_t size =
        strlen(HASerializerJsonDataPrefix) +
        strlen(HASerializerJsonDataSuffix);

    for (uint8_t i = 0; i < _entriesNb; i++) {
        const uint16_t entrySize = calculateEntrySize(&_entries[i]);
        if (entrySize == 0) {
            continue;
        }

        size += entrySize;

        // items separator
        if (i > 0) {
            size += strlen(HASerializerJsonPropertiesSeparator);
        }
    }

    return size;
}

bool HASerializer::flush() const
{
    HAMqtt* mqtt = HAMqtt::instance();
    if (!mqtt || (_deviceType && !mqtt->getDevice())) {
        return false;
    }

    mqtt->writePayload(HASerializerJsonDataPrefix);

    for (uint8_t i = 0; i < _entriesNb; i++) {
        if (i > 0) {
            mqtt->writePayload(HASerializerJsonPropertiesSeparator);
        }

        if (!flushEntry(&_entries[i])) {
            return false;
        }
    }

    mqtt->writePayload(HASerializerJsonDataSuffix);
    return true;
}

uint16_t HASerializer::calculateEntrySize(const SerializerEntry* entry) const
{
    switch (entry->type) {
    case PropertyEntryType:
        return
            // property name
            strlen(HASerializerJsonPropertyPrefix) +
            strlen(entry->property) +
            strlen(HASerializerJsonPropertySuffix) +
            // property value
            calculatePropertyValueSize(entry);

    case TopicEntryType:
        return calculateTopicEntrySize(entry);

    case FlagEntryType:
        return calculateFlagSize(
            static_cast<FlagType>(entry->subtype)
        );

    default:
        return 0;
    }
}

uint16_t HASerializer::calculateTopicEntrySize(
    const SerializerEntry* entry
) const
{
    uint16_t size = 0;

    // property name
    size +=
        strlen(HASerializerJsonPropertyPrefix) +
        strlen(entry->property) +
        strlen(HASerializerJsonPropertySuffix);

    // topic escape
    size += 2 * strlen(HASerializerJsonEscapeChar);

    // topic
    if (entry->value) {
        size += strlen(static_cast<const char*>(entry->value));
    } else {
        if (!_deviceType) {
            return 0;
        }

        size += calculateDataTopicLength(
            _deviceType->uniqueId(),
            entry->property
        ) - 1; // exclude null terminator
    }

    return size;
}

uint16_t HASerializer::calculateFlagSize(const FlagType flag) const
{
    const HAMqtt* mqtt = HAMqtt::instance();
    const HADevice* device = mqtt->getDevice();

    if (flag == WithDevice && device->getSerializer()) {
        const uint16_t deviceLength = device->getSerializer()->calculateSize();
        if (deviceLength == 0) {
            return 0;
        }

        return
            strlen(HASerializerJsonPropertyPrefix) +
            strlen(HADeviceProperty) +
            strlen(HASerializerJsonPropertySuffix) +
            deviceLength;
    } else if (flag == WithUniqueId && _deviceType) {
        uint16_t uniqueIdLength = strlen(_deviceType->uniqueId());

        if (device->isExtendedUniqueIdsEnabled()) {
            uniqueIdLength += strlen(device->getUniqueId()) + 1; // with separator
        }

        return
            // property name
            strlen(HASerializerJsonPropertyPrefix) +
            strlen(HAUniqueIdProperty) +
            strlen(HASerializerJsonPropertySuffix) +
            // property value
            2 * strlen(HASerializerJsonEscapeChar) +
            uniqueIdLength;
    }

    return 0;
}

uint16_t HASerializer::calculatePropertyValueSize(
    const SerializerEntry* entry
) const
{
    switch (entry->subtype) {
    case ConstCharPropertyValue:
    case ProgmemPropertyValue: {
        const char* value = static_cast<const char*>(entry->value);
        const uint16_t len =
            entry->subtype == ConstCharPropertyValue ? strlen(value) : strlen(value);
        return 2 * strlen(HASerializerJsonEscapeChar) + len;
    }

    case BoolPropertyType: {
        const bool value = *static_cast<const bool*>(entry->value);
        return value ? strlen(HATrue) : strlen(HAFalse);
    }

    case NumberPropertyType: {
        const HANumeric* value = static_cast<const HANumeric*>(
            entry->value
        );
        return value->calculateSize();
    }

    case ArrayPropertyType: {
        const HASerializerArray* array = static_cast<const HASerializerArray*>(
            entry->value
        );
        return array->calculateSize();
    }

    default:
        return 0;
    }
}

bool HASerializer::flushEntry(const SerializerEntry* entry) const
{
    HAMqtt* mqtt = HAMqtt::instance();

    switch (entry->type) {
    case PropertyEntryType: {
        mqtt->writePayload(HASerializerJsonPropertyPrefix);
        mqtt->writePayload(entry->property);
        mqtt->writePayload(HASerializerJsonPropertySuffix);

        return flushEntryValue(entry);
    }

    case TopicEntryType:
        return flushTopic(entry);

    case FlagEntryType:
        return flushFlag(entry);

    default:
        return true;
    }
}

bool HASerializer::flushEntryValue(const SerializerEntry* entry) const
{
    HAMqtt* mqtt = HAMqtt::instance();

    switch (entry->subtype) {
    case ConstCharPropertyValue:
    case ProgmemPropertyValue: {
        const char* value = static_cast<const char*>(entry->value);
        mqtt->writePayload(HASerializerJsonEscapeChar);

        if (entry->subtype == ConstCharPropertyValue) {
            mqtt->writePayload(value, strlen(value));
        } else {
            mqtt->writePayload(value);
        }

        mqtt->writePayload(HASerializerJsonEscapeChar);
        return true;
    }

    case BoolPropertyType: {
        const bool value = *static_cast<const bool*>(entry->value);
        mqtt->writePayload(value ? HATrue : HAFalse);
        return true;
    }

    case NumberPropertyType: {
        const HANumeric* value = static_cast<const HANumeric*>(
            entry->value
        );

        char tmp[HANumeric::MaxDigitsNb + 1];
        const uint16_t length = value->toStr(tmp);

        mqtt->writePayload(tmp, length);
        return true;
    }

    case ArrayPropertyType: {
        const HASerializerArray* array = static_cast<const HASerializerArray*>(
            entry->value
        );
        const uint16_t size = array->calculateSize();
        char tmp[size + 1]; // including null terminator
        tmp[0] = 0;
        array->serialize(tmp);
        mqtt->writePayload(tmp, size);

        return true;
    }

    default:
        return false;
    }
}

bool HASerializer::flushTopic(const SerializerEntry* entry) const
{
    HAMqtt* mqtt = HAMqtt::instance();

    // property name
    mqtt->writePayload(HASerializerJsonPropertyPrefix);
    mqtt->writePayload(entry->property);
    mqtt->writePayload(HASerializerJsonPropertySuffix);

    // value (escaped)
    mqtt->writePayload(HASerializerJsonEscapeChar);

    if (entry->value) {
        const char* topic = static_cast<const char*>(entry->value);
        mqtt->writePayload(topic, strlen(topic));
    } else {
        const uint16_t length = calculateDataTopicLength(
            _deviceType->uniqueId(),
            entry->property
        );
        if (length == 0) {
            return false;
        }

        char topic[length];
        generateDataTopic(
            topic,
            _deviceType->uniqueId(),
            entry->property
        );

        mqtt->writePayload(topic, length - 1);
    }

    mqtt->writePayload(HASerializerJsonEscapeChar);
    return true;
}

bool HASerializer::flushFlag(const SerializerEntry* entry) const
{
    HAMqtt* mqtt = HAMqtt::instance();
    const HADevice* device = mqtt->getDevice();
    const FlagType flag = static_cast<FlagType>(entry->subtype);

    if (flag == WithDevice && device) {
        // property name
        mqtt->writePayload(HASerializerJsonPropertyPrefix);
        mqtt->writePayload(HADeviceProperty);
        mqtt->writePayload(HASerializerJsonPropertySuffix);

        // property value
        return device->getSerializer()->flush();
    } else if (flag == WithUniqueId && _deviceType) {
        // property name
        mqtt->writePayload(HASerializerJsonPropertyPrefix);
        mqtt->writePayload(HAUniqueIdProperty);
        mqtt->writePayload(HASerializerJsonPropertySuffix);

        // value
        const char* uniqueId = _deviceType->uniqueId();
        mqtt->writePayload(HASerializerJsonEscapeChar);

        if (device->isExtendedUniqueIdsEnabled()) {
            const char* deviceUniqueId = device->getUniqueId();
            mqtt->writePayload(deviceUniqueId, strlen(deviceUniqueId));
            mqtt->writePayload(HASerializerUnderscore);
        }

        mqtt->writePayload(uniqueId, strlen(uniqueId));
        mqtt->writePayload(HASerializerJsonEscapeChar);

        return true;
    }

    return false;
}