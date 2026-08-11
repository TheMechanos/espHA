#include "HADictionary.h"

// components
const char HAComponentBinarySensor[] = {"binary_sensor"};
const char HAComponentButton[] = {"button"};
const char HAComponentCamera[] = {"camera"};
const char HAComponentCover[] = {"cover"};
const char HAComponentDeviceTracker[] = {"device_tracker"};
const char HAComponentDeviceAutomation[] = {"device_automation"};
const char HAComponentLock[] = {"lock"};
const char HAComponentNumber[] = {"number"};
const char HAComponentSelect[] = {"select"};
const char HAComponentSensor[] = {"sensor"};
const char HAComponentSwitch[] = {"switch"};
const char HAComponentTag[] = {"tag"};
const char HAComponentScene[] = {"scene"};
const char HAComponentFan[] = {"fan"};
const char HAComponentLight[] = {"light"};
const char HAComponentClimate[] = {"climate"};
const char HAComponentText[] = {"text"};

// decorators
const char HASerializerSlash[] = {"/"};
const char HASerializerJsonDataPrefix[] = {"{"};
const char HASerializerJsonDataSuffix[] = {"}"};
const char HASerializerJsonPropertyPrefix[] = {"\""};
const char HASerializerJsonPropertySuffix[] = {"\":"};
const char HASerializerJsonEscapeChar[] = {"\""};
const char HASerializerJsonPropertiesSeparator[] = {","};
const char HASerializerJsonArrayPrefix[] = {"["};
const char HASerializerJsonArraySuffix[] = {"]"};
const char HASerializerUnderscore[] = {"_"};

// properties
const char HADeviceIdentifiersProperty[] = {"ids"};
const char HADeviceManufacturerProperty[] = {"mf"};
const char HADeviceModelProperty[] = {"mdl"};
const char HADeviceSoftwareVersionProperty[] = {"sw"};
const char HADeviceConfigurationUrlProperty[] = {"cu"};
const char HANameProperty[] = {"name"};
const char HAUniqueIdProperty[] = {"uniq_id"};
const char HAObjectIdProperty[] = {"obj_id"};
const char HADeviceProperty[] = {"dev"};
const char HADeviceClassProperty[] = {"dev_cla"};
const char HAStateClassProperty[] = {"stat_cla"};
const char HAIconProperty[] = {"ic"};
const char HARetainProperty[] = {"ret"};
const char HASourceTypeProperty[] = {"src_type"};
const char HAEncodingProperty[] = {"e"};
const char HAOptimisticProperty[] = {"opt"};
const char HAAutomationTypeProperty[] = {"atype"};
const char HATypeProperty[] = {"type"};
const char HASubtypeProperty[] = {"stype"};
const char HAForceUpdateProperty[] = {"frc_upd"};
const char HAUnitOfMeasurementProperty[] = {"unit_of_meas"};
const char HAValueTemplateProperty[] = {"val_tpl"};
const char HAOptionsProperty[] = {"options"};
const char HAMinProperty[] = {"min"};
const char HAMaxProperty[] = {"max"};
const char HAStepProperty[] = {"step"};
const char HAModeProperty[] = {"mode"};
const char HACommandTemplateProperty[] = {"cmd_tpl"};
const char HASpeedRangeMaxProperty[] = {"spd_rng_max"};
const char HASpeedRangeMinProperty[] = {"spd_rng_min"};
const char HABrightnessScaleProperty[] = {"bri_scl"};
const char HAMinMiredsProperty[] = {"min_mirs"};
const char HAMaxMiredsProperty[] = {"max_mirs"};
const char HATemperatureUnitProperty[] = {"temp_unit"};
const char HAMinTempProperty[] = {"min_temp"};
const char HAMaxTempProperty[] = {"max_temp"};
const char HATempStepProperty[] = {"temp_step"};
const char HAFanModesProperty[] = {"fan_modes"};
const char HASwingModesProperty[] = {"swing_modes"};
const char HAModesProperty[] = {"modes"};
const char HATemperatureCommandTemplateProperty[] = {"temp_cmd_tpl"};
const char HAPayloadOnProperty[] = {"pl_on"};
const char HAExpireAfterProperty[] = {"exp_aft"};

