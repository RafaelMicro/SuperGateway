
#include <AppMain.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/PlatformManager.h>
#include "Bridged.h"

int main(int argc, char * argv[])
{
    matter_init(argc, argv);
    chip::DeviceLayer::PlatformMgr().RunEventLoop();
    return 0;
}
