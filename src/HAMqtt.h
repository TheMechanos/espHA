#pragma once

#include <string>
#include <cstdint>
#include "espHADefines.h"

#define HAMQTT_CALLBACK(name) void (*name)()
#define HAMQTT_STATE_CALLBACK(name) void (*name)(ConnectionState state)
#define HAMQTT_MESSAGE_CALLBACK(name) void (*name)(const char* topic, const uint8_t* payload, uint16_t length)
#define HAMQTT_DEFAULT_PORT 1883

// Opaque type of the ESP-IDF "mqtt" component (mqtt_client.h). Kept forward-declared here
// so this header stays framework-agnostic and doesn't pull ESP-IDF headers into every
// translation unit that includes ArduinoHA. See HAMqtt.cpp for the real usage.
struct esp_mqtt_client;

#define HAMQTT_DEFAULT_DEVICES_LIMIT 24

class HADevice;
class HABaseDeviceType;

/**
 * This class is the central point of the library where instances of all device types are
 * stored, and the wrapper around the MQTT connection.
 *
 * It talks directly to the ESP-IDF "mqtt" component (esp_mqtt_client_*, from mqtt_client.h).
 * The MQTT client manages its own TCP connection and reconnects on its own background
 * FreeRTOS task, so this class doesn't need a Client&/WiFiClient at all.
 */
class HAMqtt
{
public:
    enum ConnectionState {
        StateConnecting = -5,
        StateConnectionTimeout = -4,
        StateConnectionLost = -3,
        StateConnectionFailed = -2,
        StateDisconnected = -1,
        StateConnected = 0,
        StateBadProtocol = 1,
        StateBadClientId = 2,
        StateUnavailable = 3,
        StateBadCredentials = 4,
        StateUnauthorized = 5
    };

    /**
     * Returns existing instance (singleton) of the HAMqtt class.
     * It may be a null pointer if the HAMqtt object was never constructed or it was destroyed.
     */
    inline static HAMqtt* instance()
        { return _instance; }

    /**
     * Creates a new instance of the HAMqtt class.
     * Please note that only one instance of the class can be initialized at the same time.
     *
     * @param device An instance of the HADevice class representing your device.
     * @param maxDevicesTypesNb The maximum number of device types (sensors, switches, etc.) that you're going to implement.
     */
    explicit HAMqtt(
        HADevice& device,
        const uint8_t maxDevicesTypesNb = HAMQTT_DEFAULT_DEVICES_LIMIT
    );

    /**
     * Removes singleton of the HAMqtt class.
     */
    ~HAMqtt();

    /**
     * Sets the prefix of the Home Assistant discovery topics.
     * It needs to match the prefix set in the HA admin panel.
     * The default prefix is "homeassistant".
     *
     * @param prefix The discovery topics' prefix.
     */
    inline void setDiscoveryPrefix(const char* prefix)
        { _discoveryPrefix = prefix; }

    /**
     * Returns the discovery topics' prefix.
     */
    inline const char* getDiscoveryPrefix() const
        { return _discoveryPrefix; }

    /**
     * Sets prefix of the data topics.
     * It may be useful if you want to pass MQTT traffic through a bridge.
     * The default prefix is "aha".
     *
     * @param prefix The data topics' prefix.
     */
    inline void setDataPrefix(const char* prefix)
        { _dataPrefix = prefix; }

    /**
     * Returns the data topics' prefix.
     */
    inline const char* getDataPrefix() const
        { return _dataPrefix; }

    /**
     * Returns instance of the device assigned to the HAMqtt class.
     * It's the same object (pointer) that was passed to the HAMqtt constructor.
     */
    inline HADevice const* getDevice() const
        { return &_device; }

    /**
     * Registers a new callback method that will be called when the device receives an MQTT message.
     * Please note that the callback is also fired by internal MQTT messages used by the library.
     * You should always verify the topic of the received message.
     *
     * @param callback Callback method.
     */
    inline void onMessage(HAMQTT_MESSAGE_CALLBACK(callback))
        { _messageCallback = callback; }

