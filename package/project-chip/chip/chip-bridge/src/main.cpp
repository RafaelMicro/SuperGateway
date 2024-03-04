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

#include "CommissionableInit.h"
#include "Device.h"
#include "main.h"
#include <app/server/Server.h>

#include <cassert>
#include <iostream>
#include <vector>

using namespace chip;
using namespace chip::app;
using namespace chip::Credentials;
using namespace chip::Inet;
using namespace chip::Transport;
using namespace chip::DeviceLayer;
using namespace chip::app::Clusters;

// #define NONE             "\033[m"
// #define RED              "\033[0;32;31m"
// #define LIGHT_RED        "\033[1;31m"
// #define GREEN            "\033[0;32;32m"
// #define LIGHT_GREEN      "\033[1;32m"
// #define BLUE             "\033[0;32;34m"
// #define LIGHT_BLUE       "\033[1;34m" 
// #define CYAN             "\033[0;36m"
// #define PUPLE            "\033[0;35m"
// #define BRON             "\033[0;33m"
// #define YELLOW           "\033[1;33m"
// #define WHITE            "\033[1;37m" 

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
    _EndDevice  ED;
};

struct _EndDevice ED[EndDeviceMax];
struct _Coordinator CR;
struct _MatterEndDevice MED[EndDeviceMax];

char EndDevice_Filename[]="/usr/local/var/lib/ez-zbgw/zbdb/sc_enddevice.dat";
char Coordinator_Filename[]="/usr/local/var/lib/ez-zbgw/zbdb/sc_coordinator.dat";
// int light_id = 0;
int light_count = 0;
bool add_ep = false;

DeviceOnOff *Light[EndDeviceMax];

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
const int16_t initialMeasuredValue = 100;

#define DEVICE_TYPE_BRIDGED_NODE 0x0013
#define DEVICE_TYPE_LO_ON_OFF_LIGHT 0x0100
#define DEVICE_TYPE_POWER_SOURCE 0x0011
#define DEVICE_TYPE_TEMP_SENSOR 0x0302
#define DEVICE_VERSION_DEFAULT 1

