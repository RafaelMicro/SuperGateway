
#ifndef __CLUSTER_MACRO_H__
#define __CLUSTER_MACRO_H__
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/util/af-types.h>
#include <app/util/af.h>
#include <app/util/attribute-storage.h>
#include <app/util/util.h>
#include <lib/support/ZclString.h>

#define ZAP_GET_CLUSTER_VERSION(cluster) ZCL_CLUSTER_#cluster
#define ZAP_DEVICE_TYPE(type) {type, DEVICE_VERSION_DEFAULT}
#define ZAP_MASK 0 | ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE)
#define ZAP_EMPTY ZAP_EMPTY_DEFAULT()

#define ZAP_CHAR_STRING ZAP_TYPE(CHAR_STRING)
#define ZAP_ARRAY ZAP_TYPE(ARRAY)
#define ZAP_BOOL  ZAP_TYPE(BOOLEAN)
#define ZAP_INT8  ZAP_TYPE(INT8S)
#define ZAP_INT16 ZAP_TYPE(INT16S)
#define ZAP_INT24 ZAP_TYPE(INT24S)
#define ZAP_INT32 ZAP_TYPE(INT32S)
#define ZAP_BITMAP8  ZAP_TYPE(BITMAP8)
#define ZAP_BITMAP16 ZAP_TYPE(BITMAP16)
#define ZAP_BITMAP24 ZAP_TYPE(BITMAP24)
#define ZAP_BITMAP32 ZAP_TYPE(BITMAP32)
#define ZAP_ENUM8 ZAP_TYPE(ENUM8)
#define ZAP_ENUM16 ZAP_TYPE(ENUM16)
#define ZAP_ENUM24 ZAP_TYPE(ENUM24)
#define ZAP_ENUM32 ZAP_TYPE(ENUM32)

#define ZAP_MAX_ARRAY_SIZE  254
#define ZAP_MAX_LABLE_SIZE  32
#define DEVICE_VERSION_DEFAULT 1

#define ZAP_END_ELEMENT { ZAP_EMPTY, 0xFFFD, 2, ZAP_INT16, ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE) }
#define ZAP_RECORD(id, val, type) { ZAP_EMPTY, id, val, type, ZAP_MASK }
#define ZAP_BASIC(featuremap, revision) \
        ZAP_RECORD(chip::app::Clusters::Globals::Attributes::GeneratedCommandList::Id, ZAP_MAX_ARRAY_SIZE, ZAP_ARRAY), \
        ZAP_RECORD(chip::app::Clusters::Globals::Attributes::AcceptedCommandList::Id, ZAP_MAX_ARRAY_SIZE, ZAP_ARRAY), \
        ZAP_RECORD(chip::app::Clusters::Globals::Attributes::EventList::Id, ZAP_MAX_ARRAY_SIZE, ZAP_ARRAY), \
        ZAP_RECORD(chip::app::Clusters::Globals::Attributes::AttributeList::Id, ZAP_MAX_ARRAY_SIZE, ZAP_ARRAY), \
        ZAP_RECORD(chip::app::Clusters::Globals::Attributes::FeatureMap::Id, featuremap, ZAP_BITMAP32), \
        ZAP_RECORD(chip::app::Clusters::Globals::Attributes::ClusterRevision::Id, revision, ZAP_INT16)


#define DEFAULT_ONOFF_LIGHT_NAME             "RB_ONOFF_LIGHT_"
#define DEFAULT_DIMMABLE_LIGHT_NAME          "RB_DIMMABLE_LIGHT_"
#define DEFAULT_ONOFF_LIGHT_SWITCH_NAME      "RB_ONOFF_LIGHT_SWITCH_"
#define DEFAULT_CONTACT_SENSOR_NAME          "RB_CONTACT_SENSOR_"
#define DEFAULT_COLOR_TEMPERATURE_LIGHT_NAME "RB_COLOR_TEMPERATURE_LIGHT_"
#define DEFAULT_NAME "RF_ROOM"


#define DEVICE_TYPE_BRIDGED_NODE  (0x0013)

#define DEVICE_TYPE_ON_OFF_LIGHT            (0x0100)
#define DEVICE_TYPE_DIMABLE_LIGHT           (0x0101)
#define DEVICE_TYPE_COLOR_TEMPERATURE_LIGHT (0x010C)
#define DEVICE_TYPE_EXTENDED_COLOR_LIGHT    (0x010D)

