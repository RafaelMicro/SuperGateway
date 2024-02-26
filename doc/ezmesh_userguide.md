# Usage
## Start EZmesh Manually
### EZMesh Daemon
Start ezmesh daemon :
```
# Assuming the ttyUSB0 is an EZMesh agent(RCP dongle)
/usr/bin/ezmeshd -c /usr/etc/ez_config.ini
```
### Bluetooth
Start ez-bluetooth to create a vitrul HCI device:
```
/usr/bin/ez-bluetooth /tmp/pts_hci
```
Attach HCI device: 
```
hciattach /tmp/pts_hci any
```
After complete attatch HCI device, you can use `hciconfig` to check hci is up running.

### Thread
Start openthread board router: 
```
/usr/sbin/otbr-agent -I wpan0 -B eth0 spinel+ezmesh://ezmeshd_0?iid=2 trel://eth0
```
After start otbr-agent, you can use `ot-ctl` by command line.
Please see [OpenTHread CLI Overview](https://openthread.io/reference/cli)

### Matter
#### Controller
Located: /usr/sbin/chip-tool
Please see [CHIP Tool Guide](https://github.com/project-chip/connectedhomeip/blob/master/docs/guides/chip_tool_guide.md) document.

#### OTA Provider
Located: /usr/sbin/ota-provider-app
Please see [Usage of ota-provider-app](https://github.com/project-chip/connectedhomeip/tree/master/examples/ota-provider-app/linux#usage)https://github.com/project-chip/connectedhomeip/tree/master/examples/ota-provider-app/linux#usage

#### Birdge Application
Located: /usr/sbin/bridge-app
