/*
 *
 *    Copyright (c) 2021 Project CHIP Authors
 *    All rights reserved.
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

#include <AppMain.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/PlatformManager.h>

#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app-common/zap-generated/attribute-type.h>
#include <app-common/zap-generated/attributes/Accessors.h>
#include <platform/CHIPDeviceLayer.h>
#include <app/ConcreteAttributePath.h>
#include <app/EventLogging.h>
#include <app/reporting/reporting.h>
#include <app/util/af-types.h>
#include <app/util/af.h>
#include <app/util/attribute-storage.h>
#include <app/util/util.h>
#include <credentials/DeviceAttestationCredsProvider.h>
#include <credentials/examples/DeviceAttestationCredsExample.h>
#include <lib/core/CHIPError.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/ZclString.h>
#include <platform/CommissionableDataProvider.h>
#include <setup_payload/QRCodeSetupPayloadGenerator.h>
#include <setup_payload/SetupPayload.h>

#include <sys/ioctl.h>
#include <sys/stat.h>
#include <time.h>
#include <signal.h>
#include <thread>
#include <chrono>
#include <semaphore>
#include <queue>
#include <vector>
#include <functional>
#include <cstdio>
#include <atomic>

#include "CommissionableInit.h"
#include "Device.h"
#include "main.h"
#include <app/server/Server.h>

#include <cassert>
#include <iostream>

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

using namespace chip;
using namespace chip::app;
using namespace chip::Credentials;
using namespace chip::Inet;
using namespace chip::Transport;
using namespace chip::DeviceLayer;
using namespace chip::app::Clusters;

#define NONE             "\033[m"
#define RED              "\033[0;32;31m"
#define LIGHT_RED        "\033[1;31m"
#define GREEN            "\033[0;32;32m"
#define LIGHT_GREEN      "\033[1;32m"
#define BLUE             "\033[0;32;34m"
#define LIGHT_BLUE       "\033[1;34m"
#define CYAN             "\033[0;36m"
#define PUPLE            "\033[0;35m"
#define BRON             "\033[0;33m"
#define YELLOW           "\033[1;33m"
#define WHITE            "\033[1;37m"

#define EndDeviceMax              250
#define EndPointMax               10
#define ClusterIDMax              30

struct _EndPoint
{
    uint8_t ep;
    uint16_t devidId;
    uint16_t clusterCounts;
    uint16_t clusterID[ClusterIDMax];
};

struct _EndDevice
{
    uint8_t  MacAddress[8];   //Mac Address(64bits)
    uint8_t  Active;
    uint16_t ShortAddress;    //Short Address(16bits)
    uint8_t ep_counts;
    struct _EndPoint ep_list[EndPointMax];
};

struct _Coordinator
{
    uint8_t  MacAddress[8];   //Mac Address(64bits)
    uint16_t PANID;
    uint16_t DevCount;
    uint16_t ARCount;
    uint8_t  CHANNEL;
    uint8_t  EXT_PAN_ID[8];
};

typedef struct _MatterEndDevice
{
    EndpointId  endpoint;
    ClusterId   clusterId;
    AttributeId attributeId;
    // _EndDevice  ED;
    uint8_t ep;
    uint16_t ShortAddress;
} _MatterEndDevice_t;

typedef struct gw_command
{
    uint8_t buffer[1024];
    int len;
} gw_command_t;

typedef struct ias_zone_notify
{
    EndpointId endpoint;
    bool status;
} ias_zone_notify_t;

std::queue<gw_command_t> gw_rx_command_queue;
std::queue<gw_command_t> gw_tx_command_queue;
std::queue<gw_command_t> gw_read_response_queue;

std::vector<_MatterEndDevice_t> bridge_device;

std::binary_semaphore wait_tx_sem {1};
// std::binary_semaphore read_attribute_sem {0};

typedef struct binary_semaphore
{
    std::binary_semaphore sem;

    // constexpr default_binary_semaphore() : sem (0) {}
    constexpr explicit binary_semaphore(auto count) : sem(count) {}
} binary_semaphore_t;

binary_semaphore_t *read_attribute_sem[20];

// int read_attribute_sem_count = 0;

// std::function<void(intptr_t)> read_attr_cb;

struct _EndDevice ED[EndDeviceMax];
struct _Coordinator CR;
// struct _MatterEndDevice MED[EndDeviceMax];

const char EndDevice_Filename[] = "/usr/local/var/lib/ez-zbgw/zbdb/sc_enddevice.dat";
const char Coordinator_Filename[] = "/usr/local/var/lib/ez-zbgw/zbdb/sc_coordinator.dat";

// int light_id = 0;
int device_index = 0;
bool timeout = false;
bool add_ep = false;
int sockfd = 0;
timer_t *tid;

DeviceLight *Light[EndDeviceMax];
DeviceTempSensor *TempSensor[EndDeviceMax];
DeviceContactSensor *ContactSensor[EndDeviceMax];

namespace {

const int kNodeLabelSize = 32;
// Current ZCL implementation of Struct uses a max-size array of 254 bytes
const int kDescriptorAttributeArraySize = 254;

EndpointId gCurrentEndpointId;
EndpointId gFirstDynamicEndpointId;
// Power source is on the same endpoint as the composed device
Device * gDevices[CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT + 1];
std::vector<Room *> gRooms;
std::vector<Action *> gActions;

const int16_t minMeasuredValue     = -27315;
const int16_t maxMeasuredValue     = 32766;
const int16_t initialMeasuredValue = 2100;

#define DEVICE_TYPE_BRIDGED_NODE          0x0013
#define DEVICE_TYPE_CONTACT_SENSOR        0x0015
#define DEVICE_TYPE_LO_ON_OFF_LIGHT       0x0100
#define DEVICE_TYPE_DIMMABLE_LIGHT        0x0101
#define DEVICE_TYPE_COLOR_TEMP_LIGHT      0x010C
#define DEVICE_TYPE_EXTENDED_COLOR_LIGHT  0x010D
#define DEVICE_TYPE_POWER_SOURCE          0x0011
#define DEVICE_TYPE_TEMP_SENSOR           0x0302
#define DEVICE_VERSION_DEFAULT            1

// Declare Descriptor cluster attributes
DECLARE_DYNAMIC_ATTRIBUTE_LIST_BEGIN(descriptorAttrs)
DECLARE_DYNAMIC_ATTRIBUTE(Descriptor::Attributes::DeviceTypeList::Id, ARRAY, kDescriptorAttributeArraySize, 0), /* device list */
DECLARE_DYNAMIC_ATTRIBUTE(Descriptor::Attributes::ServerList::Id, ARRAY, kDescriptorAttributeArraySize, 0),     /* server list */
DECLARE_DYNAMIC_ATTRIBUTE(Descriptor::Attributes::ClientList::Id, ARRAY, kDescriptorAttributeArraySize, 0),     /* client list */
DECLARE_DYNAMIC_ATTRIBUTE(Descriptor::Attributes::PartsList::Id, ARRAY, kDescriptorAttributeArraySize, 0),      /* parts list */
DECLARE_DYNAMIC_ATTRIBUTE_LIST_END();

// Declare Bridged Device Basic Information cluster attributes
DECLARE_DYNAMIC_ATTRIBUTE_LIST_BEGIN(bridgedDeviceBasicAttrs)
DECLARE_DYNAMIC_ATTRIBUTE(BridgedDeviceBasicInformation::Attributes::NodeLabel::Id, CHAR_STRING, kNodeLabelSize, 0), /* NodeLabel */
DECLARE_DYNAMIC_ATTRIBUTE(BridgedDeviceBasicInformation::Attributes::Reachable::Id, BOOLEAN, 1, 0),                  /* Reachable */
DECLARE_DYNAMIC_ATTRIBUTE(BridgedDeviceBasicInformation::Attributes::FeatureMap::Id, BITMAP32, 4, 0),                /* feature map */
DECLARE_DYNAMIC_ATTRIBUTE_LIST_END();

DECLARE_DYNAMIC_ATTRIBUTE_LIST_BEGIN(tempSensorAttrs)
DECLARE_DYNAMIC_ATTRIBUTE(TemperatureMeasurement::Attributes::MeasuredValue::Id, INT16S, 2, 0),    /* Measured Value */
DECLARE_DYNAMIC_ATTRIBUTE(TemperatureMeasurement::Attributes::MinMeasuredValue::Id, INT16S, 2, 0), /* Min Measured Value */
DECLARE_DYNAMIC_ATTRIBUTE(TemperatureMeasurement::Attributes::MaxMeasuredValue::Id, INT16S, 2, 0), /* Max Measured Value */
DECLARE_DYNAMIC_ATTRIBUTE(TemperatureMeasurement::Attributes::FeatureMap::Id, BITMAP32, 4, 0),     /* FeatureMap */
DECLARE_DYNAMIC_ATTRIBUTE_LIST_END();

// Declare On/Off cluster attributes
DECLARE_DYNAMIC_ATTRIBUTE_LIST_BEGIN(onOffAttrs)
DECLARE_DYNAMIC_ATTRIBUTE(OnOff::Attributes::OnOff::Id, BOOLEAN, 1, 0), /* on/off */
DECLARE_DYNAMIC_ATTRIBUTE_LIST_END();

DECLARE_DYNAMIC_ATTRIBUTE_LIST_BEGIN(levelAttrs)
DECLARE_DYNAMIC_ATTRIBUTE(Clusters::LevelControl::Attributes::CurrentLevel::Id, INT8U, 1, 0),
DECLARE_DYNAMIC_ATTRIBUTE(Clusters::LevelControl::Attributes::MinLevel::Id, INT8U, 1, 0),
DECLARE_DYNAMIC_ATTRIBUTE(Clusters::LevelControl::Attributes::MaxLevel::Id, INT8U, 1, 0),
DECLARE_DYNAMIC_ATTRIBUTE_LIST_END();

DECLARE_DYNAMIC_ATTRIBUTE_LIST_BEGIN(colorTemperatureAttrs)
DECLARE_DYNAMIC_ATTRIBUTE(Clusters::ColorControl::Attributes::CurrentHue::Id, INT8U, 1, 0),
DECLARE_DYNAMIC_ATTRIBUTE(Clusters::ColorControl::Attributes::CurrentSaturation::Id, INT8U, 1, 0),
DECLARE_DYNAMIC_ATTRIBUTE(Clusters::ColorControl::Attributes::ColorTemperatureMireds::Id, INT16U, 2, 0),
DECLARE_DYNAMIC_ATTRIBUTE(Clusters::ColorControl::Attributes::RemainingTime::Id, INT16U, 2, 0),
DECLARE_DYNAMIC_ATTRIBUTE(Clusters::ColorControl::Attributes::ColorMode::Id, ENUM8, 1, 0),
DECLARE_DYNAMIC_ATTRIBUTE(Clusters::ColorControl::Attributes::ColorTempPhysicalMinMireds::Id, INT16U, 2, 0),
DECLARE_DYNAMIC_ATTRIBUTE(Clusters::ColorControl::Attributes::ColorTempPhysicalMaxMireds::Id, INT16U, 2, 0),
DECLARE_DYNAMIC_ATTRIBUTE(Clusters::ColorControl::Attributes::StartUpColorTemperatureMireds::Id, INT16U, 2, 0),
DECLARE_DYNAMIC_ATTRIBUTE(Clusters::ColorControl::Attributes::ColorCapabilities::Id, BITMAP16, 2, 0),
DECLARE_DYNAMIC_ATTRIBUTE(Clusters::ColorControl::Attributes::FeatureMap::Id, BITMAP32, 4, 0),     /* FeatureMap */
DECLARE_DYNAMIC_ATTRIBUTE_LIST_END();

