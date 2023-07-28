include $(TOPDIR)/rules.mk

# Name, version and release number
# The name and version of your package are used to define the variable to point to the build directory of your package: $(PKG_BUILD_DIR)
PKG_NAME:=lcd_test
PKG_VERSION:=1.0
PKG_RELEASE:=1

# Source settings (i.e. where to find the source codes)
# This is a custom variable, used below
SOURCE_DIR:=/home/greenhand/20.working/20.x-optical/01.mDR/01.dev/30.route_board/openwrt-TrolinkAsp_release0625/lcd_test

include $(INCLUDE_DIR)/package.mk

# Package definition; instructs on how and where our package will appear in the overall configuration menu ('make menuconfig')
define Package/lcd_test
  SECTION:=examples
  CATEGORY:=Examples
  TITLE:=LCD test
endef

# Package description; a more verbose description on what our package does
define Package/lcd_test/description
  A simple lcd (on i2c) test -application.
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
	#$(TARGET_CC) $(TARGET_CFLAGS) -o $(PKG_BUILD_DIR)/lcd_test.o -c $(PKG_BUILD_DIR)/lcd_test.c
	#$(TARGET_CC) $(TARGET_LDFLAGS) -lm -o $(PKG_BUILD_DIR)/$1 $(PKG_BUILD_DIR)/lcd_test.o
	$(MAKE) -C $(PKG_BUILD_DIR) CC="$(TARGET_CC)" CFLAGS="$(TARGET_CFLAGS)" LDFLAGS="$(TARGET_LDFLAGS)"
endef

# Package install instructions; create a directory inside the package to hold our executable, and then copy the executable we built previously into the folder
define Package/lcd_test/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/lcd_test $(1)/usr/bin
endef

# This command is always the last, it uses the definitions and variables we give above in order to get the job done
$(eval $(call BuildPackage,lcd_test))