    /**
     * Registers a new callback method that will be called each time a connection to the MQTT broker is acquired.
     * The callback is also fired after reconnecting to the broker.
     * You can use this method to register topics' subscriptions.
     *
     * @param callback Callback method.
     */
    inline void onConnected(HAMQTT_CALLBACK(callback))
        { _connectedCallback = callback; }

    /**
     * Registers a new callback method that will be called each time the connection to the MQTT broker is lost.
     *
     * @param callback Callback method.
     */
    inline void onDisconnected(HAMQTT_CALLBACK(callback))
        { _disconnectedCallback = callback; }

    /**
     * Registers a new callback method that will be called each time the connection state changes.
     *
     * @param callback Callback method.
     */
    inline void onStateChanged(HAMQTT_STATE_CALLBACK(callback))
        { _stateChangedCallback = callback; }

    /**
     * Returns the current state of the MQTT connection.
     */
    inline ConnectionState getState() const
        { return _currentState; }

    /**
     * Sets parameters of the MQTT connection using the hostname and port.
     * The library will try to connect to the broker in first loop cycle.
     * Please note that the library automatically reconnects to the broker if connection is lost.
     *
     * @param serverHostname Hostname of the MQTT broker.
     * @param serverPort Port of the MQTT broker.
     * @param username Username for authentication. It can be nullptr if the anonymous connection needs to be performed.
     * @param password Password for authentication. It can be nullptr if the anonymous connection needs to be performed.
     */
    bool begin(
        const char* serverHostname,
        const uint16_t serverPort = HAMQTT_DEFAULT_PORT,
        const char* username = nullptr,
        const char* password = nullptr
    );

    /**
     * Sets parameters of the MQTT connection using the hostname and the default port (1883).
     * The library will try to connect to the broker in first loop cycle.
     * Please note that the library automatically reconnects to the broker if connection is lost.
     *
     * @param serverHostname Hostname of the MQTT broker.
     * @param username Username for authentication. It can be nullptr if the anonymous connection needs to be performed.
     * @param password Password for authentication. It can be nullptr if the anonymous connection needs to be performed.
     */
    bool begin(
        const char* serverHostname,
        const char* username,
        const char* password
    );

    /**
     * Disconnects from the MQTT broker and releases the underlying esp-mqtt client.
     * A subsequent call to HAMqtt::begin() will create a fresh client.
     */
    bool disconnect();

    /**
     * Should be called in the sketch loop.
     * It detects MQTT connection state changes and fires the connected/disconnected callbacks.
     * The actual network I/O runs on esp-mqtt's own background task, so this is lightweight.
     */
    void loop();

    /**
     * Returns `true` if the connection with the MQTT broker is acquired.
     */
    bool isConnected() const;

    /**
     * Sets the keep alive time of the MQTT connection.
     * By default it's 15 seconds.
     *
     * @note Must be called before HAMqtt::begin() to take effect - it's applied when the
     *       underlying esp-mqtt client is created.
     * @param keepAlive Time in seconds.
     */
    void setKeepAlive(uint16_t keepAlive);

    /**
     * Sets the buffer size for the MQTT connection.
     * By default it's the esp-mqtt default (1024 bytes).
     *
     * @note Must be called before HAMqtt::begin() to take effect, same as setKeepAlive().
     * @param size Size of the buffer.
     */
    bool setBufferSize(uint16_t size);

    /**
     * Adds a new device's type to the MQTT.
     * Each time the connection with MQTT broker is acquired, the HAMqtt class
     * calls "onMqttConnected" method in all devices' types instances.
     *
     * @note The HAMqtt class doesn't take ownership of the given pointer.
     * @param deviceType Instance of the device's type (HASwitch, HABinarySensor, etc.).
     */
    void addDeviceType(HABaseDeviceType* deviceType);