//DECLARE_DYNAMIC_ATTRIBUTE_LIST_BEGIN(extendedLightAttrs)
//DECLARE_DYNAMIC_ATTRIBUTE(Clusters::ColorControl::Attributes::CurrentHue::Id, INT8U, 1, 0),
//DECLARE_DYNAMIC_ATTRIBUTE(Clusters::ColorControl::Attributes::CurrentSaturation::Id, INT8U, 1, 0),
//DECLARE_DYNAMIC_ATTRIBUTE(Clusters::ColorControl::Attributes::ColorTemperatureMireds::Id, INT16U, 2, 0),
//DECLARE_DYNAMIC_ATTRIBUTE(Clusters::ColorControl::Attributes::RemainingTime::Id, INT16U, 2, 0),
//DECLARE_DYNAMIC_ATTRIBUTE(Clusters::ColorControl::Attributes::ColorMode::Id, ENUM8, 1, 0),
//DECLARE_DYNAMIC_ATTRIBUTE(Clusters::ColorControl::Attributes::ColorTempPhysicalMinMireds::Id, INT16U, 2, 0),
//DECLARE_DYNAMIC_ATTRIBUTE(Clusters::ColorControl::Attributes::ColorTempPhysicalMaxMireds::Id, INT16U, 2, 0),
//DECLARE_DYNAMIC_ATTRIBUTE(Clusters::ColorControl::Attributes::StartUpColorTemperatureMireds::Id, INT16U, 2, 0),
//DECLARE_DYNAMIC_ATTRIBUTE(Clusters::ColorControl::Attributes::ColorCapabilities::Id, BITMAP16, 2, 0),
//DECLARE_DYNAMIC_ATTRIBUTE(Clusters::ColorControl::Attributes::FeatureMap::Id, BITMAP32, 4, 0),     /* FeatureMap */
//DECLARE_DYNAMIC_ATTRIBUTE_LIST_END();

// Declare On/Off cluster attributes
DECLARE_DYNAMIC_ATTRIBUTE_LIST_BEGIN(booleanAttrs)
DECLARE_DYNAMIC_ATTRIBUTE(BooleanState::Attributes::StateValue::Id, BOOLEAN, 1, 0),
DECLARE_DYNAMIC_ATTRIBUTE_LIST_END();

constexpr CommandId onOffIncomingCommands[] = {
    app::Clusters::OnOff::Commands::Off::Id,
    app::Clusters::OnOff::Commands::On::Id,
    app::Clusters::OnOff::Commands::Toggle::Id,
    app::Clusters::OnOff::Commands::OffWithEffect::Id,
    app::Clusters::OnOff::Commands::OnWithRecallGlobalScene::Id,
    app::Clusters::OnOff::Commands::OnWithTimedOff::Id,
    kInvalidCommandId,
};

constexpr CommandId levelIncomingCommands[] = {
    app::Clusters::LevelControl::Commands::MoveToLevel::Id,
    app::Clusters::LevelControl::Commands::Move::Id,
    app::Clusters::LevelControl::Commands::Step::Id,
    app::Clusters::LevelControl::Commands::Stop::Id,
    app::Clusters::LevelControl::Commands::MoveToLevelWithOnOff::Id,
    app::Clusters::LevelControl::Commands::StepWithOnOff::Id,
    app::Clusters::LevelControl::Commands::StopWithOnOff::Id,
    app::Clusters::LevelControl::Commands::MoveToClosestFrequency::Id,
    kInvalidCommandId,
};

constexpr CommandId colorTemperatureIncomingCommands[] = {
    app::Clusters::ColorControl::Commands::MoveToHue::Id,
    app::Clusters::ColorControl::Commands::MoveHue::Id,
    app::Clusters::ColorControl::Commands::StepHue::Id,
    app::Clusters::ColorControl::Commands::MoveToSaturation::Id,
    app::Clusters::ColorControl::Commands::MoveSaturation::Id,
    app::Clusters::ColorControl::Commands::StepSaturation::Id,
    app::Clusters::ColorControl::Commands::MoveToHueAndSaturation::Id,
    app::Clusters::ColorControl::Commands::MoveToColor::Id,
    app::Clusters::ColorControl::Commands::MoveColor::Id,
    app::Clusters::ColorControl::Commands::StepColor::Id,
    app::Clusters::ColorControl::Commands::MoveToColorTemperature::Id,
    app::Clusters::ColorControl::Commands::EnhancedMoveToHue::Id,
    app::Clusters::ColorControl::Commands::EnhancedMoveHue::Id,
    app::Clusters::ColorControl::Commands::EnhancedStepHue::Id,
    app::Clusters::ColorControl::Commands::EnhancedMoveToHueAndSaturation::Id,
    app::Clusters::ColorControl::Commands::ColorLoopSet::Id,
    app::Clusters::ColorControl::Commands::StopMoveStep::Id,
    app::Clusters::ColorControl::Commands::MoveColorTemperature::Id,
    app::Clusters::ColorControl::Commands::StepColorTemperature::Id,          
    kInvalidCommandId,
};

// ---------------------------------------------------------------------------
//
// TEMPERATURE SENSOR ENDPOINT: contains the following clusters:
//   - Temperature measurement
//   - Descriptor
//   - Bridged Device Basic Information
DECLARE_DYNAMIC_CLUSTER_LIST_BEGIN(bridgedTempSensorClusters)
DECLARE_DYNAMIC_CLUSTER(TemperatureMeasurement::Id, tempSensorAttrs, nullptr, nullptr),
DECLARE_DYNAMIC_CLUSTER(Descriptor::Id, descriptorAttrs, nullptr, nullptr),
DECLARE_DYNAMIC_CLUSTER(BridgedDeviceBasicInformation::Id, bridgedDeviceBasicAttrs, nullptr, nullptr),
DECLARE_DYNAMIC_CLUSTER_LIST_END;

// Declare Bridged Light endpoint
DECLARE_DYNAMIC_ENDPOINT(bridgedTempSensorEndpoint, bridgedTempSensorClusters);
DataVersion gTempSensorDataVersions[ArraySize(bridgedTempSensorClusters)];

const EmberAfDeviceType gBridgedTempSensorDeviceTypes[] = { { DEVICE_TYPE_TEMP_SENSOR, DEVICE_VERSION_DEFAULT },
                                                            { DEVICE_TYPE_BRIDGED_NODE, DEVICE_VERSION_DEFAULT } };

// ---------------------------------------------------------------------------
//
// LIGHT ENDPOINT: contains the following clusters:
//   - On/Off
//   - Descriptor
//   - Bridged Device Basic Information


DECLARE_DYNAMIC_CLUSTER_LIST_BEGIN(bridgedLightClusters)
DECLARE_DYNAMIC_CLUSTER(OnOff::Id, onOffAttrs, onOffIncomingCommands, nullptr),
DECLARE_DYNAMIC_CLUSTER(Descriptor::Id, descriptorAttrs, nullptr, nullptr),
DECLARE_DYNAMIC_CLUSTER(BridgedDeviceBasicInformation::Id, bridgedDeviceBasicAttrs, nullptr, nullptr)
DECLARE_DYNAMIC_CLUSTER_LIST_END;

// Declare Bridged Light endpoint
DECLARE_DYNAMIC_ENDPOINT(bridgedLightEndpoint, bridgedLightClusters);
DataVersion gLightDataVersions[ArraySize(bridgedLightClusters)];

const EmberAfDeviceType gBridgedOnOffDeviceTypes[] = { { DEVICE_TYPE_LO_ON_OFF_LIGHT, DEVICE_VERSION_DEFAULT },
                                                       { DEVICE_TYPE_BRIDGED_NODE, DEVICE_VERSION_DEFAULT } };


// ---------------------------------------------------------------------------
//
// DIMMING ENDPOINT: contains the following clusters:
//   - On/Off
//   - Level
//   - Descriptor
//   - Bridged Device Basic Information

DECLARE_DYNAMIC_CLUSTER_LIST_BEGIN(bridgedDimmingClusters)
DECLARE_DYNAMIC_CLUSTER(OnOff::Id, onOffAttrs, onOffIncomingCommands, nullptr),
DECLARE_DYNAMIC_CLUSTER(LevelControl::Id, levelAttrs, levelIncomingCommands, nullptr),
DECLARE_DYNAMIC_CLUSTER(Descriptor::Id, descriptorAttrs, nullptr, nullptr),
DECLARE_DYNAMIC_CLUSTER(BridgedDeviceBasicInformation::Id, bridgedDeviceBasicAttrs, nullptr, nullptr)
DECLARE_DYNAMIC_CLUSTER_LIST_END;

// Declare Bridged Light endpoint
DECLARE_DYNAMIC_ENDPOINT(bridgedDimmingEndpoint, bridgedDimmingClusters);
DataVersion gDimmingDataVersions[ArraySize(bridgedDimmingClusters)];

const EmberAfDeviceType gBridgedDimmingDeviceTypes[] = { { DEVICE_TYPE_DIMMABLE_LIGHT, DEVICE_VERSION_DEFAULT },
                                                         { DEVICE_TYPE_BRIDGED_NODE, DEVICE_VERSION_DEFAULT } };


// ---------------------------------------------------------------------------
//
// CT ENDPOINT: contains the following clusters:
//   - On/Off
//   - Level
//   - Color Temperature
//   - Descriptor
//   - Bridged Device Basic Information

DECLARE_DYNAMIC_CLUSTER_LIST_BEGIN(bridgedColorTemperatureClusters)
DECLARE_DYNAMIC_CLUSTER(OnOff::Id, onOffAttrs, onOffIncomingCommands, nullptr),
DECLARE_DYNAMIC_CLUSTER(LevelControl::Id, levelAttrs, levelIncomingCommands, nullptr),
DECLARE_DYNAMIC_CLUSTER(ColorControl::Id, colorTemperatureAttrs, colorTemperatureIncomingCommands, nullptr),
DECLARE_DYNAMIC_CLUSTER(Descriptor::Id, descriptorAttrs, nullptr, nullptr),
DECLARE_DYNAMIC_CLUSTER(BridgedDeviceBasicInformation::Id, bridgedDeviceBasicAttrs, nullptr, nullptr)
DECLARE_DYNAMIC_CLUSTER_LIST_END;

