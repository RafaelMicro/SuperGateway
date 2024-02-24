include $(TOPDIR)/rules.mk

PKG_NAME:=chip-ota-provider
PKG_RELEASE:=1
PKG_LICENSE:=Apache-2.0
PKG_LICENSE_FILES:=LICENSE
LOCAL_SOURCE_DIR:=$(abspath $(realpath $(dir $(lastword $(MAKEFILE_LIST))))/../../../third_party/connectedhomeip)

PKG_BUILD_PARALLEL:=1
PKG_BUILD_DEPENDS:=gn/host

include $(INCLUDE_DIR)/package.mk

TARGET_CXXFLAGS += -Wno-format-nonliteral
TARGET_CFLAGS += -Wno-format-nonliteral

# The build environment contains host tools that can be shared between targets
CHIP_BUILD_ENV_DIR:=$(STAGING_DIR_HOST)/share/chip-build-env
CHIP_OTA_PROVIDER_OUT_DIR:=$(PKG_BUILD_DIR)/out/chip-ota-provider

define Package/chip-ota-provider
	SECTION:=net
	CATEGORY:=Network
	TITLE:=Matter chip-ota-provider
	URL:=https://github.com/project-chip/connectedhomeip
	DEPENDS:=+libstdcpp +libopenssl +glib2 +libavahi-client
endef

define Package/chip-ota-provider/description
	Matter OTA provider
	Integrates a router / access point with the Matter IoT ecosystem.
endef

define Build/Configure
	mkdir -p $(CHIP_OTA_PROVIDER_OUT_DIR) && cd $(CHIP_OTA_PROVIDER_OUT_DIR) && \
		$(CONFIGURE_VARS) $(LOCAL_SOURCE_DIR)/scripts/configure \
			--build-env-dir="$(CHIP_BUILD_ENV_DIR)" \
			--project=examples/ota-provider-app/linux \
			--target=$(GNU_TARGET_NAME) \
			--enable-network-layer-ble=no \
			--enable-openthread=no \
			--enable-wifi=no \
			--mdns='platform'\
			--enable-tracing-support=no

endef

define Build/Compile
	$(NINJA) -C $(CHIP_OTA_PROVIDER_OUT_DIR) chip-ota-provider-app
endef

define Package/chip-ota-provider/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(CHIP_OTA_PROVIDER_OUT_DIR)/chip-ota-provider-app $(1)/usr/sbin/ota-provider-app
endef

$(eval $(call BuildPackage,chip-ota-provider))