    /**
     * Publishes the MQTT message with given topic and payload.
     * Message won't be published if the connection with the MQTT broker is not established.
     * In this case method returns false.
     *
     * @param topic The topic to publish.
     * @param payload The payload to publish (it may be empty const char).
     * @param retained Specifies whether message should be retained.
     */
    bool publish(const char* topic, const char* payload, bool retained = false);

    /**
     * Begins publishing of a message with the given properties.
     * When this method returns true the payload can be written using HAMqtt::writePayload method.
     *
     * @param topic Topic of the published message.
     * @param payloadLength Length of the payload (bytes) that's going to be published.
     * @param retained Specifies whether the published message should be retained.
     */
    bool beginPublish(const char* topic, uint16_t payloadLength, bool retained = false);

    /**
     * Writes given string to the outgoing message buffer.
     * Please note that before writing any data the HAMqtt::beginPublish method
     * needs to be called.
     *
     * @param data The string to publish.
     * @param length Length of the data (bytes).
     */
    void writePayload(const char* data, const uint16_t length);

    /**
     * Writes given data to the outgoing message buffer.
     * Please note that before writing any data the HAMqtt::beginPublish method
     * needs to be called.
     *
     * @param data The data to publish.
     * @param length Length of the data (bytes).
     */
    void writePayload(const uint8_t* data, const uint16_t length);

    /**
     * Writes the given null-terminated string to the outgoing message buffer.
     * Its length is computed with strlen(), so it must not contain embedded null bytes.
     * Please note that before writing any data the HAMqtt::beginPublish method
     * needs to be called.
     *
     * @param data The null-terminated string to publish.
     */
    void writePayload(const char* data);

    /**
     * Finishes publishing of a message.
     * After calling this method the message will be processed by the broker.
     */
    bool endPublish();

    /**
     * Subscribes to the given topic.
     * Whenever a new message is received the onMqttMessage callback in all
     * devices types is called.
     *
     * Please note that you need to subscribe topic each time the connection
     * with the broker is acquired.
     *
     * @param topic Topic to subscribe.
     */
    bool subscribe(const char* topic);

    /**
     * Enables the last will message that will be produced when the device disconnects from the broker.
     * If you want to change availability of the device in Home Assistant panel
     * please use enableLastWill() method from the HADevice class instead.
     *
     * @param lastWillTopic The topic to publish.
     * @param lastWillMessage The message (payload) to publish.
     * @param lastWillRetain Specifies whether the published message should be retained.
     */
    inline void setLastWill(
        const char* lastWillTopic,
        const char* lastWillMessage,
        bool lastWillRetain
    ) {
        _lastWillTopic = lastWillTopic;
        _lastWillMessage = lastWillMessage;
        _lastWillRetain = lastWillRetain;
    }

    /**
     * Processes MQTT message received from the broker (subscription).
     *
     * @note Do not use this method on your own. It's only for the internal purpose.
     * @param topic Topic of the message.
     * @param payload Content of the message.
     * @param length Length of the message.
     */
    void processMessage(const char* topic, const uint8_t* payload, uint16_t length);

    inline uint8_t getDevicesTypesNb() const
        { return _devicesTypesNb; }

    inline uint8_t getDevicesTypesNb_toreg() const
        { return _devicesTypesNb_toreg; }

    inline HABaseDeviceType** getDevicesTypes() const
        { return _devicesTypes; }

    void  (* callback_loop)(int src); //  callback for long time spending in mqqt
    void  set_callback_loop( void (* _callback_loop)(int src)); 

    /// Interval between MQTT reconnects (milliseconds).
    /// @note Kept for source compatibility. esp-mqtt reconnects on its own background task,
    ///       so this value is currently unused.
    uint16_t ReconnectInterval;
///Get the state of the MQTT connection.
    int  getState(void); 

private:

    /// Living instance of the HAMqtt class. It can be nullptr.
    static HAMqtt* _instance;

    /**
     * This method is called each time the connection with MQTT broker is acquired.
     */
    void onConnectedLogic();

    /**
     * Sets the state of the MQTT connection.
     */
    void setState(ConnectionState state);