// Declare Bridged Light endpoint
DECLARE_DYNAMIC_ENDPOINT(bridgedColorTemperatureEndpoint, bridgedColorTemperatureClusters);
DataVersion gColorTemperatureDataVersions[ArraySize(bridgedColorTemperatureClusters)];

const EmberAfDeviceType gBridgedColorTemperatureDeviceTypes[] = { { DEVICE_TYPE_COLOR_TEMP_LIGHT, DEVICE_VERSION_DEFAULT },
                                                                  { DEVICE_TYPE_BRIDGED_NODE, DEVICE_VERSION_DEFAULT } };

// ---------------------------------------------------------------------------
//
// CONTACT SENSOR ENDPOINT: contains the following clusters:
//   - Boolean
//   - Descriptor
//   - Bridged Device Basic Information

DECLARE_DYNAMIC_CLUSTER_LIST_BEGIN(bridgedContactSensorClusters)
DECLARE_DYNAMIC_CLUSTER(BooleanState::Id, booleanAttrs, nullptr, nullptr),
DECLARE_DYNAMIC_CLUSTER(Descriptor::Id, descriptorAttrs, nullptr, nullptr),
DECLARE_DYNAMIC_CLUSTER(BridgedDeviceBasicInformation::Id, bridgedDeviceBasicAttrs, nullptr, nullptr)
DECLARE_DYNAMIC_CLUSTER_LIST_END;

// Declare Bridged Light endpoint
DECLARE_DYNAMIC_ENDPOINT(bridgedContactSensorEndpoint, bridgedContactSensorClusters);
DataVersion gContactSensorDataVersions[ArraySize(bridgedContactSensorClusters)];

const EmberAfDeviceType gBridgedContactSensorDeviceTypes[] = { { DEVICE_TYPE_CONTACT_SENSOR, DEVICE_VERSION_DEFAULT },
                                                               { DEVICE_TYPE_BRIDGED_NODE, DEVICE_VERSION_DEFAULT } };

namespace {

static void gw_cmd_transmit(uint8_t *cmd, int len)
{
    uint8_t checksum = 0;

    for (int i = 0; i < cmd[4]; i++)
    {
        checksum += cmd[4 + i];
    }
    cmd[len - 1] = ~checksum;

    // printf(LIGHT_RED"send command: ");
    // for (int i = 0; i < len; i++)
    // {
    //     printf("%02x ", cmd[i]);
    // }
    // printf(NONE"\r\n");

    gw_command_t queue;

    queue.len = len;
    memcpy(queue.buffer, cmd, queue.len);
    gw_tx_command_queue.push(queue);
    free(cmd);
}

static void gw_cmd_read_attr_req(intptr_t arg)
{
    // printf(LIGHT_RED"%s", __FUNCTION__);
    // printf(NONE"\r\n");

    auto path = reinterpret_cast<app::ConcreteAttributePath *>(arg);

    EndpointId endpoint     = path->mEndpointId;
    ClusterId clusterId     = path->mClusterId;
    AttributeId attributeId = path->mAttributeId;

    Platform::Delete(path);

    uint16_t endpointIndex  = emberAfGetDynamicIndexFromEndpoint(endpoint);
    uint8_t cmd[] = {0xFF, 0xFC, 0xFC, 0xFF,
                           0x0C,
                           0x00, 0x00, 0x02, 0x00,
                           0x00, 0x00,
                           0x00, 0x00,
                           0x00, 0x00,
                           0x00, 0x00,
                           0x00};
    uint8_t *pCmd = (uint8_t *)malloc(sizeof(uint8_t) * sizeof(cmd));

    cmd[9] = (uint8_t)(bridge_device[endpointIndex].ShortAddress & 0xFF);
    cmd[10] = (uint8_t)((bridge_device[endpointIndex].ShortAddress >> 8) & 0xFF);
    cmd[12] = bridge_device[endpointIndex].ep;
    cmd[13] = (uint8_t)(clusterId & 0xFF);
    cmd[14] = (uint8_t)((clusterId >> 8) & 0xFF);
    cmd[15] = (uint8_t)(attributeId & 0xFF);
    cmd[16] = (uint8_t)((attributeId >> 8) & 0xFF);
    
    memcpy(pCmd, cmd, sizeof(cmd));

    gw_cmd_transmit(pCmd, sizeof(cmd));
}

static void timerCallback(System::Layer *, void * callbackContext)
{
    wait_tx_sem.release();
    read_attribute_sem[0]->sem.release();
    delete read_attribute_sem[0];
    timeout = true;
}

static bool gw_cmd_read_req(uint8_t *buffer, intptr_t path, uint16_t size)
{
    do
    {
        timeout = false;
        gw_cmd_read_attr_req(reinterpret_cast<intptr_t>(path));

        CHIP_ERROR err = DeviceLayer::SystemLayer().StartTimer(
                chip::System::Clock::Milliseconds32(5000), timerCallback, nullptr);

        if (err != CHIP_NO_ERROR)
        {
            printf(LIGHT_RED"Start timer error");
            printf(NONE"\r\n");
        }
        read_attribute_sem[0] = new binary_semaphore_t(0);
        read_attribute_sem[0]->sem.acquire();
        // read_attribute_sem_count++;
        if (!timeout)
        {
            // printf(LIGHT_RED"recv done");
            // printf(NONE"\r\n");
            DeviceLayer::SystemLayer().CancelTimer(timerCallback, nullptr);
        }
        if (!gw_read_response_queue.empty())
        {
            gw_command_t resp = gw_read_response_queue.front();

            memcpy(buffer, &resp.buffer[19], size);
            gw_read_response_queue.pop();
        }
        else
        {
            return false;
        }
    } while (0);  

    return true;
}

// static void gw_cmd_read_attr_resp(uint8_t *buffer, gw_command_t resp, uint8_t size)
// {
//     uint8_t resp[10] = {0};

//     memcpy(buffer, &resp.buffer[19], size);
// }

static void gw_cmd_onoff_req(intptr_t arg, uint8_t OnOff)
{
    // printf(LIGHT_RED"%s", __FUNCTION__);
    // printf(NONE"\r\n");

    auto path = reinterpret_cast<app::ConcreteAttributePath *>(arg);

    EndpointId endpoint     = path->mEndpointId;
    // ClusterId clusterId     = path->mClusterId;
    // AttributeId attributeId = path->mAttributeId;

    Platform::Delete(path);

    uint16_t endpointIndex  = emberAfGetDynamicIndexFromEndpoint(endpoint);
    uint8_t cmd[] = {0xFF, 0xFC, 0xFC, 0xFF, 0x09, 0xFF, 0x00, 0x07, 0x00, 0xFF, 0xFF, 0x00, 0xFF, 0x00, 0x00};
    uint8_t *pCmd = (uint8_t *)malloc(sizeof(uint8_t) * sizeof(cmd));

    cmd[5] = (uint8_t)OnOff;
    cmd[9] = (uint8_t)(bridge_device[endpointIndex].ShortAddress & 0xFF);
    cmd[10] = (uint8_t)((bridge_device[endpointIndex].ShortAddress >> 8) & 0xFF);
    cmd[12] = bridge_device[endpointIndex].ep;

    memcpy(pCmd, cmd, sizeof(cmd));

    gw_cmd_transmit(pCmd, sizeof(cmd));
}

static void gw_cmd_move_to_level_req(intptr_t arg, uint8_t level)
{
    // printf(LIGHT_RED"%s", __FUNCTION__);
    // printf(NONE"\r\n");

    auto path = reinterpret_cast<app::ConcreteAttributePath *>(arg);

    EndpointId endpoint     = path->mEndpointId;
    // ClusterId clusterId     = path->mClusterId;
    // AttributeId attributeId = path->mAttributeId;

    Platform::Delete(path);

    uint16_t endpointIndex  = emberAfGetDynamicIndexFromEndpoint(endpoint);
    uint8_t cmd[] = {0xFF, 0xFC, 0xFC, 0xFF, 0x0C, 0x00, 0x00, 0x09, 0x00, 0xFF, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0x00, 0x00};
    uint8_t *pCmd = (uint8_t *)malloc(sizeof(uint8_t) * sizeof(cmd));
    
    cmd[9] = (uint8_t)(bridge_device[endpointIndex].ShortAddress & 0xFF);
    cmd[10] = (uint8_t)((bridge_device[endpointIndex].ShortAddress >> 8) & 0xFF);
    cmd[12] = bridge_device[endpointIndex].ep;
    cmd[14] = level;

    memcpy(pCmd, cmd, sizeof(cmd));

    gw_cmd_transmit(pCmd, sizeof(cmd));
}

static void gw_cmd_move_to_hue_req(intptr_t arg, uint8_t hue)
{
    // printf(LIGHT_RED"%s", __FUNCTION__);
    // printf(NONE"\r\n");

    auto path = reinterpret_cast<app::ConcreteAttributePath *>(arg);

    EndpointId endpoint     = path->mEndpointId;
    // ClusterId clusterId     = path->mClusterId;
    // AttributeId attributeId = path->mAttributeId;

    Platform::Delete(path);

    uint16_t endpointIndex  = emberAfGetDynamicIndexFromEndpoint(endpoint);
    uint8_t cmd[] = {0xFF, 0xFC, 0xFC, 0xFF, 0x0D, 0x00, 0x00, 0x21, 0x00, 0xFF, 0xFF ,0x00, 0xFF, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x00};
    uint8_t *pCmd = (uint8_t *)malloc(sizeof(uint8_t) * sizeof(cmd));
    
    cmd[9] = (uint8_t)(bridge_device[endpointIndex].ShortAddress & 0xFF);
    cmd[10] = (uint8_t)((bridge_device[endpointIndex].ShortAddress >> 8) & 0xFF);
    cmd[12] = bridge_device[endpointIndex].ep;
    cmd[14] = hue;

    memcpy(pCmd, cmd, sizeof(cmd));

    gw_cmd_transmit(pCmd, sizeof(cmd));
}

static void gw_cmd_move_to_saturation_req(intptr_t arg, uint8_t saturation)
{
    // printf(LIGHT_RED"%s", __FUNCTION__);
    // printf(NONE"\r\n");

    auto path = reinterpret_cast<app::ConcreteAttributePath *>(arg);

    EndpointId endpoint     = path->mEndpointId;
    // ClusterId clusterId     = path->mClusterId;
    // AttributeId attributeId = path->mAttributeId;

    Platform::Delete(path);

    uint16_t endpointIndex  = emberAfGetDynamicIndexFromEndpoint(endpoint);
    uint8_t cmd[] = {0xFF, 0xFC, 0xFC, 0xFF, 0x0D, 0x03, 0x00, 0x21, 0x00, 0xFF, 0xFF ,0x00, 0xFF, 0x00, 0xFF, 0x00, 0x00, 0x00, 0x00};
    uint8_t *pCmd = (uint8_t *)malloc(sizeof(uint8_t) * sizeof(cmd));
    
    cmd[9] = (uint8_t)(bridge_device[endpointIndex].ShortAddress & 0xFF);
    cmd[10] = (uint8_t)((bridge_device[endpointIndex].ShortAddress >> 8) & 0xFF);
    cmd[12] = bridge_device[endpointIndex].ep;
    cmd[14] = saturation;

    memcpy(pCmd, cmd, sizeof(cmd));

    gw_cmd_transmit(pCmd, sizeof(cmd));
}

static void gw_cmd_move_to_color_temperature_req(intptr_t arg, uint16_t Mireds)
{
    // printf(LIGHT_RED"%s", __FUNCTION__);
    // printf(NONE"\r\n");

    auto path = reinterpret_cast<app::ConcreteAttributePath *>(arg);

    EndpointId endpoint     = path->mEndpointId;
    // ClusterId clusterId     = path->mClusterId;
    // AttributeId attributeId = path->mAttributeId;

    Platform::Delete(path);

    uint16_t endpointIndex  = emberAfGetDynamicIndexFromEndpoint(endpoint);
    uint8_t cmd[] = {0xFF, 0xFC, 0xFC, 0xFF, 0x0D, 0x0A, 0x00, 0x21, 0x00, 0xFF, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0xFF, 0x00, 0x00, 0x00};
    uint8_t *pCmd = (uint8_t *)malloc(sizeof(uint8_t) * sizeof(cmd));
    
    cmd[9] = (uint8_t)(bridge_device[endpointIndex].ShortAddress & 0xFF);
    cmd[10] = (uint8_t)((bridge_device[endpointIndex].ShortAddress >> 8) & 0xFF);
    cmd[12] = bridge_device[endpointIndex].ep;
    cmd[14] = (uint8_t)(Mireds & 0xFF);
	cmd[15] = (uint8_t)(Mireds >> 8);

    memcpy(pCmd, cmd, sizeof(cmd));

    gw_cmd_transmit(pCmd, sizeof(cmd));
}

void MatterReportingAttributeChangeCallback(const ConcreteAttributePath & aPath)
{
    // EndpointId endpoint     = aPath.mEndpointId;
    // ClusterId clusterId     = aPath.mClusterId;
    // AttributeId attributeId = aPath.mAttributeId;

    printf(LIGHT_RED"%s", __FUNCTION__);
    printf(NONE"\r\n");
}

void CallReportingCallback(intptr_t closure)
{
    printf(LIGHT_RED"%s", __FUNCTION__);
    printf(NONE"\r\n");
    auto path = reinterpret_cast<app::ConcreteAttributePath *>(closure);
    MatterReportingAttributeChangeCallback(*path);
    Platform::Delete(path);
}

void ScheduleReportingCallback(Device * dev, ClusterId cluster, AttributeId attribute)
{
    printf(LIGHT_RED"%s", __FUNCTION__);
    printf(NONE"\r\n");
    auto * path = Platform::New<app::ConcreteAttributePath>(dev->GetEndpointId(), cluster, attribute);
    PlatformMgr().ScheduleWork(CallReportingCallback, reinterpret_cast<intptr_t>(path));
}

} // anonymous namespace

