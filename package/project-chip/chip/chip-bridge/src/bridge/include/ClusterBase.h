
#ifndef __CLUSTER_BASE_H__
#define __CLUSTER_BASE_H__
#pragma once

#include <app/util/attribute-storage.h>

#include <stdbool.h>
#include <stdint.h>

#include <functional>
#include <vector>

typedef enum DeviceAction
{
    ActionReachable,
    ActionLocation,
    ActionName,
    ActionOnOff,
    ActionLevelControl
} Changed_t;

class DeviceBase
{
public:
    static const int kDeviceNameSize = 32;
    Changed_t kChanged;
    
    DeviceBase(std::string szDeviceName, std::string szLocation);
    virtual ~DeviceBase() {};

    bool IsReachable();
    void SetReachable(bool aReachable);
    void SetName(std::string szDeviceName);
    void SetLocation(std::string szLocation);
    inline void SetEndpointId(chip::EndpointId id) { mEndpointId = id; };
    inline chip::EndpointId GetEndpointId() { return mEndpointId; };
    inline void SetDeviceType(uint8_t deviceType) { mDeviceType = deviceType; };
    inline uint8_t GetDeviceType() { return mDeviceType; };
    inline void SetParentEndpointId(chip::EndpointId id) { mParentEndpointId = id; };
    inline chip::EndpointId GetParentEndpointId() { return mParentEndpointId; };
    inline std::string GetName() { return mName; };
    inline std::string GetLocation() { return mLocation; };
    inline std::string GetZone() { return mZone; };
    inline void SetZone(std::string zone) { mZone = zone; };

private:
    virtual void HandleDeviceChange(DeviceBase * device, Changed_t changeMask) = 0;

protected:
    bool mReachable;
    chip::EndpointId mEndpointId;
    std::string mName;
    std::string mLocation;
    // char mName[kDeviceNameSize];
    // char mLocation[kDeviceNameSize];
    chip::EndpointId mParentEndpointId;
    uint8_t mDeviceType;
    std::string mZone;
};

class ClusterOnOff
{
public:
    bool GetOnOffState();
    void SetOnOff(bool aOn);
    void Toggle();
    bool mOn;
    virtual ~ClusterOnOff() {};
};

class ClusterLevelControl
{
public:
    uint8_t GetCurrentLevel();
    void SetCurrentLevel(uint8_t aCurrentLevel);
    uint8_t mCurrentLevel;
    virtual ~ClusterLevelControl() {};
};

class ClusterBooleanState
{
public:
    bool GetStateValue();
    void SetStateValue(bool aStateValue);
    bool mStateValue;
    virtual ~ClusterBooleanState() {};
};

#endif // __CLUSTER_BASE_H__
