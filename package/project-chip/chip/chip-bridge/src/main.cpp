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

#include <pthread.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <semaphore.h> 
#include <thread>
#include <chrono>
// #include <semaphore>
// #include <queue>
// #include <vector>

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
    unsigned char ep;
    unsigned short devidId;
    unsigned short clusterCounts;
    unsigned short clusterID[ClusterIDMax];
};

struct _EndDevice
{
    unsigned char  MacAddress[8];   //Mac Address(64bits)
    unsigned char  Active;	
    unsigned short ShortAddress;    //Short Address(16bits)
    unsigned char ep_counts;
    struct _EndPoint ep_list[EndPointMax];
};

struct _Coordinator
{
    unsigned char  MacAddress[8];   //Mac Address(64bits)
    unsigned short PANID;
    unsigned short DevCount;
    unsigned short ARCount;
    unsigned char  CHANNEL;
    unsigned char  EXT_PAN_ID[8];
};

struct _MatterEndDevice
{
    EndpointId  endpoint;
    ClusterId   clusterId;
    AttributeId attributeId;
    // _EndDevice  ED;
    unsigned char ep;
    unsigned short ShortAddress;
};

typedef struct gw_command
{   
    unsigned char buffer[1024];
    int len;
} gw_command_t;

std::queue<gw_command_t> gw_rx_command_queue;
std::queue<gw_command_t> gw_tx_command_queue;
std::queue<gw_command_t> gw_tx_response_queue;

sem_t wait_tx_sem, read_attribute_sem; 

struct _EndDevice ED[EndDeviceMax];
struct _Coordinator CR;
struct _MatterEndDevice MED[EndDeviceMax];

const char EndDevice_Filename[] = "/usr/local/var/lib/ez-zbgw/zbdb/sc_enddevice.dat";
const char Coordinator_Filename[] = "/usr/local/var/lib/ez-zbgw/zbdb/sc_coordinator.dat";

// int light_id = 0;
int device_index = 0;
int light_count = 0;
int tempsensor_count = 0;
bool add_ep = true;
int sockfd = 0;

DeviceOnOff *Light[EndDeviceMax];
DeviceTempSensor *TempSensor[EndDeviceMax];

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

#define DEVICE_TYPE_BRIDGED_NODE    0x0013
#define DEVICE_TYPE_LO_ON_OFF_LIGHT 0x0100
#define DEVICE_TYPE_POWER_SOURCE    0x0011
#define DEVICE_TYPE_TEMP_SENSOR     0x0302
#define DEVICE_VERSION_DEFAULT      1

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

// Declare On/Off cluster attributes
DECLARE_DYNAMIC_ATTRIBUTE_LIST_BEGIN(onOffAttrs)
DECLARE_DYNAMIC_ATTRIBUTE(OnOff::Attributes::OnOff::Id, BOOLEAN, 1, 0), /* on/off */
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

