
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
#include "CommissionableInit.h"
#include "bridged_platfrom/Device.h"
#include "DeviceLibrary.h"
#include "DeviceBase.h"
#include "ApplicationCluster.h"
#include <app/server/Server.h>

#include <cassert>
#include <iostream>
#include <vector>

EmberAfStatus emberAfExternalAttributeReadCallback(EndpointId endpoint, ClusterId clusterId,
                                                   const EmberAfAttributeMetadata * attributeMetadata, uint8_t * buffer,
                                                   uint16_t maxReadLength)
{
    uint16_t endpointIndex = emberAfGetDynamicIndexFromEndpoint(endpoint);
    if(endpointIndex >= CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT) { return EMBER_ZCL_STATUS_FAILURE; }
    if (Rafael::DeviceLibrary::DeviceMgr().GetDeviceList(endpointIndex) == nullptr) { return EMBER_ZCL_STATUS_FAILURE; }
    return RafaelCluster::BridgedMgr().BridgedHandleReadEvent(endpointIndex, endpoint, clusterId, attributeMetadata, buffer, maxReadLength);
}

EmberAfStatus emberAfExternalAttributeWriteCallback(EndpointId endpoint, ClusterId clusterId,
                                                    const EmberAfAttributeMetadata * attributeMetadata, uint8_t * buffer)
{
    uint16_t epIndex = emberAfGetDynamicIndexFromEndpoint(endpoint);

    if (epIndex >= CHIP_DEVICE_CONFIG_DYNAMIC_ENDPOINT_COUNT) { return EMBER_ZCL_STATUS_FAILURE; }
    return RafaelCluster::BridgedMgr().BridgedHandleWriteEvent(epIndex, endpoint, clusterId, attributeMetadata, buffer);
}

bool emberAfActionsClusterInstantActionCallback(app::CommandHandler * commandObj, const app::ConcreteCommandPath & commandPath,
                                                const Clusters::Actions::Commands::InstantAction::DecodableType & commandData)
{
    commandObj->AddStatus(commandPath, Protocols::InteractionModel::Status::NotFound);
    return true;
}

// void ApplicationShutdown() {}

#define POLL_INTERVAL_MS (100)
bool kbhit()
{
    int byteswaiting;
    ioctl(0, FIONREAD, &byteswaiting);
    return byteswaiting > 0;
}

void * bridge_polling_thread(void * context)
{
    std::string name = "", room = DEFAULT_NAME;
    uint16_t deviceType, dev_index;
    std::stringstream ss;
    while (true)
    {
        if (kbhit())
        {
            int ch = getchar();
            if (ch=='1') { 
                name = DEFAULT_ONOFF_LIGHT_NAME +  std::to_string(dev_index++);
                DeviceAttOnOffLight devLight(name, room);
                deviceType = DEVICE_TYPE_ON_OFF_LIGHT;
                Rafael::DeviceLibrary::DeviceMgr().publishDevice<DeviceOnOffLight, DeviceAttOnOffLight>(&devLight, deviceType);
            }
            else if(ch=='2') { 
                name = DEFAULT_ONOFF_LIGHT_NAME +  std::to_string(dev_index++);
                DeviceAttOnOffLightSwitch devLightSwitch(name, room);
                deviceType = DEVICE_TYPE_ON_OFF_LIGHT_SWITCH;
                Rafael::DeviceLibrary::DeviceMgr().publishDevice<DeviceOnOffLightSwitch, DeviceAttOnOffLightSwitch>(&devLightSwitch, deviceType);
            }
            else if(ch=='3') { 
                name = DEFAULT_ONOFF_LIGHT_NAME +  std::to_string(dev_index++);
                DeviceAttContactSensor devContactSensor(name, room);
                deviceType = DEVICE_TYPE_CONTACT_SENSOR;
                Rafael::DeviceLibrary::DeviceMgr().publishDevice<DeviceContactSensor, DeviceAttContactSensor>(&devContactSensor, deviceType);
            }
            continue;
        }
        usleep(POLL_INTERVAL_MS * 1000);
    }
    return nullptr;
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

    // Initialize device attestation config
    SetDeviceAttestationCredentialsProvider(Examples::GetExampleDACProvider());

    Rafael::DeviceLibrary::DeviceMgr().Init();    
    RafaelCluster::BridgedMgr().Init();
    // Disable last fixed endpoint, which is used as a placeholder for all of the
    // supported clusters so that ZAP will generated the requisite code.
    emberAfEndpointEnableDisable(emberAfEndpointFromIndex(static_cast<uint16_t>(emberAfFixedEndpointCount() - 1)), false);
    {
        pthread_t poll_thread;
        int res = pthread_create(&poll_thread, nullptr, bridge_polling_thread, nullptr);
        if (res)
        {
            printf("Error creating polling thread: %d\n", res);
            exit(1);
        }
    }
    // registerAttributeAccessOverride(&gPowerAttrAccess);
    return 0;
}