namespace {

void gw_cmd_on_req(uint16_t saddr, uint8_t ep)
{
    // uint8_t cmd[] = {0xFF, 0xFC, 0xFC, 0xFF, 0x09, 0x01, 0x00, 0x07, 0x00, 0xFF, 0xFF, 0x00, 0xFF, 0x00, 0xBF}; 

    ChipLogProgress(DeviceLayer, "\n\n**** %s\n\n", __FUNCTION__);
    // cmd[9] = (uint8_t)(saddr & 0xFF); 
    // cmd[10] = (uint8_t)((saddr >> 8) & 0xFF); 
    // cmd[12] = ep;       
}

void gw_cmd_off_req(uint16_t saddr, uint8_t ep)
{
    // uint8_t cmd[] = {0xFF, 0xFC, 0xFC, 0xFF, 0x09, 0x00, 0x00, 0x07, 0x00, 0xFF, 0xFF, 0x00, 0xFF, 0x00, 0xBF}; 

    ChipLogProgress(DeviceLayer, "\n\n**** %s\n\n", __FUNCTION__);
    // cmd[9] = (uint8_t)(saddr & 0xFF); 
    // cmd[10] = (uint8_t)((saddr >> 8) & 0xFF); 
    // cmd[12] = ep;       
}

void MatterReportingAttributeChangeCallback(EndpointId endpoint, ClusterId clusterId, AttributeId attributeId)
{
    ChipLogProgress(DeviceLayer, "\n\n**** %s\n\n", __FUNCTION__);

    // AttributePathParams info;
    // info.mClusterId   = clusterId;
    // info.mAttributeId = attributeId;
    // info.mEndpointId  = endpoint;

    uint16_t endpointIndex = emberAfGetDynamicIndexFromEndpoint(endpoint);
    ChipLogProgress(DeviceLayer, "Matter Endpoint: %x", endpoint);
    ChipLogProgress(DeviceLayer, "Matter Cluster: %x", clusterId);
    ChipLogProgress(DeviceLayer, "Matter Attribute: %x", attributeId);
    ChipLogProgress(DeviceLayer, "Matter OnOff: %s", Light[endpointIndex]->IsOn() ? "On" : "Off");
    ChipLogProgress(DeviceLayer, "Zigbee Address: %x", MED[endpointIndex].ED.ShortAddress);
    for (int i = 0; i < MED[endpointIndex].ED.ep_counts; i++)
    {
        if (MED[endpointIndex].ED.ep_list[i].ep <= 0xF0) 
        {
            ChipLogProgress(DeviceLayer, "Zigbee Endpoint: %x", MED[endpointIndex].ED.ep_list[i].ep);
            if (Light[endpointIndex]->IsOn())
            {
                gw_cmd_on_req(MED[endpointIndex].ED.ShortAddress, MED[endpointIndex].ED.ep_list[i].ep);
            }
            else
            {
                gw_cmd_off_req(MED[endpointIndex].ED.ShortAddress, MED[endpointIndex].ED.ep_list[i].ep);
            }
        }
    }

    // chip::DeviceLayer::PlatformMgr().ScheduleWork();
}

void MatterReportingAttributeChangeCallback(const ConcreteAttributePath & aPath)
{
    // ChipLogProgress(DeviceLayer, "\n\n**** %s\n\n", __FUNCTION__);
    return MatterReportingAttributeChangeCallback(aPath.mEndpointId, aPath.mClusterId, aPath.mAttributeId);
}

void CallReportingCallback(intptr_t closure){
    // ChipLogProgress(DeviceLayer, "\n\n**** %s\n\n", __FUNCTION__);
    auto path = reinterpret_cast<app::ConcreteAttributePath *>(closure);
    MatterReportingAttributeChangeCallback(*path);
    Platform::Delete(path);
};

void ScheduleReportingCallback(Device * dev, ClusterId cluster, AttributeId attribute){
    // ChipLogProgress(DeviceLayer, "\n\n**** %s\n\n", __FUNCTION__);
    auto * path = Platform::New<app::ConcreteAttributePath>(dev->GetEndpointId(), cluster, attribute);
    PlatformMgr().ScheduleWork(CallReportingCallback, reinterpret_cast<intptr_t>(path));
};

} // anonymous namespace

void HandleDeviceStatusChanged(Device * dev, Device::Changed_t itemChangedMask)
{
    // ChipLogProgress(DeviceLayer, "\n\n**** %s\n\n", __FUNCTION__);
    if (itemChangedMask & Device::kChanged_Reachable) { ScheduleReportingCallback(dev, BridgedDeviceBasicInformation::Id, BridgedDeviceBasicInformation::Attributes::Reachable::Id); }
    if (itemChangedMask & Device::kChanged_Name) { ScheduleReportingCallback(dev, BridgedDeviceBasicInformation::Id, BridgedDeviceBasicInformation::Attributes::NodeLabel::Id); }
}

void HandleDeviceOnOffStatusChanged(DeviceOnOff * dev, DeviceOnOff::Changed_t itemChangedMask)
{
    // ChipLogProgress(DeviceLayer, "\n\n**** %s\n\n", __FUNCTION__);
    if (itemChangedMask & (DeviceOnOff::kChanged_Reachable | DeviceOnOff::kChanged_Name | DeviceOnOff::kChanged_Location))
        HandleDeviceStatusChanged(static_cast<Device *>(dev), (Device::Changed_t) itemChangedMask);
    if (itemChangedMask & DeviceOnOff::kChanged_OnOff)
        ScheduleReportingCallback(dev, OnOff::Id, OnOff::Attributes::OnOff::Id);
}

