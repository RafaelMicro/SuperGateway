
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

#include <pthread.h>
#include <sys/ioctl.h>

#include "BridgedManager.h"
#include "Bridged.h"
#include "CommissionableInit.h"
#include "platfrom/Device.h"
#include "DeviceLibrary.h"
#include "DeviceBase.h"
#include "ApplicationCluster.h"
#include <app/server/Server.h>

#include <cassert>
#include <iostream>
#include <vector>

namespace RafaelCluster
{
BridgedManager * gInstance = nullptr;
BridgedManager & BridgedMgr() { return (gInstance != nullptr)? *gInstance : BridgedMgrImpl(); }
void SetBridgedMgr(BridgedManager * instance) { if (instance != nullptr) { gInstance = instance; } }
} // namespace RafaelCluster


EmberAfStatus emberAfExternalAttributeReadCallback(EndpointId endpoint, ClusterId clusterId,
                                                   const EmberAfAttributeMetadata * attributeMetadata, uint8_t * buffer,
                                                   uint16_t maxReadLength)
{
    uint16_t endpointIndex = emberAfGetDynamicIndexFromEndpoint(endpoint);
    if (endpointIndex >= CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT) { return EMBER_ZCL_STATUS_FAILURE; }
    if (Rafael::DeviceLibrary::DeviceMgr().GetDeviceList(endpoint) == nullptr) { return EMBER_ZCL_STATUS_FAILURE; }
    return RafaelCluster::BridgedMgr().BridgedHandleReadEvent(endpointIndex, endpoint, clusterId, attributeMetadata, buffer, maxReadLength);
}

EmberAfStatus emberAfExternalAttributeWriteCallback(EndpointId endpoint, ClusterId clusterId,
                                                    const EmberAfAttributeMetadata * attributeMetadata, uint8_t * buffer)
{
    uint16_t epIndex = emberAfGetDynamicIndexFromEndpoint(endpoint);
    return  epIndex < CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT ? \
            RafaelCluster::BridgedMgr().BridgedHandleWriteEvent(epIndex, endpoint, clusterId, attributeMetadata, buffer): \
            EMBER_ZCL_STATUS_FAILURE;
}

bool emberAfActionsClusterInstantActionCallback(app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
                                                const Clusters::Actions::Commands::InstantAction::DecodableType & commandData)
{
    commandObj->AddStatus(commandPath, Protocols::InteractionModel::Status::NotFound);
    return true;
}

int matter_init(int argc, char * argv[])
{
    if (ChipLinuxAppInit(argc, argv) != 0) { return -1; }

    static chip::CommonCaseDeviceServerInitParams initParams;
    (void) initParams.InitializeStaticResourcesBeforeServerInit();

#if CHIP_DEVICE_ENABLE_PORT_PARAMS
    // use a different service port to make testing possible with other sample devices running on same host
    initParams.operationalServicePort = LinuxDeviceOptions::GetInstance().securedDevicePort;
#endif

    initParams.interfaceId = LinuxDeviceOptions::GetInstance().interfaceId;
    chip::Server::GetInstance().Init(initParams);
    SetDeviceAttestationCredentialsProvider(Examples::GetExampleDACProvider());

    Rafael::DeviceLibrary::DeviceMgr().Init();
    RafaelCluster::BridgedMgr().Init();
    return 0;
}
