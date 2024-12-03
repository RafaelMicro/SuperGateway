include $(TOPDIR)/rules.mk

PKG_NAME:=chip
PKG_RELEASE:=1
PKG_LICENSE:=Apache-2.0
PKG_LICENSE_FILES:=LICENSE
LOCAL_SOURCE_DIR:=$(abspath $(realpath $(dir $(lastword $(MAKEFILE_LIST))))/../../../third_party/connectedhomeip)
MAKEFILE_DIR:=$(abspath $(realpath $(dir $(lastword $(MAKEFILE_LIST)))))
CHIP_BUILD_ENV_DIR:=$(STAGING_DIR_HOST)/share/chip-build-env
PKG_BUILD_PARALLEL:=1
PKG_BUILD_DEPENDS:=gn/host
PKG_CONFIG_DEPENDS:= CONFIG_CHIP_OTA_PROVIDER CONFIG_CHIP_BRIDGE CONFIG_CHIP_TOOL CONFIG_CHIP_NETWORK_MANAGER

include $(INCLUDE_DIR)/package.mk
TARGET_CXXFLAGS += -DFATCONFDIR=\"/root\" -DSYSCONFDIR=\"/root\" -DLOCALSTATEDIR=\"/root\" -Wno-format-nonliteral -Wno-unused -Wno-unknown-warning-option -fexceptions -latomic
TARGET_CFLAGS += -DFATCONFDIR=\"/root\" -DSYSCONFDIR=\"/root\" -DLOCALSTATEDIR=\"/root\" -Wno-format-nonliteral -Wno-unused -Wno-unknown-warning-option -fexceptions
TARGET_LDFLAGS += -latomic

CHIP_OTA_PROVIDER_OUT_DIR:=$(PKG_BUILD_DIR)/chip-ota-provider
OTA_PROVIDER_SOURCE_DIR:=$(abspath $(realpath $(dir $(lastword $(MAKEFILE_LIST))))/../../package/project-chip/chip/chip-bridge/src)
CHIP_BRIDGE_OUT_DIR:=$(PKG_BUILD_DIR)/chip-bidge
CHIP_TOOL_OUT_DIR:=$(PKG_BUILD_DIR)/chip-tool
CHIP_NETWORK_MANAGER_OUT_DIR:=$(PKG_BUILD_DIR)/network-manager-app


define Package/chip
	SECTION:=net
	CATEGORY:=Network
	TITLE:=Rafael Matter Host Tool
	URL:=https://github.com/project-chip/connectedhomeip
	DEPENDS:=+libstdcpp +libopenssl +glib2 +libavahi-client +libatomic
	MENU:=1
endef

define Package/chip/description
  Matter Host Features
  Integrates a router / access point with the Matter IoT ecosystem.
endef

define Package/chip/config
	source "$(SOURCE)/Config.in"
endef