#define DEVICE_TYPE_ON_OFF_PLUG_IN_UINT  (0x010A)
#define DEVICE_TYPE_DIMABLE_PLUG_IN_UINT (0x010A)
#define DEVICE_TYPE_PUMP                 (0x0303)

#define DEVICE_TYPE_ON_OFF_LIGHT_SWITCH (0x0103)
#define DEVICE_TYPE_DIMMER_SWITCH       (0x0104)
#define DEVICE_TYPE_COLOR_DIMMER_SWITCH (0x0105)
#define DEVICE_TYPE_CONTROL_BRIDGE      (0x0840)
#define DEVICE_TYPE_PUMP_CONTROLLER     (0x0304)
#define DEVICE_TYPE_GENERIC_SWITCH      (0x000F)

#define DEVICE_TYPE_CONTACT_SENSOR     (0x0015)
#define DEVICE_TYPE_LIGHT_SENSOR       (0x0106)
#define DEVICE_TYPE_OCCUPANCY_SENSOR   (0x0107)
#define DEVICE_TYPE_TEMPERATURE_SENSOR (0x0302)
#define DEVICE_TYPE_PRESSURE_SENSOR    (0x0305)
#define DEVICE_TYPE_FLOW_SENSOR        (0x0306)
#define DEVICE_TYPE_HUMIDITY_SENSOR    (0x0307)
#define DEVICE_TYPE_ON_OFF_SENSOR      (0x0850)
#define DEVICE_TYPE_SMOKE_CO_ALARM     (0x0076)

#define DEVICE_TYPE_DOOR_LOCK                  (0x000A)
#define DEVICE_TYPE_DOOR_LOCK_CONTROLLER       (0x000B)
#define DEVICE_TYPE_WINDOW_COVERING            (0x0202)
#define DEVICE_TYPE_WINDOW_COVERING_CONTROLLER (0x0203)

#define DEVICE_TYPE_HEATING_COOLING_UINT (0x0300)
#define DEVICE_TYPE_THERMOSTAT           (0x0301)
#define DEVICE_TYPE_FAN                  (0x002B)
#define DEVICE_TYPE_AIR_PURIFIER         (0x002D)
#define DEVICE_TYPE_AIR_QUALITY_SENSOR   (0x002C)

#define DEVICE_TYPE_BASIC_VIDEO_PLAYER   (0x0028)
#define DEVICE_TYPE_CASTIN_VIDEO_PLAYER  (0x0023)
#define DEVICE_TYPE_SPEAKER              (0x0022)
#define DEVICE_TYPE_CONTENT_APP          (0x0024)
#define DEVICE_TYPE_CASTIN_VIDEO_CLIENT  (0x0029)
#define DEVICE_TYPE_VIDEO_REMOTE_CONTROL (0x002A)

#define DEVICE_TYPE_MODE_SELECT (0x0027)

#define DEVICE_TYPE_ROBOTIC_VACUUM_CLEANER (0x0074)

#define DEVICE_TYPE_REFRIGERATOR                   (0x0070)
#define DEVICE_TYPE_TEMPERATURE_CONTROLLED_CABINET (0x0071)
#define DEVICE_TYPE_ROOM_AIR_CONDITIONER           (0x0072)
#define DEVICE_TYPE_LAUNDRY_WASHHER                (0x0073)
#define DEVICE_TYPE_DISHWASHER                     (0x0075)

#define DEVICE_TYPE_POWER_SOURCE  (0x0011)
#define DEVICE_TYPE_TEMP_SENSOR   (0x0302)

// =====================
// ====== General ======
// =====================
#define ZCL_CLUSTER_REVISION_IDENTIFY (4u)
#define ZCL_CLUSTER_REVISION_GROUPS (4u)
#define ZCL_CLUSTER_REVISION_SCENES (5u)
#define ZCL_CLUSTER_REVISION_ON_OFF (5u)
#define ZCL_CLUSTER_REVISION_LEVEL_CONTROL (5u)
#define ZCL_CLUSTER_REVISION_BOOLEAN_STATE (1u)
#define ZCL_CLUSTER_REVISION_MODE_SELECT (2u)
#define ZCL_CLUSTER_REVISION_LOW_POWER (1u)
#define ZCL_CLUSTER_REVISION_WAKE_ON_LAN (1u)
#define ZCL_CLUSTER_REVISION_SWITCH (1u)
#define ZCL_CLUSTER_REVISION_OPERATIONAL_STATE (1u)

