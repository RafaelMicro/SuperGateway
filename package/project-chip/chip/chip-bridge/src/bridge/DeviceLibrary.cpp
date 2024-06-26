
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

#include <pthread.h>
#include <sys/ioctl.h>

#include "CommissionableInit.h"
#include "platfrom/Device.h"
#include "DeviceLibrary.h"
#include "DeviceBase.h"
#include "ClusterMacro.h"
#include "ApplicationCluster.h"
#include "config/INIConfig.h"
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

#ifndef BRIDGE_CONFIG_FILE
#define BRIDGE_CONFIG_FILE "/tmp/chip_rafael_bridged.ini"
#endif

namespace Rafael {
namespace DeviceLibrary {

DeviceManager DeviceManager::sDeviceManager;
std::vector<deviceEP_t*> DeviceManager::epList;
chip::EndpointId DeviceManager::gCurrentEndpointId;
std::set<chip::EndpointId> DeviceManager::runningEP;
DeviceManager & DeviceMgr(void) { return DeviceManager::sDeviceManager; };

deviceEP_t* DeviceManager::GetDeviceList(uint16_t idx) {
    for (auto ep : epList) if(ep->endpointId == idx) return ep; 
    return nullptr;
};

deviceEP_t* DeviceManager::GetDeviceIndex(uint16_t idx) {
    for (auto ep : epList)  if(ep->endpointIndex == idx) return ep; 
    return nullptr;
};

void DeviceManager::AddDeviceList(deviceEP_t * dev) { 
    if(epList.size() > CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT) {
        ChipLogProgress(DeviceLayer, "Fully epList, Dynamic EP count: %d", CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT);
        return ;
    } 
    epList.push_back(dev);
};

void DeviceManager::DelDeviceList(uint16_t idx) { 
    for(uint16_t i = 0; i < epList.size(); i++) {
        if(epList[i]->endpointId == idx) {
            delete epList[i];
            epList.erase(std::next(epList.begin(), i));
        }
    }
};

void DeviceManager::ListDeviceList() { 
    ChipLogProgress(DeviceLayer, "EPId\tEPIndex\tdeviceType\tname\tlocation");
    for(auto ep : epList) 
        ChipLogProgress(DeviceLayer, "%d\t%d\t%04x\t%s\t%s", 
                            ep->endpointId, ep->endpointIndex, ep->deviceType, 
                            ep->name.c_str(), ep->location.c_str());
};

chip::EndpointId gFirstDynamicEndpointId = 0;
std::vector<Room *> gRooms;
std::vector<Action *> gActions;

EndpointListInfo::EndpointListInfo(uint16_t endpointListId, std::string name, 
    chip::app::Clusters::Actions::EndpointListTypeEnum type)
{
    mEndpointListId = endpointListId;
    mName           = name;
    mType           = type;
}

EndpointListInfo::EndpointListInfo(uint16_t endpointListId, std::string name, 
    chip::app::Clusters::Actions::EndpointListTypeEnum type, chip::EndpointId endpointId)
{
    mEndpointListId = endpointListId;
    mName           = name;
    mType           = type;
    mEndpoints.push_back(endpointId);
}

void EndpointListInfo::AddEndpointId(chip::EndpointId endpointId) { mEndpoints.push_back(endpointId); }

Room::Room(std::string name, uint16_t endpointListId, 
    chip::app::Clusters::Actions::EndpointListTypeEnum type, bool isVisible)
{
    mName           = name;
    mEndpointListId = endpointListId;
    mType           = type;
    mIsVisible      = isVisible;
}

void CallReportingCallback(intptr_t closure){
    auto path = reinterpret_cast<app::ConcreteAttributePath *>(closure);
    MatterReportingAttributeChangeCallback(*path);
    Platform::Delete(path);
};

void ScheduleReportingCallback(DeviceBase * dev, ClusterId cluster, AttributeId attribute){
    auto * path = Platform::New<app::ConcreteAttributePath>(dev->GetEndpointId(), cluster, attribute);
    PlatformMgr().ScheduleWork(CallReportingCallback, reinterpret_cast<intptr_t>(path));
};

void HandleDeviceStatusChanged(DeviceBase * dev, Changed_t itemChanged)
{
    switch (itemChanged)
    {
    case ActionReachable:{
        ScheduleReportingCallback(dev, BridgedDeviceBasicInformation::Id, BridgedDeviceBasicInformation::Attributes::Reachable::Id);
        break;}
    case ActionName:{
        ScheduleReportingCallback(dev, BridgedDeviceBasicInformation::Id, BridgedDeviceBasicInformation::Attributes::NodeLabel::Id);
        break;}
    default: { break; }
    }
}

Action::Action(uint16_t actionId, std::string name, Actions::ActionTypeEnum type, uint16_t endpointListId, 
               uint16_t supportedCommands, Actions::ActionStateEnum status, bool isVisible)
{
    mActionId          = actionId;
    mName              = name;
    mType              = type;
    mEndpointListId    = endpointListId;
    mSupportedCommands = supportedCommands;
    mStatus            = status;
    mIsVisible         = isVisible;
}

void HandleDeviceOnOffStatusChanged(DeviceBase * dev, Changed_t itemChanged)
{
    switch (itemChanged)
    {
    case ActionReachable:
    case ActionLocation:
    case ActionName:{
        HandleDeviceStatusChanged(static_cast<DeviceBase *>(dev), itemChanged);
        break;}
    case ActionOnOff:{
        ScheduleReportingCallback(dev, OnOff::Id, OnOff::Attributes::OnOff::Id);
        break;}
    default: { break; }
    }
}

std::vector<EndpointListInfo> GetEndpointListInfo(chip::EndpointId parentId)
{
    std::vector<EndpointListInfo> infoList;
    for (auto room : gRooms)
    {
        if (!room->getIsVisible()) continue; 
        EndpointListInfo info(room->getEndpointListId(), room->getName(), room->getType());
        for(uint8_t index = 0; index < CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT; index++)
        {
            deviceEP_t* dev = DeviceMgr().GetDeviceList(index);
            if ((dev != nullptr) && (dev->endpointId == parentId))
            {
                std::string location = (room->getType() == Actions::EndpointListTypeEnum::kZone)? dev->name : dev->location;
                if (room->getName().compare(location) == 0) info.AddEndpointId(dev->endpointId);
            }
        }
        if (info.GetEndpointListSize() > 0) infoList.push_back(info);
    }
    return infoList;
}

std::vector<Action *> GetActionListInfo(chip::EndpointId parentId) { return gActions; };

chip::EndpointId DeviceManager::GetEndpointId()
{
    for(chip::EndpointId i=gFirstDynamicEndpointId; i<CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT+gFirstDynamicEndpointId; i++)
        if(runningEP.find(i)==runningEP.end()) return i;
    return -1;
}

void DeviceManager::AddEndpointId(chip::EndpointId val) { runningEP.insert(val); }

void DeviceManager::DelEndpointId(chip::EndpointId val) { runningEP.erase(val); }

void DeviceManager::Init()
{
    INIConfig::GetInstance()->Init(BRIDGE_CONFIG_FILE);
    gFirstDynamicEndpointId = static_cast<chip::EndpointId>(
        static_cast<int>(emberAfEndpointFromIndex(static_cast<uint16_t>(emberAfFixedEndpointCount() - 1))) + 1);
    gCurrentEndpointId = gFirstDynamicEndpointId;
    emberAfEndpointEnableDisable(emberAfEndpointFromIndex(static_cast<uint16_t>(emberAfFixedEndpointCount() - 1)), false);
    std::string epid;
    for(int i=gFirstDynamicEndpointId; i<CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT+3+1;i++)
    {
        epid = std::to_string(i);
        if(!INIConfig::GetInstance()->CheckSection(epid)) break;
        auto deviceType = INIConfig::GetInstance()->GetAttbute<std::string>(epid, "deviceType");
        if(deviceType == "OnOffLight"){
            Rafael::DeviceLibrary::DeviceMgr().publishDevice<DeviceOnOffLight, DeviceAttOnOffLight>(
                INIConfig::GetInstance()->GetAttbute<std::string>(epid, "name"),
                INIConfig::GetInstance()->GetAttbute<std::string>(epid, "location"),
                DEVICE_TYPE_ON_OFF_LIGHT);
        }
        else if(deviceType == "DimmableLight"){
            Rafael::DeviceLibrary::DeviceMgr().publishDevice<DeviceDimmableLight, DeviceAttDimmableLight>(
                INIConfig::GetInstance()->GetAttbute<std::string>(epid, "name"),
                INIConfig::GetInstance()->GetAttbute<std::string>(epid, "location"),
                DEVICE_TYPE_ON_OFF_LIGHT_SWITCH);
        }
        else if(deviceType == "ColorTemperatureLight"){
            Rafael::DeviceLibrary::DeviceMgr().publishDevice<DeviceColorTemperatureLight, DeviceAttColorTemperatureLight>(
                INIConfig::GetInstance()->GetAttbute<std::string>(epid, "name"),
                INIConfig::GetInstance()->GetAttbute<std::string>(epid, "location"),
                DEVICE_TYPE_ON_OFF_LIGHT_SWITCH);
        }
        else if(deviceType == "OnOffLightSwitch"){
            Rafael::DeviceLibrary::DeviceMgr().publishDevice<DeviceOnOffLightSwitch, DeviceAttOnOffLightSwitch>(
                INIConfig::GetInstance()->GetAttbute<std::string>(epid, "name"),
                INIConfig::GetInstance()->GetAttbute<std::string>(epid, "location"),
                DEVICE_TYPE_ON_OFF_LIGHT_SWITCH);
        }
        else if(deviceType == "ContactSensor"){
            Rafael::DeviceLibrary::DeviceMgr().publishDevice<DeviceContactSensor, DeviceAttContactSensor>(
                INIConfig::GetInstance()->GetAttbute<std::string>(epid, "name"),
                INIConfig::GetInstance()->GetAttbute<std::string>(epid, "location"),
                DEVICE_TYPE_CONTACT_SENSOR);
        }
    }
    return ;
}

} // namespace DeeviceLibrary
} // namespace Rafael