    /**
     * Raw esp-mqtt event handler, registered via esp_mqtt_client_register_event().
     * Declared with plain C types (void* /const char* /int32_t) instead of the esp-mqtt
     * typedefs so this header doesn't need to include any ESP-IDF headers - esp_event_base_t
     * is itself just a `const char*` alias, so the signature is binary-identical to what
     * esp_mqtt_client_register_event() expects. The cast to the real event types happens
     * in HAMqtt.cpp, which does include mqtt_client.h.
     */
    static void mqttEventHandler(void* handlerArgs, const char* base, int32_t eventId, void* eventData);

    /// Handles a MQTT_EVENT_DATA event (possibly one of several fragments of one message).
    void handleIncomingMessage(void* eventData);

    /// Instance of the HADevice passed to the constructor.
    const HADevice& _device;

    /// The callback method that will be called when an MQTT message is received.
    HAMQTT_MESSAGE_CALLBACK(_messageCallback);

    /// The callback method that will be called when the MQTT connection is acquired.
    HAMQTT_CALLBACK(_connectedCallback);

    /// The callback method that will be called when the MQTT connection is lost.
    HAMQTT_CALLBACK(_disconnectedCallback);

    /// The callback method that will be called when the MQTT connection state changes.
    HAMQTT_STATE_CALLBACK(_stateChangedCallback);

    /// Specifies whether the HAMqtt::begin method was ever called.
    bool _initialized;

    /// Teh discovery prefix that's used for the configuration messages.
    const char* _discoveryPrefix;

    /// The data prefix that's used for publishing data messages.
    const char* _dataPrefix;

    /// The username used for the authentication. It's set in the HAMqtt::begin method.
    const char* _username;

    /// The password used for the authentication. It's set in the HAMqtt::begin method.
    const char* _password;

    /// Time of the last connection activity (milliseconds since boot).
    uint32_t _lastConnectionActivity;

    /// The amount of registered devices types.
    uint8_t _devicesTypesNb;
    /// The amount of devices types to register
    uint8_t _devicesTypesNb_toreg;

    /// The maximum amount of devices types that can be registered.
    uint8_t _maxDevicesTypesNb;

    /// Pointers of all registered devices types (array of pointers).
    HABaseDeviceType** _devicesTypes;

    /// The last will topic set by HAMqtt::setLastWill
    const char* _lastWillTopic;

    /// The last will message set by HAMqtt::setLastWill
    const char* _lastWillMessage;

    /// The last will retain set by HAMqtt::setLastWill
    bool _lastWillRetain;

    /// The last known state of the MQTT connection.
    ConnectionState _currentState;

    /// Handle of the underlying esp-mqtt client. Created in begin(), destroyed in
    /// disconnect() and in the destructor.
    struct esp_mqtt_client* _mqttClient;

    /// Set from the MQTT event task (mqttEventHandler), read from the caller's task (loop()).
    volatile bool _mqttConnected;

    /// Keep-alive time (seconds) requested via setKeepAlive(), applied in begin().
    uint16_t _keepAlive;

    /// Buffer size requested via setBufferSize(), applied in begin().
    uint16_t _bufferSize;

    /// Topic of the message currently being assembled via beginPublish/writePayload/endPublish.
    /// esp_mqtt_client_publish() takes the whole payload at once (no streaming write API), so
    /// beginPublish/writePayload just buffer the payload here and endPublish flushes it in one call.
    std::string _publishTopic;

    /// Payload buffer used by beginPublish/writePayload/endPublish. See _publishTopic.
    std::string _publishBuffer;

    /// Retained flag captured by beginPublish, used by endPublish.
    bool _publishRetained;

    /// Topic of the message currently being reassembled from one or more MQTT_EVENT_DATA
    /// fragments (esp-mqtt splits messages larger than the buffer size across several events).
    std::string _incomingTopic;

    /// Payload bytes received so far for the message in _incomingTopic.
    std::string _incomingPayload;
};
