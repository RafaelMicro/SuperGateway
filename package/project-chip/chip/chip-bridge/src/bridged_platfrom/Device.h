
#ifndef __DEVICE_H__
#define __DEVICE_H__
#pragma once

#include "ClusterBase.h"

class DeviceOnOffLight : public DeviceBase, public ClusterOnOff, public ClusterLevelControl
{
public:
    DeviceOnOffLight(const char * szDeviceName, std::string szLocation);
    // uint8_t GetCurrentLevel();
    // void SetCurrentLevel(uint8_t aCurrentLevel);
    // bool GetOnOffState();
    // void SetOnOff(bool aOn);
    // void Toggle();
    // uint8_t mCurrentLevel;
    using DeviceCB_fn = std::function<void(DeviceOnOffLight *, Changed_t)>;
    DeviceCB_fn mChanged_CB;
    // static DeviceOnOffLight devices[CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT + 1];

private:
    void HandleDeviceChange(DeviceBase * device, Changed_t changeMask);
};

class DeviceOnOffLightSwitch : public DeviceBase, public ClusterOnOff
{
public:
    DeviceOnOffLightSwitch(const char * szDeviceName, std::string szLocation);
    // bool GetOnOffState();
    // void SetOnOff(bool aOn);
    // void Toggle();
    using DeviceCB_fn = std::function<void(DeviceOnOffLightSwitch *, Changed_t)>;
    DeviceCB_fn mChanged_CB;
    // static DeviceOnOffLightSwitch devices[CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT + 1];

private:
    void HandleDeviceChange(DeviceBase * device, Changed_t changeMask);
};


class DeviceContactSensor : public DeviceBase, public ClusterBooleanState
{
public:
    DeviceContactSensor(const char * szDeviceName, std::string szLocation);
    // bool GetStateValue();
    // void SetStateValue(bool aOn);
    using DeviceCB_fn = std::function<void(DeviceContactSensor *, Changed_t)>;
    DeviceCB_fn mChanged_CB;
    // static DeviceContactSensor devices[CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT + 1];

private:
    void HandleDeviceChange(DeviceBase * device, Changed_t changeMask);
};


// class Device
// {
// public:
//     static const int kDeviceNameSize = 32;

//     enum Changed_t
//     {
//         kChanged_Reachable = 1u << 0,
//         kChanged_Location  = 1u << 1,
//         kChanged_Name      = 1u << 2,
//         kChanged_Last      = kChanged_Name,
//     } Changed;

//     Device(const char * szDeviceName, std::string szLocation);
//     virtual ~Device() {}

//     bool IsReachable();
//     void SetReachable(bool aReachable);
//     void SetName(const char * szDeviceName);
//     void SetLocation(std::string szLocation);
//     inline void SetEndpointId(chip::EndpointId id) { mEndpointId = id; };
//     inline chip::EndpointId GetEndpointId() { return mEndpointId; };
//     inline void SetParentEndpointId(chip::EndpointId id) { mParentEndpointId = id; };
//     inline chip::EndpointId GetParentEndpointId() { return mParentEndpointId; };
//     inline char * GetName() { return mName; };
//     inline std::string GetLocation() { return mLocation; };
//     inline std::string GetZone() { return mZone; };
//     inline void SetZone(std::string zone) { mZone = zone; };

// private:
//     virtual void HandleDeviceChange(Device * device, Device::Changed_t changeMask) = 0;

// protected:
//     bool mReachable;
//     char mName[kDeviceNameSize];
//     std::string mLocation;
//     chip::EndpointId mEndpointId;
//     chip::EndpointId mParentEndpointId;
//     std::string mZone;
// };

// class DeviceOnOff : public Device
// {
// public:
//     enum Changed_t
//     {
//         kChanged_OnOff = kChanged_Last << 1,
//     } Changed;

//     DeviceOnOff(const char * szDeviceName, std::string szLocation);

//     virtual bool GetOnOffState();
//     virtual void SetOnOff(bool aOn);
//     virtual void Toggle();

//     using DeviceCallback_fn = std::function<void(DeviceOnOff *, DeviceOnOff::Changed_t)>;
//     void SetChangeCallback(DeviceCallback_fn aChanged_CB);

// private:
//     virtual void HandleDeviceChange(Device * device, Device::Changed_t changeMask);
//     bool mOn;
//     DeviceCallback_fn mChanged_CB;
// };




// class DeviceOnOff : public Device
// {
// public:
//     enum Changed_t
//     {
//         kChanged_OnOff = kChanged_Last << 1,
//     } Changed;

//     DeviceOnOff(const char * szDeviceName, std::string szLocation);

//     bool IsOn();
//     void SetOnOff(bool aOn);
//     void Toggle();

//     using DeviceCallback_fn = std::function<void(DeviceOnOff *, DeviceOnOff::Changed_t)>;
//     void SetChangeCallback(DeviceCallback_fn aChanged_CB);

// private:
//     void HandleDeviceChange(Device * device, Device::Changed_t changeMask);
//     bool mOn;
//     DeviceCallback_fn mChanged_CB;
// };