namespace {

static void gw_cmd_read_attr_req(intptr_t arg)
{
    // ChipLogProgress(DeviceLayer, "\n\n**** %s\n\n", __FUNCTION__);
    printf(LIGHT_RED"%s", __FUNCTION__);
    printf(NONE"\r\n");

    auto path = reinterpret_cast<app::ConcreteAttributePath *>(arg);

    EndpointId endpoint     = path->mEndpointId;
    ClusterId clusterId     = path->mClusterId;
    AttributeId attributeId = path->mAttributeId;

    Platform::Delete(path);

    uint16_t endpointIndex  = emberAfGetDynamicIndexFromEndpoint(endpoint);

    // ChipLogProgress(DeviceLayer, "Matter Endpoint: %x", endpoint);
    // ChipLogProgress(DeviceLayer, "Matter Cluster: %x", clusterId);
    // ChipLogProgress(DeviceLayer, "Matter Attribute: %x", attributeId);
    // ChipLogProgress(DeviceLayer, "Zigbee Address: %x", MED[endpointIndex].ShortAddress);   
    // ChipLogProgress(DeviceLayer, "Zigbee Endpoint: %x", MED[endpointIndex].ep);

    uint8_t cmd[] = {0xFF, 0xFC, 0xFC, 0xFF, 
                     0x0C, 
                     0x00, 0x00, 0x02, 0x00, 
                     0x00, 0x00, 
                     0x00, 0x00, 
                     0x00, 0x00, 
                     0x00, 0x00, 
                     0x00}; 

    cmd[9] = (uint8_t)(MED[endpointIndex].ShortAddress & 0xFF); 
    cmd[10] = (uint8_t)((MED[endpointIndex].ShortAddress >> 8) & 0xFF); 
    cmd[12] = MED[endpointIndex].ep;
    cmd[13] = (uint8_t)(clusterId & 0xFF);
    cmd[14] = (uint8_t)((clusterId >> 8) & 0xFF);  
    cmd[15] = (uint8_t)(attributeId & 0xFF);
    cmd[16] = (uint8_t)((attributeId >> 8) & 0xFF);  
    
    printf(LIGHT_RED"send command: ");
    for (unsigned long i = 0; i < sizeof(cmd); i++)
    {
        printf("%02x ", cmd[i]);
    }
    printf(NONE"\r\n");

    gw_command_t queue;

    queue.len = sizeof(cmd);
    memcpy(queue.buffer, cmd, queue.len);

    // sem_wait(&wait_tx_sem);
    gw_tx_command_queue.push(queue);
}

static void gw_cmd_onoff_on_req(intptr_t arg)
{
    // ChipLogProgress(DeviceLayer, "\n\n**** %s\n\n", __FUNCTION__);
    printf(LIGHT_RED"%s", __FUNCTION__);
    printf(NONE"\r\n");

    auto path = reinterpret_cast<app::ConcreteAttributePath *>(arg);

    EndpointId endpoint     = path->mEndpointId;
    // ClusterId clusterId     = path->mClusterId;
    // AttributeId attributeId = path->mAttributeId;

    Platform::Delete(path);

    uint16_t endpointIndex  = emberAfGetDynamicIndexFromEndpoint(endpoint);

    uint8_t cmd[] = {0xFF, 0xFC, 0xFC, 0xFF, 0x09, 0x01, 0x00, 0x07, 0x00, 0xFF, 0xFF, 0x00, 0xFF, 0x00, 0xBF}; 

    cmd[9] = (uint8_t)(MED[endpointIndex].ShortAddress & 0xFF); 
    cmd[10] = (uint8_t)((MED[endpointIndex].ShortAddress >> 8) & 0xFF); 
    cmd[12] = MED[endpointIndex].ep;   
  
    printf(LIGHT_RED"send command: ");
    for (unsigned long i = 0; i < sizeof(cmd); i++)
    {
        printf("%02x ", cmd[i]);
    }
    printf(NONE"\r\n");

    gw_command_t queue;

    queue.len = sizeof(cmd);
    memcpy(queue.buffer, cmd, queue.len);

    // sem_wait(&wait_tx_sem);
    gw_tx_command_queue.push(queue); 
}

static void gw_cmd_onoff_off_req(intptr_t arg)
{
    // ChipLogProgress(DeviceLayer, "\n\n**** %s\n\n", __FUNCTION__);
    printf(LIGHT_RED"%s", __FUNCTION__);
    printf(NONE"\r\n");

    auto path = reinterpret_cast<app::ConcreteAttributePath *>(arg);

    EndpointId endpoint     = path->mEndpointId;
    // ClusterId clusterId     = path->mClusterId;
    // AttributeId attributeId = path->mAttributeId;

    Platform::Delete(path);

    uint16_t endpointIndex  = emberAfGetDynamicIndexFromEndpoint(endpoint);

    uint8_t cmd[] = {0xFF, 0xFC, 0xFC, 0xFF, 0x09, 0x00, 0x00, 0x07, 0x00, 0xFF, 0xFF, 0x00, 0xFF, 0x00, 0xBF}; 

    cmd[9] = (uint8_t)(MED[endpointIndex].ShortAddress & 0xFF); 
    cmd[10] = (uint8_t)((MED[endpointIndex].ShortAddress >> 8) & 0xFF); 
    cmd[12] = MED[endpointIndex].ep;    

    printf(LIGHT_RED"send command: ");
    for (unsigned long i = 0; i < sizeof(cmd); i++)
    {
        printf("%02x ", cmd[i]);
    }
    printf(NONE"\r\n");

    gw_command_t queue;

    queue.len = sizeof(cmd);
    memcpy(queue.buffer, cmd, queue.len);

    // sem_wait(&wait_tx_sem);
    gw_tx_command_queue.push(queue);  
}

// static void HandleZigbeeGatewayRequestCallback(intptr_t arg)
// {
//     ChipLogProgress(DeviceLayer, "\n\n**** %s\n\n", __FUNCTION__);

//     auto path = reinterpret_cast<app::ConcreteAttributePath *>(arg);

//     EndpointId endpoint     = path->mEndpointId;
//     ClusterId clusterId     = path->mClusterId;
//     AttributeId attributeId = path->mAttributeId;
//     uint16_t endpointIndex  = emberAfGetDynamicIndexFromEndpoint(endpoint);

//     ChipLogProgress(DeviceLayer, "Matter Endpoint: %x", endpoint);
//     ChipLogProgress(DeviceLayer, "Matter Cluster: %x", clusterId);
//     ChipLogProgress(DeviceLayer, "Matter Attribute: %x", attributeId);
//     ChipLogProgress(DeviceLayer, "Zigbee Address: %x", MED[endpointIndex].ShortAddress);   
//     ChipLogProgress(DeviceLayer, "Zigbee Endpoint: %x", MED[endpointIndex].ep);

//     switch (clusterId)
//     {
//     case OnOff::Id:
//         ChipLogProgress(DeviceLayer, "Matter OnOff: %s", Light[endpointIndex]->IsOn() ? "On" : "Off");
//         if (Light[endpointIndex]->IsOn() && Light[endpointIndex]->IsReachable())
//         {
//             gw_cmd_onoff_on_req(MED[endpointIndex].ShortAddress, MED[endpointIndex].ep);
//         }
//         else
//         {
//             gw_cmd_onoff_off_req(MED[endpointIndex].ShortAddress, MED[endpointIndex].ep);
//         }
//         break;
//     case LevelControl::Id:
//         break;
//     case ColorControl::Id:
//         break;
//     case BridgedDeviceBasicInformation::Id:
//         if (attributeId == BridgedDeviceBasicInformation::Attributes::Reachable::Id)
//         {
//             ChipLogProgress(DeviceLayer, "Matter changed Reachable");
//         }
//     default:
//         break;
//     }
//     Platform::Delete(path);
// }

void MatterReportingAttributeChangeCallback(const ConcreteAttributePath & aPath)
{
    // EndpointId endpoint     = aPath.mEndpointId;
    // ClusterId clusterId     = aPath.mClusterId;
    // AttributeId attributeId = aPath.mAttributeId;

    // ChipLogProgress(DeviceLayer, "\n\n**** %s\n\n", __FUNCTION__);
    printf(LIGHT_RED"%s", __FUNCTION__);
    printf(NONE"\r\n");
}

void CallReportingCallback(intptr_t closure)
{
    // ChipLogProgress(DeviceLayer, "\n\n**** %s\n\n", __FUNCTION__);
    printf(LIGHT_RED"%s", __FUNCTION__);
    printf(NONE"\r\n");
    auto path = reinterpret_cast<app::ConcreteAttributePath *>(closure);
    MatterReportingAttributeChangeCallback(*path);
    Platform::Delete(path);
}

void ScheduleReportingCallback(Device * dev, ClusterId cluster, AttributeId attribute)
{
    // ChipLogProgress(DeviceLayer, "\n\n**** %s\n\n", __FUNCTION__);
    printf(LIGHT_RED"%s", __FUNCTION__);
    printf(NONE"\r\n");
    auto * path = Platform::New<app::ConcreteAttributePath>(dev->GetEndpointId(), cluster, attribute);
    PlatformMgr().ScheduleWork(CallReportingCallback, reinterpret_cast<intptr_t>(path));
}

} // anonymous namespace

