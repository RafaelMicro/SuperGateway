
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
void cliGenOnOffLight(CLI_Token_t *TK);
void cligenOnOffLightSwitch(CLI_Token_t *TK);
void cliGenContactSensor(CLI_Token_t *TK);
void cliDelOnOffLight(CLI_Token_t *TK);
void cliListEndpoint(CLI_Token_t *TK);

CLI_Func_t cli_func[] = {
  {"help", strlen("help"), cliHelp, "print help message"},
  {"genOnOffLight", strlen("genOnOffLight"), cliGenOnOffLight, "generate OnOff Light"},
  {"genOnOffLightSwitch", strlen("genOnOffLightSwitch"), cligenOnOffLightSwitch, "generate OnOff Light Switch"},
  {"genContactSensor", strlen("genContactSensor"), cliGenContactSensor, "generate Contact Sensor"},
  {"DelOnOffLight", strlen("DelOnOffLight"), cliDelOnOffLight, "Delete OnOff Light"},
  {"ListEndpoint", strlen("ListEndpoint"), cliListEndpoint, "List all Endpoint"},
  {NULL, 0, NULL, NULL}
};

void cliHelp(CLI_Token_t *TK) {
  for (CLI_Func_t *p = cli_func; p->name != NULL; p++) printf(" - %s : %s\n", p->name, p->help_msg);
}

void cliGenOnOffLight(CLI_Token_t *TK) {
  std::string name = DEFAULT_ONOFF_LIGHT_NAME + std::to_string(dev_index++);
  DeviceAttOnOffLight devLight(name, room);
  Rafael::DeviceLibrary::DeviceMgr().publishDevice<DeviceOnOffLight, DeviceAttOnOffLight>(
    &devLight, DEVICE_TYPE_ON_OFF_LIGHT);
}

void cligenOnOffLightSwitch(CLI_Token_t *TK) {
  std::string name = DEFAULT_ONOFF_LIGHT_SWITCH_NAME + std::to_string(dev_index++);
  DeviceAttOnOffLightSwitch devLightSwitch(name, room);
  Rafael::DeviceLibrary::DeviceMgr().publishDevice<DeviceOnOffLightSwitch, DeviceAttOnOffLightSwitch>(
    &devLightSwitch, DEVICE_TYPE_ON_OFF_LIGHT_SWITCH);
}

void cliGenContactSensor(CLI_Token_t *TK) {
  std::string name = DEFAULT_CONTACT_SENSOR_NAME + std::to_string(dev_index++);
  DeviceAttContactSensor devContactSensor(name, room);
  Rafael::DeviceLibrary::DeviceMgr().publishDevice<DeviceContactSensor, DeviceAttContactSensor>(
    &devContactSensor, DEVICE_TYPE_CONTACT_SENSOR);
}

void cliDelOnOffLight(CLI_Token_t *TK) {
  uint16_t EPId = (uint16_t)atoi(TK->token_list[1].ptr);
  Rafael::DeviceLibrary::DeviceMgr().DelDeviceEndpoint<DeviceOnOffLight>(EPId);
}

void cliListEndpoint(CLI_Token_t *TK) { Rafael::DeviceLibrary::DeviceMgr().ListDeviceList(); }
