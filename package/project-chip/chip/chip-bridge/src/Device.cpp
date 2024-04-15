/*
 *
 *    Copyright (c) 2021-2022 Project CHIP Authors
 *    Copyright (c) 2019 Google LLC.
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

#include "Device.h"
#include <cstdio>
#include <platform/CHIPDeviceLayer.h>

using namespace chip::app::Clusters::Actions;

Device::Device(const char * szDeviceName, std::string szLocation)
{
    chip::Platform::CopyString(mName, szDeviceName);
    mLocation   = szLocation;
    mReachable  = false;
    mEndpointId = 0;
}

bool Device::IsReachable()
{
    return mReachable;
}

void Device::SetReachable(bool aReachable)
{
    bool changed = (mReachable != aReachable);
    
    mReachable = aReachable;

    if (aReachable)
    {
        ChipLogProgress(DeviceLayer, "Device[%s]: ONLINE", mName);
    }
    else
    {
        ChipLogProgress(DeviceLayer, "Device[%s]: OFFLINE", mName);
    }

    if (changed)
    {
        HandleDeviceChange(this, kChanged_Reachable);
    }
}

void Device::SetName(const char * szName)
{
    bool changed = (strncmp(mName, szName, sizeof(mName)) != 0);

    ChipLogProgress(DeviceLayer, "Device[%s]: New Name=\"%s\"", mName, szName);

    chip::Platform::CopyString(mName, szName);

    if (changed)
    {
        HandleDeviceChange(this, kChanged_Name);
    }
}

void Device::SetLocation(std::string szLocation)
{
    bool changed = (mLocation.compare(szLocation) != 0);

    mLocation = szLocation;

    if (changed)
    {
        HandleDeviceChange(this, kChanged_Location);
    }
}

DeviceLight::DeviceLight(const char * szDeviceName, std::string szLocation) : Device(szDeviceName, szLocation)
{
    mOn = false;
}

bool DeviceLight::IsOn()
{
    return mOn;
}

void DeviceLight::SetColorMode(uint8_t aColorMode)
{
    bool changed;

    changed = aColorMode ^ mColorMode;
    mColorMode     = aColorMode;
    ChipLogProgress(DeviceLayer, "Device[%s]: %d", mName, aColorMode);

    if ((changed) && (mChanged_CB))
    {
        mChanged_CB(this, kChanged_ColorMode);
    }
}

void DeviceLight::SetOnOff(bool aOn)
{
    bool changed;

    changed = aOn ^ mOn;
    mOn     = aOn;
    ChipLogProgress(DeviceLayer, "Device[%s]: %s", mName, aOn ? "ON" : "OFF");

    if ((changed) && (mChanged_CB))
    {
        mChanged_CB(this, kChanged_OnOff);
    }
}

void DeviceLight::SetCurrentLevel(unsigned char aCurrentLevel)
{
    ChipLogProgress(DeviceLayer, "Device[%s] set level: %d", mName, aCurrentLevel);    
    bool changed;

    changed = aCurrentLevel ^ mCurrentLevel;
    mCurrentLevel = aCurrentLevel;
    if ((changed) && (mChanged_CB))
    {
        mChanged_CB(this, kChanged_CurrentLevel);
    }
}

void DeviceLight::SetCurrentHue(unsigned char aCurrentHue)
{
    ChipLogProgress(DeviceLayer, "Device[%s] set hue: %d", mName, aCurrentHue);
    bool changed;

    changed = aCurrentHue ^ mCurrentHue;
    mCurrentHue = aCurrentHue;
    if ((changed) && (mChanged_CB))
    {
        mChanged_CB(this, kChanged_CurrentHue);
    }
}

void DeviceLight::SetCurrentSaturation(unsigned char aCurrentSaturation)
{
    ChipLogProgress(DeviceLayer, "Device[%s] set saturation: %d", mName, aCurrentSaturation);
    bool changed;

    changed = aCurrentSaturation ^ mCurrentSaturation;
    mCurrentSaturation = aCurrentSaturation;
    if ((changed) && (mChanged_CB))
    {
        mChanged_CB(this, kChanged_CurrentSaturation);
    }
}

void DeviceLight::SetColorTemperatureMireds(unsigned short aColorTemperatureMireds)
{
    ChipLogProgress(DeviceLayer, "Device[%s] set mireds: %d", mName, aColorTemperatureMireds);
    bool changed;

    changed = aColorTemperatureMireds ^ mColorTemperatureMireds;
    mColorTemperatureMireds = aColorTemperatureMireds;
    if ((changed) && (mChanged_CB))
    {
        mChanged_CB(this, kChanged_ColorTemperatureMireds);
    }
}

uint8_t DeviceLight::GetColorMode()
{
    return mColorMode;
}

uint8_t DeviceLight::GetCurrentLevel()
{
    return mCurrentLevel;
}

uint8_t DeviceLight::GetCurrentHue()
{
    return mCurrentHue;
}

uint8_t DeviceLight::GetCurrentSaturation()
{
    return mCurrentSaturation;
}

uint16_t DeviceLight::GetColorTemperatureMireds()
{
    return mColorTemperatureMireds;
}

void DeviceLight::Toggle()
{
    bool aOn = !IsOn();
    SetOnOff(aOn);
}

void DeviceLight::SetColorModeVal(uint8_t aColorMode)
{
    mColorMode = aColorMode;
}

void DeviceLight::SetOnOffVal(bool aOn)
{
    mOn = aOn;
}

void DeviceLight::SetCurrentLevelVal(uint8_t aCurrentLevel)
{
    mCurrentLevel = aCurrentLevel;
}

void DeviceLight::SetCurrentHueVal(uint8_t aCurrentHue)
{
    mCurrentHue = aCurrentHue;
}

void DeviceLight::SetCurrentSaturationVal(uint8_t aCurrentSaturation)
{
    mCurrentSaturation = aCurrentSaturation;
}

void DeviceLight::SetColorTemperatureMiredsVal(uint16_t aColorTemperatureMireds)
{
    mColorTemperatureMireds = aColorTemperatureMireds;
}

void DeviceLight::SetChangeCallback(DeviceCallback_fn aChanged_CB)
{
    mChanged_CB = aChanged_CB;
}

void DeviceLight::HandleDeviceChange(Device * device, Device::Changed_t changeMask)
{
    if (mChanged_CB)
    {
        mChanged_CB(this, (DeviceLight::Changed_t) changeMask);
    }
}

DeviceContactSensor::DeviceContactSensor(const char * szDeviceName, std::string szLocation) : Device(szDeviceName, szLocation)
{
    mStatus = false;
}

bool DeviceContactSensor::IsOpen()
{
    return mStatus;
}

void DeviceContactSensor::Set(bool aStatus)
{
    mStatus = aStatus;
}

void DeviceContactSensor::SetChangeCallback(DeviceCallback_fn aChanged_CB)
{
    mChanged_CB = aChanged_CB;
}

void DeviceContactSensor::HandleDeviceChange(Device * device, Device::Changed_t changeMask)
{
    if (mChanged_CB)
    {
        mChanged_CB(this, (DeviceContactSensor::Changed_t) changeMask);
    }
}

DeviceSwitch::DeviceSwitch(const char * szDeviceName, std::string szLocation, uint32_t aFeatureMap) :
    Device(szDeviceName, szLocation)
{
    mNumberOfPositions = 2;
    mCurrentPosition   = 0;
    mMultiPressMax     = 2;
    mFeatureMap        = aFeatureMap;
}

void DeviceSwitch::SetNumberOfPositions(uint8_t aNumberOfPositions)
{
    bool changed;

    changed            = aNumberOfPositions != mNumberOfPositions;
    mNumberOfPositions = aNumberOfPositions;

    if ((changed) && (mChanged_CB))
    {
        mChanged_CB(this, kChanged_NumberOfPositions);
    }
}

void DeviceSwitch::SetCurrentPosition(uint8_t aCurrentPosition)
{
    bool changed;

    changed          = aCurrentPosition != mCurrentPosition;
    mCurrentPosition = aCurrentPosition;

    if ((changed) && (mChanged_CB))
    {
        mChanged_CB(this, kChanged_CurrentPosition);
    }
}

void DeviceSwitch::SetMultiPressMax(uint8_t aMultiPressMax)
{
    bool changed;

    changed        = aMultiPressMax != mMultiPressMax;
    mMultiPressMax = aMultiPressMax;

    if ((changed) && (mChanged_CB))
    {
        mChanged_CB(this, kChanged_MultiPressMax);
    }
}

void DeviceSwitch::SetChangeCallback(DeviceCallback_fn aChanged_CB)
{
    mChanged_CB = aChanged_CB;
}

void DeviceSwitch::HandleDeviceChange(Device * device, Device::Changed_t changeMask)
{
    if (mChanged_CB)
    {
        mChanged_CB(this, (DeviceSwitch::Changed_t) changeMask);
    }
}

DeviceTempSensor::DeviceTempSensor(const char * szDeviceName, std::string szLocation, int16_t min, int16_t max,
                                   int16_t measuredValue) :
    Device(szDeviceName, szLocation),
    mMin(min), mMax(max), mMeasurement(measuredValue)
{}

void DeviceTempSensor::SetMeasuredValue(int16_t measurement)
{
    // Limit measurement based on the min and max.
    if (measurement < mMin)
    {
        measurement = mMin;
    }
    else if (measurement > mMax)
    {
        measurement = mMax;
    }

    bool changed = mMeasurement != measurement;

    ChipLogProgress(DeviceLayer, "TempSensorDevice[%s]: New measurement=\"%d\"", mName, measurement);

    mMeasurement = measurement;

    if (changed && mChanged_CB)
    {
        mChanged_CB(this, kChanged_MeasurementValue);
    }
}

void DeviceTempSensor::SetChangeCallback(DeviceCallback_fn aChanged_CB)
{
    mChanged_CB = aChanged_CB;
}

void DeviceTempSensor::HandleDeviceChange(Device * device, Device::Changed_t changeMask)
{
    if (mChanged_CB)
    {
        mChanged_CB(this, (DeviceTempSensor::Changed_t) changeMask);
    }
}

void ComposedDevice::HandleDeviceChange(Device * device, Device::Changed_t changeMask)
{
    if (mChanged_CB)
    {
        mChanged_CB(this, (ComposedDevice::Changed_t) changeMask);
    }
}

void DevicePowerSource::HandleDeviceChange(Device * device, Device::Changed_t changeMask)
{
    if (mChanged_CB)
    {
        mChanged_CB(this, (DevicePowerSource::Changed_t) changeMask);
    }
}

void DevicePowerSource::SetBatChargeLevel(uint8_t aBatChargeLevel)
{
    bool changed;

    changed         = aBatChargeLevel != mBatChargeLevel;
    mBatChargeLevel = aBatChargeLevel;

    if ((changed) && (mChanged_CB))
    {
        mChanged_CB(this, kChanged_BatLevel);
    }
}

void DevicePowerSource::SetDescription(std::string aDescription)
{
    bool changed;

    changed      = aDescription != mDescription;
    mDescription = aDescription;

    if ((changed) && (mChanged_CB))
    {
        mChanged_CB(this, kChanged_Description);
    }
}

void DevicePowerSource::SetEndpointList(std::vector<chip::EndpointId> aEndpointList)
{
    bool changed  = aEndpointList != mEndpointList;
    mEndpointList = aEndpointList;

    if (changed && mChanged_CB)
    {
        mChanged_CB(this, kChanged_EndpointList);
    }
}

EndpointListInfo::EndpointListInfo(uint16_t endpointListId, std::string name, EndpointListTypeEnum type)
{
    mEndpointListId = endpointListId;
    mName           = name;
    mType           = type;
}

EndpointListInfo::EndpointListInfo(uint16_t endpointListId, std::string name, EndpointListTypeEnum type,
                                   chip::EndpointId endpointId)
{
    mEndpointListId = endpointListId;
    mName           = name;
    mType           = type;
    mEndpoints.push_back(endpointId);
}

void EndpointListInfo::AddEndpointId(chip::EndpointId endpointId)
{
    mEndpoints.push_back(endpointId);
}

Room::Room(std::string name, uint16_t endpointListId, EndpointListTypeEnum type, bool isVisible)
{
    mName           = name;
    mEndpointListId = endpointListId;
    mType           = type;
    mIsVisible      = isVisible;
}

Action::Action(uint16_t actionId, std::string name, ActionTypeEnum type, uint16_t endpointListId, uint16_t supportedCommands,
               ActionStateEnum status, bool isVisible)
{
    mActionId          = actionId;
    mName              = name;
    mType              = type;
    mEndpointListId    = endpointListId;
    mSupportedCommands = supportedCommands;
    mStatus            = status;
    mIsVisible         = isVisible;
}