void HandleDeviceStatusChanged(Device * dev, Device::Changed_t itemChangedMask)
{
    // ChipLogProgress(DeviceLayer, "\n\n**** %s\n\n", __FUNCTION__);
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

void HandleDeviceOnOffStatusChanged(DeviceOnOff * dev, DeviceOnOff::Changed_t itemChangedMask)
{
    // ChipLogProgress(DeviceLayer, "\n\n**** %s\n\n", __FUNCTION__);
    printf(LIGHT_RED"%s", __FUNCTION__);
    printf(NONE"\r\n");
    if (itemChangedMask & (DeviceOnOff::kChanged_Reachable | DeviceOnOff::kChanged_Name | DeviceOnOff::kChanged_Location))
    {
        HandleDeviceStatusChanged(static_cast<Device *>(dev), (Device::Changed_t) itemChangedMask);
    }
    if (itemChangedMask & DeviceOnOff::kChanged_OnOff)
    {
        ScheduleReportingCallback(dev, OnOff::Id, OnOff::Attributes::OnOff::Id);

        auto * path = Platform::New<app::ConcreteAttributePath>(dev->GetEndpointId(), OnOff::Id, OnOff::Attributes::OnOff::Id);
        // PlatformMgr().ScheduleWork(HandleZigbeeGatewayRequestCallback, reinterpret_cast<intptr_t>(path));
        // EndpointId endpoint     = dev->GetEndpointId();
        // ClusterId clusterId     = OnOff::Id;
        // AttributeId attributeId = OnOff::Attributes::OnOff::Id;
        uint16_t endpointIndex  = emberAfGetDynamicIndexFromEndpoint(dev->GetEndpointId());

        sem_wait(&wait_tx_sem);
        if (Light[endpointIndex]->IsOn() && Light[endpointIndex]->IsReachable())
        {
            gw_cmd_onoff_on_req(reinterpret_cast<intptr_t>(path));
        }
        else
        {
            gw_cmd_onoff_off_req(reinterpret_cast<intptr_t>(path));
        }
        printf(LIGHT_RED"recv done");
        printf(NONE"\r\n");
    }
}

void HandleDeviceTempSensorStatusChanged(DeviceTempSensor * dev, DeviceTempSensor::Changed_t itemChangedMask)
{
    // ChipLogProgress(DeviceLayer, "\n\n**** %s\n\n", __FUNCTION__);
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
    // ChipLogProgress(DeviceLayer, "\n\n**** %s\n\n", __FUNCTION__);
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

void AddTempSensorEP(int tempsensor_number)
{
    // ChipLogProgress(DeviceLayer, "\n\n**** %s\n\n", __FUNCTION__);
    printf(LIGHT_RED"%s", __FUNCTION__);
    printf(NONE"\r\n");
    std::string node = "TempSensor " +  std::to_string(tempsensor_number + 1);
    std::string room = "Office";

    TempSensor[tempsensor_number] = new DeviceTempSensor(node.c_str(), room, minMeasuredValue, maxMeasuredValue, initialMeasuredValue);
    TempSensor[tempsensor_number]->SetReachable(true);
    TempSensor[tempsensor_number]->SetChangeCallback(&HandleDeviceTempSensorStatusChanged);

    AddDeviceEndpoint(TempSensor[tempsensor_number], &bridgedTempSensorEndpoint, Span<const EmberAfDeviceType>(gBridgedTempSensorDeviceTypes),
                      Span<DataVersion>(gTempSensorDataVersions), 1);
}

void AddLightEP(int light_number)
{
    // ChipLogProgress(DeviceLayer, "\n\n**** %s\n\n", __FUNCTION__);
    printf(LIGHT_RED"%s", __FUNCTION__);
    printf(NONE"\r\n");
    std::string node = "Light " +  std::to_string(light_number + 1);
    std::string room = "Office";

    Light[light_number] = new DeviceOnOff(node.c_str(), room);
    Light[light_number]->SetReachable(true);
    Light[light_number]->SetChangeCallback(&HandleDeviceOnOffStatusChanged);

    AddDeviceEndpoint(Light[light_number], &bridgedLightEndpoint, Span<const EmberAfDeviceType>(gBridgedOnOffDeviceTypes),
                      Span<DataVersion>(gLightDataVersions), 1);
};

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
                if (ED[i].ep_list[j].clusterID[k] == 0x0006)
                {
                    MED[device_index].ep = ED[i].ep_list[j].ep;
                    MED[device_index].ShortAddress = ED[i].ShortAddress;
                    MED[device_index].endpoint = (EndpointId)i;
                    MED[device_index].clusterId = 0x0006;
                    MED[device_index].attributeId = 0;
                    device_index++;
                }
                else if (ED[i].ep_list[j].clusterID[k] == 0x0402)
                {
                    MED[device_index].ep = ED[i].ep_list[j].ep;
                    MED[device_index].ShortAddress = ED[i].ShortAddress;
                    MED[device_index].endpoint = (EndpointId)i;
                    MED[device_index].clusterId = 0x0402;
                    MED[device_index].attributeId = 0;
                    device_index++;
                }
            }
        }
    }
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
    // ChipLogProgress(NotSpecified, "ChipEventHandler: %x", aEvent->Type);
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
#define ZCL_TEMPERATURE_SENSOR_CLUSTER_REVISION (1u)
#define ZCL_TEMPERATURE_SENSOR_FEATURE_MAP (0u)
#define ZCL_POWER_SOURCE_CLUSTER_REVISION (2u)

