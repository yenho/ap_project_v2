#
# Copyright (c) 2014 Qualcomm Atheros, Inc.
#
# All Rights Reserved.
# Qualcomm Atheros Confidential and Proprietary.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=fast-classifier
PKG_SOURCE_PROTO:=git
PKG_BRANCH:=master
PKG_RELEASE:=1

LOCAL_SRC:=$(TOPDIR)/qca/src/shortcut-fe
include $(INCLUDE_DIR)/local-development.mk
ifeq ($(DUMP)$(PKG_VERSION),)
  PKG_REV:=$(shell git ls-remote $(PKG_SOURCE_URL) $(PKG_BRANCH) | cut -b -7)
  PKG_VERSION:=g$(PKG_REV)
endif

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)
PKG_SOURCE_VERSION:=$(PKG_REV)

include $(INCLUDE_DIR)/package.mk

define KernelPackage/$(PKG_NAME)/Default
  SECTION:=kernel
  CATEGORY:=Kernel modules
  SUBMENU:=Network Support
  DEPENDS:=+kmod-shortcut-fe
  TITLE:=Kernel driver for FAST Classifier
  FILES:=$(PKG_BUILD_DIR)/fast-classifier/fast-classifier.ko
  KCONFIG:=CONFIG_NF_CONNTRACK_CHAIN_EVENTS=y
  PROVIDES:=$(PKG_NAME)
endef

define KernelPackage/$(PKG_NAME)
  $(call KernelPackage/$(PKG_NAME)/Default)
  AUTOLOAD:=$(call AutoLoad,51,fast-classifier)
endef

define KernelPackage/$(PKG_NAME)-noload
  $(call KernelPackage/$(PKG_NAME)/Default)
endef

define KernelPackage/$(PKG_NAME)/Default/description
FAST Classifier talks to SFE to make decisions about offloading connections
endef

define KernelPackage/$(PKG_NAME)/description
$(call KernelPackage/$(PKG_NAME)/Default/description)
endef

define KernelPackage/$(PKG_NAME)-noload/description
$(call KernelPackage/$(PKG_NAME)/Default/description)

This package does not load $(PKG_NAME) at boot by default
endef

define Package/fast-classifier-example
  TITLE:=Example user space program for fast-classifier
  DEPENDS:=+libnl-bf +libfast-classifier
endef

define Package/fast-classifier-example/description
Example user space program that communicates with fast
classifier kernel module
endef

define Build/Compile
	$(MAKE) -C "$(LINUX_DIR)" \
		CONFIG_FAST_CLASSIFIER=m \
		CROSS_COMPILE="$(TARGET_CROSS)" \
		ARCH="$(LINUX_KARCH)" \
		SUBDIRS="$(PKG_BUILD_DIR)/fast-classifier" \
		EXTRA_CFLAGS="$(EXTRA_CFLAGS)" \
		modules

	$(TARGET_CC) -o $(PKG_BUILD_DIR)/userspace_fast_classifier \
		-I $(PKG_BUILD_DIR)/fast-classifier \
		-I$(STAGING_DIR)/usr/include/libnl3 \
		-lnl-genl-3 -lnl-3 \
		$(PKG_BUILD_DIR)/fast-classifier/userspace_example.c
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_BUILD_DIR)/fast-classifier/fast-classifier.h $(1)/usr/include/
endef


define Package/fast-classifier-example/install
	$(INSTALL_DIR) $(1)/sbin
	$(CP) $(PKG_BUILD_DIR)/userspace_fast_classifier $(1)/sbin/
endef

$(eval $(call KernelPackage,$(PKG_NAME)))
$(eval $(call KernelPackage,$(PKG_NAME)-noload))
$(eval $(call BuildPackage,fast-classifier-example))
