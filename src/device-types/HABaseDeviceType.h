#pragma once

#include <cstdint>
#include "../espHADefines.h"

class HAMqtt;
class HASerializer;

class HABaseDeviceType
{
public:
    enum NumberPrecision {
        /// No digits after the decimal point.
        PrecisionP0 = 0,

        /// One digit after the decimal point.
        PrecisionP1,

        /// Two digits after the decimal point.
        PrecisionP2,

        /// Three digits after the decimal point.
        PrecisionP3
    };

    enum EntityCategory : uint8_t {
        /// Primary entity (no `entity_category` in the discovery payload).
        EntityCategoryNone = 0,

        /// Entity that changes the device's configuration.
        EntityCategoryConfig,

        /// Entity that exposes diagnostic information about the device.
        EntityCategoryDiagnostic
    };

    /**
     * Creates a new device type instance and registers it in the HAMqtt class.
     *
     * @param componentName The name of the Home Assistant component (e.g. `binary_sensor`).
     *                      You can find all available component names in the Home Assistant documentation.
     * @param uniqueId The unique ID of the device type. It needs to be unique in a scope of the HADevice.
     */
    HABaseDeviceType(
        const char* componentName,
        const char* uniqueId
    );

    /**
     * Returns unique ID of the device type.
     */
    inline const char* uniqueId() const
        { return _uniqueId; }

    /**
     * Returns component name defined by the device type.
     * It's used for the MQTT discovery topic.
     */
    inline const char* componentName() const
        { return _componentName; }

    /**
     * Returns `true` if the availability was configured for this device type.
     */
    inline bool isAvailabilityConfigured() const
        { return (_availability != AvailabilityDefault); }

    /**
     * Returns online state of the device type.
     */
    inline bool isOnline() const
        { return (_availability == AvailabilityOnline); }

    bool setUniqueIdStr(char* pstr);
    bool setNameUniqueIdStr(char* unic_prefix, const char* name, const char *uid);

    /**
     * Sets name of the device type that will be used as a label in the HA panel.
     * Keep the name short to save the resources.
     *
     * @param name The device type name.
     */
    inline void setName(const char* name)
        { _name = name; }

    /**
     * Returns name of the deviced type that was assigned via setName method.
     * It can be nullptr if there is no name assigned.
     */
    inline const char* getName() const
        { return _name; }

    /**
     * Sets object ID that will be used by HA to generate entity ID.
     * Keep the ID short to save the resources.
     *
     * @param objectId The object ID.
     */
    inline void setObjectId(const char* objectId)
        { _objectId = objectId; }

    /**
     * Returns the object ID that was set by setObjectId method.
     * It can be nullptr if there is no ID assigned.
     */
    inline const char* getObjectId() const
        { return _objectId; }

    /**
     * Sets the entity category used by HA to group the entity on the device page.
     * Note: HA accepts `EntityCategoryConfig` only for entities that control the device
     * (e.g. number, select, switch, button, text). Sensors should use `EntityCategoryDiagnostic`.
     *
     * @param category The entity category.
     */
    inline void setEntityCategory(const EntityCategory category)
        { _entityCategory = category; }

    /**
     * Returns the entity category that was set by setEntityCategory method.
     */
    inline EntityCategory getEntityCategory() const
        { return _entityCategory; }

    /**
     * Sets whether the entity should be enabled when it's first added to HA.
     * Disabled entities still exist in HA but they need to be enabled manually by the user.
     *
     * @param enabled Specifies whether the entity is enabled by default.
     */
    inline void setEnabledByDefault(bool enabled)
        { _enabledByDefault = enabled; }

    /**
     * Returns `true` if the entity is enabled by default (default behavior).
     */
    inline bool isEnabledByDefault() const
        { return _enabledByDefault; }

