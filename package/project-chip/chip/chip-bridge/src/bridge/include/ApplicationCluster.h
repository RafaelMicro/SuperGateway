
#ifndef __APPLICATION_CLUSTER_H__
#define __APPLICATION_CLUSTER_H__

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

#include "CommissionableInit.h"
#include "platfrom/Device.h"
#include "DeviceLibrary.h"
#include "ClusterMacro.h"
#include <app/server/Server.h>

#include <cassert>
#include <iostream>
#include <vector>

namespace RafaelCluster
{
    using namespace chip;
    using namespace chip::app;
    class ClusterInterface
    {
        public:
        static ClusterId clusterId;
        static EmberAfAttributeMetadata *Attrs;
        static uint8_t AttSize;
        static CommandId *serverCommands;
        static CommandId *clientCommands;
        static EventId *Events;
    };
    extern EmberAfAttributeMetadata descriptorAttrs[5];
    extern EmberAfAttributeMetadata bridgedDeviceBasicAttrs[4];
    /* =======================*/
    /* ======= GENERAL =======*/
    /* =======================*/
    class Identify : public ClusterInterface
    {
        public:
        static ClusterId clusterId;
        static EmberAfAttributeMetadata Attrs[9];
        static uint8_t AttSize;
        static CommandId serverCommands[1];
        static CommandId clientCommands[3];
        static EventId Events[1];
    };
    class Groups : public ClusterInterface
    {
        public:
        static ClusterId clusterId;
        static EmberAfAttributeMetadata Attrs[9];
        static uint8_t AttSize;
        static CommandId serverCommands[5];
        static CommandId clientCommands[7];
        static EventId Events[1];
    };
    class Scenes : public ClusterInterface
    {
        public:
        static ClusterId clusterId;
        static EmberAfAttributeMetadata Attrs[8];
        static uint8_t AttSize;
        static CommandId serverCommands[10];
        static CommandId clientCommands[11];
        static EventId Events[1];
    };
    class OnOff : public ClusterInterface
    {
        public:
        static ClusterId clusterId;
        static EmberAfAttributeMetadata Attrs[12];
        static uint8_t AttSize;
        static CommandId serverCommands[1];
        static CommandId clientCommands[7];
        static EventId Events[1];
    };
    class Level : public ClusterInterface
    {
        public:
        static ClusterId clusterId;
        static EmberAfAttributeMetadata Attrs[21];
        static uint8_t AttSize;
        static CommandId serverCommands[1];
        static CommandId clientCommands[9];
        static EventId Events[1];
    };
    class BooleanState : public ClusterInterface
    {
        public:
        static ClusterId clusterId;
        static EmberAfAttributeMetadata Attrs[8];
        static uint8_t AttSize;
        static CommandId serverCommands[1];
        static CommandId clientCommands[1];
        static EventId Events[2];
    };
    class ModeSelect : public ClusterInterface
    {
        public:
        static ClusterId clusterId;
        static EmberAfAttributeMetadata Attrs[13];
        static uint8_t AttSize;
        static CommandId serverCommands[1];
        static CommandId clientCommands[2];
        static EventId Events[1];
    };
    class LowPower : public ClusterInterface
    {
        public:
        static ClusterId clusterId;
        static EmberAfAttributeMetadata Attrs[7];
        static uint8_t AttSize;
        static CommandId serverCommands[1];
        static CommandId clientCommands[2];
        static EventId Events[1];
    };
    class WakeOnLan : public ClusterInterface
    {
        public:
        static ClusterId clusterId;
        static EmberAfAttributeMetadata Attrs[8];
        static uint8_t AttSize;
        static CommandId serverCommands[1];
        static CommandId clientCommands[1];
        static EventId Events[1];
    };
    class Switch : public ClusterInterface
    {
        public:
        static ClusterId clusterId;
        static EmberAfAttributeMetadata Attrs[10];
        static uint8_t AttSize;
        static CommandId serverCommands[1];
        static CommandId clientCommands[1];
        static EventId Events[8];
    };
    class OperationalState : public ClusterInterface
    {
        public:
        static ClusterId clusterId;
        static EmberAfAttributeMetadata Attrs[13];
        static uint8_t AttSize;
        static CommandId serverCommands[1];
        static CommandId clientCommands[6];
        static EventId Events[1];
    };
    
    /* ========================*/
    /* ======= LIGHTING =======*/
    /* ========================*/
    class ColorControl : public ClusterInterface
    {
        public:
        static ClusterId clusterId;
        static EmberAfAttributeMetadata Attrs[59];
        static uint8_t AttSize;
        static CommandId serverCommands[1];
        static CommandId clientCommands[20];
        static EventId Events[1];
    };
    class BallastConfiguration : public ClusterInterface
    {
        public:
        static ClusterId clusterId;
        static EmberAfAttributeMetadata Attrs[21];
        static uint8_t AttSize;
        static CommandId serverCommands[1];
        static CommandId clientCommands[1];
        static EventId Events[1];
    };

}

#endif // __APPLICATION_CLUSTER_H__
