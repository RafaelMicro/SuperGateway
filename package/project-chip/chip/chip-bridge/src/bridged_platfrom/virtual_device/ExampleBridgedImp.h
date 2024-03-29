
#ifndef __EXAMPLE_BRIDGED_IMP_H__
#define __EXAMPLE_BRIDGED_IMP_H__

#include <AppMain.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/PlatformManager.h>

#include <app/util/af-types.h>
#include <app/util/af.h>

#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/ConcreteAttributePath.h>
#include <credentials/DeviceAttestationCredsProvider.h>
#include <credentials/examples/DeviceAttestationCredsExample.h>
#include <lib/core/CHIPError.h>

#include "BridgedManager.h"
#include "platfrom/Device.h"

namespace RafaelCluster
{

enum DeviceAction
{
    DeviceReadAttrs = 0,
    DeviceWriteAttrs,
};

class BridgedManagerImp : public BridgedManager{
public:
    EmberAfStatus BridgedHandleReadEvent(uint16_t endpointIndex, chip::EndpointId endpoint, chip::ClusterId clusterId,
                                        const EmberAfAttributeMetadata * attributeMetadata, 
                                        uint8_t * buffer, uint16_t maxReadLength) override;
    EmberAfStatus BridgedHandleWriteEvent(uint16_t endpointIndex, chip::EndpointId endpoint, chip::ClusterId clusterId,
                                        const EmberAfAttributeMetadata * attributeMetadata, 
                                        uint8_t * buffer) override;

    void Init() override;
    static BridgedManagerImp & GetDefaultInstance();
};
BridgedManager & BridgedMgrImpl();

}

#endif // __EXAMPLE_BRIDGED_IMP_H__
