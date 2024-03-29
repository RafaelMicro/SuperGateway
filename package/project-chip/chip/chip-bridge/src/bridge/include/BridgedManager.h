
#ifndef __BRIDGED_MANGER_H__
#define __BRIDGED_MANGER_H__

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

namespace RafaelCluster
{
class BridgedManager {
public:
    virtual EmberAfStatus BridgedHandleReadEvent(uint16_t endpointIndex, chip::EndpointId endpoint, chip::ClusterId clusterId,
                                        const EmberAfAttributeMetadata * attributeMetadata, 
                                        uint8_t * buffer, uint16_t maxReadLength) = 0;
    virtual EmberAfStatus BridgedHandleWriteEvent(uint16_t endpointIndex, chip::EndpointId endpoint, chip::ClusterId clusterId,
                                        const EmberAfAttributeMetadata * attributeMetadata, 
                                        uint8_t * buffer) = 0;
    virtual void Init() = 0;
    BridgedManager()          = default;
    virtual ~BridgedManager() = default;

};
BridgedManager & BridgedMgr();
extern BridgedManager & BridgedMgrImpl();
void SetBridgedMgr(BridgedManager * instance);
}

#endif // __BRIDGED_MANGER_H__
