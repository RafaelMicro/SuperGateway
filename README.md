# SuperGateway

## Introduction
The SuperGateway Project is IoT reference gateway which is build with openwrt and work with Rafael's EZMesh dongle(Rafael Multi-protocol RCP). 

The EZMesh, which uses a single Rafael's EZmesh Dongle, can support different communication protocols simultaneously.

Support protocols : Bluetooth LE 5.3 / Thread 1.3 / Zigbee 3.0 / Rafael's Sub-G

Default login address: [http://10.10.10.1](http://10.10.10.1), username: root, password: none 

## Supported Platform
|Vendor|SoC|Board|
|:---:|:---:|:---:|
|Raspberry Pi|BCM2711|Raspberry Pi 4B+|
|MediaTech|MT7981|MT7981 EVB|
|MediaTech|MT7688|Linkit Smart 7688|

## Development
To build your own firmware you need a GNU/Linux, BSD or MacOS system.

### Requirements
To build with this project, Ubuntu 22.04.04 LTS is prefereeed. And you need use the CPU based on AMD64 architecture, with at least 4GB RAM and 32 GB available disk space. Make sure your internet is accessible.

You can install a VM for having the same environment:
* Download Ubuntu 22.04 LTS image from [http://www.ubuntu.com](http://www.ubuntu.com)
* Install this image with VirtualBox (http://virtualbox.org) on the host machine. 50GB disk space reserved for the VM is recommanded

Note:
* If you're using Windows Subsystem for Linux(WSL), please see [Build sysetem setup WSL](https://openwrt.org/docs/guide-developer/toolchain/wsl) documentation.

### Quickstart
In the Ubuntu system, open the *Terminal* application and type the following commands:

1. Download SuperGateway source codes:
    ```
    git clone https://github.com/RafaelMicro/SuperGateway.git
    cd SuperGateway
    ```
2. Setup build system and load the defult config for building :
   ```
   sudo bash scripts/bootstrap
   ```
3. Setup the super gateway's SSID :
   ```
   bash scripts/setup_ssid ${SSID}
   ```
4. Now, you can build the firmware :
   ```
   bash scripts/build_image -j4 V=s
   ```
5. After the build process completes, the resulted firmware file will be under 'image'.
   
Note:
* Depending on the H/W resources of the host environment, the build process may **take more than 2 hours**.

### Guide
* [EZMesh usage](https://github.com/RafaelMicro/SuperGateway/blob/main/doc/ezmesh_userguide.md)
