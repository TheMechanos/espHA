#include "HAMqtt.h"

#include <mqtt_client.h>
#include <esp_idf_version.h>
#include <esp_timer.h>
#include <cstring>

#include "HADevice.h"
#include "device-types/HABaseDeviceType.h"

static const char* DefaultDiscoveryPrefix = "homeassistant";
static const char* DefaultDataPrefix = "aha";

HAMqtt* HAMqtt::_instance = nullptr;

HAMqtt::HAMqtt(
    HADevice& device,
    uint8_t maxDevicesTypesNb
) :
    callback_loop(nullptr),
    ReconnectInterval(10000),
    _device(device),
    _messageCallback(nullptr),
    _connectedCallback(nullptr),
    _disconnectedCallback(nullptr),
    _stateChangedCallback(nullptr),
    _initialized(false),
    _discoveryPrefix(DefaultDiscoveryPrefix),
    _dataPrefix(DefaultDataPrefix),
    _username(nullptr),
    _password(nullptr),
    _lastConnectionActivity(0),
    _devicesTypesNb(0),
    _devicesTypesNb_toreg(0),
    _maxDevicesTypesNb(maxDevicesTypesNb),
    _devicesTypes(new HABaseDeviceType*[maxDevicesTypesNb]),
    _lastWillTopic(nullptr),
    _lastWillMessage(nullptr),
    _lastWillRetain(false),
    _currentState(StateDisconnected),
    _mqttClient(nullptr),
    _mqttConnected(false),
    _keepAlive(0),
    _bufferSize(0),
    _publishRetained(false)
{
    _instance = this;
}

HAMqtt::~HAMqtt()
{
    delete[] _devicesTypes;

    if (_mqttClient) {
        esp_mqtt_client_stop(_mqttClient);
        esp_mqtt_client_destroy(_mqttClient);
        _mqttClient = nullptr;
    }

    _instance = nullptr;
}

void HAMqtt::mqttEventHandler(void* handlerArgs, const char* base, int32_t eventId, void* eventData)
{
    (void)base;

    auto* self = static_cast<HAMqtt*>(handlerArgs);
    if (self == nullptr) {
        return;
    }

    switch (static_cast<esp_mqtt_event_id_t>(eventId)) {
    case MQTT_EVENT_CONNECTED:
        ESPHA_DEBUG_LOG("MQTT_EVENT_CONNECTED");
        self->_mqttConnected = true;
        break;

    case MQTT_EVENT_DISCONNECTED:
        ESPHA_DEBUG_LOG("MQTT_EVENT_DISCONNECTED");
        self->_mqttConnected = false;
        break;

    case MQTT_EVENT_DATA:
        self->handleIncomingMessage(eventData);
        break;

    default:
        break;
    }
}

void HAMqtt::handleIncomingMessage(void* eventData)
{
    auto event = static_cast<esp_mqtt_event_handle_t>(eventData);

    // A message bigger than the client's internal buffer arrives as several MQTT_EVENT_DATA
    // events. The topic is only present on the first fragment (current_data_offset == 0);
    // accumulate the payload until current_data_offset + data_len reaches total_data_len.
    if (event->current_data_offset == 0) {
        _incomingTopic.assign(event->topic, event->topic_len);
        _incomingPayload.clear();

        if (event->total_data_len > 0) {
            _incomingPayload.reserve(event->total_data_len);
        }
    }

    if (event->data_len > 0) {
        _incomingPayload.append(event->data, event->data_len);
    }

    const int received = event->current_data_offset + event->data_len;
    if (received < event->total_data_len) {
        return; // wait for the remaining fragments
    }

    if (_incomingPayload.length() > UINT16_MAX) {
        ESPHA_DEBUG_LOG("incoming message too long, dropping");
        return;
    }

    processMessage(
        _incomingTopic.c_str(),
        reinterpret_cast<const uint8_t*>(_incomingPayload.data()),
        static_cast<uint16_t>(_incomingPayload.length())
    );
}