void HandleDeviceStatusChanged(Device * dev, Device::Changed_t itemChangedMask)
{
    printf(LIGHT_RED"%s", __FUNCTION__);
    printf(NONE"\r\n");
    if (itemChangedMask & Device::kChanged_Reachable)
    {
        ScheduleReportingCallback(dev, BridgedDeviceBasicInformation::Id, BridgedDeviceBasicInformation::Attributes::Reachable::Id);
    }
    if (itemChangedMask & Device::kChanged_Name)
    {
        ScheduleReportingCallback(dev, BridgedDeviceBasicInformation::Id, BridgedDeviceBasicInformation::Attributes::NodeLabel::Id);
    }
}

void HandleDeviceLightStatusChanged(DeviceLight * dev, DeviceLight::Changed_t itemChangedMask)
{
    // printf(LIGHT_RED"%s", __FUNCTION__);
    // printf(NONE"\r\n");
            
    // uint16_t endpointIndex  = emberAfGetDynamicIndexFromEndpoint(dev->GetEndpointId());

    if (itemChangedMask & (DeviceLight::kChanged_Reachable | DeviceLight::kChanged_Name | DeviceLight::kChanged_Location))
    {
        HandleDeviceStatusChanged(static_cast<Device *>(dev), (Device::Changed_t) itemChangedMask);
    }
    if (itemChangedMask & DeviceLight::kChanged_OnOff)
    {
        // ScheduleReportingCallback(dev, OnOff::Id, OnOff::Attributes::OnOff::Id);

        auto * path = Platform::New<app::ConcreteAttributePath>(dev->GetEndpointId(), OnOff::Id, OnOff::Attributes::OnOff::Id);
        // PlatformMgr().ScheduleWork(HandleZigbeeGatewayRequestCallback, reinterpret_cast<intptr_t>(path));
        // EndpointId endpoint     = dev->GetEndpointId();
        // ClusterId clusterId     = OnOff::Id;
        // AttributeId attributeId = OnOff::Attributes::OnOff::Id;

        printf(LIGHT_RED"OnOff Changed: %d", dev->IsOn());
        printf(NONE"\r\n");

        if (dev->IsReachable())
        {
            wait_tx_sem.acquire();
            gw_cmd_onoff_req(reinterpret_cast<intptr_t>(path), dev->IsOn());
        }
    }
    else if (itemChangedMask & DeviceLight::kChanged_CurrentLevel)
    {
        auto * path = Platform::New<app::ConcreteAttributePath>(dev->GetEndpointId(), LevelControl::Id, LevelControl::Attributes::CurrentLevel::Id);

        printf(LIGHT_RED"CurrentLevel Changed: %d", dev->GetCurrentLevel());
        printf(NONE"\r\n");
        if (dev->IsReachable())
        {
    	    wait_tx_sem.acquire();
    	    gw_cmd_move_to_level_req(reinterpret_cast<intptr_t>(path), dev->GetCurrentLevel());
        }
    }
    else if (itemChangedMask & DeviceLight::kChanged_CurrentHue)
    {
        auto * path = Platform::New<app::ConcreteAttributePath>(dev->GetEndpointId(), ColorControl::Id, ColorControl::Attributes::CurrentHue::Id);

        printf(LIGHT_RED"CurrentHue Changed: %d", dev->GetCurrentHue());
        printf(NONE"\r\n");
        if (dev->IsReachable())
        {
    	    wait_tx_sem.acquire();
    	    gw_cmd_move_to_hue_req(reinterpret_cast<intptr_t>(path), dev->GetCurrentHue());
        }
    }
    else if (itemChangedMask & DeviceLight::kChanged_CurrentSaturation)
    {
        auto * path = Platform::New<app::ConcreteAttributePath>(dev->GetEndpointId(), ColorControl::Id, ColorControl::Attributes::CurrentSaturation::Id);

        printf(LIGHT_RED"CurrentSaturation Changed: %d", dev->GetCurrentSaturation());
        printf(NONE"\r\n");
        if (dev->IsReachable())
        {
    	    wait_tx_sem.acquire();
    	    gw_cmd_move_to_saturation_req(reinterpret_cast<intptr_t>(path), dev->GetCurrentSaturation());
        }
    }
    else if (itemChangedMask & DeviceLight::kChanged_ColorTemperatureMireds)
    {
        auto * path = Platform::New<app::ConcreteAttributePath>(dev->GetEndpointId(), ColorControl::Id, ColorControl::Attributes::ColorTemperatureMireds::Id);

        printf(LIGHT_RED"ColorTemperatureMireds Changed: %d", dev->GetColorTemperatureMireds());
        printf(NONE"\r\n");
        if (dev->IsReachable())
        {
    	    wait_tx_sem.acquire();
    	    gw_cmd_move_to_color_temperature_req(reinterpret_cast<intptr_t>(path), dev->GetColorTemperatureMireds());
        }
    }
    else if (itemChangedMask & DeviceLight::kChanged_ColorMode)
    {
        // auto * path = Platform::New<app::ConcreteAttributePath>(dev->GetEndpointId(), ColorControl::Id, ColorControl::Attributes::ColorMode::Id);
        printf(LIGHT_RED"ColorMode Changed: %d", dev->GetColorMode());
        printf(NONE"\r\n");
        if (dev->IsReachable())
        {
    	    // wait_tx_sem.acquire();
    	    // gw_cmd_move_to_color_temperature_req(reinterpret_cast<intptr_t>(path), dev->GetColorTemperatureMireds());
        }
    }
}

void HandleDeviceConatactSensorStatusChanged(DeviceContactSensor * dev, DeviceContactSensor::Changed_t itemChangedMask)
{
    printf(LIGHT_RED"%s", __FUNCTION__);
    printf(NONE"\r\n");
    // if (itemChangedMask &
    //     (DeviceTempSensor::kChanged_Reachable | DeviceTempSensor::kChanged_Name | DeviceTempSensor::kChanged_Location))
    // {
    //     HandleDeviceStatusChanged(static_cast<Device *>(dev), (Device::Changed_t) itemChangedMask);
    // }
    // if (itemChangedMask & DeviceTempSensor::kChanged_MeasurementValue)
    // {
    //     ScheduleReportingCallback(dev, TemperatureMeasurement::Id, TemperatureMeasurement::Attributes::MeasuredValue::Id);
    // }
}

void HandleDeviceTempSensorStatusChanged(DeviceTempSensor * dev, DeviceTempSensor::Changed_t itemChangedMask)
{
    printf(LIGHT_RED"%s", __FUNCTION__);
    printf(NONE"\r\n");
    if (itemChangedMask &
        (DeviceTempSensor::kChanged_Reachable | DeviceTempSensor::kChanged_Name | DeviceTempSensor::kChanged_Location))
    {
        HandleDeviceStatusChanged(static_cast<Device *>(dev), (Device::Changed_t) itemChangedMask);
    }
    if (itemChangedMask & DeviceTempSensor::kChanged_MeasurementValue)
    {
        ScheduleReportingCallback(dev, TemperatureMeasurement::Id, TemperatureMeasurement::Attributes::MeasuredValue::Id);
    }
}

