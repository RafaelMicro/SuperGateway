include $(TOPDIR)/rules.mk

PKG_NAME:=chip-bridge
PKG_RELEASE:=1
PKG_LICENSE:=Apache-2.0
PKG_LICENSE_FILES:=LICENSE
LOCAL_SOURCE_DIR:=$(abspath $(realpath $(dir $(lastword $(MAKEFILE_LIST))))/../../../third_party/connectedhomeip)
APP_SOURCE_DIR:=$(abspath $(realpath $(dir $(lastword $(MAKEFILE_LIST))))/../../../package/project-chip/chip/chip-bridge/src)


PKG_BUILD_PARALLEL:=1
PKG_BUILD_DEPENDS:=gn/host

include $(INCLUDE_DIR)/package.mk

define Package/chip-bridge
	SECTION:=net
	CATEGORY:=Network
	TITLE:=Matter chip-bridge
	URL:=https://github.com/project-chip/connectedhomeip
	DEPENDS:=+libstdcpp +libopenssl +glib2 +libavahi-client
endef

define Package/chip-bridge/description
	Matter bridge
	Integrates a router / access point with the Matter IoT ecosystem.
endef

TARGET_CXXFLAGS += -Wno-format-nonliteral -Wno-unused -Wno-unknown-warning-option
TARGET_CFLAGS += -Wno-format-nonliteral -Wno-unused -Wno-unknown-warning-option

# The build environment contains host tools that can be shared between targets
CHIP_BUILD_ENV_DIR:=$(STAGING_DIR_HOST)/share/chip-build-env
CHIP_BRIDGE_OUT_DIR:=$(PKG_BUILD_DIR)/out/chip-bidge

define Build/Configure
	rm -rf $(LOCAL_SOURCE_DIR)/examples/bridge-app/rafael
	cp -rf ${APP_SOURCE_DIR} $(LOCAL_SOURCE_DIR)/examples/bridge-app/rafael
	mkdir -p $(CHIP_BRIDGE_OUT_DIR) && cd $(CHIP_BRIDGE_OUT_DIR) && \
		$(CONFIGURE_VARS) $(LOCAL_SOURCE_DIR)/scripts/configure \
			--build-env-dir="$(CHIP_BUILD_ENV_DIR)" \
			--project=examples/bridge-app/rafael \
			--target=$(GNU_TARGET_NAME) \
			--enable-network-layer-ble=no \
			--enable-openthread=no \
			--enable-wifi=no \
			--mdns='platform' \
			--enable-tracing-support=no
endef

define Build/Compile
	$(NINJA) -C $(CHIP_BRIDGE_OUT_DIR) chip-bridge-app
endef

define Package/chip-bridge/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(CHIP_BRIDGE_OUT_DIR)/chip-bridge-app $(1)/usr/sbin/bridge-app
endef

$(eval $(call BuildPackage,chip-bridge))

