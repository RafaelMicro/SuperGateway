
#ifndef __DEVICE_BASE_H__
#define __DEVICE_BASE_H__
#pragma once

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

#include <pthread.h>
#include <sys/ioctl.h>

#include "CommissionableInit.h"
#include "platfrom/Device.h"
#include "DeviceLibrary.h"
#include "ApplicationCluster.h"
#include <app/server/Server.h>

#include <cassert>
#include <iostream>
#include <vector>

using namespace chip;
using namespace chip::app;
using namespace chip::Credentials;
using namespace chip::Inet;
using namespace chip::Transport;
using namespace chip::DeviceLayer;
using namespace chip::app::Clusters;

class DeviceAttBase
{
public:
    DeviceAttBase(std::string aname, std::string alocation);
    virtual ~DeviceAttBase() {};
    EmberAfCluster *Clusters;
    uint8_t ClusterSize;
    // const EmberAfEndpointType Endpoint;
    // const EmberAfDeviceType* DeviceTypes;
    uint8_t DeviceTypeSize;
    DataVersion* DataVersions;
    std::string name;
    std::string location;
};

class DeviceAttOnOffLight : public DeviceAttBase
{
public:
    std::string deviceTypeName = "OnOffLight";
    DeviceAttOnOffLight(std::string aname, std::string alocation);
    EmberAfCluster Clusters[5] = {
        { Descriptor::Id, RafaelCluster::Descriptor::Attrs, ArraySize(RafaelCluster::Descriptor::Attrs), 
            0, ZAP_CLUSTER_MASK(SERVER), NULL, nullptr, nullptr },
        { BridgedDeviceBasicInformation::Id, RafaelCluster::BridgedDeviceBasic::Attrs, ArraySize(RafaelCluster::BridgedDeviceBasic::Attrs), 
            0, ZAP_CLUSTER_MASK(SERVER), NULL, nullptr, nullptr },
        { RafaelCluster::Scenes::clusterId, RafaelCluster::Scenes::Attrs, ArraySize(RafaelCluster::Scenes::Attrs), 
            0, ZAP_CLUSTER_MASK( SERVER ), NULL, RafaelCluster::Scenes::clientCommands, nullptr,
            RafaelCluster::Scenes::Events, ArraySize(RafaelCluster::Scenes::Events) },
        { RafaelCluster::OnOff::clusterId, RafaelCluster::OnOff::Attrs, ArraySize(RafaelCluster::OnOff::Attrs), 
            0, ZAP_CLUSTER_MASK( SERVER ), NULL, RafaelCluster::OnOff::clientCommands, nullptr, 
            RafaelCluster::OnOff::Events, ArraySize(RafaelCluster::OnOff::Events) },
        { RafaelCluster::Level::clusterId, RafaelCluster::Level::Attrs, ArraySize(RafaelCluster::Level::Attrs), 
            0, ZAP_CLUSTER_MASK( SERVER ), NULL, RafaelCluster::Level::clientCommands, nullptr, 
            RafaelCluster::Level::Events, ArraySize(RafaelCluster::Level::Events) },
        // ADD_SERVER_CLUSTER(RafaelCluster::OccupancySensing),
    };
    DataVersion DataVersions[5];
    uint8_t ClusterSize = 5;
    const EmberAfEndpointType Endpoint = { Clusters, ArraySize(Clusters), 0};
    EmberAfDeviceType DeviceTypes[2] = { 
        ZAP_DEVICE_TYPE(DEVICE_TYPE_ON_OFF_LIGHT), ZAP_DEVICE_TYPE(DEVICE_TYPE_BRIDGED_NODE)
    };
    uint8_t DeviceTypeSize = 2;
};