int AddDeviceEndpoint(Device * dev, EmberAfEndpointType * ep, const Span<const EmberAfDeviceType> & deviceTypeList,
                      const Span<DataVersion> & dataVersionStorage, chip::EndpointId parentEndpointId = chip::kInvalidEndpointId)
{
    printf(LIGHT_RED"%s", __FUNCTION__);
    printf(NONE"\r\n");
    uint8_t index = 0;
    while (index < CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT)
    {
        if (nullptr == gDevices[index])
        {
            gDevices[index] = dev;
            EmberAfStatus ret;
            while (true)
            {
                // Todo: Update this to schedule the work rather than use this lock
                DeviceLayer::StackLock lock;
                dev->SetEndpointId(gCurrentEndpointId);
                dev->SetParentEndpointId(parentEndpointId);
                ret =
                    emberAfSetDynamicEndpoint(index, gCurrentEndpointId, ep, dataVersionStorage, deviceTypeList, parentEndpointId);
                if (ret == EMBER_ZCL_STATUS_SUCCESS)
                {
                    ChipLogProgress(DeviceLayer, "Added device %s to dynamic endpoint %d (index=%d)", dev->GetName(),
                                    gCurrentEndpointId, index);
                    return index;
                }
                if (ret != EMBER_ZCL_STATUS_DUPLICATE_EXISTS)
                {
                    return -1;
                }
                // Handle wrap condition
                if (++gCurrentEndpointId < gFirstDynamicEndpointId)
                {
                    gCurrentEndpointId = gFirstDynamicEndpointId;
                }
            }
        }
        index++;
    }
    ChipLogProgress(DeviceLayer, "Failed to add dynamic endpoint: No endpoints available!");
    return -1;
}

int FileExist(const char *fname)
{
    struct stat st;
    return(stat(fname, &st) == 0);
}

void Check_Dev_Info()
{
    int i, j, k;

    for (i = 0; i < EndDeviceMax; i++)
    {
        if (ED[i].Active == 0)
            continue;
        printf(LIGHT_RED"%s", __FUNCTION__);
        printf(NONE"\r\n");
        ChipLogProgress(DeviceLayer, "Device %d", i);
        ChipLogProgress(DeviceLayer, "MAC Address: %02X%02X%02X%02X%02X%02X%02X%02X",
            ED[i].MacAddress[0],
            ED[i].MacAddress[1],
            ED[i].MacAddress[2],
            ED[i].MacAddress[3],
            ED[i].MacAddress[4],
            ED[i].MacAddress[5],
            ED[i].MacAddress[6],
            ED[i].MacAddress[7]);
        ChipLogProgress(DeviceLayer, "Short Addr: %04X", ED[i].ShortAddress);
        for (j = 0; j < ED[i].ep_counts; j++)
        {
            ChipLogProgress(DeviceLayer, "EP-%d", j);
            ChipLogProgress(DeviceLayer, "Endpoint: %d", ED[i].ep_list[j].ep);
            ChipLogProgress(DeviceLayer, "Device Id: %d", ED[i].ep_list[j].devidId);
            ChipLogProgress(DeviceLayer, "Cluster Id");
            for (k = 0; k < ED[i].ep_list[j].clusterCounts; k++)
            {
                ChipLogProgress(DeviceLayer, "0x%04X", ED[i].ep_list[j].clusterID[k]);
                if (ED[i].ep_list[j].clusterID[k] == ColorControl::Id ||
                    ED[i].ep_list[j].clusterID[k] == TemperatureMeasurement::Id ||
                    ED[i].ep_list[j].clusterID[k] == 0x0500)
                {
                    struct _MatterEndDevice dev;

                    dev.ep = ED[i].ep_list[j].ep;
                    dev.ShortAddress = ED[i].ShortAddress;
                    dev.endpoint = (EndpointId)(device_index + gFirstDynamicEndpointId);
                    dev.clusterId = ED[i].ep_list[j].clusterID[k];
                    dev.attributeId = 0;
                    bridge_device.push_back(dev);
                    device_index++;
                }
            }
        }
    }
    
    printf(LIGHT_RED"Zigbee devices: %ld", bridge_device.size());
    printf(NONE"\r\n");
}

void Clear_EndDevice_Information()
{
    int i, j;
    //---------------------------------
    for (i = 0; i < EndDeviceMax; i++)
    {
        //----------------------------
        memset(ED[i].MacAddress,0x00,8);
        //----------------------------
        ED[i].ShortAddress=0;
        ED[i].Active=0;
        ED[i].ep_counts=0;

        for(j=0;j<EndPointMax;j++)
            memset(&ED[i].ep_list[j], 0x00, sizeof(struct _EndPoint));
    }
}

void Read_EndDevice_File()
{
    FILE    *fp;
    // int     i;
    size_t  rsize;
    //----------------------------------------------------------
    // printf(GREEN"Get coordinator register endpoint device form file !\n"NONE);
    //----------------------------------------------------------
    CR.DevCount=0;
    //----------------------------------------------------------
    Clear_EndDevice_Information();
    //--------------------------------------------------------
    if (FileExist(EndDevice_Filename))
    {
       fp = fopen(EndDevice_Filename,"rb");
       if (fp != NULL)
       {
           while(1)
           {
              rsize = fread(&ED[CR.DevCount].MacAddress[0],sizeof(struct _EndDevice),1,fp);
              if (rsize != 1)
                break;
              //-------------------------------------------------------------------------
              CR.DevCount++;
           }
           //----------------------------------------------------------------------------
           fclose(fp);
       }
       else
       {
        //   printf(LIGHT_RED"EndPoint_Filename Open Failure !\n"NONE);
       }
    }
    else
    {
        // printf(LIGHT_BLUE"EndDevice table is empty !\n"NONE);
        CR.DevCount=0;
    }
}

void Open_Socket()
{
    sockfd = socket(AF_INET, SOCK_STREAM , 0);

    if (sockfd == -1)
    {
        ChipLogProgress(DeviceLayer, "Fail to create a socket.");
    }

    struct sockaddr_in info;
    bzero(&info, sizeof(info));
    info.sin_family = PF_INET;

    info.sin_addr.s_addr = inet_addr("127.0.0.1");
    info.sin_port = htons(10010);

    int err = connect(sockfd,(struct sockaddr *)&info, sizeof(info));
    if (err == -1)
    {
        ChipLogProgress(DeviceLayer, "Connection error");
    }
}

// void Close_Socket()
// {
//     close(sockfd);
// }

void ChipEventHandler(const ChipDeviceEvent * aEvent, intptr_t /* arg */)
{
    printf(LIGHT_RED"%s", __FUNCTION__);
    printf(NONE"\r\n");

    switch (aEvent->Type)
    {
    case DeviceEventType::kCHIPoBLEAdvertisingChange:
        break;
    case DeviceEventType::kCHIPoBLEConnectionClosed:
    case DeviceEventType::kFailSafeTimerExpired:
        break;
    case DeviceEventType::kThreadStateChange:
        break;
    case DeviceEventType::kThreadConnectivityChange:
        break;
    case DeviceEventType::kCHIPoBLEConnectionEstablished:
        break;
    case DeviceEventType::kCommissioningComplete:
        ChipLogProgress(NotSpecified, "kCommissioningComplete");
        break;
    // case DeviceEventType::kRemoveFabricEvent:
    //     if (chip::Server::GetInstance().GetFabricTable().FabricCount() == 0)
    //     {
    //         chip::DeviceLayer::SystemLayer().StartTimer(chip::System::Clock::Milliseconds32(1000), FactoryResetEventHandler, nullptr);
    //     }
    //     break;
    // case DeviceEventType::kOnOffAttributeChanged:
    //     break;
    // case DeviceEventType::kLevelControlAttributeChanged:
    //     break;
    // case DeviceEventType::kColorControlAttributeHSVChanged:
    //     break;
    // case DeviceEventType::kColorControlAttributeCTChanged:
    //     break;
    default:
        break;
    }
}

} // namespace

// REVISION DEFINITIONS:
// =================================================================================

#define ZCL_DESCRIPTOR_CLUSTER_REVISION (1u)

#define ZCL_BRIDGED_DEVICE_BASIC_INFORMATION_CLUSTER_REVISION (1u)
#define ZCL_BRIDGED_DEVICE_BASIC_INFORMATION_FEATURE_MAP (0u)

#define ZCL_FIXED_LABEL_CLUSTER_REVISION (1u)

#define ZCL_ON_OFF_CLUSTER_REVISION (4u)

#define ZCL_LEVEL_CLUSTER_REVISION (5u)

#define ZCL_COLOR_TEMPERATURE_CLUSTER_REVISION (5u)
#define ZCL_COLOR_TEMPERATURE_FEATURE_MAP (17u)

#define ZCL_TEMPERATURE_SENSOR_CLUSTER_REVISION (1u)
#define ZCL_TEMPERATURE_SENSOR_FEATURE_MAP (0u)

#define ZCL_POWER_SOURCE_CLUSTER_REVISION (2u)

// ---------------------------------------------------------------------------

int RemoveDeviceEndpoint(Device * dev)
{
    printf(LIGHT_RED"%s", __FUNCTION__);
    printf(NONE"\r\n");
    uint8_t index = 0;
    while (index < CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT)
    {
        if (gDevices[index] == dev)
        {
            // Todo: Update this to schedule the work rather than use this lock
            DeviceLayer::StackLock lock;
            EndpointId ep   = emberAfClearDynamicEndpoint(index);
            gDevices[index] = nullptr;
            ChipLogProgress(DeviceLayer, "Removed device %s from dynamic endpoint %d (index=%d)", dev->GetName(), ep, index);
            // Silence complaints about unused ep when progress logging
            // disabled.
            UNUSED_VAR(ep);
            return index;
        }
        index++;
    }
    return -1;
}

std::vector<EndpointListInfo> GetEndpointListInfo(chip::EndpointId parentId)
{
    printf(LIGHT_RED"%s", __FUNCTION__);
    printf(NONE"\r\n");
    std::vector<EndpointListInfo> infoList;

    for (auto room : gRooms)
    {
        if (room->getIsVisible())
        {
            EndpointListInfo info(room->getEndpointListId(), room->getName(), room->getType());
            int index = 0;
            while (index < CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT)
            {
                if ((gDevices[index] != nullptr) && (gDevices[index]->GetParentEndpointId() == parentId))
                {
                    std::string location;
                    if (room->getType() == Actions::EndpointListTypeEnum::kZone)
                    {
                        location = gDevices[index]->GetZone();
                    }
                    else
                    {
                        location = gDevices[index]->GetLocation();
                    }
                    if (room->getName().compare(location) == 0)
                    {
                        info.AddEndpointId(gDevices[index]->GetEndpointId());
                    }
                }
                index++;
            }
            if (info.GetEndpointListSize() > 0)
            {
                infoList.push_back(info);
            }
        }
    }

    return infoList;
}

std::vector<Action *> GetActionListInfo(chip::EndpointId parentId)
{
    printf(LIGHT_RED"%s", __FUNCTION__);
    printf(NONE"\r\n");
    return gActions;
}