// class DeviceSwitch : public Device
// {
// public:
//     enum Changed_t
//     {
//         kChanged_NumberOfPositions = kChanged_Last << 1,
//         kChanged_CurrentPosition   = kChanged_Last << 2,
//         kChanged_MultiPressMax     = kChanged_Last << 3,
//     } Changed;

//     DeviceSwitch(const char * szDeviceName, std::string szLocation, uint32_t aFeatureMap);

//     void SetNumberOfPositions(uint8_t aNumberOfPositions);
//     void SetCurrentPosition(uint8_t aCurrentPosition);
//     void SetMultiPressMax(uint8_t aMultiPressMax);

//     inline uint8_t GetNumberOfPositions() { return mNumberOfPositions; };
//     inline uint8_t GetCurrentPosition() { return mCurrentPosition; };
//     inline uint8_t GetMultiPressMax() { return mMultiPressMax; };
//     inline uint32_t GetFeatureMap() { return mFeatureMap; };

//     using DeviceCallback_fn = std::function<void(DeviceSwitch *, DeviceSwitch::Changed_t)>;
//     void SetChangeCallback(DeviceCallback_fn aChanged_CB);

// private:
//     void HandleDeviceChange(Device * device, Device::Changed_t changeMask);

// private:
//     uint8_t mNumberOfPositions;
//     uint8_t mCurrentPosition;
//     uint8_t mMultiPressMax;
//     uint32_t mFeatureMap;
//     DeviceCallback_fn mChanged_CB;
// };

// class DeviceTempSensor : public Device
// {
// public:
//     enum Changed_t
//     {
//         kChanged_MeasurementValue = kChanged_Last << 1,
//     } Changed;

//     DeviceTempSensor(const char * szDeviceName, std::string szLocation, int16_t min, int16_t max, int16_t measuredValue);

//     inline int16_t GetMeasuredValue() { return mMeasurement; };
//     void SetMeasuredValue(int16_t measurement);

//     using DeviceCallback_fn = std::function<void(DeviceTempSensor *, DeviceTempSensor::Changed_t)>;
//     void SetChangeCallback(DeviceCallback_fn aChanged_CB);

//     const int16_t mMin;
//     const int16_t mMax;

// private:
//     void HandleDeviceChange(Device * device, Device::Changed_t changeMask);

// private:
//     int16_t mMeasurement;
//     DeviceCallback_fn mChanged_CB;
// };

// class ComposedDevice : public Device
// {
// public:
//     ComposedDevice(const char * szDeviceName, std::string szLocation) : Device(szDeviceName, szLocation){};

//     using DeviceCallback_fn = std::function<void(ComposedDevice *, ComposedDevice::Changed_t)>;

//     void SetChangeCallback(DeviceCallback_fn aChanged_CB);

// private:
//     void HandleDeviceChange(Device * device, Device::Changed_t changeMask);

// private:
//     DeviceCallback_fn mChanged_CB;
// };

// class DevicePowerSource : public Device
// {
// public:
//     enum Changed_t
//     {
//         kChanged_BatLevel     = kChanged_Last << 1,
//         kChanged_Description  = kChanged_Last << 2,
//         kChanged_EndpointList = kChanged_Last << 3,
//     } Changed;

//     DevicePowerSource(const char * szDeviceName, std::string szLocation,
//                       chip::BitFlags<chip::app::Clusters::PowerSource::Feature> aFeatureMap) :
//         Device(szDeviceName, szLocation),
//         mFeatureMap(aFeatureMap){};

//     using DeviceCallback_fn = std::function<void(DevicePowerSource *, DevicePowerSource::Changed_t)>;
//     void SetChangeCallback(DeviceCallback_fn aChanged_CB) { mChanged_CB = aChanged_CB; }

//     void SetBatChargeLevel(uint8_t aBatChargeLevel);
//     void SetDescription(std::string aDescription);
//     void SetEndpointList(std::vector<chip::EndpointId> mEndpointList);

//     inline uint32_t GetFeatureMap() { return mFeatureMap.Raw(); };
//     inline uint8_t GetBatChargeLevel() { return mBatChargeLevel; };
//     inline uint8_t GetOrder() { return mOrder; };
//     inline uint8_t GetStatus() { return mStatus; };
//     inline std::string GetDescription() { return mDescription; };
//     std::vector<chip::EndpointId> & GetEndpointList() { return mEndpointList; }

// private:
//     void HandleDeviceChange(Device * device, Device::Changed_t changeMask);

// private:
//     uint8_t mBatChargeLevel  = 0;
//     uint8_t mOrder           = 0;
//     uint8_t mStatus          = 0;
//     std::string mDescription = "Primary Battery";
//     chip::BitFlags<chip::app::Clusters::PowerSource::Feature> mFeatureMap;
//     DeviceCallback_fn mChanged_CB;
//     // This is linux, vector is not going to kill us here and it's easier. Plus, post c++11, storage is contiguous with .data()
//     std::vector<chip::EndpointId> mEndpointList;
// };




#endif // __DEVICE_H__
