
#ifndef __DEVICE_LIBRARY_H__
#define __DEVICE_LIBRARY_H__

#pragma once

#include <app/util/attribute-storage.h>
#include <lib/core/DataModelTypes.h>

typedef struct {
    uint16_t endpointId;
    uint16_t deviceType;
    std::string name;
    std::string location;
    bool reachable;
} deviceEP_t;

#ifndef DataVersion
typedef uint32_t DataVersion;
#endif

template <typename T>
struct device_t{
    uint16_t endpointId;
    T* device;
};

namespace Rafael {
namespace DeviceLibrary {

class DeviceManager
{
protected:
    static std::vector<deviceEP_t*> epList; // CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT
    template <typename T>
    static std::vector<device_t<T>*> devList;
    static chip::EndpointId gCurrentEndpointId;

private:
    friend DeviceManager & DeviceMgr(void);
    static DeviceManager sDeviceManager;

public:
    static void Init();
    static deviceEP_t* GetDeviceList(uint16_t i) { return (i< epList.size())? epList[i] : nullptr; }
    static void AddDeviceList(deviceEP_t * dev) { epList.push_back(dev);};
    template <typename T>
    static device_t<T> * GetDevice(uint16_t i) { return (i < devList<T>.size())? devList<T>[i] : nullptr; }
    template <typename T>
    static void AddDevice(device_t<T> * dev) { devList<T>.push_back(dev); };
    template <typename T, class M>
    static int AddDeviceEndpoint(M* dev_set, device_t<T>* epDevice, 
        deviceEP_t* epType, chip::EndpointId parentEPId = chip::kInvalidEndpointId)
    {
        auto dev = epDevice->device;
        EmberAfStatus ret;
        ChipLogProgress(DeviceLayer, "\n\n**** %s\nGen device EP_Id:%d, Device_Type:%d\n", 
                        __FUNCTION__, gCurrentEndpointId, epType->deviceType);
        
        for(uint8_t index = 0; index < CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT; index++)
        {
            if (GetDeviceList(index) != nullptr)  { continue; }
            chip::DeviceLayer::StackLock lock;
            dev->SetEndpointId(gCurrentEndpointId);
            dev->SetParentEndpointId(parentEPId);
            chip::Span<DataVersion> DV(&dev_set->DataVersions[0], dev_set->ClusterSize);
            chip::Span<const EmberAfDeviceType> DT(&dev_set->DeviceTypes[0], dev_set->DeviceTypeSize);
            ret = emberAfSetDynamicEndpoint(index, gCurrentEndpointId, &dev_set->Endpoint, DV, DT, parentEPId);
            ChipLogProgress(DeviceLayer, "emberAfSetDynamicEndpoint ret: %d", ret);
            if (ret == EMBER_ZCL_STATUS_SUCCESS)
            {
                AddDevice(epDevice);
                AddDeviceList(epType);
                ChipLogProgress(DeviceLayer, "Added device %s to dynamic endpoint %d (index=%d)", 
                                dev->GetName(), gCurrentEndpointId, index);
                gCurrentEndpointId++;
                return index;
            }
            if (ret != EMBER_ZCL_STATUS_DUPLICATE_EXISTS) return -1;
        }
        ChipLogProgress(DeviceLayer, "Failed to add dynamic endpoint: No endpoints available!");
        return -1;
    };

    template <class T, class M>
    static void publishDevice(M* dev, uint16_t deviceType)
    {
        device_t<T> epDevice;
        deviceEP_t epType;
        T DeviceInst((char *)dev->name.c_str(), dev->location);
        DeviceInst.SetReachable(true);

        epDevice.device = &DeviceInst;
        epDevice.endpointId = gCurrentEndpointId;

        epType.deviceType = deviceType;
        epType.endpointId = gCurrentEndpointId;
        epType.reachable = DeviceInst.IsReachable();
        epType.name = dev->name;
        epType.location = dev->location;

        AddDeviceEndpoint<T>(dev, &epDevice, &epType, 1);
    };
};
DeviceManager & DeviceMgr(void);
template <typename T>
std::vector<device_t<T>*> DeviceManager::devList;

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