EmberAfStatus HandleReadBridgedDeviceBasicAttribute(Device * dev, chip::AttributeId attributeId, uint8_t * buffer,
                                                    uint16_t maxReadLength)
{
    printf(LIGHT_RED"%s", __FUNCTION__);
    printf(NONE"\r\n");
    using namespace BridgedDeviceBasicInformation::Attributes;

    ChipLogProgress(DeviceLayer, "HandleReadBridgedDeviceBasicAttribute: attrId=%d, maxReadLength=%d", attributeId, maxReadLength);

    if ((attributeId == Reachable::Id) && (maxReadLength == 1))
    {
        *buffer = dev->IsReachable() ? 1 : 0;
    }
    else if ((attributeId == NodeLabel::Id) && (maxReadLength == 32))
    {
        MutableByteSpan zclNameSpan(buffer, maxReadLength);
        MakeZclCharString(zclNameSpan, dev->GetName());
    }
    else if ((attributeId == ClusterRevision::Id) && (maxReadLength == 2))
    {
        uint16_t rev = ZCL_BRIDGED_DEVICE_BASIC_INFORMATION_CLUSTER_REVISION;
        memcpy(buffer, &rev, sizeof(rev));
    }
    else if ((attributeId == FeatureMap::Id) && (maxReadLength == 4))
    {
        uint32_t featureMap = ZCL_BRIDGED_DEVICE_BASIC_INFORMATION_FEATURE_MAP;
        memcpy(buffer, &featureMap, sizeof(featureMap));
    }
    else
    {
        return EMBER_ZCL_STATUS_FAILURE;
    }

    return EMBER_ZCL_STATUS_SUCCESS;
}

EmberAfStatus HandleReadBooleanAttribute(DeviceLight * dev,
                                         chip::AttributeId attributeId,
                                         uint8_t * buffer,
                                         uint16_t maxReadLength)
{
    // printf(LIGHT_RED"%s", __FUNCTION__);
    // printf(NONE"\r\n");
    // ChipLogProgress(DeviceLayer, "HandleReadBooleanAttribute: attrId=%d, maxReadLength=%d", attributeId, maxReadLength);

    uint16_t endpointIndex  = emberAfGetDynamicIndexFromEndpoint(dev->GetEndpointId());

    if (attributeId == BooleanState::Attributes::StateValue::Id)
    {
        *buffer = ContactSensor[endpointIndex]->IsOpen();
    }
    return EMBER_ZCL_STATUS_SUCCESS;
}

EmberAfStatus HandleWriteBooleanAttribute(DeviceLight * dev, chip::AttributeId attributeId, uint8_t * buffer)
{
    // printf(LIGHT_RED"%s", __FUNCTION__);
    // printf(NONE"\r\n");
    // ChipLogProgress(DeviceLayer, "HandleWriteBooleanAttribute: attrId=%d", attributeId);

    uint16_t endpointIndex  = emberAfGetDynamicIndexFromEndpoint(dev->GetEndpointId());

    if ((attributeId == BooleanState::Attributes::StateValue::Id) && (dev->IsReachable()))
    {
        ContactSensor[endpointIndex]->Set(*buffer);

        printf(LIGHT_RED"status: %s", *buffer ? "Close" : "Open");
        printf(NONE"\r\n");
        
    }
    else
    {
        return EMBER_ZCL_STATUS_FAILURE;
    }

    return EMBER_ZCL_STATUS_SUCCESS;
}

EmberAfStatus HandleWriteOnOffAttribute(DeviceLight * dev, chip::AttributeId attributeId, uint8_t * buffer)
{
    // printf(LIGHT_RED"%s", __FUNCTION__);
    // printf(NONE"\r\n");
    // ChipLogProgress(DeviceLayer, "HandleWriteOnOffAttribute: attrId=%d", attributeId);

    if ((attributeId == OnOff::Attributes::OnOff::Id) && (dev->IsReachable()))
    {
        bool OnOff;

        OnOff = *buffer ? true : false;
        dev->SetOnOff(OnOff);
    }
    else
    {
        return EMBER_ZCL_STATUS_FAILURE;
    }

    return EMBER_ZCL_STATUS_SUCCESS;
}

EmberAfStatus HandleWriteLevelControAttribute(DeviceLight * dev, chip::AttributeId attributeId, uint8_t * buffer)
{
    // printf(LIGHT_RED"%s", __FUNCTION__);
    // printf(NONE"\r\n");
    // ChipLogProgress(DeviceLayer, "HandleWriteLevelControAttribute: attrId=%d", attributeId);

    if ((attributeId == LevelControl::Attributes::CurrentLevel::Id) && (dev->IsReachable()))
    {
        uint8_t CurrentLevel = buffer[0];
        dev->SetCurrentLevel(CurrentLevel);
    }
    else
    {
        return EMBER_ZCL_STATUS_FAILURE;
    }

    return EMBER_ZCL_STATUS_SUCCESS;
}

EmberAfStatus HandleWriteColorControlAttribute(DeviceLight * dev, chip::AttributeId attributeId, uint8_t * buffer)
{
    // printf(LIGHT_RED"%s, attribute: %d", __FUNCTION__, attributeId);
    // printf(NONE"\r\n");
    // ChipLogProgress(DeviceLayer, "HandleWriteColorControAttribute: attrId=%d", attributeId);

    if ((attributeId == ColorControl::Attributes::CurrentHue::Id) && (dev->IsReachable()))
    {
        uint8_t CurrentHue = buffer[0];
        dev->SetCurrentHue(CurrentHue);
    }
    else if ((attributeId == ColorControl::Attributes::CurrentSaturation::Id) && (dev->IsReachable()))
    {
        uint8_t CurrentSaturation = buffer[0];
        dev->SetCurrentSaturation(CurrentSaturation);
    }
    else if ((attributeId == ColorControl::Attributes::ColorTemperatureMireds::Id) && (dev->IsReachable()))
    {
        uint16_t ColorTemperatureMireds = buffer[0] | buffer[1] << 8;
        dev->SetColorTemperatureMireds(ColorTemperatureMireds);
    }
    else if ((attributeId == ColorControl::Attributes::ColorMode::Id) && (dev->IsReachable()))
    {
        uint8_t ColorMode = buffer[0];
        dev->SetColorMode(ColorMode);
    }
    else
    {
        return EMBER_ZCL_STATUS_FAILURE;
    }

    return EMBER_ZCL_STATUS_SUCCESS;
}

EmberAfStatus HandleReadOnOffAttribute(DeviceLight * dev,
                                       chip::AttributeId attributeId,
                                       uint8_t * buffer,
                                       uint16_t maxReadLength)
{
    // printf(LIGHT_RED"%s", __FUNCTION__);
    // printf(NONE"\r\n");
    // ChipLogProgress(DeviceLayer, "HandleReadOnOffAttribute: attrId=%d, maxReadLength=%d", attributeId, maxReadLength);

    if ((attributeId == OnOff::Attributes::OnOff::Id) && (maxReadLength == 1))
    {
        auto * path = Platform::New<app::ConcreteAttributePath>(dev->GetEndpointId(),
                                                                OnOff::Id,
                                                                OnOff::Attributes::OnOff::Id);
    
        wait_tx_sem.acquire();
        if (gw_cmd_read_req(buffer, reinterpret_cast<intptr_t>(path), maxReadLength))
        {
            uint8_t OnOff;

            memcpy(&OnOff, buffer, maxReadLength);
            dev->SetOnOffVal(OnOff);
            printf(LIGHT_RED"Read OnOff: %d", OnOff);
            printf(NONE"\r\n");
        }
    }
    else if ((attributeId == OnOff::Attributes::ClusterRevision::Id) && (maxReadLength == 2))
    {
        uint16_t rev = ZCL_ON_OFF_CLUSTER_REVISION;
        memcpy(buffer, &rev, sizeof(rev));
    }
    else
    {
        return EMBER_ZCL_STATUS_FAILURE;
    }

    return EMBER_ZCL_STATUS_SUCCESS;
}

EmberAfStatus HandleReadLevelControlAttribute(DeviceLight * dev,
                                              chip::AttributeId attributeId,
                                              uint8_t * buffer,
                                              uint16_t maxReadLength)
{
    // printf(LIGHT_RED"%s", __FUNCTION__);
    // printf(NONE"\r\n");
    // ChipLogProgress(DeviceLayer, "HandleReadLevelControlAttribute: attrId=%d, maxReadLength=%d", attributeId, maxReadLength);

    if ((attributeId == Clusters::LevelControl::Attributes::CurrentLevel::Id) && (maxReadLength == 1))
    {
        auto * path = Platform::New<app::ConcreteAttributePath>(dev->GetEndpointId(),
                                                                LevelControl::Id,
                                                                LevelControl::Attributes::CurrentLevel::Id);

        wait_tx_sem.acquire();
        if (gw_cmd_read_req(buffer, reinterpret_cast<intptr_t>(path), maxReadLength))
        {
            uint8_t CurrentLevel;

            memcpy(&CurrentLevel, buffer, maxReadLength);
            dev->SetCurrentLevelVal(CurrentLevel);
            printf(LIGHT_RED"Read CurrentLevel: %d", CurrentLevel);
            printf(NONE"\r\n");        
        }
    }
    else if ((attributeId == Clusters::LevelControl::Attributes::MinLevel::Id) && (maxReadLength == 1))
    {
        *buffer = 0x1;
    }
    else if ((attributeId == Clusters::LevelControl::Attributes::MaxLevel::Id) && (maxReadLength == 1))
    {
        *buffer = 0xFE;
    }
    return EMBER_ZCL_STATUS_SUCCESS;
}

