
#ifndef __DEVICE_H__
#define __DEVICE_H__
#pragma once

#include "ClusterBase.h"

class DeviceOnOffLight : public DeviceBase, public ClusterOnOff, public ClusterLevelControl
{
public:
    DeviceOnOffLight(std::string szDeviceName, std::string szLocation);
    using DeviceCB_fn = std::function<void(DeviceOnOffLight *, Changed_t)>;
    DeviceCB_fn mChanged_CB;

private:
    void HandleDeviceChange(DeviceBase * device, Changed_t changeMask);
};

class DeviceOnOffLightSwitch : public DeviceBase, public ClusterOnOff
{
public:
    DeviceOnOffLightSwitch(std::string szDeviceName, std::string szLocation);
    using DeviceCB_fn = std::function<void(DeviceOnOffLightSwitch *, Changed_t)>;
    DeviceCB_fn mChanged_CB;

private:
    void HandleDeviceChange(DeviceBase * device, Changed_t changeMask);
};


class DeviceContactSensor : public DeviceBase, public ClusterBooleanState
{
public:
    DeviceContactSensor(std::string szDeviceName, std::string szLocation);
    using DeviceCB_fn = std::function<void(DeviceContactSensor *, Changed_t)>;
    DeviceCB_fn mChanged_CB;

private:
    void HandleDeviceChange(DeviceBase * device, Changed_t changeMask);
};

#endif // __DEVICE_H__