// topics
const char HAConfigTopic[] = {"config"};
const char HAAvailabilityTopic[] = {"avty_t"};
const char HATopic[] = {"t"};
const char HAStateTopic[] = {"stat_t"};
const char HACommandTopic[] = {"cmd_t"};
const char HAPositionTopic[] = {"pos_t"};
const char HAPercentageStateTopic[] = {"pct_stat_t"};
const char HAPercentageCommandTopic[] = {"pct_cmd_t"};
const char HABrightnessCommandTopic[] = {"bri_cmd_t"};
const char HABrightnessStateTopic[] = {"bri_stat_t"};
const char HAColorTemperatureCommandTopic[] = {"clr_temp_cmd_t"};
const char HAColorTemperatureStateTopic[] = {"clr_temp_stat_t"};
const char HACurrentTemperatureTopic[] = {"curr_temp_t"};
const char HAActionTopic[] = {"act_t"};
const char HAAuxCommandTopic[] = {"aux_cmd_t"};
const char HAAuxStateTopic[] = {"aux_stat_t"};
const char HAPowerCommandTopic[] = {"pow_cmd_t"};
const char HAFanModeCommandTopic[] = {"fan_mode_cmd_t"};
const char HAFanModeStateTopic[] = {"fan_mode_stat_t"};
const char HASwingModeCommandTopic[] = {"swing_mode_cmd_t"};
const char HASwingModeStateTopic[] = {"swing_mode_stat_t"};
const char HAModeCommandTopic[] = {"mode_cmd_t"};
const char HAModeStateTopic[] = {"mode_stat_t"};
const char HATemperatureCommandTopic[] = {"temp_cmd_t"};
const char HATemperatureStateTopic[] = {"temp_stat_t"};
const char HARGBCommandTopic[] = {"rgb_cmd_t"};
const char HARGBStateTopic[] = {"rgb_stat_t"};
const char HAJsonAttributesTopic[] = {"json_attr_t"};

// misc
const char HAOnline[] = {"online"};
const char HAOffline[] = {"offline"};
const char HAStateOn[] = {"ON"};
const char HAStateOff[] = {"OFF"};
const char HAStateLocked[] = {"LOCKED"};
const char HAStateUnlocked[] = {"UNLOCKED"};
const char HAStateNone[] = {"None"};
const char HATrue[] = {"true"};
const char HAFalse[] = {"false"};
const char HAHome[] = {"home"};
const char HANotHome[] = {"not_home"};
const char HATrigger[] = {"trigger"};
const char HAModeBox[] = {"box"};
const char HAModeSlider[] = {"slider"};

// covers
const char HAClosedState[] = {"closed"};
const char HAClosingState[] = {"closing"};
const char HAOpenState[] = {"open"};
const char HAOpeningState[] = {"opening"};
const char HAStoppedState[] = {"stopped"};

// commands
const char HAOpenCommand[] = {"OPEN"};
const char HACloseCommand[] = {"CLOSE"};
const char HAStopCommand[] = {"STOP"};
const char HALockCommand[] = {"LOCK"};
const char HAUnlockCommand[] = {"UNLOCK"};

// device tracker
const char HAGPSType[] = {"gps"};
const char HARouterType[] = {"router"};
const char HABluetoothType[] = {"bluetooth"};
const char HABluetoothLEType[] = {"bluetooth_le"};

// camera
const char HAEncodingBase64[] = {"b64"};

// trigger
const char HAButtonShortPressType[] = {"button_short_press"};
const char HAButtonShortReleaseType[] = {"button_short_release"};
const char HAButtonLongPressType[] = {"button_long_press"};
const char HAButtonLongReleaseType[] = {"button_long_release"};
const char HAButtonDoublePressType[] = {"button_double_press"};
const char HAButtonTriplePressType[] = {"button_triple_press"};
const char HAButtonQuadruplePressType[] = {"button_quadruple_press"};
const char HAButtonQuintuplePressType[] = {"button_quintuple_press"};
const char HATurnOnSubtype[] = {"turn_on"};
const char HATurnOffSubtype[] = {"turn_off"};
const char HAButton1Subtype[] = {"button_1"};
const char HAButton2Subtype[] = {"button_2"};
const char HAButton3Subtype[] = {"button_3"};
const char HAButton4Subtype[] = {"button_4"};
const char HAButton5Subtype[] = {"button_5"};
const char HAButton6Subtype[] = {"button_6"};

// actions
const char HAActionOff[] = {"off"};
const char HAActionHeating[] = {"heating"};
const char HAActionCooling[] = {"cooling"};
const char HAActionDrying[] = {"drying"};
const char HAActionIdle[] = {"idle"};
const char HAActionFan[] = {"fan"};

// fan modes
const char HAFanModeAuto[] = {"auto"};
const char HAFanModeLow[] = {"low"};
const char HAFanModeMedium[] = {"medium"};
const char HAFanModeHigh[] = {"high"};

// swing modes
const char HASwingModeOn[] = {"on"};
const char HASwingModeOff[] = {"off"};

// HVAC modes
const char HAModeAuto[] = {"auto"};
const char HAModeOff[] = {"off"};
const char HAModeCool[] = {"cool"};
const char HAModeHeat[] = {"heat"};
const char HAModeDry[] = {"dry"};
const char HAModeFanOnly[] = {"fan_only"};

// other
const char HAHexMap[] = {"0123456789abcdef"};

// value templates
const char HAValueTemplateFloatP1[] = {"{{int(float(value)*10**1)}}"};
const char HAValueTemplateFloatP2[] = {"{{int(float(value)*10**2)}}"};
const char HAValueTemplateFloatP3[] = {"{{int(float(value)*10**3)}}"};
const char HATemperatureUnitC[] = {"C"};
const char HATemperatureUnitF[] = {"F"};