EmberAfStatus HandleReadColorControlAttribute(DeviceLight * dev,
                                              chip::AttributeId attributeId,
                                              uint8_t * buffer,
                                              uint16_t maxReadLength)
{
    // printf(LIGHT_RED"%s", __FUNCTION__);
    // printf(NONE"\r\n");
    // ChipLogProgress(DeviceLayer, "HandleReadColorControlAttribute: attrId=%d, maxReadLength=%d", attributeId, maxReadLength);

    if ((attributeId == Clusters::ColorControl::Attributes::CurrentHue::Id) && (maxReadLength == 1))
    {
        auto * path = Platform::New<app::ConcreteAttributePath>(dev->GetEndpointId(),
                                                                ColorControl::Id,
                                                                ColorControl::Attributes::CurrentHue::Id);
        
        wait_tx_sem.acquire();
        if (gw_cmd_read_req(buffer, reinterpret_cast<intptr_t>(path), maxReadLength))
        {
            uint8_t CurrentHue;

            memcpy(&CurrentHue, buffer, maxReadLength);
            dev->SetCurrentHueVal(CurrentHue);
            printf(LIGHT_RED"Read CurrentHue: %d", CurrentHue);
            printf(NONE"\r\n");        
        }
    }
    else if ((attributeId == Clusters::ColorControl::Attributes::CurrentSaturation::Id) && (maxReadLength == 1))
    {
        auto * path = Platform::New<app::ConcreteAttributePath>(dev->GetEndpointId(),
                                                                ColorControl::Id,
                                                                ColorControl::Attributes::CurrentSaturation::Id);

        wait_tx_sem.acquire();
        if (gw_cmd_read_req(buffer, reinterpret_cast<intptr_t>(path), maxReadLength))
        {
            uint8_t CurrentSaturation;

            memcpy(&CurrentSaturation, buffer, maxReadLength);
            dev->SetCurrentSaturationVal(CurrentSaturation);
            printf(LIGHT_RED"Read CurrentSaturation: %d", CurrentSaturation);
            printf(NONE"\r\n");        
        }
    }
    else if ((attributeId == Clusters::ColorControl::Attributes::ColorTemperatureMireds::Id) && (maxReadLength == 2))
    {
        auto * path = Platform::New<app::ConcreteAttributePath>(dev->GetEndpointId(),
                                                                ColorControl::Id,
                                                                ColorControl::Attributes::ColorTemperatureMireds::Id);

        wait_tx_sem.acquire();
        if (gw_cmd_read_req(buffer, reinterpret_cast<intptr_t>(path), maxReadLength))
        {
            uint16_t ColorTemperatureMireds;

            memcpy(&ColorTemperatureMireds, buffer, maxReadLength);
            dev->SetColorTemperatureMiredsVal(ColorTemperatureMireds);
            printf(LIGHT_RED"Read ColorTemperatureMireds: %d", ColorTemperatureMireds);
            printf(NONE"\r\n");        
        }
    }
    else if ((attributeId == Clusters::ColorControl::Attributes::RemainingTime::Id) && (maxReadLength == 2))
    {
        uint16_t RemainingTime = 0x0;
        memcpy(buffer, &RemainingTime, sizeof(RemainingTime));
    }
    else if ((attributeId == Clusters::ColorControl::Attributes::ColorMode::Id) && (maxReadLength == 1))
    {
        auto * path = Platform::New<app::ConcreteAttributePath>(dev->GetEndpointId(),
                                                                ColorControl::Id,
                                                                ColorControl::Attributes::ColorMode::Id);

        wait_tx_sem.acquire();
        if (gw_cmd_read_req(buffer, reinterpret_cast<intptr_t>(path), maxReadLength))
        {
            uint8_t ColorMode;

            memcpy(&ColorMode, buffer, maxReadLength);
            printf(LIGHT_RED"Read ColorMode: %d", ColorMode);
            printf(NONE"\r\n");        
        }
    }
    else if ((attributeId == Clusters::ColorControl::Attributes::ColorCapabilities::Id) && (maxReadLength == 2))
    {
        uint16_t ColorCapabilities = 0x11;
        memcpy(buffer, &ColorCapabilities, sizeof(ColorCapabilities));
    }
    else if ((attributeId == Clusters::ColorControl::Attributes::ColorTempPhysicalMinMireds::Id) && (maxReadLength == 2))
    {
        uint16_t ColorTempPhysicalMinMireds = 153;
        memcpy(buffer, &ColorTempPhysicalMinMireds, sizeof(ColorTempPhysicalMinMireds));
    }
    else if ((attributeId == Clusters::ColorControl::Attributes::ColorTempPhysicalMaxMireds::Id) && (maxReadLength == 2))
    {
        uint16_t ColorTempPhysicalMaxMireds = 370;
        memcpy(buffer, &ColorTempPhysicalMaxMireds, sizeof(ColorTempPhysicalMaxMireds));
    }
    else if ((attributeId == Clusters::ColorControl::Attributes::StartUpColorTemperatureMireds::Id) && (maxReadLength == 2))
    {
        uint16_t StartUpColorTemperatureMireds = 153;
        memcpy(buffer, &StartUpColorTemperatureMireds, sizeof(StartUpColorTemperatureMireds));
    }
    else if ((attributeId == Clusters::ColorControl::Attributes::FeatureMap::Id) && (maxReadLength == 4))
    {
        uint32_t featureMap = ZCL_COLOR_TEMPERATURE_FEATURE_MAP;
        memcpy(buffer, &featureMap, sizeof(featureMap));
    }
    else if ((attributeId == Clusters::ColorControl::Attributes::ClusterRevision::Id) && (maxReadLength == 2))
    {
        uint16_t clusterRevision = ZCL_COLOR_TEMPERATURE_CLUSTER_REVISION;
        memcpy(buffer, &clusterRevision, sizeof(clusterRevision));
    }
    return EMBER_ZCL_STATUS_SUCCESS;
}

EmberAfStatus HandleReadTempMeasurementAttribute(DeviceTempSensor * dev, chip::AttributeId attributeId, uint8_t * buffer,
                                                 uint16_t maxReadLength)
{
    // printf(LIGHT_RED"%s", __FUNCTION__);
    // printf(NONE"\r\n");
    using namespace TemperatureMeasurement::Attributes;

    if ((attributeId == MeasuredValue::Id) && (maxReadLength == 2))
    {
        int16_t measuredValue = dev->GetMeasuredValue();
        memcpy(buffer, &measuredValue, sizeof(measuredValue));
    }
    else if ((attributeId == MinMeasuredValue::Id) && (maxReadLength == 2))
    {
        int16_t minValue = dev->mMin;
        memcpy(buffer, &minValue, sizeof(minValue));
    }
    else if ((attributeId == MaxMeasuredValue::Id) && (maxReadLength == 2))
    {
        int16_t maxValue = dev->mMax;
        memcpy(buffer, &maxValue, sizeof(maxValue));
    }
    else if ((attributeId == FeatureMap::Id) && (maxReadLength == 4))
    {
        uint32_t featureMap = ZCL_TEMPERATURE_SENSOR_FEATURE_MAP;
        memcpy(buffer, &featureMap, sizeof(featureMap));
    }
    else if ((attributeId == ClusterRevision::Id) && (maxReadLength == 2))
    {
        uint16_t clusterRevision = ZCL_TEMPERATURE_SENSOR_CLUSTER_REVISION;
        memcpy(buffer, &clusterRevision, sizeof(clusterRevision));
    }
    else
    {
        return EMBER_ZCL_STATUS_FAILURE;
    }

    return EMBER_ZCL_STATUS_SUCCESS;
}

EmberAfStatus emberAfExternalAttributeReadCallback(EndpointId endpoint, ClusterId clusterId,
                                                   const EmberAfAttributeMetadata * attributeMetadata, uint8_t * buffer,
                                                   uint16_t maxReadLength)
{
    // printf(LIGHT_RED"%s", __FUNCTION__);
    // printf(NONE"\r\n");
    uint16_t endpointIndex = emberAfGetDynamicIndexFromEndpoint(endpoint);

    ChipLogProgress(DeviceLayer, "endpoint: %d, index: %d, cluster: %d, attribute: %d", 
        endpoint, endpointIndex, clusterId, attributeMetadata->attributeId);

    EmberAfStatus ret = EMBER_ZCL_STATUS_FAILURE;

    if ((endpointIndex < CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT) && (gDevices[endpointIndex] != nullptr))
    {
        Device * dev = gDevices[endpointIndex];

        if (clusterId == BridgedDeviceBasicInformation::Id)
        {
            ret = HandleReadBridgedDeviceBasicAttribute(dev, attributeMetadata->attributeId, buffer, maxReadLength);
        }
        else if (clusterId == OnOff::Id)
        {
            ret = HandleReadOnOffAttribute(static_cast<DeviceLight *>(dev), attributeMetadata->attributeId, buffer, maxReadLength);
        }
        else if (clusterId == LevelControl::Id)
        {
            ret = HandleReadLevelControlAttribute(static_cast<DeviceLight *>(dev), attributeMetadata->attributeId, buffer, maxReadLength);
        }
        else if (clusterId == ColorControl::Id)
        {
            ret = HandleReadColorControlAttribute(static_cast<DeviceLight *>(dev), attributeMetadata->attributeId, buffer, maxReadLength);     
        }
        else if (clusterId == TemperatureMeasurement::Id)
        {
            ret = HandleReadTempMeasurementAttribute(static_cast<DeviceTempSensor *>(dev), attributeMetadata->attributeId, buffer,
                                                     maxReadLength);
        }
        else if (clusterId == BooleanState::Id)
        {
            ret = HandleReadBooleanAttribute(static_cast<DeviceLight *>(dev), attributeMetadata->attributeId, buffer,
                                                     maxReadLength);
        }
    }

    return ret;
}

class BridgedPowerSourceAttrAccess : public AttributeAccessInterface
{
public:
    // Register on all endpoints.
    BridgedPowerSourceAttrAccess() : AttributeAccessInterface(Optional<EndpointId>::Missing(), PowerSource::Id) {}

    CHIP_ERROR
    Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder) override
    {
        printf(LIGHT_RED"%s", __FUNCTION__);
        printf(NONE"\r\n");
        uint16_t powerSourceDeviceIndex = CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT;

        if ((gDevices[powerSourceDeviceIndex] != nullptr))
        {
            DevicePowerSource * dev = static_cast<DevicePowerSource *>(gDevices[powerSourceDeviceIndex]);
            if (aPath.mEndpointId != dev->GetEndpointId())
            {
                return CHIP_IM_GLOBAL_STATUS(UnsupportedEndpoint);
            }
            switch (aPath.mAttributeId)
            {
            case PowerSource::Attributes::BatChargeLevel::Id:
                aEncoder.Encode(dev->GetBatChargeLevel());
                break;
            case PowerSource::Attributes::Order::Id:
                aEncoder.Encode(dev->GetOrder());
                break;
            case PowerSource::Attributes::Status::Id:
                aEncoder.Encode(dev->GetStatus());
                break;
            case PowerSource::Attributes::Description::Id:
                aEncoder.Encode(chip::CharSpan(dev->GetDescription().c_str(), dev->GetDescription().size()));
                break;
            case PowerSource::Attributes::EndpointList::Id: {
                std::vector<chip::EndpointId> & list = dev->GetEndpointList();
                DataModel::List<EndpointId> dm_list(chip::Span<chip::EndpointId>(list.data(), list.size()));
                aEncoder.Encode(dm_list);
                break;
            }
            case PowerSource::Attributes::ClusterRevision::Id:
                aEncoder.Encode(ZCL_POWER_SOURCE_CLUSTER_REVISION);
                break;
            case PowerSource::Attributes::FeatureMap::Id:
                aEncoder.Encode(dev->GetFeatureMap());
                break;
            case PowerSource::Attributes::BatReplacementNeeded::Id:
                aEncoder.Encode(false);
                break;
            case PowerSource::Attributes::BatReplaceability::Id:
                aEncoder.Encode(PowerSource::BatReplaceabilityEnum::kNotReplaceable);
                break;
            default:
                return CHIP_IM_GLOBAL_STATUS(UnsupportedAttribute);
            }
        }
        return CHIP_NO_ERROR;
    }
};

BridgedPowerSourceAttrAccess gPowerAttrAccess;