int AddDeviceEndpoint(Device * dev, EmberAfEndpointType * ep, const Span<const EmberAfDeviceType> & deviceTypeList,
                      const Span<DataVersion> & dataVersionStorage, chip::EndpointId parentEndpointId = chip::kInvalidEndpointId)
{
    // ChipLogProgress(DeviceLayer, "\n\n**** %s\n\n", __FUNCTION__);
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

void AddLightEP(int light_number)
{
    // ChipLogProgress(DeviceLayer, "\n\n**** %s\n\n", __FUNCTION__);
    std::string node = "Light " +  std::to_string(light_number + 1);
    std::string room = "Office";

    Light[light_number] = new DeviceOnOff(node.c_str(), room);
    Light[light_number]->SetReachable(true);
    Light[light_number]->SetChangeCallback(HandleDeviceOnOffStatusChanged);

    EmberAfAttributeMetadata onOffAttrs[] = { 
        { ZAP_EMPTY_DEFAULT(), OnOff::Attributes::OnOff::Id, 1, ZAP_TYPE(BOOLEAN), 0 | ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE) },
        { ZAP_EMPTY_DEFAULT(), 0xFFFD, 2, ZAP_TYPE(INT16U), ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE) }
    };
    EmberAfAttributeMetadata descriptorAttrs[] = {   
        { ZAP_EMPTY_DEFAULT(), Descriptor::Attributes::DeviceTypeList::Id, kDescriptorAttributeArraySize, ZAP_TYPE(ARRAY), 0 | ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE) },
        { ZAP_EMPTY_DEFAULT(), Descriptor::Attributes::ServerList::Id, kDescriptorAttributeArraySize, ZAP_TYPE(ARRAY), 0 | ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE) },
        { ZAP_EMPTY_DEFAULT(), Descriptor::Attributes::ClientList::Id, kDescriptorAttributeArraySize, ZAP_TYPE(ARRAY), 0 | ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE) },
        { ZAP_EMPTY_DEFAULT(), Descriptor::Attributes::PartsList::Id, kDescriptorAttributeArraySize, ZAP_TYPE(ARRAY), 0 | ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE) },
        { ZAP_EMPTY_DEFAULT(), 0xFFFD, 2, ZAP_TYPE(INT16U), ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE) }
    };
    EmberAfAttributeMetadata bridgedDeviceBasicAttrs[] = {
        { ZAP_EMPTY_DEFAULT(), BridgedDeviceBasicInformation::Attributes::NodeLabel::Id, kNodeLabelSize, ZAP_TYPE(CHAR_STRING), 0 | ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE) },
        { ZAP_EMPTY_DEFAULT(), BridgedDeviceBasicInformation::Attributes::Reachable::Id, 1, ZAP_TYPE(BOOLEAN), 0 | ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE) },
        { ZAP_EMPTY_DEFAULT(), BridgedDeviceBasicInformation::Attributes::FeatureMap::Id, 4, ZAP_TYPE(BITMAP32), 0 | ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE) },
        { ZAP_EMPTY_DEFAULT(), 0xFFFD, 2, ZAP_TYPE(INT16U), ZAP_ATTRIBUTE_MASK(EXTERNAL_STORAGE) }
    };
    constexpr CommandId onOffIncomingCommands[] = {
        app::Clusters::OnOff::Commands::Off::Id,
        app::Clusters::OnOff::Commands::On::Id,
        app::Clusters::OnOff::Commands::Toggle::Id,
        app::Clusters::OnOff::Commands::OffWithEffect::Id,
        app::Clusters::OnOff::Commands::OnWithRecallGlobalScene::Id,
        app::Clusters::OnOff::Commands::OnWithTimedOff::Id,
        kInvalidCommandId,
    };
    EmberAfCluster bridgedLightClusters[] = { /*(clusterId, clusterAttrs, incomingCommands, outgoingCommands)*/
        { OnOff::Id, onOffAttrs, ArraySize(onOffAttrs), 0, ZAP_CLUSTER_MASK(SERVER), NULL, onOffIncomingCommands, nullptr },
        { Descriptor::Id, descriptorAttrs, ArraySize(descriptorAttrs), 0, ZAP_CLUSTER_MASK(SERVER), NULL, nullptr, nullptr },
        { BridgedDeviceBasicInformation::Id, bridgedDeviceBasicAttrs, ArraySize(bridgedDeviceBasicAttrs), 0, 
        ZAP_CLUSTER_MASK(SERVER), NULL, nullptr, nullptr }
    };
    EmberAfEndpointType bridgedLightEndpoint = {
        bridgedLightClusters, ArraySize(bridgedLightClusters), 0 
    };

    const EmberAfDeviceType BridgedOnOffDeviceTypes[] = { { DEVICE_TYPE_LO_ON_OFF_LIGHT, DEVICE_VERSION_DEFAULT },
                                                        { DEVICE_TYPE_BRIDGED_NODE, DEVICE_VERSION_DEFAULT } };
    DataVersion* LightDataVersions = (DataVersion *) malloc(ArraySize(bridgedLightClusters) * sizeof (DataVersion));
    size_t LightDataVersionsLength = ArraySize(bridgedLightClusters);

    AddDeviceEndpoint(Light[light_number], &bridgedLightEndpoint, Span<const EmberAfDeviceType>(BridgedOnOffDeviceTypes),
        Span<DataVersion>(LightDataVersions, LightDataVersionsLength), 1);
};

