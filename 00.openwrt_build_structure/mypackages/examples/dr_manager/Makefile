include $(TOPDIR)/rules.mk

# Name, version and release number
# The name and version of your package are used to define the variable to point to the build directory of your package: $(PKG_BUILD_DIR)
PKG_NAME:=dr_manager
PKG_VERSION:=0.2.3_0.2.0
PKG_RELEASE:=1

# Source settings (i.e. where to find the source codes)
# This is a custom variable, used below
SOURCE_DIR:=/home/greenhand/20.working/20.x-optical/01.mDR/01.dev/30.route_board/openwrt-TrolinkAsp_release0625/dr_manager

include $(INCLUDE_DIR)/package.mk

# Package definition; instructs on how and where our package will appear in the overall configuration menu ('make menuconfig')
define Package/dr_manager
	SECTION:=examples
	CATEGORY:=Examples
	TITLE:=DR Manager
	DEPENDS:=+libmodbus libpthread
endef

# Package description; a more verbose description on what our package does
define Package/dr_manager/description
	DR manager application.
endef

# Package preparation instructions; create the build directory and copy the source code. 
# The last command is necessary to ensure our preparation instructions remain compatible with the patching system.
define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	cp -r $(SOURCE_DIR)/* $(PKG_BUILD_DIR)
	$(Build/Patch)
endef

# Package build instructions; invoke the target-specific compiler to first compile the source file, and then to link the file into the final executable
define Build/Compile 
	$(MAKE) -C $(PKG_BUILD_DIR) CC="$(TARGET_CC)" CFLAGS="$(TARGET_CFLAGS) " LDFLAGS="$(TARGET_LDFLAGS)"
endef

# Package install instructions; create a directory inside the package to hold our executable, and then copy the executable we built previously into the folder
define Package/dr_manager/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/dr_manager $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/*.sh $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/at_cmd_processor/*.sh $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/gpio_key_monitor $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/mb_tcp_test_client $(1)/usr/bin
endef

# This command is always the last, it uses the definitions and variables we give above in order to get the job done
$(eval $(call BuildPackage,dr_manager))
