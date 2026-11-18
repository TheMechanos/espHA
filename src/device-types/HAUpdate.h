#pragma once

#include "HABaseDeviceType.h"

#ifndef CONFIG_ESPHA_DISABLE_UPDATE_FEATURE

#define HAUPDATE_CALLBACK(name) void (*name)(HAUpdate* sender)

/**
 * HAUpdate represents a firmware update entity in the Home Assistant panel.
 * It reports the installed and the latest available version, release information and
 * the progress of a running installation, and forwards the "install" command from HA
 * to the application.
 *
 * The library doesn't download or flash anything on its own. Discovering new releases
 * and performing the OTA is up to the application.
 *
 * @note All setters of this class and HAMqtt::loop() must be called from the same task.
 *       The class doesn't use any locking.
 * @note `update_percentage` requires Home Assistant 2024.11 or newer.
 * @note
 * You can find more information about this entity in the Home Assistant documentation:
 * https://www.home-assistant.io/integrations/update.mqtt/
 */
class HAUpdate : public HABaseDeviceType
{
public:
    /**
     * @param uniqueId The unique ID of the update entity. It needs to be unique in a scope of your device.
     */
    HAUpdate(const char* uniqueId);

    /**
     * Frees the strings copied by the state setters.
     */
    ~HAUpdate();

    /**
     * Sets the title of the software/firmware shown in HA (e.g. "Boiler controller firmware").
     * The pointer is stored, so the string must outlive the entity.
     *
     * @param title The title.
     */
    inline void setTitle(const char* title)
        { _title = title; }

    /**
     * Sets icon of the entity.
     * Any icon from MaterialDesignIcons.com (for example: `mdi:home`).
     *
     * @param icon The icon name.
     */
    inline void setIcon(const char* icon)
        { _icon = icon; }

    /**
     * Sets class of the device. The default is `firmware`, pass nullptr to omit it.
     * See: https://www.home-assistant.io/integrations/update/#device-classes
     *
     * @param deviceClass The class name.
     */
    inline void setDeviceClass(const char* deviceClass)
        { _class = deviceClass; }

    /**
     * Registers callback that will be called each time the "install" command is received from HA.
     * The install button is shown in HA only if the callback is registered before the MQTT
     * connection is acquired. Commands received while an installation is in progress are ignored.
     *
     * @warning The callback is executed in the esp-mqtt client task. Don't perform the OTA
     *          inside it - only signal your own task (task notification, queue, flag), otherwise
     *          the MQTT connection (keep-alive, progress publishing) is blocked for the whole update.
     * @param callback
     */
    inline void onInstall(HAUPDATE_CALLBACK(callback))
        { _installCallback = callback; }

    /**
     * Sets the currently installed version. The string is copied.
     * If it's never set, the version from esp_app_get_description() is reported.
     * The state is published only if the value changes.
     *
     * @param version The installed version. nullptr restores the default.
     * @returns `false` if the state couldn't be published (it's republished after reconnect).
     */
    bool setInstalledVersion(const char* version);

    /**
     * Sets the latest available release. All strings are copied and the state is published once.
     * The state is published only if any of the values changes.
     *
     * @param latestVersion The latest available version. nullptr means it's unknown.
     * @param summary Short release summary (HA limits it to 255 characters). It can be nullptr.
     * @param url URL of the release notes. It can be nullptr.
     * @returns `false` if the state couldn't be published (it's republished after reconnect).
     */
    bool setRelease(
        const char* latestVersion,
        const char* summary = nullptr,
        const char* url = nullptr
    );

    /**
     * Sets whether an installation is in progress.
     * Setting `false` also clears the progress percentage (e.g. after a failed update).
     * The state is published only if the value changes.
     *
     * @param inProgress
     * @returns `false` if the state couldn't be published.
     */
    bool setInProgress(bool inProgress);

    /**
     * Sets the progress of the running installation and marks it as in progress.
     * Values above 100 are clamped. The state is published only if the value changes.
     *
     * @param percentage The progress (0-100).
     * @returns `false` if the state couldn't be published.
     */
    bool setProgress(uint8_t percentage);

    /**
     * Returns the reported installed version (the one set via setInstalledVersion or
     * the version from esp_app_get_description()).
     */
    const char* getInstalledVersion() const;

    /**
     * Returns the latest version set via setRelease. It can be nullptr.
     */
    inline const char* getLatestVersion() const
        { return _latestVersion; }

    /**
     * Returns `true` if an installation is in progress.
     */
    inline bool isInProgress() const
        { return _inProgress; }

    /**
     * Returns the progress percentage or -1 if it's unknown.
     */
    inline int16_t getProgress() const
        { return _progress; }

protected:
    virtual void buildSerializer() override;
    virtual void onMqttConnected() override;
    virtual void onMqttMessage(
        const char* topic,
        const uint8_t* payload,
        const uint16_t length
    ) override;

private:
    /**
     * Publishes the whole state JSON on the state topic (retained).
     */
    bool publishState();

    /**
     * Replaces `dst` with a heap copy of `src` (nullptr allowed).
     * Returns `true` if the value has changed.
     */
    static bool replaceString(char*& dst, const char* src);

    /// The title of the software. It can be nullptr.
    const char* _title;

    /// The icon of the entity. It can be nullptr.
    const char* _icon;

    /// The device class. It can be nullptr.
    const char* _class;

    /// Owned copy of the installed version. nullptr means the app description version is used.
    char* _installedVersion;

    /// Owned copy of the latest version. It can be nullptr.
    char* _latestVersion;

    /// Owned copy of the release summary. It can be nullptr.
    char* _releaseSummary;

    /// Owned copy of the release notes URL. It can be nullptr.
    char* _releaseUrl;

    /// Specifies whether an installation is in progress.
    bool _inProgress;

    /// The progress percentage (0-100) or -1 if unknown.
    int16_t _progress;

    /// The callback that will be called when the install command is received.
    HAUPDATE_CALLBACK(_installCallback);
};

#endif