int FileExist(char *fname)
{
    struct stat st;
    return(stat(fname, &st)==0);
}

void Check_Dev_Info()
{
    int i, j, k;

    for (i = 0; i < EndDeviceMax; i++) 
    {
        if (ED[i].Active == 0)
            continue;
        // ChipLogProgress(DeviceLayer, "\r\n\r\n **** Check Zigbee Device Cluster \r\n\r\n");
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
                    MED[light_count].ED = ED[i];
                    MED[light_count].endpoint = (EndpointId)i;
                    MED[light_count].clusterId = 6;
                    MED[light_count].attributeId = 0;
                    light_count++;
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

void ChipEventHandler(const ChipDeviceEvent * aEvent, intptr_t /* arg */)
{
    ChipLogProgress(NotSpecified, "ChipEventHandler: %x", aEvent->Type);
    
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
    return gActions;
}

EmberAfStatus HandleReadBridgedDeviceBasicAttribute(Device * dev, chip::AttributeId attributeId, uint8_t * buffer,
                                                    uint16_t maxReadLength)
{
    // ChipLogProgress(DeviceLayer, "\n\n**** %s\n\n", __FUNCTION__);
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

EmberAfStatus HandleReadOnOffAttribute(DeviceOnOff * dev, chip::AttributeId attributeId, uint8_t * buffer, uint16_t maxReadLength)
{
    // ChipLogProgress(DeviceLayer, "\n\n**** %s\n\n", __FUNCTION__);
    ChipLogProgress(DeviceLayer, "HandleReadOnOffAttribute: attrId=%d, maxReadLength=%d", attributeId, maxReadLength);

    if ((attributeId == OnOff::Attributes::OnOff::Id) && (maxReadLength == 1))
    {
        *buffer = dev->IsOn() ? 1 : 0;
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
    // ChipLogProgress(DeviceLayer, "\n\n**** %s", __FUNCTION__);
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
    commandObj->AddStatus(commandPath, Protocols::InteractionModel::Status::NotFound);
    return true;
}

void ApplicationInit() 
{
    Read_EndDevice_File();
    Check_Dev_Info();
    add_ep = true;
}

void ApplicationShutdown() {}

#define POLL_INTERVAL_MS (100)
// uint8_t poll_prescale = 0;
// const int16_t oneDegree = 100;

void *bridge_polling_thread(void * context)
{
    while (true)
    {
        if (add_ep)
        {
            ChipLogProgress(DeviceLayer, "Light Count: %d", light_count);
            for (int i = 0; i < light_count; i++)
            {
                AddLightEP(i);
            }
            add_ep = false;
        }

        // Sleep to avoid tight loop reading commands
        usleep(POLL_INTERVAL_MS * 1000);
    }

    return nullptr;
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
    {
        pthread_t poll_thread;
        int res = pthread_create(&poll_thread, nullptr, bridge_polling_thread, nullptr);
        if (res)
        {
            printf("Error creating polling thread: %d\n", res);
            exit(1);
        }
    }
    // Run CHIP

    ApplicationInit();
    // chip::DeviceLayer::PlatformMgr().ScheduleWork(AddLightEP);
    registerAttributeAccessOverride(&gPowerAttrAccess);
    chip::DeviceLayer::PlatformMgr().AddEventHandler(ChipEventHandler, 0);
    chip::DeviceLayer::PlatformMgr().RunEventLoop();

    return 0;
}
