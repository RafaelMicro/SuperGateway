# SuperGateway

## Introduction
The SuperGateway Project is IoT reference gateway which is build with openwrt and work with Rafael's EZMesh dongle(Rafael Multi-protocol RCP). 

The EZMesh, which uses a single Rafael's EZmesh Dongle, can support different communication protocols simultaneously.

Support protocols : Bluetooth LE 5.3 / Thread 1.3 / Zigbee 3.0 / Rafael's Sub-G

## Supported Platform
|Vendor|SoC|Board|
|:---:|:---:|:---:|
|Raspberry Pi|BCM2711|Raspberry Pi 4B+|
|MediaTech|MT7981|MT7981 EVB|
|MediaTech|MT7688|Linkit Smart 7688|

## Build the firmware from sources
This section describes how to build the firmware for suppoted pltform from source codes.

### Host environment
The following operations are performed under a Ubuntu LTS 22.04 environment. For a Windows or a Mac OS X host computer, you can install a VM for having the same environment:
* Download Ubuntu 22.04 LTS image from [http://www.ubuntu.com](http://www.ubuntu.com)
* Install this image with VirtualBox (http://virtualbox.org) on the host machine. 50GB disk space reserved for the VM is recommanded

### Steps
In the Ubuntu system, open the *Terminal* application and type the following commands:

1. Download SuperGateway source codes:
    ```
    git clone https://github.com/RafaelMicro/SuperGateway.git
    ```
2. Setup build system and load the defult config for building :
   ```
   bash scripts/bootstrap
   ```
3. Setup the super gateway's SSID :
   ```
   bash scripts/setup_ssid ${SSID}
   ```
4. Now, you can build the firmware :
   ```
   bash scripts/build_image -j4 V=s
   ```
5. The firmware image will genetated at "Image" folder, you can flash the firmware into your platform!