class DeviceAttDimmableLight : public DeviceAttBase
{
public:
    std::string deviceTypeName = "DimmableLight";
    DeviceAttDimmableLight(std::string aname, std::string alocation);
    EmberAfCluster Clusters[5] = {
        { Descriptor::Id, RafaelCluster::Descriptor::Attrs, ArraySize(RafaelCluster::Descriptor::Attrs), 
            0, ZAP_CLUSTER_MASK(SERVER), NULL, nullptr, nullptr },
        { BridgedDeviceBasicInformation::Id, RafaelCluster::BridgedDeviceBasic::Attrs, ArraySize(RafaelCluster::BridgedDeviceBasic::Attrs), 
            0, ZAP_CLUSTER_MASK(SERVER), NULL, nullptr, nullptr },
        { RafaelCluster::Scenes::clusterId, RafaelCluster::Scenes::Attrs, ArraySize(RafaelCluster::Scenes::Attrs), 
            0, ZAP_CLUSTER_MASK( SERVER ), NULL, RafaelCluster::Scenes::clientCommands, nullptr,
            RafaelCluster::Scenes::Events, ArraySize(RafaelCluster::Scenes::Events) },
        { RafaelCluster::OnOff::clusterId, RafaelCluster::OnOff::Attrs, ArraySize(RafaelCluster::OnOff::Attrs), 
            0, ZAP_CLUSTER_MASK( SERVER ), NULL, RafaelCluster::OnOff::clientCommands, nullptr, 
            RafaelCluster::OnOff::Events, ArraySize(RafaelCluster::OnOff::Events) },
        { RafaelCluster::Level::clusterId, RafaelCluster::Level::Attrs, ArraySize(RafaelCluster::Level::Attrs), 
            0, ZAP_CLUSTER_MASK( SERVER ), NULL, RafaelCluster::Level::clientCommands, nullptr, 
            RafaelCluster::Level::Events, ArraySize(RafaelCluster::Level::Events) },
        // ADD_SERVER_CLUSTER(RafaelCluster::OccupancySensing),
    };
    DataVersion DataVersions[5];
    uint8_t ClusterSize = 5;
    const EmberAfEndpointType Endpoint = { Clusters, ArraySize(Clusters), 0};
    EmberAfDeviceType DeviceTypes[2] = { 
        ZAP_DEVICE_TYPE(DEVICE_TYPE_DIMABLE_LIGHT), ZAP_DEVICE_TYPE(DEVICE_TYPE_BRIDGED_NODE)
    };
    uint8_t DeviceTypeSize = 2;
};

class DeviceAttColorTemperatureLight : public DeviceAttBase
{
public:
    std::string deviceTypeName = "ColorTemperatureLight";
    DeviceAttColorTemperatureLight(std::string aname, std::string alocation);
    EmberAfCluster Clusters[6] = {
        { Descriptor::Id, RafaelCluster::Descriptor::Attrs, ArraySize(RafaelCluster::Descriptor::Attrs), 
            0, ZAP_CLUSTER_MASK(SERVER), NULL, nullptr, nullptr },
        { BridgedDeviceBasicInformation::Id, RafaelCluster::BridgedDeviceBasic::Attrs, ArraySize(RafaelCluster::BridgedDeviceBasic::Attrs), 
            0, ZAP_CLUSTER_MASK(SERVER), NULL, nullptr, nullptr },
        { RafaelCluster::Scenes::clusterId, RafaelCluster::Scenes::Attrs, ArraySize(RafaelCluster::Scenes::Attrs), 
            0, ZAP_CLUSTER_MASK( SERVER ), NULL, RafaelCluster::Scenes::clientCommands, nullptr,
            RafaelCluster::Scenes::Events, ArraySize(RafaelCluster::Scenes::Events) },
        { RafaelCluster::OnOff::clusterId, RafaelCluster::OnOff::Attrs, ArraySize(RafaelCluster::OnOff::Attrs), 
            0, ZAP_CLUSTER_MASK( SERVER ), NULL, RafaelCluster::OnOff::clientCommands, nullptr, 
            RafaelCluster::OnOff::Events, ArraySize(RafaelCluster::OnOff::Events) },
        { RafaelCluster::Level::clusterId, RafaelCluster::Level::Attrs, ArraySize(RafaelCluster::Level::Attrs), 
            0, ZAP_CLUSTER_MASK( SERVER ), NULL, RafaelCluster::Level::clientCommands, nullptr, 
            RafaelCluster::Level::Events, ArraySize(RafaelCluster::Level::Events) },
        { RafaelCluster::ColorControl::clusterId, RafaelCluster::ColorControl::Attrs, ArraySize(RafaelCluster::ColorControl::Attrs), 
            0, ZAP_CLUSTER_MASK( SERVER ), NULL, RafaelCluster::ColorControl::clientCommands, nullptr, 
            RafaelCluster::ColorControl::Events, ArraySize(RafaelCluster::ColorControl::Events) },
    };
    DataVersion DataVersions[6];
    uint8_t ClusterSize = 6;
    const EmberAfEndpointType Endpoint = { Clusters, ArraySize(Clusters), 0};
    const EmberAfDeviceType DeviceTypes[2] = { 
        ZAP_DEVICE_TYPE(DEVICE_TYPE_COLOR_TEMPERATURE_LIGHT), ZAP_DEVICE_TYPE(DEVICE_TYPE_BRIDGED_NODE)
    };
    uint8_t DeviceTypeSize = 2;
};

