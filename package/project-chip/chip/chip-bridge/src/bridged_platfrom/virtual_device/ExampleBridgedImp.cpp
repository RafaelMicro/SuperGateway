
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
#include <platform/CommissionableDataProvider.h>
#include <setup_payload/QRCodeSetupPayloadGenerator.h>
#include <setup_payload/SetupPayload.h>

#include "ExampleBridgedImp.h"
#include "BridgedManager.h"
#include "CommissionableInit.h"
#include "platfrom/Device.h"
#include "DeviceLibrary.h"
#include "DeviceBase.h"
#include "ApplicationCluster.h"
#include "platfrom/cli.h"

#include <cassert>
#include <iostream>
#include <vector>

using namespace chip;
using namespace chip::app;
using namespace chip::Credentials;
using namespace chip::Inet;
using namespace chip::Transport;
using namespace chip::DeviceLayer;
using namespace Rafael::DeviceLibrary;

namespace RafaelCluster
{

void * bridge_cli_thread(void * context)
{
    uint8_t ret;
    CLI_Token_t TK;
    str_space_t input;
    input.ptr = (char *)malloc(sizeof(char) * CLI_MAX_BUFFER_SIZE);
    input.len = CLI_MAX_BUFFER_SIZE;
    while (true) {
        fgets((char *)input.ptr, input.len, stdin);
        ret = cli_interpreter(&input, &TK);
    };
    return nullptr;
};

static BridgedManagerImp sInstance;
BridgedManagerImp & BridgedManagerImp::GetDefaultInstance() { return sInstance; };
BridgedManager & BridgedMgrImpl() { return BridgedManagerImp::GetDefaultInstance(); };
void BridgedManagerImp::Init(){
    pthread_t poll_thread;
    int res = pthread_create(&poll_thread, nullptr, bridge_cli_thread, nullptr);
    if (res) {
        printf("Error creating polling thread: %d\n", res);
        exit(1);
    }
    return;
};

template<class T>
EmberAfStatus HandleReadColorControlAttribute(T * dev, chip::AttributeId attId, uint8_t *buffer, uint16_t maxLen)
{
    ChipLogProgress(DeviceLayer, "%s", __FUNCTION__);
    switch(attId)
    {
        case Clusters::ColorControl::Attributes::CurrentHue::Id:{
            *buffer = dev->GetCurrentHue();
            break;}
        case Clusters::ColorControl::Attributes::CurrentSaturation::Id:{
            *buffer = dev->GetCurrentSaturation();
            break;}
        case Clusters::ColorControl::Attributes::RemainingTime::Id:{
            uint16_t rev = 0;
            memcpy(buffer, &rev, sizeof(rev));
            break;}
        case Clusters::ColorControl::Attributes::CurrentX::Id:{
            auto rev = dev->GetCurrentX();
            memcpy(buffer, &rev, sizeof(rev));
            break;}
        case Clusters::ColorControl::Attributes::CurrentY::Id:{
            auto rev = dev->GetCurrentY();
            memcpy(buffer, &rev, sizeof(rev));
            break;}
        case Clusters::ColorControl::Attributes::DriftCompensation::Id:{
            *buffer = 0;
            break;}
        case Clusters::ColorControl::Attributes::CompensationText::Id:{
            break;}
        case Clusters::ColorControl::Attributes::ColorTemperatureMireds::Id:{
            uint16_t rev = 0;
            memcpy(buffer, &rev, sizeof(rev));
            break;}
        case Clusters::ColorControl::Attributes::ColorMode::Id:{
            *buffer = 1;
            break;}
        case Clusters::ColorControl::Attributes::Options::Id:{
            break;}
        case Clusters::ColorControl::Attributes::EnhancedCurrentHue::Id:{
            auto rev = dev->GetEnhancedCurrentHue();
            memcpy(buffer, &rev, sizeof(rev));
            break;}
        case Clusters::ColorControl::Attributes::EnhancedColorMode::Id:{
            auto rev = dev->GetEnhancedColorMode();
            memcpy(buffer, &rev, sizeof(rev));
            break;}
        case Clusters::ColorControl::Attributes::ColorLoopActive::Id:{
            *buffer = 0;
            break;}
        case Clusters::ColorControl::Attributes::ColorLoopDirection::Id:{
            *buffer = 0;
            break;}
        case Clusters::ColorControl::Attributes::ColorLoopTime::Id:{
            uint16_t rev = 0x0019;
            memcpy(buffer, &rev, sizeof(rev));
            break;}
        case Clusters::ColorControl::Attributes::ColorLoopStartEnhancedHue::Id:{
            uint16_t rev = 0x2300;
            memcpy(buffer, &rev, sizeof(rev));
            break;}
        case Clusters::ColorControl::Attributes::ColorLoopStoredEnhancedHue::Id:{
            uint16_t rev = 0;
            memcpy(buffer, &rev, sizeof(rev));
            break;}
        case Clusters::ColorControl::Attributes::ColorCapabilities::Id:{
            uint16_t rev = 0;
            memcpy(buffer, &rev, sizeof(rev));
            break;}
        case Clusters::ColorControl::Attributes::ColorTempPhysicalMinMireds::Id:{
            uint16_t rev = 0;
            memcpy(buffer, &rev, sizeof(rev));
            break;}
        case Clusters::ColorControl::Attributes::ColorTempPhysicalMaxMireds::Id:{
            uint16_t rev = 0xFEFF;
            memcpy(buffer, &rev, sizeof(rev));
            break;}
        case Clusters::ColorControl::Attributes::CoupleColorTempToLevelMinMireds::Id:{
            uint16_t rev = 0;
            memcpy(buffer, &rev, sizeof(rev));
            break;}
        case Clusters::ColorControl::Attributes::StartUpColorTemperatureMireds::Id:{
            uint16_t rev = 0;
            memcpy(buffer, &rev, sizeof(rev));
            break;}
        case Clusters::ColorControl::Attributes::NumberOfPrimaries::Id:{
            *buffer = 0;
            break;}
        case Clusters::ColorControl::Attributes::Primary1X::Id:{
            uint16_t rev = 0;
            memcpy(buffer, &rev, sizeof(rev));
            break;}
        case Clusters::ColorControl::Attributes::Primary1Y::Id:{
            uint16_t rev = 0;
            memcpy(buffer, &rev, sizeof(rev));
            break;}
        case Clusters::ColorControl::Attributes::Primary1Intensity::Id:{
            *buffer = 0;
            break;}
        case Clusters::ColorControl::Attributes::Primary2X::Id:{
            uint16_t rev = 0;
            memcpy(buffer, &rev, sizeof(rev));
            break;}
        case Clusters::ColorControl::Attributes::Primary2Y::Id:{
            uint16_t rev = 0;
            memcpy(buffer, &rev, sizeof(rev));
            break;}
        case Clusters::ColorControl::Attributes::Primary2Intensity::Id:{
            *buffer = 0;
            break;}
        case Clusters::ColorControl::Attributes::Primary3X::Id:{
            uint16_t rev = 0;
            memcpy(buffer, &rev, sizeof(rev));
            break;}
        case Clusters::ColorControl::Attributes::Primary3Y::Id:{
            uint16_t rev = 0;
            memcpy(buffer, &rev, sizeof(rev));
            break;}
        case Clusters::ColorControl::Attributes::Primary3Intensity::Id:{
            *buffer = 0;
            break;}
        case Clusters::ColorControl::Attributes::Primary4X::Id:{
            uint16_t rev = 0;
            memcpy(buffer, &rev, sizeof(rev));
            break;}
        case Clusters::ColorControl::Attributes::Primary4Y::Id:{
            uint16_t rev = 0;
            memcpy(buffer, &rev, sizeof(rev));
            break;}
        case Clusters::ColorControl::Attributes::Primary4Intensity::Id:{
            *buffer = 0;
            break;}
        case Clusters::ColorControl::Attributes::Primary5X::Id:{
            uint16_t rev = 0;
            memcpy(buffer, &rev, sizeof(rev));
            break;}
        case Clusters::ColorControl::Attributes::Primary5Y::Id:{
            uint16_t rev = 0;
            memcpy(buffer, &rev, sizeof(rev));
            break;}
        case Clusters::ColorControl::Attributes::Primary5Intensity::Id:{
            *buffer = 0;
            break;}
        case Clusters::ColorControl::Attributes::Primary6X::Id:{
            uint16_t rev = 0;
            memcpy(buffer, &rev, sizeof(rev));
            break;}
        case Clusters::ColorControl::Attributes::Primary6Y::Id:{
            uint16_t rev = 0;
            memcpy(buffer, &rev, sizeof(rev));
            break;}
        case Clusters::ColorControl::Attributes::Primary6Intensity::Id:{
            *buffer = 0;
            break;}
        case Clusters::ColorControl::Attributes::WhitePointX::Id:{
            uint16_t rev = 0;
            memcpy(buffer, &rev, sizeof(rev));
            break;}
        case Clusters::ColorControl::Attributes::WhitePointY::Id:{
            uint16_t rev = 0;
            memcpy(buffer, &rev, sizeof(rev));
            break;}
        case Clusters::ColorControl::Attributes::ColorPointRX::Id:{
            uint16_t rev = 0;
            memcpy(buffer, &rev, sizeof(rev));
            break;}
        case Clusters::ColorControl::Attributes::ColorPointRY::Id:{
            uint16_t rev = 0;
            memcpy(buffer, &rev, sizeof(rev));
            break;}
        case Clusters::ColorControl::Attributes::ColorPointRIntensity::Id:{
            *buffer = 0;
            break;}
        case Clusters::ColorControl::Attributes::ColorPointGX::Id:{
            uint16_t rev = 0;
            memcpy(buffer, &rev, sizeof(rev));
            break;}
        case Clusters::ColorControl::Attributes::ColorPointGY::Id:{
            uint16_t rev = 0;
            memcpy(buffer, &rev, sizeof(rev));
            break;}
        case Clusters::ColorControl::Attributes::ColorPointGIntensity::Id:{
            *buffer = 0;
            break;}
        case Clusters::ColorControl::Attributes::ColorPointBX::Id:{
            uint16_t rev = 0;
            memcpy(buffer, &rev, sizeof(rev));
            break;}
        case Clusters::ColorControl::Attributes::ColorPointBY::Id:{
            uint16_t rev = 0;
            memcpy(buffer, &rev, sizeof(rev));
            break;}
        case Clusters::ColorControl::Attributes::ColorPointBIntensity::Id:{
            *buffer = 0;
            break;}
        case Clusters::ColorControl::Attributes::ClusterRevision::Id:{
            uint16_t rev = ZCL_CLUSTER_REVISION_COLOR_CONTROL;
            memcpy(buffer, &rev, sizeof(rev));
            break;}
        case Clusters::ColorControl::Attributes::FeatureMap::Id:{
            uint16_t rev = 0x1F;
            memcpy(buffer, &rev, sizeof(rev));
            break;}
        default: { return EMBER_ZCL_STATUS_FAILURE; }
    }
    return EMBER_ZCL_STATUS_SUCCESS;
};

template<class T>
EmberAfStatus HandleWriteColorControlAttribute(T * dev, chip::AttributeId attId, uint8_t * buffer)
{
    switch(attId)
    {
        case Clusters::ColorControl::Attributes::ClusterRevision::Id:{
            uint16_t rev = ZCL_CLUSTER_REVISION_COLOR_CONTROL;
            memcpy(buffer, &rev, sizeof(rev));
            break;}
        case Clusters::ColorControl::Attributes::CurrentHue::Id:{
            dev->SetCurrentHue(*buffer);
            break;}
        case Clusters::ColorControl::Attributes::CurrentSaturation::Id:{
            dev->SetCurrentSaturation(*buffer);
            break;}
        case Clusters::ColorControl::Attributes::CurrentX::Id:{
            dev->SetCurrentX(*buffer);
            break;}
        case Clusters::ColorControl::Attributes::CurrentY::Id:{
            dev->SetCurrentY(*buffer);
            break;}
        case Clusters::ColorControl::Attributes::EnhancedCurrentHue::Id:{
            dev->SetEnhancedCurrentHue(*buffer);
            break;}
        case Clusters::ColorControl::Attributes::EnhancedColorMode::Id:{
            dev->SetEnhancedColorMode(*buffer);
            break;}
        default: { return EMBER_ZCL_STATUS_FAILURE; }
    }
    return EMBER_ZCL_STATUS_SUCCESS;
};

template<class T>
EmberAfStatus HandleReadLevelControlAttribute(T * dev, chip::AttributeId attId, uint8_t *buffer, uint16_t maxLen)
{
    switch(attId)
    {
        case Clusters::LevelControl::Attributes::CurrentLevel::Id:{
            *buffer = dev->GetCurrentLevel();
            break;}
        case Clusters::LevelControl::Attributes::RemainingTime::Id:{
            uint16_t rev = 0;
            memcpy(buffer, &rev, sizeof(rev));
            break;}
        case Clusters::LevelControl::Attributes::MinLevel::Id:{
            *buffer = 1;
            break;}
        case Clusters::LevelControl::Attributes::MaxLevel::Id:{
            *buffer = 254;
            break;}
        case Clusters::LevelControl::Attributes::CurrentFrequency::Id:{
            uint16_t rev = 0;
            memcpy(buffer, &rev, sizeof(rev));
            break;}
        case Clusters::LevelControl::Attributes::MinFrequency::Id:{
            uint16_t rev = 0;
            memcpy(buffer, &rev, sizeof(rev));
            break;}
        case Clusters::LevelControl::Attributes::MaxFrequency::Id:{
            uint16_t rev = 0;
            memcpy(buffer, &rev, sizeof(rev));
            break;}
        case Clusters::LevelControl::Attributes::OnOffTransitionTime::Id:{
            uint16_t rev = 0;
            memcpy(buffer, &rev, sizeof(rev));
            break;}
        case Clusters::LevelControl::Attributes::OnLevel::Id:{
            *buffer = 1;
            break;}
        case Clusters::LevelControl::Attributes::OnTransitionTime::Id:{
            uint16_t rev = 0;
            memcpy(buffer, &rev, sizeof(rev));
            break;}
        case Clusters::LevelControl::Attributes::OffTransitionTime::Id:{
            uint16_t rev = 0;
            memcpy(buffer, &rev, sizeof(rev));
            break;}
        case Clusters::LevelControl::Attributes::DefaultMoveRate::Id:{
            uint16_t rev = 0;
            memcpy(buffer, &rev, sizeof(rev));
            break;}
        case Clusters::LevelControl::Attributes::Options::Id:{
            uint16_t rev = 0;
            memcpy(buffer, &rev, sizeof(rev));
            break;}
        case Clusters::LevelControl::Attributes::StartUpCurrentLevel::Id:{
            uint16_t rev = 0;
            memcpy(buffer, &rev, sizeof(rev));
            break;}
        case Clusters::LevelControl::Attributes::ClusterRevision::Id:{
            uint16_t rev = ZCL_CLUSTER_REVISION_LEVEL_CONTROL;
            memcpy(buffer, &rev, sizeof(rev));
            break;}
        case Clusters::LevelControl::Attributes::FeatureMap::Id:{
            uint16_t rev = ZCL_CLUSTER_REVISION_LEVEL_CONTROL;
            memcpy(buffer, &rev, sizeof(rev));
            break;}
        default: { return EMBER_ZCL_STATUS_FAILURE; }
    }
    return EMBER_ZCL_STATUS_SUCCESS;
};

template<class T>
EmberAfStatus HandleWriteLevelControlAttribute(T * dev, chip::AttributeId attId, uint8_t * buffer)
{
    switch(attId)
    {
        case Clusters::LevelControl::Attributes::CurrentLevel::Id:{
            dev->SetCurrentLevel(*buffer);
            break;}
        case Clusters::LevelControl::Attributes::ClusterRevision::Id:{
            uint16_t rev = ZCL_CLUSTER_REVISION_LEVEL_CONTROL;
            memcpy(buffer, &rev, sizeof(rev));
            break;}
        default: { return EMBER_ZCL_STATUS_FAILURE; }
    }
    return EMBER_ZCL_STATUS_SUCCESS;
};

template<class T>
EmberAfStatus HandleReadScenesAttribute(T *dev, chip::AttributeId attId, uint8_t * buffer, uint16_t maxLen)
{
    switch(attId)
    {
        case Clusters::Scenes::Attributes::NameSupport::Id:{
            *buffer = 0;
            break;}
        case Clusters::Scenes::Attributes::ClusterRevision::Id:{
            uint16_t rev = ZCL_CLUSTER_REVISION_SCENES;
            memcpy(buffer, &rev, sizeof(rev));
            break;}
        case Clusters::Scenes::Attributes::FeatureMap::Id:{
            uint16_t rev = ZCL_CLUSTER_REVISION_SCENES;
            memcpy(buffer, &rev, sizeof(rev));
            break;}
        default: { return EMBER_ZCL_STATUS_FAILURE; }
    }
    return EMBER_ZCL_STATUS_SUCCESS;
};

EmberAfStatus HandleWriteScenesAttribute(chip::AttributeId attId, uint8_t * buffer)
{
    switch(attId)
    {
        case Clusters::Scenes::Attributes::NameSupport::Id:{ break; }
        case Clusters::Scenes::Attributes::ClusterRevision::Id:{
            uint16_t rev = ZCL_CLUSTER_REVISION_SCENES;
            memcpy(buffer, &rev, sizeof(rev));
            break;}
        default: { return EMBER_ZCL_STATUS_FAILURE; }
    }
    return EMBER_ZCL_STATUS_SUCCESS;
};

template<class T>
EmberAfStatus HandleReadGroupsAttribute(T * dev, chip::AttributeId attId, uint8_t * buffer, uint16_t maxLen)
{
    switch(attId)
    {
        case Clusters::Groups::Attributes::NameSupport::Id:{
            *buffer = 0;
            break;}
        case Clusters::Groups::Attributes::ClusterRevision::Id:{
            uint16_t rev = ZCL_CLUSTER_REVISION_GROUPS;
            memcpy(buffer, &rev, sizeof(rev));
            break;}
        case Clusters::Groups::Attributes::FeatureMap::Id:{
            uint16_t rev = ZCL_CLUSTER_REVISION_GROUPS;
            memcpy(buffer, &rev, sizeof(rev));
            break;}
        default: { return EMBER_ZCL_STATUS_FAILURE; }
    }
    return EMBER_ZCL_STATUS_SUCCESS;
};

EmberAfStatus HandleWriteGroupsAttribute(chip::AttributeId attId, uint8_t * buffer)
{
    switch(attId)
    {
        case Clusters::Groups::Attributes::NameSupport::Id:{ break; }
        case Clusters::Groups::Attributes::ClusterRevision::Id:{
            uint16_t rev = ZCL_CLUSTER_REVISION_GROUPS;
            memcpy(buffer, &rev, sizeof(rev));
            break;}
        default: { return EMBER_ZCL_STATUS_FAILURE; }
    }
    return EMBER_ZCL_STATUS_SUCCESS;
};

template<class T>
EmberAfStatus HandleReadIdentifyAttribute(T *dev, chip::AttributeId attId, uint8_t * buffer, uint16_t maxLen)
{
    switch(attId)
    {
        case Clusters::Identify::Attributes::IdentifyTime::Id:{
            uint16_t rev = 0;
            memcpy(buffer, &rev, sizeof(rev));
            break;}
        case Clusters::Identify::Attributes::IdentifyType::Id:{
            *buffer = 0;
            break;}
        case Clusters::Identify::Attributes::ClusterRevision::Id:{
            uint16_t rev = ZCL_CLUSTER_REVISION_IDENTIFY;
            memcpy(buffer, &rev, sizeof(rev));
            break;}
        case Clusters::Identify::Attributes::FeatureMap::Id:{
            uint16_t rev = ZCL_CLUSTER_REVISION_IDENTIFY;
            memcpy(buffer, &rev, sizeof(rev));
            break;}
        default: { return EMBER_ZCL_STATUS_FAILURE; }
    }
    return EMBER_ZCL_STATUS_SUCCESS;
};

EmberAfStatus HandleWriteIdentifyAttribute(chip::AttributeId attId, uint8_t * buffer)
{
    switch(attId)
    {
        case Clusters::Identify::Attributes::IdentifyTime::Id:{ break; }
        case Clusters::Identify::Attributes::IdentifyType::Id:{ break; }
        case Clusters::Identify::Attributes::ClusterRevision::Id:{
            uint16_t rev = ZCL_CLUSTER_REVISION_IDENTIFY;
            memcpy(buffer, &rev, sizeof(rev));
            break;}
        default: { return EMBER_ZCL_STATUS_SUCCESS; }
    }
    return EMBER_ZCL_STATUS_SUCCESS;
};

template<class T>
EmberAfStatus HandleReadOnOffAttribute(T* dev, chip::AttributeId attId, uint8_t * buffer, uint16_t maxLen)
{
    switch(attId)
    {
        case Clusters::OnOff::Attributes::OnOff::Id:{
            *buffer = dev->GetOnOffState() ? 1 : 0;
            break;}
        case Clusters::OnOff::Attributes::GlobalSceneControl::Id:{
            *buffer = 1;
            break;}
        case Clusters::OnOff::Attributes::OnTime::Id:{
            uint16_t rev = 0;
            memcpy(buffer, &rev, sizeof(rev));
            break;}
        case Clusters::OnOff::Attributes::OffWaitTime::Id:{
            uint16_t rev = 0;
            memcpy(buffer, &rev, sizeof(rev));
            break;}
        case Clusters::OnOff::Attributes::ClusterRevision::Id:{
            uint16_t rev = ZCL_CLUSTER_REVISION_ON_OFF;
            memcpy(buffer, &rev, sizeof(rev));
            break;}
        case Clusters::OnOff::Attributes::FeatureMap::Id:{
            uint16_t rev = ZCL_CLUSTER_REVISION_ON_OFF;
            memcpy(buffer, &rev, sizeof(rev));
            break;}
        default: { return EMBER_ZCL_STATUS_FAILURE; }
    }
    return EMBER_ZCL_STATUS_SUCCESS;
};

template<class T>
EmberAfStatus HandleWriteOnOffAttribute(T* dev, chip::AttributeId attId, uint8_t* buffer)
{
    switch(attId)
    {
        case Clusters::OnOff::Attributes::OnOff::Id:{
            dev->SetOnOff((*buffer)?true:false);
            break;
        }
        case Clusters::OnOff::Attributes::ClusterRevision::Id:{
            uint16_t rev = ZCL_CLUSTER_REVISION_ON_OFF;
            memcpy(buffer, &rev, sizeof(rev));
            break;
        }
        default: { return EMBER_ZCL_STATUS_SUCCESS; }
    }
    return EMBER_ZCL_STATUS_SUCCESS;
};

template<class T>
EmberAfStatus HandleReadBooleanStateAttribute(T* dev, chip::AttributeId attId, uint8_t * buffer, uint16_t maxLen)
{
    switch(attId)
    {
        case Clusters::BooleanState::Attributes::StateValue::Id:{
            *buffer = dev->GetStateValue() ? 1 : 0;
            break;}
        case Clusters::BooleanState::Attributes::ClusterRevision::Id:{
            uint16_t rev = ZCL_CLUSTER_REVISION_BOOLEAN_STATE;
            memcpy(buffer, &rev, sizeof(rev));
            break;}
        case Clusters::BooleanState::Attributes::FeatureMap::Id:{
            uint16_t rev = (0u);
            memcpy(buffer, &rev, sizeof(rev));
            break;}
        default: { return EMBER_ZCL_STATUS_FAILURE; }
    }
    return EMBER_ZCL_STATUS_SUCCESS;
};

template<class T>
EmberAfStatus HandleWriteBooleanStateAttribute(T* dev, chip::AttributeId attId, uint8_t* buffer)
{
    switch(attId)
    {
        case Clusters::BooleanState::Attributes::StateValue::Id:{
            dev->SetStateValue((*buffer)?true:false);
            break;}
        case Clusters::BooleanState::Attributes::ClusterRevision::Id:{
            uint16_t rev = ZCL_CLUSTER_REVISION_BOOLEAN_STATE;
            memcpy(buffer, &rev, sizeof(rev));
            break;}
        default: { return EMBER_ZCL_STATUS_SUCCESS; }
    }
    return EMBER_ZCL_STATUS_SUCCESS;
};

template<class T>
EmberAfStatus HandleReadBridgedDeviceBasicAttribute(T * dev, chip::AttributeId attId, uint8_t * buffer, uint16_t maxLen)
{    
    switch(attId)
    {
        case Clusters::BridgedDeviceBasicInformation::Attributes::Reachable::Id:{
            *buffer = dev->IsReachable() ? 1 : 0;
            break;}
        case Clusters::BridgedDeviceBasicInformation::Attributes::NodeLabel::Id:{
            // MutableByteSpan zclNameSpan(buffer, maxLen);
            // MakeZclCharString(zclNameSpan, dev->GetName());
            break;}
        case Clusters::BridgedDeviceBasicInformation::Attributes::ClusterRevision::Id:{
            uint16_t rev = (2u);
            memcpy(buffer, &rev, sizeof(rev));
            break;}
        case Clusters::BridgedDeviceBasicInformation::Attributes::FeatureMap::Id:{
            uint32_t featureMap = (0u);
            memcpy(buffer, &featureMap, sizeof(featureMap));
            break;}
        default: { return EMBER_ZCL_STATUS_FAILURE; }
    }
    return EMBER_ZCL_STATUS_SUCCESS;
};


EmberAfStatus HandleOnOffLightAttribute(DeviceAction action, uint16_t endpoint, 
    ClusterId clusterId, chip::AttributeId attId, uint8_t * buf, uint16_t maxLen)
{
    auto dev = Rafael::DeviceLibrary::DeviceMgr().GetDevice<DeviceOnOffLight, DeviceAttOnOffLight>((uint16_t)endpoint);
    if(action == DeviceAction::DeviceReadAttrs)
    {
        switch(clusterId)
        {
        case Clusters::BridgedDeviceBasicInformation::Id: { return HandleReadBridgedDeviceBasicAttribute(dev->device, attId, buf, maxLen); }
        case Clusters::OnOff::Id: { return HandleReadOnOffAttribute(dev->device, attId, buf, maxLen); }
        case Clusters::LevelControl::Id: { return HandleReadLevelControlAttribute(dev->device, attId, buf, maxLen); }
        case Clusters::Scenes::Id: { return HandleReadScenesAttribute(dev->device, attId, buf, maxLen); }
        case Clusters::Groups::Id: { return HandleReadGroupsAttribute(dev->device, attId, buf, maxLen); }
        case Clusters::Identify::Id: { return HandleReadIdentifyAttribute(dev->device, attId, buf, maxLen); }
        default: { return EMBER_ZCL_STATUS_FAILURE; }
        }
    }
    else if(action == DeviceAction::DeviceWriteAttrs)
    {
        switch (clusterId)
        {
        case Clusters::OnOff::Id:{ return HandleWriteOnOffAttribute(dev->device, attId, buf); }
        case Clusters::LevelControl::Id:{ return HandleWriteLevelControlAttribute(dev->device, attId, buf); }
        case Clusters::Scenes::Id:{ return HandleWriteScenesAttribute(attId, buf); }
        case Clusters::Groups::Id:{ return HandleWriteGroupsAttribute(attId, buf); }
        case Clusters::Identify::Id:{ return HandleWriteIdentifyAttribute(attId, buf); }
        default: { return EMBER_ZCL_STATUS_FAILURE; }
        }
    }
    return EMBER_ZCL_STATUS_SUCCESS;
};

EmberAfStatus HandleColorTemperatureLightAttribute(DeviceAction action, uint16_t endpoint, 
    ClusterId clusterId, chip::AttributeId attId, uint8_t * buf, uint16_t maxLen)
{
    ChipLogProgress(DeviceLayer, "%s", __FUNCTION__);
    auto dev = Rafael::DeviceLibrary::DeviceMgr().GetDevice<DeviceColorTemperatureLight, DeviceAttColorTemperatureLight>((uint16_t)endpoint);
    if(action == DeviceAction::DeviceReadAttrs)
    {
        switch(clusterId)
        {
        case Clusters::BridgedDeviceBasicInformation::Id: { return HandleReadBridgedDeviceBasicAttribute(dev->device, attId, buf, maxLen); }
        case Clusters::OnOff::Id: { return HandleReadOnOffAttribute(dev->device, attId, buf, maxLen); }
        case Clusters::LevelControl::Id: { return HandleReadLevelControlAttribute(dev->device, attId, buf, maxLen); }
        case Clusters::ColorControl::Id: { return HandleReadColorControlAttribute(dev->device, attId, buf, maxLen); }
        case Clusters::Scenes::Id: { return HandleReadScenesAttribute(dev->device, attId, buf, maxLen); }
        case Clusters::Groups::Id: { return HandleReadGroupsAttribute(dev->device, attId, buf, maxLen); }
        case Clusters::Identify::Id: { return HandleReadIdentifyAttribute(dev->device, attId, buf, maxLen); }
        default: { return EMBER_ZCL_STATUS_FAILURE; }
        }
    }
    else if(action == DeviceAction::DeviceWriteAttrs)
    {
        switch (clusterId)
        {
        case Clusters::OnOff::Id:{ return HandleWriteOnOffAttribute(dev->device, attId, buf); }
        case Clusters::LevelControl::Id:{ return HandleWriteLevelControlAttribute(dev->device, attId, buf); }
        case Clusters::ColorControl::Id: { return HandleWriteColorControlAttribute(dev->device, attId, buf); }
        case Clusters::Scenes::Id:{ return HandleWriteScenesAttribute(attId, buf); }
        case Clusters::Groups::Id:{ return HandleWriteGroupsAttribute(attId, buf); }
        case Clusters::Identify::Id:{ return HandleWriteIdentifyAttribute(attId, buf); }
        default: { return EMBER_ZCL_STATUS_FAILURE; }
        }
    }
    return EMBER_ZCL_STATUS_SUCCESS;
};

EmberAfStatus HandleOnOffLightSwitchAttribute(DeviceAction action, uint16_t endpointIndex, 
    ClusterId clusterId, chip::AttributeId attId, uint8_t * buf, uint16_t maxLen)
{
    auto dev = Rafael::DeviceLibrary::DeviceMgr().GetDevice<DeviceOnOffLightSwitch, DeviceAttOnOffLightSwitch>((uint16_t)endpointIndex);
    if(action == DeviceAction::DeviceReadAttrs)
    {
        switch(clusterId)
        {
        case Clusters::BridgedDeviceBasicInformation::Id: { return HandleReadBridgedDeviceBasicAttribute(dev->device, attId, buf, maxLen);}
        case Clusters::OnOff::Id: { return HandleReadOnOffAttribute(dev->device, attId, buf, maxLen); }
        default: { return EMBER_ZCL_STATUS_FAILURE; }
        }
    }
    else if(action == DeviceAction::DeviceWriteAttrs)
    {
        switch (clusterId)
        {
        case Clusters::OnOff::Id:{ return HandleWriteOnOffAttribute(dev->device, attId, buf); }
        default: { return EMBER_ZCL_STATUS_FAILURE; }
        }
    }
    return EMBER_ZCL_STATUS_SUCCESS;
};

EmberAfStatus HandleContactSensorAttribute(DeviceAction action, uint16_t endpointIndex, 
    ClusterId clusterId, chip::AttributeId attId, uint8_t * buf, uint16_t maxLen)
{
    auto dev = Rafael::DeviceLibrary::DeviceMgr().GetDevice<DeviceContactSensor, DeviceAttContactSensor>((uint16_t)endpointIndex);
    if(action == DeviceAction::DeviceReadAttrs)
    {
        switch(clusterId)
        {
        case Clusters::BridgedDeviceBasicInformation::Id: { return HandleReadBridgedDeviceBasicAttribute(dev->device, attId, buf, maxLen); }
        case Clusters::BooleanState::Id: { return HandleReadBooleanStateAttribute(dev->device, attId, buf, maxLen); }
        default: { return EMBER_ZCL_STATUS_FAILURE; }
        }
    }
    else if(action == DeviceAction::DeviceWriteAttrs)
    {
        switch (clusterId)
        {
        case Clusters::BooleanState::Id:{ return HandleWriteBooleanStateAttribute(dev->device, attId, buf); }
        default: { return EMBER_ZCL_STATUS_FAILURE; }
        }
    }
    return EMBER_ZCL_STATUS_SUCCESS;
};

EmberAfStatus BridgedManagerImp::BridgedHandleReadEvent(uint16_t endpointIndex, EndpointId endpoint, 
    ClusterId clusterId, const EmberAfAttributeMetadata * attributeMetadata, uint8_t * buffer, uint16_t maxReadLength)
{
    ChipLogProgress(DeviceLayer, "%s", __FUNCTION__);
    deviceEP_t* dev = DeviceManager::GetDeviceList(endpoint);
    switch (dev->deviceType)
    {
    case (DEVICE_TYPE_ON_OFF_LIGHT):{ return HandleOnOffLightAttribute(DeviceAction::DeviceReadAttrs, endpoint, clusterId, attributeMetadata->attributeId, buffer, maxReadLength); }
    case (DEVICE_TYPE_COLOR_TEMPERATURE_LIGHT):{ return HandleColorTemperatureLightAttribute(DeviceAction::DeviceReadAttrs, endpoint, clusterId, attributeMetadata->attributeId, buffer, maxReadLength); }
    case (DEVICE_TYPE_ON_OFF_LIGHT_SWITCH):{ return HandleOnOffLightSwitchAttribute(DeviceAction::DeviceReadAttrs, endpoint, clusterId, attributeMetadata->attributeId, buffer, maxReadLength); }
    case (DEVICE_TYPE_CONTACT_SENSOR):{ return HandleContactSensorAttribute(DeviceAction::DeviceReadAttrs, endpoint, clusterId, attributeMetadata->attributeId, buffer, maxReadLength); }
    default:{ break; }
    }
    return EMBER_ZCL_STATUS_SUCCESS;
};

EmberAfStatus BridgedManagerImp::BridgedHandleWriteEvent(uint16_t endpointIndex, EndpointId endpoint, 
    ClusterId clusterId, const EmberAfAttributeMetadata * attributeMetadata, uint8_t * buffer)
{
    deviceEP_t* dev = DeviceManager::GetDeviceList(endpoint);
    if(!dev->reachable) return EMBER_ZCL_STATUS_FAILURE;
    switch (dev->deviceType)
    {
    case (DEVICE_TYPE_ON_OFF_LIGHT):{ return HandleOnOffLightAttribute(DeviceAction::DeviceWriteAttrs, endpoint, clusterId, attributeMetadata->attributeId, buffer, 0); }
    case (DEVICE_TYPE_COLOR_TEMPERATURE_LIGHT):{ return HandleColorTemperatureLightAttribute(DeviceAction::DeviceWriteAttrs, endpoint, clusterId, attributeMetadata->attributeId, buffer, 0); }
    case (DEVICE_TYPE_ON_OFF_LIGHT_SWITCH):{ return HandleOnOffLightSwitchAttribute(DeviceAction::DeviceWriteAttrs, endpoint, clusterId, attributeMetadata->attributeId, buffer, 0); }
    case (DEVICE_TYPE_CONTACT_SENSOR):{ return HandleContactSensorAttribute(DeviceAction::DeviceWriteAttrs, endpoint, clusterId, attributeMetadata->attributeId, buffer, 0); }
    default:{ break; }
    }
    return EMBER_ZCL_STATUS_SUCCESS;
};

}
