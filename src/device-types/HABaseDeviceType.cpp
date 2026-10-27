#include <cstring>

#include "HABaseDeviceType.h"
#include "../HAMqtt.h"
#include "../HADevice.h"
#include "../utils/HAUtils.h"
#include "../utils/HASerializer.h"

HABaseDeviceType::HABaseDeviceType(
    const char* componentName,
    const char* uniqueId
) :
    _componentName(componentName),
    _uniqueId(uniqueId),
    _name(nullptr),
    _objectId(nullptr),
    _entityCategory(EntityCategoryNone),
    _enabledByDefault(true),
    _serializer(nullptr),
    _availability(AvailabilityDefault)
{
    if (mqtt()) {
        mqtt()->addDeviceType(this);
    }
}

bool HABaseDeviceType::setUniqueIdStr(char* pstr)
{
    if (_uniqueId) {
        return false; // unique ID cannot be changed at runtime once it's set
    }


    char* dst = strdup(pstr); // include null terminator

    _uniqueId = dst;

    return true;
}


bool HABaseDeviceType::setNameUniqueIdStr(char* unic_prefix, const char* name, const char *uid)

{  char str[80], *dst;

    if (_uniqueId) {
        return false; // unique ID cannot be changed at runtime once it's set
    }

   strcpy(str,unic_prefix);	
   strcat(str,"_");	
   strcat(str,uid);	

   dst = strdup(str); // include null terminator

   _uniqueId = dst;
    _name = strdup(name);

    return true;
}


void HABaseDeviceType::setAvailability(bool online)
{
    _availability = (online ? AvailabilityOnline : AvailabilityOffline);
    publishAvailability();
}

HAMqtt* HABaseDeviceType::mqtt()
{
    return HAMqtt::instance();
}

void HABaseDeviceType::subscribeTopic(
    const char* uniqueId,
    const char* topic
)
{
    const uint16_t topicLength = HASerializer::calculateDataTopicLength(
        uniqueId,
        topic
    );
    if (topicLength == 0) {
        return;
    }

    char fullTopic[topicLength];
    if (!HASerializer::generateDataTopic(
        fullTopic,
        uniqueId,
        topic
    )) {
        return;
    }

    HAMqtt::instance()->subscribe(fullTopic);
}

void HABaseDeviceType::onMqttMessage(
    const char* topic,
    const uint8_t* payload,
    const uint16_t length
)
{
    (void)topic;
    (void)payload;
    (void)length;
}

const char* HABaseDeviceType::entityCategoryStr() const
{
    switch (_entityCategory) {
    case EntityCategoryConfig:
        return HAEntityCategoryConfig;

    case EntityCategoryDiagnostic:
        return HAEntityCategoryDiagnostic;

    default:
        return nullptr;
    }
}

void HABaseDeviceType::destroySerializer()
{
    if (_serializer) {
        delete _serializer;
        _serializer = nullptr;
    }
}

void HABaseDeviceType::publishConfig()
{
    buildSerializer();

    if (_serializer == nullptr) {
        return;
    }

    const uint16_t topicLength = HASerializer::calculateConfigTopicLength(
        componentName(),
        uniqueId()
    );
    const uint16_t dataLength = _serializer->calculateSize();

    if (topicLength > 0 && dataLength > 0) {
        char topic[topicLength];
        HASerializer::generateConfigTopic(
            topic,
            componentName(),
            uniqueId()
        );

        if (mqtt()->beginPublish(topic, dataLength, true)) {
            _serializer->flush();
            mqtt()->endPublish();
        }
    }

    destroySerializer();
}

void HABaseDeviceType::publishAvailability()
{
    const HADevice* device = mqtt()->getDevice();
    if (
        !device ||
        device->isSharedAvailabilityEnabled() ||
        !isAvailabilityConfigured()
    ) {
        return;
    }

    publishOnDataTopic(
        HAAvailabilityTopic,
        _availability == AvailabilityOnline
            ? HAOnline
            : HAOffline,
        true
    );
}

bool HABaseDeviceType::publishOnDataTopic(
    const char* topic,
    const char* payload,
    bool retained
)
{
    if (!payload) {
        return false;
    }

    return publishOnDataTopic(
        topic,
        reinterpret_cast<const uint8_t*>(payload),
        strlen(payload),
        retained,
        true
    );
}

bool HABaseDeviceType::publishOnDataTopic(
    const char* topic,
    const uint8_t* payload,
    const uint16_t length,
    bool retained,
    bool isNullTerminated
)
{
    if (!payload) {
        return false;
    }

    const uint16_t topicLength = HASerializer::calculateDataTopicLength(
        uniqueId(),
        topic
    );
    if (topicLength == 0) {
        return false;
    }

    char fullTopic[topicLength];
    if (!HASerializer::generateDataTopic(
        fullTopic,
        uniqueId(),
        topic
    )) {
        return false;
    }

    if (mqtt()->beginPublish(fullTopic, length, retained)) {
        if (isNullTerminated) {
            mqtt()->writePayload(reinterpret_cast<const char*>(payload));
        } else {
            mqtt()->writePayload(payload, length);
        }

        return mqtt()->endPublish();
    }

    return false;
}