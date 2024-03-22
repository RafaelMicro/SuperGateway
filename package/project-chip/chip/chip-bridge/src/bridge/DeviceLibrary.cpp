
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
// #include <platform/CommissionableDataProvider.h>
// #include <setup_payload/QRCodeSetupPayloadGenerator.h>
// #include <setup_payload/SetupPayload.h>

#include <pthread.h>
#include <sys/ioctl.h>

#include "CommissionableInit.h"
#include "bridged_platfrom/Device.h"
#include "DeviceLibrary.h"
#include "ApplicationCluster.h"
// #include "ClusterDefine.h"
// #include "ClusterMacro.h"
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

namespace Rafael {
namespace DeviceLibrary {

DeviceManager DeviceManager::sDeviceManager;
std::vector<deviceEP_t*> DeviceManager::epList;
chip::EndpointId DeviceManager::gCurrentEndpointId;
DeviceManager & DeviceMgr(void) { return DeviceManager::sDeviceManager; }


static int light_id = 0;
static int switch_id = 0;
static int contact_sensor = 0;

chip::EndpointId gFirstDynamicEndpointId = 0;
// Power source is on the same endpoint as the composed device
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

Action::Action(uint16_t actionId, std::string name, Actions::ActionTypeEnum type, 
               uint16_t endpointListId, uint16_t supportedCommands,
               Actions::ActionStateEnum status, bool isVisible)
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
    ChipLogProgress(DeviceLayer, "\n\n**** %s\n\n", __FUNCTION__);
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

int RemoveDeviceEndpoint(DeviceBase * dev)
{
    ChipLogProgress(DeviceLayer, "\n\n**** %s\n\n", __FUNCTION__);
    for(uint8_t index = 0; index < CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT; index++)
    {
        if (DeviceMgr().GetDeviceList(index) != nullptr) { continue; }
        // Todo: Update this to schedule the work rather than use this lock
        DeviceLayer::StackLock lock;
        EndpointId ep = emberAfClearDynamicEndpoint(index);
        // DeviceMgr().SetDevice(index, nullptr);
        ChipLogProgress(DeviceLayer, "Removed device %s from dynamic endpoint %d (index=%d)", dev->GetName(), ep, index);
        // Silence complaints about unused ep when progress logging disabled.
        UNUSED_VAR(ep);
        return index;
    }
    return -1;
}

std::vector<EndpointListInfo> GetEndpointListInfo(chip::EndpointId parentId)
{
    ChipLogProgress(DeviceLayer, "\n\n**** %s\n\n", __FUNCTION__);
    std::vector<EndpointListInfo> infoList;

    for (auto room : gRooms)
    {
        if (room->getIsVisible())
        {
            EndpointListInfo info(room->getEndpointListId(), room->getName(), room->getType());
            for(uint8_t index = 0; index < CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT; index++)
            {
                deviceEP_t* dev = DeviceMgr().GetDeviceList(index);
                if ((dev != nullptr) && (dev->endpointId == parentId))
                {
                    std::string location = (room->getType() == Actions::EndpointListTypeEnum::kZone)? dev->name :dev->location;
                    if (room->getName().compare(location) == 0) info.AddEndpointId(dev->endpointId);
                }
            }
            if (info.GetEndpointListSize() > 0) infoList.push_back(info);
        }
    }
    return infoList;
}

std::vector<Action *> GetActionListInfo(chip::EndpointId parentId) { return gActions; };

void DeviceManager::Init()
{
    gFirstDynamicEndpointId = static_cast<chip::EndpointId>(
        static_cast<int>(emberAfEndpointFromIndex(static_cast<uint16_t>(emberAfFixedEndpointCount() - 1))) + 1);
    gCurrentEndpointId = gFirstDynamicEndpointId;
    return ;
}

} // namespace DeeviceLibrary
} // namespace Rafael