    /**
     * Sets availability of the device type.
     * Setting the initial availability enables availability reporting for this device type.
     * Please note that not all device types support this feature.
     * Follow HA documentation of a specific device type to get more information.
     *
     * @param online Specifies whether the device type is online.
     */
    virtual void setAvailability(bool online);

protected:
    /**
     * Returns instance of the HAMqtt class.
     */
    static HAMqtt* mqtt();

    /**
     * Subscribes to the given data topic.
     *
     * @param uniqueId THe unique ID of the device type assigned via the constructor.
     * @param topic Topic to subscribe.
     */
    static void subscribeTopic(
        const char* uniqueId,
        const char* topic
    );

    /**
     * This method should build serializer that will be used for publishing the configuration.
     * The serializer is built each time the MQTT connection is acquired.
     * Follow implementation of the existing device types to get better understanding of the logic.
     */
    virtual void buildSerializer() { };

    /**
     * This method is called each time the MQTT connection is acquired.
     * Each device type should publish its configuration and availability.
     * It can be also used for subscribing to MQTT topics.
     */
    virtual void onMqttConnected() = 0;

    /**
     * This method is called each time the device receives a MQTT message.
     * It can be any MQTT message so the method should always verify the topic.
     *
     * @param topic The topic on which the message was produced.
     * @param payload The payload of the message. It can be nullptr.
     * @param length The length of the payload.
     */
    virtual void onMqttMessage(
        const char* topic,
        const uint8_t* payload,
        const uint16_t length
    );

    /**
     * Returns dictionary string of the entity category or nullptr for EntityCategoryNone.
     */
    const char* entityCategoryStr() const;

    /**
     * Returns pointer to the `enabled_by_default` value or nullptr if the entity is enabled
     * by default, so the property is published only when it differs from the HA default.
     */
    inline const bool* disabledByDefaultValue() const
        { return _enabledByDefault ? nullptr : &_enabledByDefault; }

    /**
     * Destroys the existing serializer.
     */
    void destroySerializer();

    /**
     * Publishes configuration of this device type on the HA discovery topic.
     */
    void publishConfig();

    /**
     * Publishes current availability of the device type.
     * The message is only produced if the availability is configured for this device type.
     */
    void publishAvailability();

    /**
     * Publishes the given null-terminated string on the data topic.
     * Its length is computed with strlen(), so it must not contain embedded null bytes.
     *
     * @param topic The topic to publish on.
     * @param payload The message's payload.
     * @param retained Specifies whether the message should be retained.
     */
    bool publishOnDataTopic(
        const char* topic,
        const char* payload,
        bool retained = false
    );

    /**
     * Publishes the given data on the data topic.
     *
     * @param topic The topic to publish on.
     * @param payload The message's payload.
     * @param length The length of the payload.
     * @param retained Specifies whether the message should be retained.
     * @param isNullTerminated Specifies whether `payload` is a null-terminated string (its
     *                         length is then recomputed with strlen() instead of trusting
     *                         `length`, matching the writePayload(const char*) overload).
     */
    bool publishOnDataTopic(
        const char* topic,
        const uint8_t* payload,
        const uint16_t length,
        bool retained = false,
        bool isNullTerminated = false
    );

    /// The component name that was assigned via the constructor.
    const char* const _componentName;

    /// The unique ID that was assigned via the constructor.
    const char* _uniqueId;

    /// The name that was set using setName method. It can be nullptr.
    const char* _name;

    /// The object ID that was set using setObjectId method. It can be nullptr.
    const char* _objectId;

    /// The entity category that was set using setEntityCategory method.
    EntityCategory _entityCategory;

    /// Specifies whether the entity is enabled by default in HA.
    bool _enabledByDefault;

    /// HASerializer that belongs to this device type. It can be nullptr.
    HASerializer* _serializer;

private:
    enum Availability {
        AvailabilityDefault = 0,
        AvailabilityOnline,
        AvailabilityOffline
    };

    /// The current availability of this device type. AvailabilityDefault means that the initial availability was never set.
    Availability _availability;
    friend class HAMqtt;
};
