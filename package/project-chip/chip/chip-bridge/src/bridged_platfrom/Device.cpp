
#include "bridged_platfrom/Device.h"

#include <cstdio>
#include <platform/CHIPDeviceLayer.h>

#include "bridged_platfrom/Device.h"
#include "DeviceLibrary.h"
#include "ApplicationCluster.h"

using namespace chip::app::Clusters::Actions;

DeviceOnOffLight::DeviceOnOffLight(const char * szDeviceName, std::string szLocation) : DeviceBase(szDeviceName, szLocation)
{
    mOn = false;
    mCurrentLevel = 0;
}

void DeviceOnOffLight::HandleDeviceChange(DeviceBase * device, Changed_t changeMask)
{
    if (mChanged_CB) mChanged_CB(this, changeMask);
}

DeviceOnOffLightSwitch::DeviceOnOffLightSwitch(const char * szDeviceName, std::string szLocation) : DeviceBase(szDeviceName, szLocation)
{
    mOn = false;
}

void DeviceOnOffLightSwitch::HandleDeviceChange(DeviceBase * device, Changed_t changeMask)
{
    if (mChanged_CB) mChanged_CB(this, changeMask);
}

DeviceContactSensor::DeviceContactSensor(const char * szDeviceName, std::string szLocation) : DeviceBase(szDeviceName, szLocation)
{
    mStateValue = false;
}

void DeviceContactSensor::HandleDeviceChange(DeviceBase * device, Changed_t changeMask)
{
    if (mChanged_CB) mChanged_CB(this, changeMask);
}