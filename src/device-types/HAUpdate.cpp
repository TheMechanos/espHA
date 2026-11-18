#include "HAUpdate.h"
#ifndef CONFIG_ESPHA_DISABLE_UPDATE_FEATURE

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

#include "esp_app_desc.h"

#include "../HAMqtt.h"
#include "../utils/HASerializer.h"

namespace {

// Appends `"key":` to the JSON output.
void appendJsonKey(std::string& out, const char* key)
{
    out += '"';
    out += key;
    out += "\":";
}

// Appends a quoted and escaped JSON string value.
void appendJsonString(std::string& out, const char* value)
{
    out += '"';

    for (const char* p = value; *p; p++) {
        const unsigned char c = static_cast<unsigned char>(*p);

        switch (c) {
        case '"':
            out += "\\\"";
            break;

        case '\\':
            out += "\\\\";
            break;

        case '\n':
            out += "\\n";
            break;

        case '\r':
            out += "\\r";
            break;

        case '\t':
            out += "\\t";
            break;

        default:
            if (c < 0x20) {
                char buf[7];
                snprintf(buf, sizeof(buf), "\\u%04x", c);
                out += buf;
            } else {
                out += static_cast<char>(c);
            }
            break;
        }
    }

    out += '"';
}

} // namespace

HAUpdate::HAUpdate(const char* uniqueId) :
    HABaseDeviceType(HAComponentUpdate, uniqueId),
    _title(nullptr),
    _icon(nullptr),
    _class(HAUpdateDeviceClassFirmware),
    _installedVersion(nullptr),
    _latestVersion(nullptr),
    _releaseSummary(nullptr),
    _releaseUrl(nullptr),
    _inProgress(false),
    _progress(-1),
    _installCallback(nullptr)
{

}

HAUpdate::~HAUpdate()
{
    free(_installedVersion);
    free(_latestVersion);
    free(_releaseSummary);
    free(_releaseUrl);
}

bool HAUpdate::setInstalledVersion(const char* version)
{
    if (!replaceString(_installedVersion, version)) {
        return true;
    }

    return publishState();
}

bool HAUpdate::setRelease(
    const char* latestVersion,
    const char* summary,
    const char* url
)
{
    // bitwise OR on purpose - every field has to be replaced
    const bool changed =
        replaceString(_latestVersion, latestVersion) |
        replaceString(_releaseSummary, summary) |
        replaceString(_releaseUrl, url);

    if (!changed) {
        return true;
    }

    return publishState();
}

bool HAUpdate::setInProgress(bool inProgress)
{
    const int16_t progress = inProgress ? _progress : -1;
    if (inProgress == _inProgress && progress == _progress) {
        return true;
    }

    _inProgress = inProgress;
    _progress = progress;
    return publishState();
}

bool HAUpdate::setProgress(uint8_t percentage)
{
    const int16_t progress = percentage > 100 ? 100 : percentage;
    if (_inProgress && progress == _progress) {
        return true;
    }

    _inProgress = true;
    _progress = progress;
    return publishState();
}

const char* HAUpdate::getInstalledVersion() const
{
    if (_installedVersion) {
        return _installedVersion;
    }

    return esp_app_get_description()->version;
}

void HAUpdate::buildSerializer()
{
    if (_serializer || !uniqueId()) {
        return;
    }

    _serializer = new HASerializer(this, 13); // 13 - max properties nb
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
    _serializer->set(HATitleProperty, _title);
    _serializer->set(HASerializer::WithDevice);
    _serializer->set(HASerializer::WithAvailability);
    _serializer->topic(HAStateTopic);

    // without the command topic HA doesn't offer the install action
    if (_installCallback) {
        _serializer->topic(HACommandTopic);
    }
}

void HAUpdate::onMqttConnected()
{
    if (!uniqueId()) {
        return;
    }

    publishConfig();
    publishAvailability();
    publishState();

    if (_installCallback) {
        subscribeTopic(uniqueId(), HACommandTopic);
    }
}

void HAUpdate::onMqttMessage(
    const char* topic,
    const uint8_t* payload,
    const uint16_t length
)
{
    if (!_installCallback || !HASerializer::compareDataTopics(
        topic,
        uniqueId(),
        HACommandTopic
    )) {
        return;
    }

    const size_t commandLength = strlen(HAInstallCommand);
    if (
        !payload ||
        length != commandLength ||
        memcmp(payload, HAInstallCommand, commandLength) != 0
    ) {
        return;
    }

    // ignore repeated clicks while the update is running
    if (_inProgress) {
        return;
    }

    _installCallback(this);
}

bool HAUpdate::publishState()
{
    if (!uniqueId()) {
        return false;
    }

    std::string json;
    json.reserve(128);
    json += '{';

    appendJsonKey(json, HAInstalledVersionKey);
    appendJsonString(json, getInstalledVersion());

    if (_latestVersion) {
        json += ',';
        appendJsonKey(json, HALatestVersionKey);
        appendJsonString(json, _latestVersion);
    }

    if (_releaseSummary) {
        json += ',';
        appendJsonKey(json, HAReleaseSummaryKey);
        appendJsonString(json, _releaseSummary);
    }

    if (_releaseUrl) {
        json += ',';
        appendJsonKey(json, HAReleaseUrlKey);
        appendJsonString(json, _releaseUrl);
    }

    json += ',';
    appendJsonKey(json, HAInProgressKey);
    json += _inProgress ? HATrue : HAFalse;

    // always present, so HA clears the previous value when the update finishes
    json += ',';
    appendJsonKey(json, HAUpdatePercentageKey);
    if (_progress >= 0) {
        json += std::to_string(_progress);
    } else {
        json += HANull;
    }

    json += '}';

    return publishOnDataTopic(HAStateTopic, json.c_str(), true);
}

bool HAUpdate::replaceString(char*& dst, const char* src)
{
    if (dst == nullptr && src == nullptr) {
        return false;
    }

    if (dst && src && strcmp(dst, src) == 0) {
        return false;
    }

    free(dst);
    dst = src ? strdup(src) : nullptr;
    return true;
}

#endif