bool HAMqtt::begin(
    const char* serverHostname,
    const uint16_t serverPort,
    const char* username,
    const char* password
)
{
    ESPHA_DEBUG_LOG("init server %s:%u", serverHostname, serverPort);

    if (_device.getUniqueId() == nullptr) {
        ESPHA_DEBUG_LOG("init failed: missing device unique ID");
        return false;
    }

    if (_initialized) {
        ESPHA_DEBUG_LOG("already initialized");
        return false;
    }

    _username = username;
    _password = password;

    esp_mqtt_client_config_t config = {};

#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 0, 0)
    config.broker.address.hostname = serverHostname;
    config.broker.address.port = serverPort;
    config.broker.address.transport = MQTT_TRANSPORT_OVER_TCP;
    config.credentials.client_id = _device.getUniqueId();

    if (username) {
        config.credentials.username = username;
    }

    if (password) {
        config.credentials.authentication.password = password;
    }

    if (_lastWillTopic) {
        config.session.last_will.topic = _lastWillTopic;
        config.session.last_will.msg = _lastWillMessage;
        config.session.last_will.msg_len = _lastWillMessage ? strlen(_lastWillMessage) : 0;
        config.session.last_will.retain = _lastWillRetain;
    }

    if (_keepAlive > 0) {
        config.session.keepalive = _keepAlive;
    }

    if (_bufferSize > 0) {
        config.buffer.size = _bufferSize;
        config.buffer.out_size = _bufferSize;
    }
#else
    config.host = serverHostname;
    config.port = serverPort;
    config.transport = MQTT_TRANSPORT_OVER_TCP;
    config.client_id = _device.getUniqueId();
    config.username = username;
    config.password = password;

    if (_lastWillTopic) {
        config.lwt_topic = _lastWillTopic;
        config.lwt_msg = _lastWillMessage;
        config.lwt_msg_len = _lastWillMessage ? strlen(_lastWillMessage) : 0;
        config.lwt_retain = _lastWillRetain;
    }

    if (_keepAlive > 0) {
        config.keepalive = _keepAlive;
    }

    if (_bufferSize > 0) {
        config.buffer_size = _bufferSize;
        config.out_buffer_size = _bufferSize;
    }
#endif

    _mqttClient = esp_mqtt_client_init(&config);
    if (_mqttClient == nullptr) {
        ESPHA_DEBUG_LOG("esp_mqtt_client_init failed");
        return false;
    }

    esp_mqtt_client_register_event(
        _mqttClient,
        MQTT_EVENT_ANY,
        &HAMqtt::mqttEventHandler,
        this
    );

    if (esp_mqtt_client_start(_mqttClient) != ESP_OK) {
        ESPHA_DEBUG_LOG("esp_mqtt_client_start failed");
        esp_mqtt_client_destroy(_mqttClient);
        _mqttClient = nullptr;
        return false;
    }

    _initialized = true;
    return true;
}

bool HAMqtt::begin(
    const char* serverHostname,
    const char* username,
    const char* password
)
{
    return begin(serverHostname, HAMQTT_DEFAULT_PORT, username, password);
}

bool HAMqtt::disconnect()
{
    if (!_initialized) {
        return false;
    }

    ESPHA_DEBUG_LOG("disconnecting");

    _initialized = false;
    _lastConnectionActivity = 0;

    if (_mqttClient) {
        esp_mqtt_client_stop(_mqttClient);
        esp_mqtt_client_destroy(_mqttClient);
        _mqttClient = nullptr;
    }

    _mqttConnected = false;
    setState(StateDisconnected);

    return true;
}

void HAMqtt::loop()
{
    if (!_initialized) {
        return;
    }

    // esp-mqtt reconnects on its own background FreeRTOS task once esp_mqtt_client_start()
    // was called from begin(). There's nothing to drive here besides noticing the
    // _mqttConnected transition (set from mqttEventHandler) and firing the callbacks.
    bool connected = _mqttConnected;
    ConnectionState newState = connected ? StateConnected : StateDisconnected;

    if (_currentState != newState) {
        if (newState == StateConnected) {
            _lastConnectionActivity = static_cast<uint32_t>(esp_timer_get_time() / 1000);
        }

        setState(newState);
    }
}

bool HAMqtt::isConnected() const
{
    return _mqttConnected;
}

void HAMqtt::setKeepAlive(uint16_t keepAlive)
{
    // Applied in begin(): must be called before it to take effect.
    _keepAlive = keepAlive;
}

bool HAMqtt::setBufferSize(uint16_t size)
{
    // Applied in begin(): must be called before it to take effect.
    _bufferSize = size;
    return true;
}

