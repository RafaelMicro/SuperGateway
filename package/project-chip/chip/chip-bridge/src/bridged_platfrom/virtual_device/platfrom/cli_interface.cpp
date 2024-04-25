
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
#include <string.h>
#include <stdio.h>

#include "BridgedManager.h"
#include "CommissionableInit.h"
#include "platfrom/Device.h"
#include "DeviceLibrary.h"
#include "DeviceBase.h"
#include "ApplicationCluster.h"
#include "cli.h"
#include <app/server/Server.h>

#include <cassert>
#include <iostream>
#include <vector>

uint16_t dev_index = 0;
std::string room = DEFAULT_NAME;

void cliHelp(CLI_Token_t *TK);
void cliListEndpoint(CLI_Token_t *TK);

void cliGenOnOffLight(CLI_Token_t *TK);
void cliGenDimmableLight(CLI_Token_t *TK);
void cliGenColorTemperatureLight(CLI_Token_t *TK);
void cliGenOnOffLightSwitch(CLI_Token_t *TK);
void cliGenContactSensor(CLI_Token_t *TK);

void cliDelOnOffLight(CLI_Token_t *TK);
void cliDelDimmableLight(CLI_Token_t *TK);
void cliDelColorTemperatureLight(CLI_Token_t *TK);
void cliDelOnOffLightSwitch(CLI_Token_t *TK);
void cliDelContactSensor(CLI_Token_t *TK);

CLI_Func_t cli_func[] = {
  {"help", strlen("help"), cliHelp, "print help message"},
  {"ListEndpoint", strlen("ListEndpoint"), cliListEndpoint, "List all Endpoint"},

  {"genOnOffLight", strlen("genOnOffLight"), cliGenOnOffLight, "generate OnOff Light"},
  {"genDimmableLight", strlen("genDimmableLight"), cliGenDimmableLight, "generate Dimmable Light"},
  {"genColorTemperatureLight", strlen("genColorTemperatureLight"), cliGenColorTemperatureLight, "generate Color Temperature Light"},
  {"genOnOffLightSwitch", strlen("genOnOffLightSwitch"), cliGenOnOffLightSwitch, "generate OnOff Light Switch"},
  {"genContactSensor", strlen("genContactSensor"), cliGenContactSensor, "generate Contact Sensor"},

  {"DelOnOffLight", strlen("DelOnOffLight"), cliDelOnOffLight, "Delete OnOff Light"},
  {"DelDimmableLight", strlen("DelDimmableLight"), cliDelDimmableLight, "Delete Dimmable Light"},
  {"DelColorTemperatureLight", strlen("DelColorTemperatureLight"), cliDelColorTemperatureLight, "Delete Color Temperature Light"},
  {"DelOnOffLightSwitch", strlen("DelOnOffLightSwitch"), cliDelOnOffLightSwitch, "Delete OnOff Light Switch"},
  {"DelContactSensor", strlen("DelContactSensor"), cliDelContactSensor, "Delete Contact Sensor"},

  {NULL, 0, NULL, NULL}
};

void cliHelp(CLI_Token_t *TK) {
  for (CLI_Func_t *p = cli_func; p->name != NULL; p++) printf(" - %s : %s\n", p->name, p->help_msg);
}

void cliGenOnOffLight(CLI_Token_t *TK) {
  std::string light_name = DEFAULT_ONOFF_LIGHT_NAME + std::to_string(dev_index++);
  Rafael::DeviceLibrary::DeviceMgr().publishDevice<DeviceOnOffLight, DeviceAttOnOffLight>(
    light_name, room, DEVICE_TYPE_ON_OFF_LIGHT);
}

void cliGenOnOffLightSwitch(CLI_Token_t *TK) {
  std::string name = DEFAULT_ONOFF_LIGHT_SWITCH_NAME + std::to_string(dev_index++);
  Rafael::DeviceLibrary::DeviceMgr().publishDevice<DeviceOnOffLightSwitch, DeviceAttOnOffLightSwitch>(
    name, room, DEVICE_TYPE_ON_OFF_LIGHT_SWITCH);
}

void cliGenContactSensor(CLI_Token_t *TK) {
  std::string name = DEFAULT_CONTACT_SENSOR_NAME + std::to_string(dev_index++);
  Rafael::DeviceLibrary::DeviceMgr().publishDevice<DeviceContactSensor, DeviceAttContactSensor>(
    name, room, DEVICE_TYPE_CONTACT_SENSOR);
}

void cliGenColorTemperatureLight(CLI_Token_t *TK) {
  std::string name = DEFAULT_CONTACT_SENSOR_NAME + std::to_string(dev_index++);
  Rafael::DeviceLibrary::DeviceMgr().publishDevice<DeviceColorTemperatureLight, DeviceAttColorTemperatureLight>(
    name, room, DEVICE_TYPE_COLOR_TEMPERATURE_LIGHT);
}

void cliGenDimmableLight(CLI_Token_t *TK) {
  std::string name = DEFAULT_CONTACT_SENSOR_NAME + std::to_string(dev_index++);
  Rafael::DeviceLibrary::DeviceMgr().publishDevice<DeviceDimmableLight, DeviceAttDimmableLight>(
    name, room, DEVICE_TYPE_COLOR_TEMPERATURE_LIGHT);
}

void cliDelOnOffLight(CLI_Token_t *TK) {
  if(TK->token_list[1].ptr==nullptr) return;
  uint16_t EPId = (uint16_t)atoi(TK->token_list[1].ptr);
  Rafael::DeviceLibrary::DeviceMgr().DelDeviceEndpoint<DeviceOnOffLight, DeviceAttOnOffLight>(EPId);
}

void cliDelOnOffLightSwitch(CLI_Token_t *TK) {
  if(TK->token_list[1].ptr==nullptr) return;
  uint16_t EPId = (uint16_t)atoi(TK->token_list[1].ptr);
  Rafael::DeviceLibrary::DeviceMgr().DelDeviceEndpoint<DeviceOnOffLightSwitch, DeviceAttOnOffLightSwitch>(EPId);
}

void cliDelContactSensor(CLI_Token_t *TK) {
  if(TK->token_list[1].ptr==nullptr) return;
  uint16_t EPId = (uint16_t)atoi(TK->token_list[1].ptr);
  Rafael::DeviceLibrary::DeviceMgr().DelDeviceEndpoint<DeviceContactSensor, DeviceAttContactSensor>(EPId);
}

void cliDelColorTemperatureLight(CLI_Token_t *TK) {
  if(TK->token_list[1].ptr==nullptr) return;
  uint16_t EPId = (uint16_t)atoi(TK->token_list[1].ptr);
  Rafael::DeviceLibrary::DeviceMgr().DelDeviceEndpoint<DeviceColorTemperatureLight, DeviceAttColorTemperatureLight>(EPId);
}

void cliDelDimmableLight(CLI_Token_t *TK) {
  if(TK->token_list[1].ptr==nullptr) return;
  uint16_t EPId = (uint16_t)atoi(TK->token_list[1].ptr);
  Rafael::DeviceLibrary::DeviceMgr().DelDeviceEndpoint<DeviceDimmableLight, DeviceAttDimmableLight>(EPId);
}

void cliListEndpoint(CLI_Token_t *TK) { Rafael::DeviceLibrary::DeviceMgr().ListDeviceList(); }
