
#include <app-common/zap-generated/cluster-objects.h>
#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include <app/AttributeAccessInterface.h>
#include <app/util/attribute-storage.h>
#include <lib/support/CodeUtils.h>
#include <lib/support/logging/CHIPLogging.h>
#include <vector>

#include "bridged-actions-stub.h"
#include "platfrom/Device.h"
#include "DeviceLibrary.h"
#include "ApplicationCluster.h"

using namespace chip;
using namespace chip::app;
using namespace chip::app::Clusters;
using namespace chip::app::Clusters::Actions::Attributes;

constexpr uint16_t ActionsAttrAccess::ClusterRevision;
ActionsAttrAccess gAttrAccess;
const char SetupUrl[] = "https://example.com";

using namespace Rafael::DeviceLibrary;

CHIP_ERROR ActionsAttrAccess::ReadActionListAttribute(EndpointId endpoint, AttributeValueEncoder & aEncoder)
{
    CHIP_ERROR err = aEncoder.EncodeList([&endpoint](const auto & encoder) -> CHIP_ERROR {
        std::vector<Action *> actionList = GetActionListInfo(endpoint);
        for (auto action : actionList)
        {
            if (action->getIsVisible())
            {
                Actions::Structs::ActionStruct::Type actionStruct = { action->getActionId(), 
                CharSpan::fromCharString(action->getName().c_str()), action->getType(),
                    action->getEndpointListId(), action->getSupportedCommands(), action->getStatus() };
                ReturnErrorOnFailure(encoder.Encode(actionStruct));
            }
        }

        return CHIP_NO_ERROR;
    });
    return err;
}

CHIP_ERROR ActionsAttrAccess::ReadEndpointListAttribute(EndpointId endpoint, AttributeValueEncoder & aEncoder)
{
    std::vector<EndpointListInfo> infoList = GetEndpointListInfo(endpoint);

    CHIP_ERROR err = aEncoder.EncodeList([&infoList](const auto & encoder) -> CHIP_ERROR {
        for (auto info : infoList)
        {
            Actions::Structs::EndpointListStruct::Type endpointListStruct = {
                info.GetEndpointListId(), CharSpan::fromCharString(info.GetName().c_str()), info.GetType(),
                DataModel::List<chip::EndpointId>(info.GetEndpointListData(), info.GetEndpointListSize())
            };
            ReturnErrorOnFailure(encoder.Encode(endpointListStruct));
        }
        return CHIP_NO_ERROR;
    });
    return err;
}

CHIP_ERROR ActionsAttrAccess::ReadSetupUrlAttribute(EndpointId endpoint, AttributeValueEncoder & aEncoder)
{
    return aEncoder.Encode(chip::CharSpan::fromCharString(SetupUrl));
}

CHIP_ERROR ActionsAttrAccess::ReadClusterRevision(EndpointId endpoint, AttributeValueEncoder & aEncoder)
{
    return aEncoder.Encode(ClusterRevision);
}


CHIP_ERROR ActionsAttrAccess::Read(const ConcreteReadAttributePath & aPath, AttributeValueEncoder & aEncoder)
{
    VerifyOrDie(aPath.mClusterId == Actions::Id);
    switch (aPath.mAttributeId)
    {
    case ActionList::Id:{ return ReadActionListAttribute(aPath.mEndpointId, aEncoder);}
    case EndpointLists::Id:{ return ReadEndpointListAttribute(aPath.mEndpointId, aEncoder);}
    case SetupURL::Id:{ return ReadSetupUrlAttribute(aPath.mEndpointId, aEncoder);}
    case ClusterRevision::Id:{ return ReadClusterRevision(aPath.mEndpointId, aEncoder);}
    default:{ return CHIP_NO_ERROR; }
    }
}

void MatterActionsPluginServerInitCallback() { registerAttributeAccessOverride(&gAttrAccess); }