void HAMqtt::addDeviceType(HABaseDeviceType* deviceType)
{

    _devicesTypesNb_toreg++;

    if (_devicesTypesNb + 1 > _maxDevicesTypesNb) {
        return;
    }

    _devicesTypes[_devicesTypesNb++] = deviceType;
}

bool HAMqtt::publish(const char* topic, const char* payload, bool retained)
{
    if (!isConnected()) {
        return false;
    }

    ESPHA_DEBUG_LOG("publishing %s, len: %u", topic, static_cast<unsigned>(strlen(payload)));

    return esp_mqtt_client_publish(
        _mqttClient,
        topic,
        payload,
        static_cast<int>(strlen(payload)),
        0,
        retained ? 1 : 0
    ) != -1;
}

bool HAMqtt::beginPublish(
    const char* topic,
    uint16_t payloadLength,
    bool retained
)
{
#if ESPHA_DEBUG_LEV > 3
    ESPHA_DEBUG_LOG("begin publish %s, len: %u", topic, static_cast<unsigned>(payloadLength));
#endif

    if (!isConnected()) {
        return false;
    }

    // esp_mqtt_client_publish() takes the whole payload in one call, so buffer it here
    // and flush it in endPublish() instead of streaming it out byte by byte.
    _publishTopic = topic;
    _publishBuffer.clear();
    _publishBuffer.reserve(payloadLength);
    _publishRetained = retained;
    return true;
}

void HAMqtt::writePayload(const char* data, const uint16_t length)
{
    writePayload(reinterpret_cast<const uint8_t*>(data), length);
}

void HAMqtt::writePayload(const uint8_t* data, const uint16_t length)
{
    _publishBuffer.append(reinterpret_cast<const char*>(data), length);
}

void HAMqtt::writePayload(const char* src)
{
    _publishBuffer.append(src, strlen(src));
}

bool HAMqtt::endPublish()
{
    if (!isConnected()) {
        return false;
    }

    bool result = esp_mqtt_client_publish(
        _mqttClient,
        _publishTopic.c_str(),
        _publishBuffer.data(),
        static_cast<int>(_publishBuffer.length()),
        0,
        _publishRetained ? 1 : 0
    ) != -1;

    _publishBuffer.clear();
    return result;
}

bool HAMqtt::subscribe(const char* topic)
{
    ESPHA_DEBUG_LOG("subscribing %s", topic);

    return esp_mqtt_client_subscribe(_mqttClient, topic, 0) != -1;
}

void HAMqtt::processMessage(const char* topic, const uint8_t* payload, uint16_t length)
{
    ESPHA_DEBUG_LOG("received call %s, len: %u", topic, static_cast<unsigned>(length));

    if (_messageCallback) {
        _messageCallback(topic, payload, length);
    }

    for (uint8_t i = 0; i < _devicesTypesNb; i++) {
        _devicesTypes[i]->onMqttMessage(topic, payload, length);
    }
}

void HAMqtt::onConnectedLogic()
{
    ESPHA_DEBUG_LOG("onConnectedLogic");
    if (_connectedCallback) {
        _connectedCallback();
    }

    _device.publishAvailability();

    for (uint8_t i = 0; i < _devicesTypesNb; i++) {
        if(callback_loop)
            callback_loop(1);
        _devicesTypes[i]->onMqttConnected();
    }
}

int  HAMqtt::getState(void)
{   return _currentState;
}

void HAMqtt::setState(ConnectionState state)
{
    ConnectionState previousState = _currentState;
    _currentState = state;

    ESPHA_DEBUG_LOG(
        "MQTT state changed to %d, previous state: %d",
        static_cast<int>(_currentState),
        static_cast<int>(previousState)
    );

    if (_currentState == StateConnected) {
        ESPHA_DEBUG_LOG("MQTT connected");
        onConnectedLogic();
    } else if (previousState == StateConnected && _currentState != StateConnected) {
        ESPHA_DEBUG_LOG("MQTT disconnected");

        if (_disconnectedCallback) {
            _disconnectedCallback();
        }
    }

    if (_stateChangedCallback) {
        _stateChangedCallback(_currentState);
    }
}

void  HAMqtt::set_callback_loop( void (* _callback_loop)(int src))
{
    callback_loop = _callback_loop;
    // Not forwarded anywhere else: esp-mqtt runs its own non-blocking background task,
    // so there's no blocking call here that would need a watchdog-feeding hook.
}