class DeviceAttOnOffLightSwitch : public DeviceAttBase
{
public:
    std::string deviceTypeName = "OnOffLightSwitch";
    DeviceAttOnOffLightSwitch(std::string aname, std::string alocation);
    EmberAfCluster Clusters[4] = {
        { Descriptor::Id, RafaelCluster::Descriptor::Attrs, ArraySize(RafaelCluster::Descriptor::Attrs), 
            0, ZAP_CLUSTER_MASK(SERVER), NULL, nullptr, nullptr },
        { BridgedDeviceBasicInformation::Id, RafaelCluster::BridgedDeviceBasic::Attrs, ArraySize(RafaelCluster::BridgedDeviceBasic::Attrs), 
            0, ZAP_CLUSTER_MASK(SERVER), NULL, nullptr, nullptr },
        { RafaelCluster::Scenes::clusterId, RafaelCluster::Scenes::Attrs, ArraySize(RafaelCluster::Scenes::Attrs), 
            0, ZAP_CLUSTER_MASK(CLIENT), NULL, nullptr, RafaelCluster::Scenes::serverCommands,
            RafaelCluster::Scenes::Events, ArraySize(RafaelCluster::Scenes::Events) },
        { RafaelCluster::OnOff::clusterId, RafaelCluster::OnOff::Attrs, ArraySize(RafaelCluster::OnOff::Attrs), 
            0, ZAP_CLUSTER_MASK(CLIENT), NULL, nullptr, RafaelCluster::OnOff::serverCommands,
            RafaelCluster::OnOff::Events, ArraySize(RafaelCluster::OnOff::Events) },
    };
    DataVersion DataVersions[4];
    uint8_t ClusterSize = 4;
    const EmberAfEndpointType Endpoint = { Clusters, ArraySize(Clusters), 0};
    const EmberAfDeviceType DeviceTypes[2] = { 
        ZAP_DEVICE_TYPE(DEVICE_TYPE_ON_OFF_LIGHT_SWITCH), ZAP_DEVICE_TYPE(DEVICE_TYPE_BRIDGED_NODE)
    };
    uint8_t DeviceTypeSize = 2;
};


class DeviceAttContactSensor : public DeviceAttBase
{
public:
    std::string deviceTypeName = "ContactSensor";
    DeviceAttContactSensor(std::string aname, std::string alocation);
    EmberAfCluster Clusters[3] = {
        { Descriptor::Id, RafaelCluster::Descriptor::Attrs, ArraySize(RafaelCluster::Descriptor::Attrs), 
            0, ZAP_CLUSTER_MASK(SERVER), NULL, nullptr, nullptr },
        { BridgedDeviceBasicInformation::Id, RafaelCluster::BridgedDeviceBasic::Attrs, ArraySize(RafaelCluster::BridgedDeviceBasic::Attrs), 
            0, ZAP_CLUSTER_MASK(SERVER), NULL, nullptr, nullptr },
        { RafaelCluster::BooleanState::clusterId, RafaelCluster::BooleanState::Attrs, ArraySize(RafaelCluster::BooleanState::Attrs), 
            0, ZAP_CLUSTER_MASK(SERVER), NULL, RafaelCluster::BooleanState::clientCommands, RafaelCluster::BooleanState::serverCommands,
            RafaelCluster::BooleanState::Events, ArraySize(RafaelCluster::BooleanState::Events) },
    };
    DataVersion DataVersions[3];
    uint8_t ClusterSize = 3;
    const EmberAfEndpointType Endpoint = { Clusters, ArraySize(Clusters), 0};
    const EmberAfDeviceType DeviceTypes[2] = { 
        ZAP_DEVICE_TYPE(DEVICE_TYPE_CONTACT_SENSOR), ZAP_DEVICE_TYPE(DEVICE_TYPE_BRIDGED_NODE)
    };
    uint8_t DeviceTypeSize = 2;
};


#endif // __DEVICE_BASE_H__
