
#ifndef __DEVICE_LIBRARY_H__
#define __DEVICE_LIBRARY_H__

#pragma once

#include <app/util/attribute-storage.h>
#include <lib/core/DataModelTypes.h>

#ifndef DataVersion
typedef uint32_t DataVersion;
#endif

typedef struct {
    uint16_t endpointIndex;
    uint16_t endpointId;
    uint16_t deviceType;
    std::string name;
    std::string location;
    bool reachable;
} deviceEP_t;

template <typename T, typename M>
struct device_t{
    uint16_t endpointIndex;
    uint16_t endpointId;
    T* device;
    M* deviceAtt;
};

namespace Rafael {
namespace DeviceLibrary {

class DeviceManager
{
protected:
    static std::vector<deviceEP_t*> epList; // CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT
    template <typename T, typename M>
    static std::vector<device_t<T, M>*> devList;
    static chip::EndpointId gCurrentEndpointId;

private:
    friend DeviceManager & DeviceMgr(void);
    static DeviceManager sDeviceManager;

public:
    static void Init();
    static deviceEP_t* GetDeviceList(uint16_t idx);
    static deviceEP_t* GetDeviceIndex(uint16_t idx);
    static void AddDeviceList(deviceEP_t * dev);
    static void DelDeviceList(uint16_t idx);
    static void ListDeviceList();
    template <typename T, typename M>
    static device_t<T, M>* GetDevice(uint16_t idx) {
        for(uint16_t i = 0; i < devList<T, M>.size(); i++) { 
            if(devList<T, M>[i]->endpointId == idx) { return devList<T, M>[i]; } 
        }
        return nullptr; 
    };
    template <typename T, typename M>
    static void AddDevice(device_t<T, M> * dev) { 
        if(epList.size() > CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT)
        {
            ChipLogProgress(DeviceLayer, "Fully devList, Matter Dynamic endpoint count: %d\n", CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT);
            return ;
        }
        devList<T, M>.push_back(dev);
    };
    template <typename T, typename M>
    static void DelDevice(uint16_t idx) {
        ChipLogProgress(DeviceLayer, "%s", __func__);
        for(uint16_t i = 0; i < epList.size(); i++) {
            if(epList[i]->endpointId == idx) {
                free(devList<T, M>[idx]);
                devList<T, M>.erase(std::next(devList<T, M>.begin(), idx));
            }
        }
    };

    template <class T, typename M>
    static void DelDeviceEndpoint(uint16_t idx) {
        if (GetDeviceList(idx) == nullptr)  { 
            ChipLogProgress(DeviceLayer, "Endpoint Id not found");
            return;
        }
        chip::DeviceLayer::StackLock lock;
        ChipLogProgress(DeviceLayer, "try delete device: %d", idx);
        chip::EndpointId ep = emberAfClearDynamicEndpoint(idx);
        ChipLogProgress(DeviceLayer, "Removed device from dynamic endpoint %d (index=%d)", ep, idx);
        DelDeviceList(idx);
        DelDevice<T, M>(idx);
        UNUSED_VAR(ep);
        return;
    }
    template <typename T, class M>
    static int AddDeviceEndpoint(device_t<T, M>* epDevice, deviceEP_t* epType, 
                                 chip::EndpointId parentEPId, 
                                 const chip::Span<const EmberAfDeviceType> & deviceTypeList,
                                 const chip::Span<DataVersion> & dataVersionStorage) {
        auto dev = epDevice->device;
        EmberAfStatus ret;
        ChipLogProgress(DeviceLayer, "Gen device EP_Id:%d, Device_Type:%d\n", gCurrentEndpointId, epType->deviceType);
        for(uint8_t index = 0; index <= gCurrentEndpointId + 1; index++)
        {
            if (GetDeviceIndex(index) != nullptr)  { continue; }
            chip::DeviceLayer::StackLock lock;
            epDevice->endpointIndex = index;
            epType->endpointIndex = index;
            dev->SetEndpointId(gCurrentEndpointId);
            dev->SetParentEndpointId(parentEPId);
            ret = emberAfSetDynamicEndpoint(index, gCurrentEndpointId, &epDevice->deviceAtt->Endpoint, 
                                            dataVersionStorage, deviceTypeList, parentEPId);
            ChipLogProgress(DeviceLayer, "Call Matter Stack status: emberAfSetDynamicEndpoint ret= %d", ret);
            if (ret == EMBER_ZCL_STATUS_SUCCESS)
            {
                AddDevice(epDevice);
                AddDeviceList(epType);
                ChipLogProgress(DeviceLayer, "Added device %s to dynamic endpoint %d (index= %d)", 
                                dev->GetName().c_str(), gCurrentEndpointId, index);
                gCurrentEndpointId++;
                return index;
            }
            if (ret != EMBER_ZCL_STATUS_DUPLICATE_EXISTS) return -1;
        }
        ChipLogProgress(DeviceLayer, "Failed to add dynamic endpoint: No endpoints available!");
        return -1;
    };

