
#include "BridgedManager.h"

namespace RafaelCluster
{
BridgedManager * gInstance = nullptr;
BridgedManager & BridgedMgr() { return (gInstance != nullptr)? *gInstance : BridgedMgrImpl(); }
void SetBridgedMgr(BridgedManager * instance) { if (instance != nullptr) { gInstance = instance; } }

} // namespace RafaelCluster
