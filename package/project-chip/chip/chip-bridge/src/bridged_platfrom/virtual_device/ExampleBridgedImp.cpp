
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
    printf("> ");
    fgets((char *)input.ptr, input.len, stdin);
    ret = cli_interpreter(&input, &TK);
    
  };
  return nullptr;
};

static BridgedManagerImp sInstance;
BridgedManagerImp & BridgedManagerImp::GetDefaultInstance() { return sInstance; };
BridgedManager & BridgedMgrImpl() { return BridgedManagerImp::GetDefaultInstance(); };
void BridgedManagerImp::Init(){ 
    emberAfEndpointEnableDisable(emberAfEndpointFromIndex(static_cast<uint16_t>(emberAfFixedEndpointCount() - 1)), false);
    {
        pthread_t poll_thread;
        int res = pthread_create(&poll_thread, nullptr, bridge_cli_thread, nullptr);
        if (res) {
            printf("Error creating polling thread: %d\n", res);
            exit(1);
        }
    }
    return;
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
EmberAfStatus HandleWriteOnOffAttribute(T* dev, chip::AttributeId attributeId, uint8_t* buffer)
{
    switch(attributeId)
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
EmberAfStatus HandleWriteBooleanStateAttribute(T* dev, chip::AttributeId attributeId, uint8_t* buffer)
{
    switch(attributeId)
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
            MutableByteSpan zclNameSpan(buffer, maxLen);
            MakeZclCharString(zclNameSpan, dev->GetName().c_str());
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


EmberAfStatus HandleOnOffLightAttribute(DeviceAction action, uint16_t endpointIndex, 
    ClusterId clusterId, chip::AttributeId attId, uint8_t * buf, uint16_t maxLen)
{
    auto dev = Rafael::DeviceLibrary::DeviceMgr().GetDevice<DeviceOnOffLight>((uint16_t)endpointIndex);
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


EmberAfStatus HandleOnOffLightSwitchAttribute(DeviceAction action, uint16_t endpointIndex, 
    ClusterId clusterId, chip::AttributeId attId, uint8_t * buf, uint16_t maxLen)
{
    auto dev = Rafael::DeviceLibrary::DeviceMgr().GetDevice<DeviceOnOffLightSwitch>((uint16_t)endpointIndex);
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
    auto dev = Rafael::DeviceLibrary::DeviceMgr().GetDevice<DeviceContactSensor>((uint16_t)endpointIndex);
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
    deviceEP_t* dev = DeviceManager::GetDeviceList(endpoint);
    switch (dev->deviceType)
    {
    case (DEVICE_TYPE_ON_OFF_LIGHT):{ return HandleOnOffLightAttribute(DeviceAction::DeviceReadAttrs, endpointIndex, clusterId, attributeMetadata->attributeId, buffer, maxReadLength); }
    case (DEVICE_TYPE_ON_OFF_LIGHT_SWITCH):{ return HandleOnOffLightSwitchAttribute(DeviceAction::DeviceReadAttrs, endpointIndex, clusterId, attributeMetadata->attributeId, buffer, maxReadLength); }
    case (DEVICE_TYPE_CONTACT_SENSOR):{ return HandleContactSensorAttribute(DeviceAction::DeviceReadAttrs, endpointIndex, clusterId, attributeMetadata->attributeId, buffer, maxReadLength); }
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
    case (DEVICE_TYPE_ON_OFF_LIGHT):{ return HandleOnOffLightAttribute(DeviceAction::DeviceWriteAttrs, endpointIndex, clusterId, attributeMetadata->attributeId, buffer, 0); }
    case (DEVICE_TYPE_ON_OFF_LIGHT_SWITCH):{ return HandleOnOffLightSwitchAttribute(DeviceAction::DeviceWriteAttrs, endpointIndex, clusterId, attributeMetadata->attributeId, buffer, 0); }
    case (DEVICE_TYPE_CONTACT_SENSOR):{ return HandleContactSensorAttribute(DeviceAction::DeviceWriteAttrs, endpointIndex, clusterId, attributeMetadata->attributeId, buffer, 0); }
    default:{ break; }
    }
    return EMBER_ZCL_STATUS_SUCCESS;
};

}