    template <class T, class M>
    static void publishDevice(std::string name, std::string location, uint16_t deviceType)
    {
        ChipLogProgress(DeviceLayer, "%s", __func__);
        device_t<T, M>* epDev = new device_t<T, M>();
        deviceEP_t* epType = new deviceEP_t();
        T DevInst(name, location);
        epDev->device = &DevInst; 
        epDev->deviceAtt = new M(name, location);

        epDev->device->SetReachable(true);
        epDev->endpointId = gCurrentEndpointId;

        epType->deviceType = deviceType;
        epType->endpointId = gCurrentEndpointId;
        epType->reachable = epDev->device->IsReachable();
        epType->name = std::string(name);
        epType->location = std::string(location);

        AddDeviceEndpoint<T>(epDev, epType, 1, 
                        chip::Span<const EmberAfDeviceType>(epDev->deviceAtt->DeviceTypes),
                        chip::Span<DataVersion>(epDev->deviceAtt->DataVersions));
    };
};
DeviceManager & DeviceMgr(void);
template <typename T, typename M>
std::vector<device_t<T, M>*> DeviceManager::devList;

class EndpointListInfo
{
public:
    EndpointListInfo(uint16_t endpointListId, std::string name, chip::app::Clusters::Actions::EndpointListTypeEnum type);
    EndpointListInfo(uint16_t endpointListId, std::string name, chip::app::Clusters::Actions::EndpointListTypeEnum type,
                     chip::EndpointId endpointId);
    void AddEndpointId(chip::EndpointId endpointId);
    inline uint16_t GetEndpointListId() { return mEndpointListId; };
    std::string GetName() { return mName; };
    inline chip::app::Clusters::Actions::EndpointListTypeEnum GetType() { return mType; };
    inline chip::EndpointId * GetEndpointListData() { return mEndpoints.data(); };
    inline size_t GetEndpointListSize() { return mEndpoints.size(); };

private:
    uint16_t mEndpointListId = static_cast<uint16_t>(0);
    std::string mName;
    chip::app::Clusters::Actions::EndpointListTypeEnum mType = static_cast<chip::app::Clusters::Actions::EndpointListTypeEnum>(0);
    std::vector<chip::EndpointId> mEndpoints;
};

class Room
{
public:
    Room(std::string name, uint16_t endpointListId, chip::app::Clusters::Actions::EndpointListTypeEnum type, bool isVisible);
    inline void setIsVisible(bool isVisible) { mIsVisible = isVisible; };
    inline bool getIsVisible() { return mIsVisible; };
    inline void setName(std::string name) { mName = name; };
    inline std::string getName() { return mName; };
    inline chip::app::Clusters::Actions::EndpointListTypeEnum getType() { return mType; };
    inline uint16_t getEndpointListId() { return mEndpointListId; };

private:
    bool mIsVisible;
    std::string mName;
    uint16_t mEndpointListId;
    chip::app::Clusters::Actions::EndpointListTypeEnum mType;
};

class Action
{
public:
    Action(uint16_t actionId, std::string name, chip::app::Clusters::Actions::ActionTypeEnum type, uint16_t endpointListId,
           uint16_t supportedCommands, chip::app::Clusters::Actions::ActionStateEnum status, bool isVisible);
    inline void setName(std::string name) { mName = name; };
    inline std::string getName() { return mName; };
    inline chip::app::Clusters::Actions::ActionTypeEnum getType() { return mType; };
    inline chip::app::Clusters::Actions::ActionStateEnum getStatus() { return mStatus; };
    inline uint16_t getActionId() { return mActionId; };
    inline uint16_t getEndpointListId() { return mEndpointListId; };
    inline uint16_t getSupportedCommands() { return mSupportedCommands; };
    inline void setIsVisible(bool isVisible) { mIsVisible = isVisible; };
    inline bool getIsVisible() { return mIsVisible; };

private:
    std::string mName;
    chip::app::Clusters::Actions::ActionTypeEnum mType;
    chip::app::Clusters::Actions::ActionStateEnum mStatus;
    uint16_t mActionId;
    uint16_t mEndpointListId;
    uint16_t mSupportedCommands;
    bool mIsVisible;
};

std::vector<EndpointListInfo> GetEndpointListInfo(chip::EndpointId parentId);
std::vector<Action *> GetActionListInfo(chip::EndpointId parentId);


} // namespace DeeviceLibrary
} // namespace Rafael

#endif // __DEVICE_LIBRARY_H__
