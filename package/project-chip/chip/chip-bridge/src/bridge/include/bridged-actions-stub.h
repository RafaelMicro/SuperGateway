
#ifndef __BRIDGED_ACTIONS_STUB_H__
#define __BRIDGED_ACTIONS_STUB_H__

#include <app-common/zap-generated/cluster-objects.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/AttributeAccessInterface.h>
#include <app/util/attribute-storage.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>

#include <vector>

#include "platfrom/Device.h"
#include "DeviceLibrary.h"
#include "ApplicationCluster.h"

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::Actions::Attributes;

class ActionsAttrAccess : public AttributeAccessInterface
{
public:
    // Register for the Actions cluster on all endpoints.
    ActionsAttrAccess() : AttributeAccessInterface(Optional<EndpointId>::Missing(), Actions::Id) {}

    CHIP_ERROR Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder) override;

private:
    static constexpr uint16_t ClusterRevision = 1;

    CHIP_ERROR ReadActionListAttribute(EndpointId endpoint, AttributeValueEncoder & aEncoder);
    CHIP_ERROR ReadEndpointListAttribute(EndpointId endpoint, AttributeValueEncoder & aEncoder);
    CHIP_ERROR ReadSetupUrlAttribute(EndpointId endpoint, AttributeValueEncoder & aEncoder);
    CHIP_ERROR ReadClusterRevision(EndpointId endpoint, AttributeValueEncoder & aEncoder);
};

#endif // __BRIDGED_ACTIONS_STUB_H__
