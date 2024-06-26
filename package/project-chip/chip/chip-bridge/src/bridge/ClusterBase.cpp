
#include "platfrom/Device.h"

#include <cstdio>
#include <platform/CHIPDeviceLayer.h>

#include "platfrom/Device.h"
#include "DeviceLibrary.h"
#include "ApplicationCluster.h"
#include "DeviceBase.h"

using namespace chip::app::Clusters::Actions;

DeviceAttBase::DeviceAttBase(std::string aname, std::string alocation) 
    :name(std::move(aname)), location(std::move(alocation)) {};

DeviceAttOnOffLight::DeviceAttOnOffLight(std::string aname, std::string alocation) 
    : DeviceAttBase(aname, alocation) {};

DeviceAttDimmableLight::DeviceAttDimmableLight(std::string aname, std::string alocation) 
    : DeviceAttBase(aname, alocation) {};

DeviceAttColorTemperatureLight::DeviceAttColorTemperatureLight(std::string aname, std::string alocation) 
    : DeviceAttBase(aname, alocation) {};

DeviceAttOnOffLightSwitch::DeviceAttOnOffLightSwitch(std::string aname, std::string alocation) 
    : DeviceAttBase(aname, alocation) {};

DeviceAttContactSensor::DeviceAttContactSensor(std::string aname, std::string alocation) 
    : DeviceAttBase(aname, alocation) {};


DeviceBase::DeviceBase(std::string szDeviceName, std::string szLocation)
    : mReachable(false), mEndpointId(0), mName(std::move(szDeviceName)), mLocation(std::move(szLocation)){};

bool DeviceBase::IsReachable() { return mReachable; }

void DeviceBase::SetReachable(bool aReachable)
{
    ChipLogProgress(DeviceLayer, "Device[%s]: %s", mName.c_str(), aReachable?"ONLINE":"OFFLINE");
    if (mReachable != aReachable) {
        mReachable = aReachable;
        HandleDeviceChange(this, ActionReachable);
    }
}

void DeviceBase::SetLocation(std::string szLocation)
{
    ChipLogProgress(DeviceLayer, "Device[%s]: Location=%s", mName.c_str(), szLocation.c_str());
    if (mLocation.compare(szLocation) != 0) {
        mLocation = szLocation;
        HandleDeviceChange(this, ActionLocation);
    }
}

void DeviceBase::SetName(std::string szName)
{
    ChipLogProgress(DeviceLayer, "Device[%s]: New Name=%s", mName.c_str(), szName.c_str());
    if (mLocation.compare(szName) != 0) {
        mName = szName;
        HandleDeviceChange(this, ActionLocation);
    }
}

bool ClusterOnOff::GetOnOffState() { return mOn; }

void ClusterOnOff::SetOnOff(bool aOn)
{
    ChipLogProgress(DeviceLayer, "OnOff state: %s\n", aOn?"ON":"OFF");
    bool changed = aOn ^ mOn;
    mOn = aOn;
    // if ((changed) && (mChanged_CB)) mChanged_CB(this, ActionOnOff);
}

void ClusterOnOff::Toggle()
{
    bool aOn = !GetOnOffState();
    SetOnOff(aOn);
}

uint8_t ClusterLevelControl::GetCurrentLevel() { return mCurrentLevel; }

void ClusterLevelControl::SetCurrentLevel(uint8_t aCurrentLevel)
{
    bool changed = mCurrentLevel == aCurrentLevel? false : true;
    mCurrentLevel = aCurrentLevel;
}

bool ClusterBooleanState::GetStateValue() { return mStateValue; }

void ClusterBooleanState::SetStateValue(bool aStateValue)
{
    ChipLogProgress(DeviceLayer, "State value: %s\n", aStateValue?"TRUE":"FALSE");
    bool changed = aStateValue ^ mStateValue;
    mStateValue = aStateValue;
    // if ((changed) && (mChanged_CB)) mChanged_CB(this, ActionOnOff);
}


uint8_t ClusterColorControl::GetCurrentHue() { return mCurrentHue; }

void ClusterColorControl::SetCurrentHue(uint8_t val) { 
    bool changed = ( mCurrentHue == val ) ? false : true ;
    mCurrentHue = val;
}

uint8_t ClusterColorControl::GetCurrentSaturation() { return mCurrentSaturation; }

void ClusterColorControl::SetCurrentSaturation(uint8_t val) { 
    bool changed = ( mCurrentSaturation == val ) ? false : true ;
    mCurrentSaturation = val;
}

uint16_t ClusterColorControl::GetCurrentX() { return mCurrentX; }

void ClusterColorControl::SetCurrentX(uint16_t val) { 
    bool changed = ( mCurrentX == val ) ? false : true ;
    mCurrentX = val;
}

uint16_t ClusterColorControl::GetCurrentY() { return mCurrentY; }

void ClusterColorControl::SetCurrentY(uint16_t val) { 
    bool changed = ( mCurrentY == val ) ? false : true ;
    mCurrentY = val;
}

uint16_t ClusterColorControl::GetEnhancedCurrentHue() { return mEnhancedCurrentHue; }

void ClusterColorControl::SetEnhancedCurrentHue(uint16_t val) { 
    bool changed = ( mEnhancedCurrentHue == val ) ? false : true ;
    mEnhancedCurrentHue = val;
}

uint8_t ClusterColorControl::GetEnhancedColorMode() { return mEnhancedColorMode; }

void ClusterColorControl::SetEnhancedColorMode(uint8_t val) { 
    bool changed = ( mEnhancedColorMode == val ) ? false : true ;
    mEnhancedColorMode = val;
}