// ---------------------------------------------------------------------------

int RemoveDeviceEndpoint(Device * dev)
{
    // ChipLogProgress(DeviceLayer, "\n\n**** %s\n\n", __FUNCTION__);
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
    // ChipLogProgress(DeviceLayer, "\n\n**** %s\n\n", __FUNCTION__);
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
    // ChipLogProgress(DeviceLayer, "\n\n**** %s\n\n", __FUNCTION__);
    printf(LIGHT_RED"%s", __FUNCTION__);
    printf(NONE"\r\n");
    return gActions;
}

EmberAfStatus HandleReadBridgedDeviceBasicAttribute(Device * dev, chip::AttributeId attributeId, uint8_t * buffer,
                                                    uint16_t maxReadLength)
{
    // ChipLogProgress(DeviceLayer, "\n\n**** %s\n\n", __FUNCTION__);
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

EmberAfStatus HandleReadOnOffAttribute(DeviceOnOff * dev, 
                                       chip::AttributeId attributeId, 
                                       uint8_t * buffer, 
                                       uint16_t maxReadLength)
{
    // ChipLogProgress(DeviceLayer, "\n\n**** %s\n\n", __FUNCTION__);
    printf(LIGHT_RED"%s", __FUNCTION__);
    printf(NONE"\r\n");
    ChipLogProgress(DeviceLayer, "HandleReadOnOffAttribute: attrId=%d, maxReadLength=%d", attributeId, maxReadLength);

    if ((attributeId == OnOff::Attributes::OnOff::Id) && (maxReadLength == 1))
    {
        // *buffer = dev->IsOn() ? 1 : 0;
        
        auto * path = Platform::New<app::ConcreteAttributePath>(dev->GetEndpointId(), 
                                                                OnOff::Id, 
                                                                OnOff::Attributes::OnOff::Id);
        // PlatformMgr().ScheduleWork(gw_cmd_read_attr_req, reinterpret_cast<intptr_t>(path));
        sem_wait(&wait_tx_sem);
        gw_cmd_read_attr_req(reinterpret_cast<intptr_t>(path));
        sem_wait(&read_attribute_sem);
        printf(LIGHT_RED"recv done");
        printf(NONE"\r\n");

        if (!gw_tx_response_queue.empty())
        {
            gw_command_t resp = gw_tx_response_queue.front();

            printf(LIGHT_RED"Status: %d", resp.buffer[19]);
            printf(NONE"\r\n");

            *buffer = resp.buffer[19];
            gw_tx_response_queue.pop();
        }
        else
        {
            return EMBER_ZCL_STATUS_FAILURE;
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

EmberAfStatus HandleWriteOnOffAttribute(DeviceOnOff * dev, chip::AttributeId attributeId, uint8_t * buffer)
{
    // ChipLogProgress(DeviceLayer, "\n\n**** %s\n\n", __FUNCTION__);
    printf(LIGHT_RED"%s", __FUNCTION__);
    printf(NONE"\r\n");
    ChipLogProgress(DeviceLayer, "HandleWriteOnOffAttribute: attrId=%d", attributeId);

    if ((attributeId == OnOff::Attributes::OnOff::Id) && (dev->IsReachable()))
    {
        if (*buffer)
        {
            dev->SetOnOff(true);
        }
        else
        {
            dev->SetOnOff(false);
        }
    }
    else
    {
        return EMBER_ZCL_STATUS_FAILURE;
    }

    return EMBER_ZCL_STATUS_SUCCESS;
}

EmberAfStatus HandleReadTempMeasurementAttribute(DeviceTempSensor * dev, chip::AttributeId attributeId, uint8_t * buffer,
                                                 uint16_t maxReadLength)
{
    // ChipLogProgress(DeviceLayer, "\n\n**** %s\n\n", __FUNCTION__);
    printf(LIGHT_RED"%s", __FUNCTION__);
    printf(NONE"\r\n");
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
    // ChipLogProgress(DeviceLayer, "\n\n**** %s\n\n", __FUNCTION__);
    printf(LIGHT_RED"%s", __FUNCTION__);
    printf(NONE"\r\n");
    uint16_t endpointIndex = emberAfGetDynamicIndexFromEndpoint(endpoint);
    ChipLogProgress(DeviceLayer, "endpoint: %d, endpointIndex: %d", endpoint, endpointIndex);

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
            ret = HandleReadOnOffAttribute(static_cast<DeviceOnOff *>(dev), attributeMetadata->attributeId, buffer, maxReadLength);
        }
        else if (clusterId == TemperatureMeasurement::Id)
        {
            ret = HandleReadTempMeasurementAttribute(static_cast<DeviceTempSensor *>(dev), attributeMetadata->attributeId, buffer,
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
        // ChipLogProgress(DeviceLayer, "\n\n**** %s\n\n", __FUNCTION__);
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
    // ChipLogProgress(DeviceLayer, "\n\n**** %s\n\n", __FUNCTION__);
    printf(LIGHT_RED"%s", __FUNCTION__);
    printf(NONE"\r\n");
    uint16_t endpointIndex = emberAfGetDynamicIndexFromEndpoint(endpoint);

    EmberAfStatus ret = EMBER_ZCL_STATUS_FAILURE;

    ChipLogProgress(DeviceLayer, "emberAfExternalAttributeWriteCallback: ep=%d", endpoint);

    if (endpointIndex < CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT)
    {
        Device * dev = gDevices[endpointIndex];

        if ((dev->IsReachable()) && (clusterId == OnOff::Id))
        {
            ret = HandleWriteOnOffAttribute(static_cast<DeviceOnOff *>(dev), attributeMetadata->attributeId, buffer);
        }
    }

    return ret;
}

bool emberAfActionsClusterInstantActionCallback(app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
                                                const Actions::Commands::InstantAction::DecodableType & commandData)
{
    // ChipLogProgress(DeviceLayer, "\n\n**** %s\n\n", __FUNCTION__);
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
    sem_init(&wait_tx_sem, 0, 1); 
    sem_init(&read_attribute_sem, 0, 0);
    add_ep = true;
}

void ApplicationShutdown() 
{

}

#define POLL_INTERVAL_MS (100)
// uint8_t poll_prescale = 0;
// const int16_t oneDegree = 100;

void rx_queue_process_handler()
{
    unsigned int commandId;
    gw_command_t queue;

    using namespace std::chrono_literals;

    while (true)
    {   
        if (!gw_rx_command_queue.empty())
        {
            queue = gw_rx_command_queue.front();

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
                    unsigned short temperature;

                    temperature = queue.buffer[16] | queue.buffer[17] << 8;
                    TempSensor[0]->SetMeasuredValue(temperature);
                    ChipLogProgress(DeviceLayer, "Temp: %f", (float)temperature / 100.0);
                    break;
            
                case 0x00230000:
                    ChipLogProgress(DeviceLayer, "IAS");
                    break;

                case 0x00018800:
                    ChipLogProgress(DeviceLayer, "Default response");
                    sem_post(&wait_tx_sem);
                    break;

                case 0x00028000:
                    ChipLogProgress(DeviceLayer, "Read device attribute response");
                    gw_tx_response_queue.push(queue);
                    sem_post(&wait_tx_sem);
                    sem_post(&read_attribute_sem);
                default: 
                    break;
            }
            gw_rx_command_queue.pop();
        }

        std::this_thread::sleep_for(10ms);
        std::this_thread::yield();
    }
}

void tx_queue_process_handler()
{
    gw_command_t queue;

    using namespace std::chrono_literals;

    while (true)
    {   
        if (!gw_tx_command_queue.empty())
        {
            queue = gw_tx_command_queue.front();

            send(sockfd, queue.buffer, queue.len, 0);
            gw_tx_command_queue.pop();
            // sem_wait(&wait_tx_sem);
        }

        std::this_thread::sleep_for(10ms);
        std::this_thread::yield();
    }
}

void add_device_thread_handler()
{   
    int i = 0;

    using namespace std::chrono_literals;

    while (true)
    {
        if (add_ep)
        {
            for (i = 0; i < device_index; i++)
            {
                if (MED[i].clusterId == 0x0006)
                {
                    AddLightEP(i);
                }
                else if (MED[i].clusterId == 0x0402)
                {
                    AddTempSensorEP(i);
                }
            }
            add_ep = false;
        }
        std::this_thread::sleep_for(10ms);
        std::this_thread::yield();
    }
}

void tcp_rx_thread_hanlder()
{
    unsigned char buffer[1024];
    gw_command_t queue;
    ssize_t  nbytes;
    // unsigned int commandId;

    using namespace std::chrono_literals;

    while (true)
    {
        nbytes = recv(sockfd, buffer, 1024, 0);
        if (nbytes > 0)
        {
            queue.len = (unsigned int)nbytes;
            memcpy(queue.buffer, buffer, nbytes);
            gw_rx_command_queue.push(queue);
        }
        std::this_thread::sleep_for(10ms);
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
    // add_device_thread.join();
    std::thread tcp_rx_thread(tcp_rx_thread_hanlder);
    std::thread rx_queue_process_thread(rx_queue_process_handler);
    std::thread tx_queue_process_thread(tx_queue_process_handler);

    // Run CHIP

    ApplicationInit();
    // chip::DeviceLayer::PlatformMgr().ScheduleWork(AddLightEP);
    registerAttributeAccessOverride(&gPowerAttrAccess);
    chip::DeviceLayer::PlatformMgr().AddEventHandler(ChipEventHandler, 0);
    chip::DeviceLayer::PlatformMgr().RunEventLoop();

    return 0;
}