// =====================
// ===== Lighting ======
// =====================
#define ZCL_CLUSTER_REVISION_COLOR_CONTROL (6u)
#define ZCL_CLUSTER_REVISION_BALLAST_CONFIGURATION (4u)


#define ZCL_CLUSTER_REVISION_ON_OFF_SWITCH_CONFIGURATION (1u)
#define ZCL_CLUSTER_REVISION_BINARY_INPUT_BASIC (1u)
#define ZCL_CLUSTER_REVISION_PULSE_WIDTH_MODULATION (1u)
#define ZCL_CLUSTER_REVISION_DESCRIPTOR (1u)
#define ZCL_CLUSTER_REVISION_BINDING (1u)
#define ZCL_CLUSTER_REVISION_ACCESS_CONTROL (1u)
#define ZCL_CLUSTER_REVISION_ACTIONS (1u)
#define ZCL_CLUSTER_REVISION_BASIC_INFORMATION (1u)
#define ZCL_CLUSTER_REVISION_OTA_SOFTWARE_UPDATE_PROVIDER (1u)
#define ZCL_CLUSTER_REVISION_OTA_SOFTWARE_UPDATE_REQUESTOR (1u)
#define ZCL_CLUSTER_REVISION_LOCALIZATION_CONFIGURATION (1u)
#define ZCL_CLUSTER_REVISION_TIME_FORMAT_LOCALIZATION (1u)
#define ZCL_CLUSTER_REVISION_UNIT_LOCALIZATION (1u)
#define ZCL_CLUSTER_REVISION_POWER_SOURCE_CONFIGURATION (1u)
#define ZCL_CLUSTER_REVISION_POWER_SOURCE (1u)
#define ZCL_CLUSTER_REVISION_GENERAL_COMMISSIONING (1u)
#define ZCL_CLUSTER_REVISION_NETWORK_COMMISSIONING (1u)
#define ZCL_CLUSTER_REVISION_DIAGNOSTIC_LOGS (1u)
#define ZCL_CLUSTER_REVISION_GENERAL_DIAGNOSTICS (1u)
#define ZCL_CLUSTER_REVISION_SOFTWARE_DIAGNOSTICS (1u)
#define ZCL_CLUSTER_REVISION_THREAD_NETWORK_DIAGNOSTICS (1u)
#define ZCL_CLUSTER_REVISION_WI_FI_NETWORK_DIAGNOSTICS (1u)
#define ZCL_CLUSTER_REVISION_ETHERNET_NETWORK_DIAGNOSTICS (1u)
#define ZCL_CLUSTER_REVISION_TIME_SYNCHRONIZATION (1u)
#define ZCL_CLUSTER_REVISION_BRIDGED_DEVICE_BASIC_INFORMATION (1u)
#define ZCL_CLUSTER_REVISION_ADMINISTRATOR_COMMISSIONING (1u)
#define ZCL_CLUSTER_REVISION_OPERATIONAL_CREDENTIALS (1u)
#define ZCL_CLUSTER_REVISION_GROUP_KEY_MANAGEMENT (1u)
#define ZCL_CLUSTER_REVISION_FIXED_LABEL (1u)
#define ZCL_CLUSTER_REVISION_USER_LABEL (1u)
#define ZCL_CLUSTER_REVISION_PROXY_CONFIGURATION (1u)
#define ZCL_CLUSTER_REVISION_PROXY_DISCOVERY (1u)
#define ZCL_CLUSTER_REVISION_PROXY_VALID (1u)
#define ZCL_CLUSTER_REVISION_ICD_MANAGEMENT (1u)
#define ZCL_CLUSTER_REVISION_LAUNDRY_WASHER_MODE (1u)
#define ZCL_CLUSTER_REVISION_REFRIGERATOR_AND_TEMPERATURE_CONTROLLED_CABINET_MODE (1u)
#define ZCL_CLUSTER_REVISION_LAUNDRY_WASHER_CONTROLS (1u)
#define ZCL_CLUSTER_REVISION_RVC_RUN_MODE (1u)
#define ZCL_CLUSTER_REVISION_RVC_CLEAN_MODE (1u)
#define ZCL_CLUSTER_REVISION_TEMPERATURE_CONTROL (1u)
#define ZCL_CLUSTER_REVISION_REFRIGERATOR_ALARM (1u)
#define ZCL_CLUSTER_REVISION_DISHWASHER_MODE (1u)
#define ZCL_CLUSTER_REVISION_AIR_QUALITY (1u)
#define ZCL_CLUSTER_REVISION_SMOKE_CO_ALARM (1u)
#define ZCL_CLUSTER_REVISION_DISHWASHER_ALARM (1u)
#define ZCL_CLUSTER_REVISION_RVC_OPERATIONAL_STATE (1u)
#define ZCL_CLUSTER_REVISION_HEPA_FILTER_MONITORING (1u)
#define ZCL_CLUSTER_REVISION_ACTIVATED_CARBON_FILTER_MONITORING (1u)
#define ZCL_CLUSTER_REVISION_DOOR_LOCK (1u)
#define ZCL_CLUSTER_REVISION_WINDOW_COVERING (1u)
#define ZCL_CLUSTER_REVISION_BARRIER_CONTROL (1u)
#define ZCL_CLUSTER_REVISION_PUMP_CONFIGURATION_AND_CONTROL (1u)
#define ZCL_CLUSTER_REVISION_THERMOSTAT (1u)
#define ZCL_CLUSTER_REVISION_FAN_CONTROL (1u)
#define ZCL_CLUSTER_REVISION_THERMOSTAT_USER_INTERFACE_CONFIGURATION (1u)
#define ZCL_CLUSTER_REVISION_ILLUMINANCE_MEASUREMENT (1u)
#define ZCL_CLUSTER_REVISION_TEMPERATURE_MEASUREMENT (1u)
#define ZCL_CLUSTER_REVISION_PRESSURE_MEASUREMENT (1u)
#define ZCL_CLUSTER_REVISION_FLOW_MEASUREMENT (1u)
#define ZCL_CLUSTER_REVISION_RELATIVE_HUMIDITY_MEASUREMENT (1u)
#define ZCL_CLUSTER_REVISION_OCCUPANCY_SENSING (1u)
#define ZCL_CLUSTER_REVISION_CARBON_MONOXIDE_CONCENTRATION_MEASUREMENT (1u)
#define ZCL_CLUSTER_REVISION_CARBON_DIOXIDE_CONCENTRATION_MEASUREMENT (1u)
#define ZCL_CLUSTER_REVISION_NITROGEN_DIOXIDE_CONCENTRATION_MEASUREMENT (1u)
#define ZCL_CLUSTER_REVISION_OZONE_CONCENTRATION_MEASUREMENT (1u)
#define ZCL_CLUSTER_REVISION_PM25_CONCENTRATION_MEASUREMENT (1u)
#define ZCL_CLUSTER_REVISION_FORMALDEHYDE_CONCENTRATION_MEASUREMENT (1u)
#define ZCL_CLUSTER_REVISION_PM1_CONCENTRATION_MEASUREMENT (1u)
#define ZCL_CLUSTER_REVISION_PM10_CONCENTRATION_MEASUREMENT (1u)
#define ZCL_CLUSTER_REVISION_TOTAL_VOLATILE_ORGANIC_COMPOUNDS_CONCENTRATION_MEASUREMENT (1u)
#define ZCL_CLUSTER_REVISION_RADON_CONCENTRATION_MEASUREMENT (1u)
#define ZCL_CLUSTER_REVISION_CHANNEL (1u)
#define ZCL_CLUSTER_REVISION_TARGET_NAVIGATOR (1u)
#define ZCL_CLUSTER_REVISION_MEDIA_PLAYBACK (1u)
#define ZCL_CLUSTER_REVISION_MEDIA_INPUT (1u)
#define ZCL_CLUSTER_REVISION_KEYPAD_INPUT (1u)
#define ZCL_CLUSTER_REVISION_CONTENT_LAUNCHER (1u)
#define ZCL_CLUSTER_REVISION_AUDIO_OUTPUT (1u)
#define ZCL_CLUSTER_REVISION_APPLICATION_LAUNCHER (1u)
#define ZCL_CLUSTER_REVISION_APPLICATION_BASIC (1u)
#define ZCL_CLUSTER_REVISION_ACCOUNT_LOGIN (1u)
#define ZCL_CLUSTER_REVISION_ELECTRICAL_MEASUREMENT (1u)
#define ZCL_CLUSTER_REVISION_UNIT_TESTING (1u)
#define ZCL_CLUSTER_REVISION_FAULT_INJECTION (1u)
#define ZCL_CLUSTER_REVISION_SAMPLE_MEI (1u)


#endif // __CLUSTER_MACRO_H__
