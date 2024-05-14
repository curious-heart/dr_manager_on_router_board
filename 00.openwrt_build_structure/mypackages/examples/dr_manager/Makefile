include $(TOPDIR)/rules.mk

# Name, version and release number
# The name and version of your package are used to define the variable to point to the build directory of your package: $(PKG_BUILD_DIR)
PKG_NAME:=dr_manager
ifdef CONFIG_manage_lcd_and_tof_here
VER_STR_1:=$(shell grep -E "const[[:blank:]]+char[[:blank:]]*\*[[:blank:]]+g_APP_VER_STR[[:blank:]]*=" ../../../dr_manager/version_def.c | grep -n "" | grep ^1 | awk -F\" '{print $$2}')
else
VER_STR_1:=$(shell grep -E "const[[:blank:]]+char[[:blank:]]*\*[[:blank:]]+g_APP_VER_STR[[:blank:]]*=" ../../../dr_manager/version_def.c | grep -n "" | grep ^2 | awk -F\" '{print $$2}')
endif

VER_STR_2:=$(shell grep -E "const[[:blank:]]+char[[:blank:]]*\*[[:blank:]]+g_gpio_processor_APP_VER_STR[[:blank:]]*=" ../../../dr_manager/gpio_key_processor/gpio_key_app_version_def.c | awk -F\" '{print $$2}')
PKG_VERSION:=$(VER_STR_1)$(VER_STR_2)
PKG_RELEASE:=

# Source settings (i.e. where to find the source codes)
# This is a custom variable, used below
SOURCE_DIR:=/home/greenhand/20.working/20.x-optical/01.mDR/01.dev/30.route_board/openwrt-TrolinkAsp_release0625/dr_manager

include $(INCLUDE_DIR)/package.mk

# Package definition; instructs on how and where our package will appear in the overall configuration menu ('make menuconfig')
define Package/$(PKG_NAME)
	SECTION:=examples
	CATEGORY:=Examples
	TITLE:=DR Manager
	DEPENDS:=+libmodbus libpthread libsqlite3
endef

define Package/$(PKG_NAME)/config
	menu "build config"
		config manage_lcd_and_tof_here
			bool "manage_lcd_and_tof_here"
			default n
			help
				manage lcd and tof from route board.
	endmenu
endef

# Package description; a more verbose description on what our package does
define Package/$(PKG_NAME)/description
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
define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/$(PKG_NAME) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/*.sh $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/at_cmd_processor/*.sh $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/dap_calc/*.sqlite3 $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/dap_calc/dap_data.txt $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/gpio_key_monitor $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/mb_tcp_test_client $(1)/usr/bin
endef

# This command is always the last, it uses the definitions and variables we give above in order to get the job done
$(eval $(call BuildPackage,$(PKG_NAME)))
