SDK = /media/ext4/android-sdk

PROJECT = test
PACKAGE = nl.rd1.groep.$(PROJECT)
ANDROID_VERSION = 7
ACTIVITY = TestActivity
AVD = tester

BUILD_DIR = bin
CLASS_DIR = $(BUILD_DIR)/classes
SRC_DIR = src
ASSET_DIR = assets
RES_DIR = res
GEN_DIR = gen

MANIFEST = AndroidManifest.xml

RES = $(BUILD_DIR)/resources.apk
DEX = $(BUILD_DIR)/classes.dex
UNALIGNED = $(BUILD_DIR)/unaligned.apk
APK = $(BUILD_DIR)/$(PROJECT).apk

TOOLS = $(SDK)/tools
PLATFORM_TOOLS = $(SDK)/platform-tools

TARGET = android-$(ANDROID_VERSION)
JAR = $(SDK)/platforms/$(TARGET)/android.jar

ANDROID = $(TOOLS)/android
ADB = $(PLATFORM_TOOLS)/adb

all: apk

project:
	$(ANDROID) create project --package $(PACKAGE) \
		--activity $(ACTIVITY) --target $(TARGET) --path .

avd:
	$(ANDROID) avd

emu:
	$(TOOLS)/emulator -avd $(AVD)

debug:
	$(TOOLS)/ddms

dirs:
	mkdir -p $(BUILD_DIR) $(CLASS_DIR) \
		$(SRC_DIR) $(ASSET_DIR) $(RES_DIR) $(GEN_DIR)

res: dirs
	$(PLATFORM_TOOLS)/aapt package -m -I $(JAR) \
		-M $(MANIFEST) -A $(ASSET_DIR) -S $(RES_DIR) -J $(GEN_DIR) -Ff $(RES)

dex: $(DEX)

$(DEX): classes
	$(PLATFORM_TOOLS)/dx --dex --output $@ $(CLASS_DIR)

classes: res
	javac -classpath '$(JAR):libs/*.jar:$(CLASS_DIR)' -d $(CLASS_DIR) \
		$(shell find $(SRC_DIR) $(GEN_DIR) -iname '*.java')

apk: $(APK)

$(APK): $(DEX)
	$(TOOLS)/apkbuilder $(UNALIGNED) -f $(DEX) -z $(RES)
	$(TOOLS)/zipalign -f 4 $(UNALIGNED) $@

install: $(APK)
	$(ADB) install -r $(APK)

uninstall:
	$(ADB) uninstall $(PACKAGE)

run:
	$(ADB) shell am start -n $(PACKAGE)/$(PACKAGE).$(ACTIVITY)

irun: install run

clean:
	rm -rf $(BUILD_DIR)/*

.PHONY: all project avd emu debug dirs res dex classes apk \
	install uninstall run irun clean
