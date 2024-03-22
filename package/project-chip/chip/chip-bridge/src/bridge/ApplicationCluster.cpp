

#include <app-common/zap-generated/ids/Attributes.h>
#include <app-common/zap-generated/ids/Clusters.h>
#include "ApplicationCluster.h"

namespace RafaelCluster
{

using namespace chip;
using namespace chip::app;

EmberAfAttributeMetadata descriptorAttrs[5] = {
    ZAP_RECORD(Clusters::Descriptor::Attributes::DeviceTypeList::Id, ZAP_MAX_ARRAY_SIZE, ZAP_ARRAY),
    ZAP_RECORD(Clusters::Descriptor::Attributes::ServerList::Id, ZAP_MAX_ARRAY_SIZE, ZAP_ARRAY),
    ZAP_RECORD(Clusters::Descriptor::Attributes::ClientList::Id, ZAP_MAX_ARRAY_SIZE, ZAP_ARRAY),
    ZAP_RECORD(Clusters::Descriptor::Attributes::PartsList::Id, ZAP_MAX_ARRAY_SIZE, ZAP_ARRAY),
    // ZAP_BASIC(0, ZCL_CLUSTER_REVISION_DESCRIPTOR),
    ZAP_END_ELEMENT,
};

EmberAfAttributeMetadata bridgedDeviceBasicAttrs[4] = {
    ZAP_RECORD(Clusters::BridgedDeviceBasicInformation::Attributes::NodeLabel::Id, ZAP_MAX_LABLE_SIZE, ZAP_CHAR_STRING),
    ZAP_RECORD(Clusters::BridgedDeviceBasicInformation::Attributes::Reachable::Id, 1, ZAP_BOOL),
    ZAP_RECORD(Clusters::BridgedDeviceBasicInformation::Attributes::FeatureMap::Id, 4, ZAP_BITMAP32),
    // ZAP_BASIC(4, ZCL_CLUSTER_REVISION_BRIDGED_DEVICE_BASIC_INFORMATION),
    ZAP_END_ELEMENT,

};

ClusterId Identify::clusterId = Clusters::Identify::Id;
EmberAfAttributeMetadata Identify::Attrs[9] = {
    ZAP_RECORD(Clusters::Identify::Attributes::IdentifyTime::Id, 0, ZAP_INT16),
    ZAP_RECORD(Clusters::Identify::Attributes::IdentifyType::Id, 0, ZAP_INT8),
    ZAP_BASIC(1, ZCL_CLUSTER_REVISION_IDENTIFY),
    ZAP_END_ELEMENT,
};
uint8_t Identify::AttSize = 9;
CommandId Identify::serverCommands[1] = { kInvalidCommandId, };
CommandId Identify::clientCommands[3] = {
    Clusters::Identify::Commands::Identify::Id,
    Clusters::Identify::Commands::TriggerEffect::Id,
    kInvalidCommandId,
};
EventId Identify::Events[1] = { kInvalidEventId, };

ClusterId Groups::clusterId = Clusters::Groups::Id;
EmberAfAttributeMetadata Groups::Attrs[9] = {
    ZAP_RECORD(Clusters::Groups::Attributes::NameSupport::Id, 0, ZAP_INT8),
    ZAP_RECORD(Clusters::Groups::Attributes::FeatureMap::Id, 0, ZAP_BITMAP32),
    ZAP_BASIC(1, ZCL_CLUSTER_REVISION_GROUPS),
    ZAP_END_ELEMENT,
};
uint8_t Groups::AttSize = 9;
CommandId Groups::serverCommands[5] = { 
    Clusters::Groups::Commands::AddGroupResponse::Id,
    Clusters::Groups::Commands::ViewGroupResponse::Id,
    Clusters::Groups::Commands::GetGroupMembershipResponse::Id,
    Clusters::Groups::Commands::RemoveGroupResponse::Id,
    kInvalidCommandId,
};
CommandId Groups::clientCommands[7] = {
    Clusters::Groups::Commands::AddGroup::Id,
    Clusters::Groups::Commands::ViewGroup::Id,
    Clusters::Groups::Commands::GetGroupMembership::Id,
    Clusters::Groups::Commands::RemoveGroup::Id,
    Clusters::Groups::Commands::RemoveAllGroups::Id,
    Clusters::Groups::Commands::AddGroupIfIdentifying::Id,
    kInvalidCommandId,
};
EventId Groups::Events[1] = { kInvalidEventId, };

ClusterId Scenes::clusterId = Clusters::Scenes::Id;
EmberAfAttributeMetadata Scenes::Attrs[8] = {
    ZAP_RECORD(Clusters::Scenes::Attributes::NameSupport::Id, 0, ZAP_INT8),
    ZAP_BASIC(0x000F, ZCL_CLUSTER_REVISION_SCENES),
    ZAP_END_ELEMENT,
};
uint8_t Scenes::AttSize = 8;
CommandId Scenes::serverCommands[10] = { 
    Clusters::Scenes::Commands::AddSceneResponse::Id,
    Clusters::Scenes::Commands::ViewSceneResponse::Id,
    Clusters::Scenes::Commands::RemoveSceneResponse::Id,
    Clusters::Scenes::Commands::RemoveAllScenesResponse::Id,
    Clusters::Scenes::Commands::StoreSceneResponse::Id,
    Clusters::Scenes::Commands::GetSceneMembershipResponse::Id,
    Clusters::Scenes::Commands::EnhancedAddSceneResponse::Id,
    Clusters::Scenes::Commands::EnhancedViewSceneResponse::Id,
    Clusters::Scenes::Commands::CopySceneResponse::Id,
    kInvalidCommandId,
};
CommandId Scenes::clientCommands[11] = {
    Clusters::Scenes::Commands::AddScene::Id,
    Clusters::Scenes::Commands::ViewScene::Id,
    Clusters::Scenes::Commands::RemoveScene::Id,
    Clusters::Scenes::Commands::RemoveAllScenes::Id,
    Clusters::Scenes::Commands::StoreScene::Id,
    Clusters::Scenes::Commands::RecallScene::Id,
    Clusters::Scenes::Commands::GetSceneMembership::Id,
    Clusters::Scenes::Commands::EnhancedAddScene::Id,
    Clusters::Scenes::Commands::EnhancedViewScene::Id,
    Clusters::Scenes::Commands::CopyScene::Id,
    kInvalidCommandId,
};
EventId Scenes::Events[1] = { kInvalidEventId, };

ClusterId OnOff::clusterId = Clusters::OnOff::Id;
EmberAfAttributeMetadata OnOff::Attrs[12] = {
    ZAP_RECORD(Clusters::OnOff::Attributes::OnOff::Id, 0, ZAP_BOOL),
    ZAP_RECORD(Clusters::OnOff::Attributes::GlobalSceneControl::Id, 1, ZAP_BOOL),
    ZAP_RECORD(Clusters::OnOff::Attributes::OnTime::Id, 0, ZAP_INT16),
    ZAP_RECORD(Clusters::OnOff::Attributes::OffWaitTime::Id, 0, ZAP_INT16),
    ZAP_RECORD(Clusters::OnOff::Attributes::StartUpOnOff::Id, 0, ZAP_INT16),
    ZAP_BASIC(0x0003, ZCL_CLUSTER_REVISION_ON_OFF),
    ZAP_END_ELEMENT,
};
uint8_t OnOff::AttSize = 12;
CommandId OnOff::serverCommands[1] = { kInvalidCommandId, };
CommandId OnOff::clientCommands[7] = { 
    Clusters::OnOff::Commands::Off::Id,
    Clusters::OnOff::Commands::On::Id,
    Clusters::OnOff::Commands::Toggle::Id,
    Clusters::OnOff::Commands::OffWithEffect::Id,
    Clusters::OnOff::Commands::OnWithRecallGlobalScene::Id,
    Clusters::OnOff::Commands::OnWithTimedOff::Id,
    kInvalidCommandId,
};
EventId OnOff::Events[1] = { kInvalidEventId, };

ClusterId Level::clusterId = Clusters::LevelControl::Id;
EmberAfAttributeMetadata Level::Attrs[21] = {
    ZAP_RECORD(Clusters::LevelControl::Attributes::CurrentLevel::Id, 0, ZAP_INT8),
    ZAP_RECORD(Clusters::LevelControl::Attributes::RemainingTime::Id, 0, ZAP_INT16),
    ZAP_RECORD(Clusters::LevelControl::Attributes::MinLevel::Id, 1, ZAP_INT8),
    ZAP_RECORD(Clusters::LevelControl::Attributes::MaxLevel::Id, 254, ZAP_INT8),
    ZAP_RECORD(Clusters::LevelControl::Attributes::CurrentFrequency::Id, 0, ZAP_INT16),
    ZAP_RECORD(Clusters::LevelControl::Attributes::MinFrequency::Id, 0, ZAP_INT16),
    ZAP_RECORD(Clusters::LevelControl::Attributes::MaxFrequency::Id, 0, ZAP_INT16),
    ZAP_RECORD(Clusters::LevelControl::Attributes::OnOffTransitionTime::Id, 0, ZAP_INT16),
    ZAP_RECORD(Clusters::LevelControl::Attributes::OnLevel::Id, 0, ZAP_INT8),
    ZAP_RECORD(Clusters::LevelControl::Attributes::OnTransitionTime::Id, 0, ZAP_INT16),
    ZAP_RECORD(Clusters::LevelControl::Attributes::OffTransitionTime::Id, 0, ZAP_INT16),
    ZAP_RECORD(Clusters::LevelControl::Attributes::DefaultMoveRate::Id, 0, ZAP_INT8),
    ZAP_RECORD(Clusters::LevelControl::Attributes::Options::Id, 0, ZAP_ENUM8),
    ZAP_RECORD(Clusters::LevelControl::Attributes::StartUpCurrentLevel::Id, 0, ZAP_INT16),
    ZAP_BASIC(0x0007, ZCL_CLUSTER_REVISION_LEVEL_CONTROL),
    ZAP_END_ELEMENT,
};
uint8_t Level::AttSize = 21;
CommandId Level::serverCommands[1] = { kInvalidCommandId, };
CommandId Level::clientCommands[9] = {
    Clusters::LevelControl::Commands::MoveToLevel::Id,
    Clusters::LevelControl::Commands::Move::Id,
    Clusters::LevelControl::Commands::Step::Id,
    Clusters::LevelControl::Commands::Stop::Id,
    Clusters::LevelControl::Commands::MoveToLevelWithOnOff::Id,
    Clusters::LevelControl::Commands::StepWithOnOff::Id,
    Clusters::LevelControl::Commands::StopWithOnOff::Id,
    Clusters::LevelControl::Commands::MoveToClosestFrequency::Id,
    kInvalidCommandId,
};
EventId Level::Events[1] = { kInvalidEventId, };

ClusterId BooleanState::clusterId = Clusters::BooleanState::Id;
EmberAfAttributeMetadata BooleanState::Attrs[8] = {
    ZAP_RECORD(Clusters::BooleanState::Attributes::StateValue::Id, 0, ZAP_BOOL),
    ZAP_BASIC(0, ZCL_CLUSTER_REVISION_BOOLEAN_STATE),
    ZAP_END_ELEMENT,
};
uint8_t BooleanState::AttSize = 8;
CommandId BooleanState::serverCommands[1] = { kInvalidCommandId, };
CommandId BooleanState::clientCommands[1] = { kInvalidCommandId, };
EventId BooleanState::Events[2] = { 
    Clusters::BooleanState::Events::StateChange::Id,
    kInvalidEventId,
};

ClusterId ModeSelect::clusterId = Clusters::ModeSelect::Id;
EmberAfAttributeMetadata ModeSelect::Attrs[13] = {
    ZAP_RECORD(Clusters::ModeSelect::Attributes::Description::Id, ZAP_MAX_LABLE_SIZE, ZAP_CHAR_STRING),
    ZAP_RECORD(Clusters::ModeSelect::Attributes::StandardNamespace::Id, 0, ZAP_ENUM16),
    ZAP_RECORD(Clusters::ModeSelect::Attributes::SupportedModes::Id, 0, ZAP_ENUM8),
    ZAP_RECORD(Clusters::ModeSelect::Attributes::CurrentMode::Id, 0, ZAP_INT8),
    ZAP_RECORD(Clusters::ModeSelect::Attributes::StartUpMode::Id, 0, ZAP_INT8),
    ZAP_RECORD(Clusters::ModeSelect::Attributes::OnMode::Id, 0, ZAP_INT8),
    ZAP_BASIC(0, ZCL_CLUSTER_REVISION_MODE_SELECT),
    ZAP_END_ELEMENT,
};
uint8_t ModeSelect::AttSize = 13;
CommandId ModeSelect::serverCommands[1] = { kInvalidCommandId, };
CommandId ModeSelect::clientCommands[2] = { 
    Clusters::ModeSelect::Commands::ChangeToMode::Id,
    kInvalidCommandId,
};
EventId ModeSelect::Events[1] = { kInvalidEventId, };

ClusterId LowPower::clusterId = Clusters::LowPower::Id;
EmberAfAttributeMetadata LowPower::Attrs[7] = {
    ZAP_BASIC(0, ZCL_CLUSTER_REVISION_LOW_POWER),
    ZAP_END_ELEMENT,
};
uint8_t LowPower::AttSize = 7;
CommandId LowPower::serverCommands[1] = { kInvalidCommandId, };
CommandId LowPower::clientCommands[2] = { 
    Clusters::LowPower::Commands::Sleep::Id,
    kInvalidCommandId,
};
EventId LowPower::Events[1] = { kInvalidEventId, };

ClusterId WakeOnLan::clusterId = Clusters::WakeOnLan::Id;
EmberAfAttributeMetadata WakeOnLan::Attrs[8] = {
    ZAP_RECORD(Clusters::WakeOnLan::Attributes::MACAddress::Id, ZAP_MAX_LABLE_SIZE, ZAP_CHAR_STRING),
    ZAP_BASIC(0, ZCL_CLUSTER_REVISION_WAKE_ON_LAN),
    ZAP_END_ELEMENT,
};
uint8_t WakeOnLan::AttSize = 8;
CommandId WakeOnLan::serverCommands[1] = { kInvalidCommandId, };
CommandId WakeOnLan::clientCommands[1] = { kInvalidCommandId, };
EventId WakeOnLan::Events[1] = { kInvalidEventId, };

ClusterId Switch::clusterId = Clusters::Switch::Id;
EmberAfAttributeMetadata Switch::Attrs[10] = {
    ZAP_RECORD(Clusters::Switch::Attributes::NumberOfPositions::Id, 2, ZAP_INT8),
    ZAP_RECORD(Clusters::Switch::Attributes::CurrentPosition::Id, 0, ZAP_INT8),
    ZAP_RECORD(Clusters::Switch::Attributes::MultiPressMax::Id, 2, ZAP_INT8),
    ZAP_BASIC(0, ZCL_CLUSTER_REVISION_SWITCH),
    ZAP_END_ELEMENT,
};
uint8_t Switch::AttSize = 10;
CommandId Switch::serverCommands[1] = { kInvalidCommandId, };
CommandId Switch::clientCommands[1] = { kInvalidCommandId, };
EventId Switch::Events[8] = { 
    Clusters::Switch::Events::SwitchLatched::Id,
    Clusters::Switch::Events::InitialPress::Id,
    Clusters::Switch::Events::LongPress::Id,
    Clusters::Switch::Events::ShortRelease::Id,
    Clusters::Switch::Events::LongRelease::Id,
    Clusters::Switch::Events::MultiPressOngoing::Id,
    Clusters::Switch::Events::MultiPressComplete::Id,
    kInvalidEventId, 
};

ClusterId OperationalState::clusterId = Clusters::OperationalState::Id;
EmberAfAttributeMetadata OperationalState::Attrs[13] = {
    ZAP_RECORD(Clusters::OperationalState::Attributes::PhaseList::Id, ZAP_MAX_ARRAY_SIZE, ZAP_ARRAY),
    ZAP_RECORD(Clusters::OperationalState::Attributes::CurrentPhase::Id, 0, ZAP_INT8),
    ZAP_RECORD(Clusters::OperationalState::Attributes::CountdownTime::Id, 0, ZAP_INT8),
    ZAP_RECORD(Clusters::OperationalState::Attributes::OperationalStateList::Id, ZAP_MAX_ARRAY_SIZE, ZAP_ARRAY),
    ZAP_RECORD(Clusters::OperationalState::Attributes::OperationalState::Id, 2, ZAP_INT8),
    ZAP_RECORD(Clusters::OperationalState::Attributes::OperationalError::Id, 0, ZAP_INT8),
    ZAP_BASIC(0, ZCL_CLUSTER_REVISION_OPERATIONAL_STATE),
    ZAP_END_ELEMENT,
};
uint8_t OperationalState::AttSize = 13;
CommandId OperationalState::serverCommands[1] = { kInvalidCommandId, };
CommandId OperationalState::clientCommands[6] = { 
    Clusters::OperationalState::Commands::Pause::Id,
    Clusters::OperationalState::Commands::Stop::Id,
    Clusters::OperationalState::Commands::Start::Id,
    Clusters::OperationalState::Commands::Resume::Id,
    Clusters::OperationalState::Commands::OperationalCommandResponse::Id,
    kInvalidCommandId,
};
EventId OperationalState::Events[1] = { kInvalidEventId, };

ClusterId ColorControl::clusterId = Clusters::ColorControl::Id;
EmberAfAttributeMetadata ColorControl::Attrs[59] = {
    ZAP_RECORD(Clusters::ColorControl::Attributes::CurrentHue::Id, 0, ZAP_INT8),
    ZAP_RECORD(Clusters::ColorControl::Attributes::CurrentSaturation::Id, 0, ZAP_INT8),
    ZAP_RECORD(Clusters::ColorControl::Attributes::RemainingTime::Id, 0, ZAP_INT16),
    ZAP_RECORD(Clusters::ColorControl::Attributes::CurrentX::Id, 0x616B, ZAP_INT16),
    ZAP_RECORD(Clusters::ColorControl::Attributes::CurrentY::Id, 0x607D, ZAP_INT16),
    ZAP_RECORD(Clusters::ColorControl::Attributes::DriftCompensation::Id, 0, ZAP_ENUM8),
    ZAP_RECORD(Clusters::ColorControl::Attributes::CompensationText::Id, ZAP_MAX_LABLE_SIZE, ZAP_CHAR_STRING),
    ZAP_RECORD(Clusters::ColorControl::Attributes::ColorTemperatureMireds::Id, 0x00FA, ZAP_INT16),
    ZAP_RECORD(Clusters::ColorControl::Attributes::ColorMode::Id, 1, ZAP_ENUM8),
    ZAP_RECORD(Clusters::ColorControl::Attributes::Options::Id, 0, ZAP_ARRAY),
    ZAP_RECORD(Clusters::ColorControl::Attributes::EnhancedCurrentHue::Id, 0, ZAP_INT16),
    ZAP_RECORD(Clusters::ColorControl::Attributes::EnhancedColorMode::Id, 1, ZAP_ENUM8),
    ZAP_RECORD(Clusters::ColorControl::Attributes::ColorLoopActive::Id, 0, ZAP_INT8),
    ZAP_RECORD(Clusters::ColorControl::Attributes::ColorLoopDirection::Id, 0, ZAP_INT8),
    ZAP_RECORD(Clusters::ColorControl::Attributes::ColorLoopTime::Id, 0x0019, ZAP_INT16),
    ZAP_RECORD(Clusters::ColorControl::Attributes::ColorLoopStartEnhancedHue::Id, 0x2300, ZAP_INT16),
    ZAP_RECORD(Clusters::ColorControl::Attributes::ColorLoopStoredEnhancedHue::Id, 0, ZAP_INT16),
    ZAP_RECORD(Clusters::ColorControl::Attributes::ColorCapabilities::Id, 0, ZAP_BITMAP16),
    ZAP_RECORD(Clusters::ColorControl::Attributes::ColorTempPhysicalMinMireds::Id, 0, ZAP_INT16),
    ZAP_RECORD(Clusters::ColorControl::Attributes::ColorTempPhysicalMaxMireds::Id, 0xFEFF, ZAP_INT16),
    ZAP_RECORD(Clusters::ColorControl::Attributes::CoupleColorTempToLevelMinMireds::Id, 0, ZAP_INT16),
    ZAP_RECORD(Clusters::ColorControl::Attributes::StartUpColorTemperatureMireds::Id, 0, ZAP_INT16),
    ZAP_RECORD(Clusters::ColorControl::Attributes::NumberOfPrimaries::Id, 0, ZAP_INT8),
    ZAP_RECORD(Clusters::ColorControl::Attributes::Primary1X::Id, 0, ZAP_INT16),
    ZAP_RECORD(Clusters::ColorControl::Attributes::Primary1Y::Id, 0, ZAP_INT16),
    ZAP_RECORD(Clusters::ColorControl::Attributes::Primary1Intensity::Id, 0, ZAP_INT8),
    ZAP_RECORD(Clusters::ColorControl::Attributes::Primary2X::Id, 0, ZAP_INT16),
    ZAP_RECORD(Clusters::ColorControl::Attributes::Primary2Y::Id, 0, ZAP_INT16),
    ZAP_RECORD(Clusters::ColorControl::Attributes::Primary2Intensity::Id, 0, ZAP_INT8),
    ZAP_RECORD(Clusters::ColorControl::Attributes::Primary3X::Id, 0, ZAP_INT16),
    ZAP_RECORD(Clusters::ColorControl::Attributes::Primary3Y::Id, 0, ZAP_INT16),
    ZAP_RECORD(Clusters::ColorControl::Attributes::Primary3Intensity::Id, 0, ZAP_INT8),
    ZAP_RECORD(Clusters::ColorControl::Attributes::Primary4X::Id, 0, ZAP_INT16),
    ZAP_RECORD(Clusters::ColorControl::Attributes::Primary4Y::Id, 0, ZAP_INT16),
    ZAP_RECORD(Clusters::ColorControl::Attributes::Primary4Intensity::Id, 0, ZAP_INT8),
    ZAP_RECORD(Clusters::ColorControl::Attributes::Primary5X::Id, 0, ZAP_INT16),
    ZAP_RECORD(Clusters::ColorControl::Attributes::Primary5Y::Id, 0, ZAP_INT16),
    ZAP_RECORD(Clusters::ColorControl::Attributes::Primary5Intensity::Id, 0, ZAP_INT8),
    ZAP_RECORD(Clusters::ColorControl::Attributes::Primary6X::Id, 0, ZAP_INT16),
    ZAP_RECORD(Clusters::ColorControl::Attributes::Primary6Y::Id, 0, ZAP_INT16),
    ZAP_RECORD(Clusters::ColorControl::Attributes::Primary6Intensity::Id, 0, ZAP_INT8),
    ZAP_RECORD(Clusters::ColorControl::Attributes::WhitePointX::Id, 0, ZAP_INT16),
    ZAP_RECORD(Clusters::ColorControl::Attributes::WhitePointY::Id, 0, ZAP_INT16),
    ZAP_RECORD(Clusters::ColorControl::Attributes::ColorPointRX::Id, 0, ZAP_INT16),
    ZAP_RECORD(Clusters::ColorControl::Attributes::ColorPointRY::Id, 0, ZAP_INT16),
    ZAP_RECORD(Clusters::ColorControl::Attributes::ColorPointRIntensity::Id, 0, ZAP_INT8),
    ZAP_RECORD(Clusters::ColorControl::Attributes::ColorPointGX::Id, 0, ZAP_INT16),
    ZAP_RECORD(Clusters::ColorControl::Attributes::ColorPointGY::Id, 0, ZAP_INT16),
    ZAP_RECORD(Clusters::ColorControl::Attributes::ColorPointGIntensity::Id, 0, ZAP_INT8),
    ZAP_RECORD(Clusters::ColorControl::Attributes::ColorPointBX::Id, 0, ZAP_INT16),
    ZAP_RECORD(Clusters::ColorControl::Attributes::ColorPointBY::Id, 0, ZAP_INT16),
    ZAP_RECORD(Clusters::ColorControl::Attributes::ColorPointBIntensity::Id, 0, ZAP_INT8),
    ZAP_BASIC(0x1F, ZCL_CLUSTER_REVISION_COLOR_CONTROL),
    ZAP_END_ELEMENT,
};
uint8_t ColorControl::AttSize = 59;
CommandId ColorControl::serverCommands[1] = { kInvalidCommandId, };
CommandId ColorControl::clientCommands[20] = { 
    Clusters::ColorControl::Commands::MoveToHue::Id,
    Clusters::ColorControl::Commands::MoveHue::Id,
    Clusters::ColorControl::Commands::StepHue::Id,
    Clusters::ColorControl::Commands::MoveToSaturation::Id,
    Clusters::ColorControl::Commands::MoveSaturation::Id,
    Clusters::ColorControl::Commands::StepSaturation::Id,
    Clusters::ColorControl::Commands::MoveToHueAndSaturation::Id,
    Clusters::ColorControl::Commands::MoveToColor::Id,
    Clusters::ColorControl::Commands::MoveColor::Id,
    Clusters::ColorControl::Commands::StepColor::Id,
    Clusters::ColorControl::Commands::MoveToColorTemperature::Id,
    Clusters::ColorControl::Commands::EnhancedMoveToHue::Id,
    Clusters::ColorControl::Commands::EnhancedMoveHue::Id,
    Clusters::ColorControl::Commands::EnhancedStepHue::Id,
    Clusters::ColorControl::Commands::EnhancedMoveToHueAndSaturation::Id,
    Clusters::ColorControl::Commands::ColorLoopSet::Id,
    Clusters::ColorControl::Commands::StopMoveStep::Id,
    Clusters::ColorControl::Commands::MoveColorTemperature::Id,
    Clusters::ColorControl::Commands::StepColorTemperature::Id,
    kInvalidCommandId,
};
EventId ColorControl::Events[1] = { kInvalidEventId, };


ClusterId BallastConfiguration::clusterId = Clusters::BallastConfiguration::Id;
EmberAfAttributeMetadata BallastConfiguration::Attrs[21] = {
    ZAP_RECORD(Clusters::BallastConfiguration::Attributes::PhysicalMinLevel::Id, 1, ZAP_INT8),
    ZAP_RECORD(Clusters::BallastConfiguration::Attributes::PhysicalMaxLevel::Id, 254, ZAP_INT8),
    ZAP_RECORD(Clusters::BallastConfiguration::Attributes::BallastStatus::Id, 0, ZAP_INT8),
    ZAP_RECORD(Clusters::BallastConfiguration::Attributes::MinLevel::Id, 1, ZAP_INT8),
    ZAP_RECORD(Clusters::BallastConfiguration::Attributes::MaxLevel::Id, 254, ZAP_INT8),
    ZAP_RECORD(Clusters::BallastConfiguration::Attributes::IntrinsicBallastFactor::Id, 0, ZAP_INT8),
    ZAP_RECORD(Clusters::BallastConfiguration::Attributes::BallastFactorAdjustment::Id, 0, ZAP_INT8),
    ZAP_RECORD(Clusters::BallastConfiguration::Attributes::LampQuantity::Id, 0, ZAP_INT8),
    ZAP_RECORD(Clusters::BallastConfiguration::Attributes::LampType::Id, ZAP_MAX_LABLE_SIZE, ZAP_CHAR_STRING),
    ZAP_RECORD(Clusters::BallastConfiguration::Attributes::LampManufacturer::Id, ZAP_MAX_LABLE_SIZE, ZAP_CHAR_STRING),
    ZAP_RECORD(Clusters::BallastConfiguration::Attributes::LampRatedHours::Id, 0, ZAP_INT24),
    ZAP_RECORD(Clusters::BallastConfiguration::Attributes::LampBurnHours::Id, 0, ZAP_INT24),
    ZAP_RECORD(Clusters::BallastConfiguration::Attributes::LampAlarmMode::Id, 0, ZAP_INT8),
    ZAP_RECORD(Clusters::BallastConfiguration::Attributes::LampBurnHoursTripPoint::Id, 0, ZAP_INT24),
    ZAP_BASIC(0, ZCL_CLUSTER_REVISION_BALLAST_CONFIGURATION),
    ZAP_END_ELEMENT,
};
uint8_t BallastConfiguration::AttSize = 21;
CommandId BallastConfiguration::serverCommands[1] = { kInvalidCommandId, };
CommandId BallastConfiguration::clientCommands[1] = { kInvalidCommandId, };
EventId BallastConfiguration::Events[1] = { kInvalidEventId, };

}