EmberAfStatus emberAfExternalAttributeWriteCallback(EndpointId endpoint, ClusterId clusterId,
                                                    const EmberAfAttributeMetadata * attributeMetadata, uint8_t * buffer)
{
    // printf(LIGHT_RED"%s", __FUNCTION__);
    // printf(NONE"\r\n");
    uint16_t endpointIndex = emberAfGetDynamicIndexFromEndpoint(endpoint);

    EmberAfStatus ret = EMBER_ZCL_STATUS_FAILURE;

    // ChipLogProgress(DeviceLayer, "emberAfExternalAttributeWriteCallback: ep=%d", endpoint);

    if (endpointIndex < CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT)
    {
        Device * dev = gDevices[endpointIndex];

        if ((dev->IsReachable()) && (clusterId == OnOff::Id))
        {
            ret = HandleWriteOnOffAttribute(static_cast<DeviceLight *>(dev), attributeMetadata->attributeId, buffer);
        }
        else if ((dev->IsReachable()) && (clusterId == BooleanState::Id))
        {
            ret = HandleWriteBooleanAttribute(static_cast<DeviceLight *>(dev), attributeMetadata->attributeId, buffer);
        }
        else if ((dev->IsReachable()) && (clusterId == LevelControl::Id))
        {
            ret = HandleWriteLevelControAttribute(static_cast<DeviceLight *>(dev), attributeMetadata->attributeId, buffer);
        }
        else if ((dev->IsReachable()) && (clusterId == ColorControl::Id))
        {
            ret = HandleWriteColorControlAttribute(static_cast<DeviceLight *>(dev), attributeMetadata->attributeId, buffer);
        }
    }

    return ret;
}

bool emberAfActionsClusterInstantActionCallback(app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
                                                const Actions::Commands::InstantAction::DecodableType & commandData)
{
    printf(LIGHT_RED"%s", __FUNCTION__);
    printf(NONE"\r\n");
    commandObj->AddStatus(commandPath, Protocols::InteractionModel::Status::NotFound);
    return true;
}

void ApplicationInit()
{
    Read_EndDevice_File();
    Check_Dev_Info();
    Open_Socket();
    add_ep = true;
}

void ApplicationShutdown()
{

}
 
// #define POLL_INTERVAL_MS (100)
// uint8_t poll_prescale = 0;
// const int16_t oneDegree = 100;

void UpdateClusterStateInternal(intptr_t arg)
{
    ias_zone_notify_t *resp = reinterpret_cast<ias_zone_notify_t *>(arg);

    printf(LIGHT_RED"endpoint: %02x, ", resp->endpoint);
    printf(LIGHT_RED"status: %s", resp->status ? "True" : "False");
    printf(NONE"\r\n");
    app::Clusters::BooleanState::Attributes::StateValue::Set(resp->endpoint, !resp->status);
}

void rx_zigbee_gateway_processing(gw_command_t queue)
{
    uint32_t commandId;

    commandId = (queue.buffer[5] << 0)   |
                (queue.buffer[6] << 8)   |
                (queue.buffer[7] << 16)  |
                (queue.buffer[8] << 24);

    printf(LIGHT_RED"recv command: ");
    for (int i = 0; i < queue.len; i++)
    {
        printf("%02x ", queue.buffer[i]);
    }
    printf(NONE"\r\n");

    switch (commandId)
    {
    case 0x00028800:
        uint16_t temperature;

        temperature = queue.buffer[16] | queue.buffer[17] << 8;
        // TempSensor[0]->SetMeasuredValue(temperature);
        ChipLogProgress(DeviceLayer, "Temp: %f", (float)temperature / 100.0);
        break;
    case 0x00230000:
        ChipLogProgress(DeviceLayer, "IAS");
        uint16_t shortaddr;
        ias_zone_notify_t resp;

        shortaddr = queue.buffer[9] | queue.buffer[10] << 8;

        for (auto dev : bridge_device)
        {
            if (dev.ShortAddress == shortaddr)
            {
                resp.endpoint = dev.endpoint;
                resp.status = (queue.buffer[13] & 0x1) ? true : false;
            }
        }

        PlatformMgr().ScheduleWork(UpdateClusterStateInternal, reinterpret_cast<intptr_t>(&resp));
        break;
    case 0x00018800:
        ChipLogProgress(DeviceLayer, "Default response");
        wait_tx_sem.release();
        break;
    case 0x00028000:
        ChipLogProgress(DeviceLayer, "Read attribute response");
        gw_read_response_queue.push(queue);
        wait_tx_sem.release();
        read_attribute_sem[read_attribute_sem_count]->sem.release();
        delete read_attribute_sem[read_attribute_sem_count];
    default:
        break;
    }
}

void rx_queue_process_handler(void)
{
    gw_command_t queue;

    // using namespace std::chrono_literals;

    while (true)
    {
        if (!gw_rx_command_queue.empty())
        {
            queue = gw_rx_command_queue.front();
            rx_zigbee_gateway_processing(queue);
            gw_rx_command_queue.pop();
        }

        // std::this_thread::sleep_for(100ms);
        std::this_thread::yield();
    }
}

void tx_queue_process_handler(void)
{
    gw_command_t queue;

    // using namespace std::chrono_literals;

    while (true)
    {
        if (!gw_tx_command_queue.empty())
        {
            queue = gw_tx_command_queue.front();

            send(sockfd, queue.buffer, queue.len, 0);
            gw_tx_command_queue.pop();
        }

        // std::this_thread::sleep_for(100ms);
        std::this_thread::yield();
    }
}

void add_device_thread_handler(void)
{
    // using namespace std::chrono_literals;

    while (true)
    {
        if (add_ep)
        {
            for (auto dev : bridge_device)
            {
                int index = dev.endpoint - gFirstDynamicEndpointId;

                if (dev.clusterId == OnOff::Id)
                {
                    std::string node = "Light " + std::to_string(index);

                    Light[index] = new DeviceLight(node.c_str(), "Office");
                    Light[index]->SetReachable(true);
                    Light[index]->SetChangeCallback(&HandleDeviceLightStatusChanged);

                    AddDeviceEndpoint(Light[index], &bridgedLightEndpoint, Span<const EmberAfDeviceType>(gBridgedOnOffDeviceTypes),
                        Span<DataVersion>(gLightDataVersions), 1);
                }
                else if (dev.clusterId == LevelControl::Id)
                {
                    std::string node = "Dimming " + std::to_string(index);

                    Light[index] = new DeviceLight(node.c_str(), "Office");
                    Light[index]->SetReachable(true);
                    Light[index]->SetChangeCallback(&HandleDeviceLightStatusChanged);

                    AddDeviceEndpoint(Light[index], &bridgedDimmingEndpoint, Span<const EmberAfDeviceType>(gBridgedDimmingDeviceTypes),
                        Span<DataVersion>(gDimmingDataVersions), 1);
                }
                else if (dev.clusterId == ColorControl::Id)
                {
                    std::string node = "ColorLight " + std::to_string(index);

                    Light[index] = new DeviceLight(node.c_str(), "Office");
                    Light[index]->SetReachable(true);
                    Light[index]->SetChangeCallback(&HandleDeviceLightStatusChanged);

                    AddDeviceEndpoint(Light[index], &bridgedColorTemperatureEndpoint, Span<const EmberAfDeviceType>(gBridgedColorTemperatureDeviceTypes),
                        Span<DataVersion>(gColorTemperatureDataVersions), 1);
                }
                else if (dev.clusterId == TemperatureMeasurement::Id)
                {
                    std::string node = "TempSensor " + std::to_string(index);

                    TempSensor[index] = new DeviceTempSensor(node.c_str(), "Office", minMeasuredValue, maxMeasuredValue, initialMeasuredValue);
                    TempSensor[index]->SetReachable(true);
                    TempSensor[index]->SetChangeCallback(&HandleDeviceTempSensorStatusChanged);

                    AddDeviceEndpoint(TempSensor[index], &bridgedTempSensorEndpoint, Span<const EmberAfDeviceType>(gBridgedTempSensorDeviceTypes),
                        Span<DataVersion>(gTempSensorDataVersions), 1);
                }
                else if (dev.clusterId == 0x0500)
                {
                    std::string node = "ContactSensor " + std::to_string(index);

                    ContactSensor[index] = new DeviceContactSensor(node.c_str(), "Office");
                    ContactSensor[index]->SetReachable(true);
                    ContactSensor[index]->SetChangeCallback(&HandleDeviceConatactSensorStatusChanged);

                    AddDeviceEndpoint(ContactSensor[index], &bridgedContactSensorEndpoint, Span<const EmberAfDeviceType>(gBridgedContactSensorDeviceTypes),
                        Span<DataVersion>(gContactSensorDataVersions), 1);
                }
            }
            add_ep = false;
        }

        // std::this_thread::sleep_for(100ms);
        std::this_thread::yield();
    }
}

void tcp_rx_thread_hanlder(void)
{
    uint8_t buffer[1024];
    gw_command_t queue;
    ssize_t  nbytes;
    // uint32_t commandId;

    // using namespace std::chrono_literals;

    while (true)
    {
        nbytes = recv(sockfd, buffer, 1024, 0);
        if (nbytes > 0)
        {
            queue.len = (uint32_t)nbytes;
            memcpy(queue.buffer, buffer, nbytes);
            gw_rx_command_queue.push(queue);
        }
        // std::this_thread::sleep_for(100ms);
        std::this_thread::yield();
    }
}

int main(int argc, char * argv[])
{
    // Clear out the device database
    memset(gDevices, 0, sizeof(gDevices));
    if (ChipLinuxAppInit(argc, argv) != 0) { return -1; }

    // Init Data Model and CHIP App Server
    static chip::CommonCaseDeviceServerInitParams initParams;
    (void) initParams.InitializeStaticResourcesBeforeServerInit();

#if CHIP_DEVICE_ENABLE_PORT_PARAMS
    // use a different service port to make testing possible with other sample devices running on same host
    initParams.operationalServicePort = LinuxDeviceOptions::GetInstance().securedDevicePort;
#endif

    initParams.interfaceId = LinuxDeviceOptions::GetInstance().interfaceId;
    chip::Server::GetInstance().Init(initParams);

    // Initialize device attestation config
    SetDeviceAttestationCredentialsProvider(Examples::GetExampleDACProvider());

    // Set starting endpoint id where dynamic endpoints will be assigned, which
    // will be the next consecutive endpoint id after the last fixed endpoint.
    gFirstDynamicEndpointId = static_cast<chip::EndpointId>(
        static_cast<int>(emberAfEndpointFromIndex(static_cast<uint16_t>(emberAfFixedEndpointCount() - 1))) + 1);
    gCurrentEndpointId = gFirstDynamicEndpointId;

    // Disable last fixed endpoint, which is used as a placeholder for all of the
    // supported clusters so that ZAP will generated the requisite code.
    emberAfEndpointEnableDisable(emberAfEndpointFromIndex(static_cast<uint16_t>(emberAfFixedEndpointCount() - 1)), false);

    std::thread add_device_thread(add_device_thread_handler);
    std::thread tcp_rx_thread(tcp_rx_thread_hanlder);
    std::thread rx_queue_process_thread(rx_queue_process_handler);
    std::thread tx_queue_process_thread(tx_queue_process_handler);

    // Run CHIP

    ApplicationInit();
    registerAttributeAccessOverride(&gPowerAttrAccess);
    chip::DeviceLayer::PlatformMgr().AddEventHandler(ChipEventHandler, 0);
    chip::DeviceLayer::PlatformMgr().RunEventLoop();

    return 0;
}