define Build/Configure
	[ ! -d "$(CHIP_OTA_PROVIDER_OUT_DIR) " ] && mkdir -p $(CHIP_OTA_PROVIDER_OUT_DIR)
	[ ! -d "$(CHIP_BRIDGE_OUT_DIR)" ] && mkdir -p $(CHIP_BRIDGE_OUT_DIR)
	[ ! -d "$(CHIP_TOOL_OUT_DIR)" ] && mkdir -p $(CHIP_TOOL_OUT_DIR)
	[ ! -d "$(CHIP_NETWORK_MANAGER_OUT_DIR)" ] && mkdir -p $(CHIP_NETWORK_MANAGER_OUT_DIR)

	if [ "$(CONFIG_CHIP_OTA_PROVIDER)" = "y" ]; then \
		if [ ! -f $(CHIP_OTA_PROVIDER_OUT_DIR)/build.ninja ]; then \
			echo "Configuring OTA Provider"; \
			cd $(CHIP_OTA_PROVIDER_OUT_DIR) && \
			$(CONFIGURE_VARS) $(LOCAL_SOURCE_DIR)/scripts/configure --build-env-dir="$(CHIP_BUILD_ENV_DIR)" \
				--project=examples/ota-provider-app/linux --target=$(GNU_TARGET_NAME) --enable-network-layer-ble=no \
				--enable-openthread=no --enable-wifi=no --mdns='platform' --enable-tracing-support=no; \
		else echo "Configuring OTA Provider no update"; fi \
	else echo "Configuring OTA Provider is disable"; fi

	if [ "$(CONFIG_CHIP_BRIDGE)" = "y" ]; then \
		if [ ! -f $(CHIP_BRIDGE_OUT_DIR)/build.ninja ]; then \
			echo "Configuring Bridge"; \
			cp -rf ${OTA_PROVIDER_SOURCE_DIR} $(LOCAL_SOURCE_DIR)/examples/bridge-app/rafael && \
			cd $(CHIP_BRIDGE_OUT_DIR) && \
			$(CONFIGURE_VARS) $(LOCAL_SOURCE_DIR)/scripts/configure --build-env-dir="$(CHIP_BUILD_ENV_DIR)" \
				--project=examples/bridge-app/rafael --target=$(GNU_TARGET_NAME) --enable-network-layer-ble=no \
				--enable-openthread=no --enable-wifi=no --mdns='platform' --enable-tracing-support=no; \
		else echo "Configuring Bridge no update"; fi \
	else echo "Configuring Bridge is disable"; fi

	if [ "$(CONFIG_CHIP_TOOL)" = "y" ]; then \
		if [ ! -f $(CHIP_TOOL_OUT_DIR)/build.ninja ]; then \
			echo "Configuring Chip Tool"; \
			cd $(CHIP_TOOL_OUT_DIR) && \
			$(CONFIGURE_VARS) $(LOCAL_SOURCE_DIR)/scripts/configure --build-env-dir="$(CHIP_BUILD_ENV_DIR)" \
				--project=examples/chip-tool  --target=$(GNU_TARGET_NAME) --enable-tracing-support=no; \
		else echo "Configuring Chip Tool no update"; fi \
	else echo "Configuring Chip Tool is disable"; fi
	
	if [ "$(CONFIG_CHIP_NETWORK_MANAGER)" = "y" ]; then \
		if [ ! -f $(CHIP_NETWORK_MANAGER_OUT_DIR)/build.ninja ]; then \
			echo "Configuring Netowrk manager"; \
			cd $(CHIP_NETWORK_MANAGER_OUT_DIR) && \
			$(CONFIGURE_VARS) $(LOCAL_SOURCE_DIR)/scripts/configure --build-env-dir="$(CHIP_BUILD_ENV_DIR)" \
				--project=examples/network-manager-app/linux  --target=$(GNU_TARGET_NAME) --enable-tracing-support=no \
				--enable-network-layer-ble=no --enable-wifi=no --mdns='platform'; \
		else echo "Configuring Network manager no update"; fi \
	else echo "Configuring Network manager is disable"; fi
endef

define Build/Compile
	if [ "$(CONFIG_CHIP_OTA_PROVIDER)" = "y" ]; then \
		ninja -C $(CHIP_OTA_PROVIDER_OUT_DIR) chip-ota-provider-app; \
	fi
	if [ "$(CONFIG_CHIP_BRIDGE)" = "y" ]; then \
		ninja -C $(CHIP_BRIDGE_OUT_DIR) chip-bridge-app; \
	fi
	if [ "$(CONFIG_CHIP_TOOL)" = "y" ]; then \
		ninja -C $(CHIP_TOOL_OUT_DIR) chip-tool; \
	fi
	if [ "$(CONFIG_CHIP_NETWORK_MANAGER)" = "y" ]; then \
		ninja -C $(CHIP_NETWORK_MANAGER_OUT_DIR) matter-network-manager-app; \
	fi
endef

define Build/Clean
endef

define Package/chip/install
	$(INSTALL_DIR) $(1)/usr/sbin
	if [ "$(CONFIG_CHIP_OTA_PROVIDER)" = "y" ]; then \
		$(INSTALL_BIN) $(CHIP_OTA_PROVIDER_OUT_DIR)/chip-ota-provider-app $(1)/usr/sbin/ota-provider-app; \
	fi
	if [ "$(CONFIG_CHIP_BRIDGE)" = "y" ]; then \
		$(INSTALL_BIN) $(CHIP_BRIDGE_OUT_DIR)/chip-bridge-app $(1)/usr/sbin/bridge-app; \
	fi
	if [ "$(CONFIG_CHIP_TOOL)" = "y" ]; then \
		$(INSTALL_BIN) $(CHIP_TOOL_OUT_DIR)/chip-tool $(1)/usr/sbin/chip-tool; \
	fi
	if [ "$(CONFIG_CHIP_NETWORK_MANAGER)" = "y" ]; then \
		$(INSTALL_BIN) $(CHIP_NETWORK_MANAGER_OUT_DIR)/matter-network-manager-app $(1)/usr/sbin/matter-net-mgmt; \
	fi
endef

$(eval $(call BuildPackage,chip))
