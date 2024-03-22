
#include "bridged_platfrom/Device.h"

#include <cstdio>
#include <platform/CHIPDeviceLayer.h>

#include "bridged_platfrom/Device.h"
#include "DeviceLibrary.h"
#include "ApplicationCluster.h"
#include "DeviceBase.h"

using namespace chip::app::Clusters::Actions;

DeviceAttBase::DeviceAttBase(std::string aname, std::string alocation) { name = aname; location = alocation; };
DeviceAttOnOffLight::DeviceAttOnOffLight(std::string aname, std::string alocation) : DeviceAttBase(aname, alocation) {};
DeviceAttOnOffLightSwitch::DeviceAttOnOffLightSwitch(std::string aname, std::string alocation) : DeviceAttBase(aname, alocation) {};
DeviceAttContactSensor::DeviceAttContactSensor(std::string aname, std::string alocation) : DeviceAttBase(aname, alocation) {};

DeviceBase::DeviceBase(const char * szDeviceName, std::string szLocation)
{
    chip::Platform::CopyString(mName, szDeviceName);
    mLocation = szLocation;
    mReachable = false;
    mEndpointId = 0;
    mName[kDeviceNameSize];
}

bool DeviceBase::IsReachable() { return mReachable; }

void DeviceBase::SetReachable(bool aReachable)
{
    ChipLogProgress(DeviceLayer, "Device[%s]: %s", mName, (aReachable)? "ONLINE" : "OFFLINE" );
    if (mReachable != aReachable)
    {
        mReachable = aReachable;
        HandleDeviceChange(this, ActionReachable);
    }
}

void DeviceBase::SetLocation(std::string szLocation)
{
    ChipLogProgress(DeviceLayer, "Device[%s]: Location=\"%s\"", mName, mLocation.c_str());
    if (mLocation.compare(szLocation) != 0)
    {
        mLocation = szLocation;
        HandleDeviceChange(this, ActionLocation);
    }
}

void DeviceBase::SetName(const char * szName)
{
    ChipLogProgress(DeviceLayer, "Device[%s]: New Name=\"%s\"", mName, szName);
    if (strncmp(mName, szName, sizeof(mName)) != 0)
    {
        chip::Platform::CopyString(mName, szName);
        HandleDeviceChange(this, ActionName);
    }
}

bool ClusterOnOff::GetOnOffState()
{
    ChipLogProgress(DeviceLayer, "\n\n**** %s\n\n", __FUNCTION__);
    return mOn;
}

void ClusterOnOff::SetOnOff(bool aOn)
{
    ChipLogProgress(DeviceLayer, "\n\n**** %s\nOnOff state: %s\n", __FUNCTION__, aOn ? "ON" : "OFF");
    bool changed = aOn ^ mOn;
    mOn = aOn;
    // if ((changed) && (mChanged_CB)) mChanged_CB(this, ActionOnOff);
}

void ClusterOnOff::Toggle()
{
    ChipLogProgress(DeviceLayer, "\n\n**** %s\n\n", __FUNCTION__);
    bool aOn = !GetOnOffState();
    SetOnOff(aOn);
}

uint8_t ClusterLevelControl::GetCurrentLevel()
{
    ChipLogProgress(DeviceLayer, "\n\n**** %s\n\n", __FUNCTION__);
    return mCurrentLevel;
}

void ClusterLevelControl::SetCurrentLevel(uint8_t aCurrentLevel)
{
    ChipLogProgress(DeviceLayer, "\n\n**** %s\nCurrent Level: %u\n", __FUNCTION__, aCurrentLevel);
    mCurrentLevel = aCurrentLevel;
}

bool ClusterBooleanState::GetStateValue()
{
    ChipLogProgress(DeviceLayer, "\n\n**** %s\n\n", __FUNCTION__);
    return mStateValue;
}

void ClusterBooleanState::SetStateValue(bool aStateValue)
{
    ChipLogProgress(DeviceLayer, "\n\n**** %s\nState value: %s\n", __FUNCTION__, aStateValue ? "TRUE" : "FALSE");
    bool changed = aStateValue ^ mStateValue;
    mStateValue = aStateValue;
    // if ((changed) && (mChanged_CB)) mChanged_CB(this, ActionOnOff);
}
