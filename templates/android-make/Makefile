#
# Tibia
#
# Copyright (C) 2023, 2024 Orastron Srl unipersonale
#
# Tibia is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, version 3 of the License.
#
# Tibia is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Tibia.  If not, see <http://www.gnu.org/licenses/>.
#
# File author: Stefano D'Angelo
#

include vars.mk

ifeq ($(HAS_MIDI_IN), yes)
MIN_API := 29
else
MIN_API := 26
endif

CC := $(ANDROID_NDK_DIR)/toolchains/llvm/prebuilt/linux-x86_64/bin/armv7a-linux-androideabi$(MIN_API)-clang
CXX := $(ANDROID_NDK_DIR)/toolchains/llvm/prebuilt/linux-x86_64/bin/armv7a-linux-androideabi$(MIN_API)-clang++
JC := javac

APKSIGNER := $(BUILD_TOOLS_DIR)/apksigner
ZIPALIGN := $(BUILD_TOOLS_DIR)/zipalign
AAPT := $(BUILD_TOOLS_DIR)/aapt
D8 := $(BUILD_TOOLS_DIR)/d8
ADB := $(ANDROID_SDK_DIR)/platform-tools/adb

JARS := \
	$(ANDROID_JAR_FILE) \
	$(ANDROIDX_CORE_FILE) \
	$(ANDROIDX_LIFECYCLE_COMMON_FILE) \
	$(ANDROIDX_VERSIONEDPARCELABLE_FILE) \
	$(KOTLIN_STDLIB_FILE) \
	$(KOTLINX_COROUTINES_CORE_FILE) \
	$(KOTLINX_COROUTINES_CORE_JVM_FILE)

CLASSES_PATH := $(subst .,/,$(JAVA_PACKAGE_NAME))

CLASSES := \
	MainActivity \
	MainActivity$$WebAppInterface
ifeq ($(HAS_MIDI_IN), yes)
	CLASSES += MainActivity$$WebAppInterface$$MidiDeviceCallback MainActivity$$WebAppInterface$$1
endif

COMMON_DIR := $(or $(COMMON_DIR),.)
DATA_DIR := $(or $(DATA_DIR),.)
PLUGIN_DIR := $(or $(PLUGIN_DIR),src)

CFLAGS := -O3 -Wall -Wpedantic -Wextra
CFLAGS_ALL := -I$(DATA_DIR)/src -I$(PLUGIN_DIR) -fPIC $(CFLAGS) $(CFLAGS_EXTRA)

CXXFLAGS := $(CFLAGS)
CXXFLAGS_ALL := -I$(DATA_DIR)/src -I$(PLUGIN_DIR) -fPIC -std=c++11 $(CXXFLAGS) $(CXXFLAGS_EXTRA)

LDFLAGS :=
LDFLAGS_ALL := -shared -static-libstdc++ -landroid
ifeq ($(HAS_MIDI_IN), yes)
	LDFLAGS += -lamidi
endif
LDFLAGS_ALL += $(LDFLAGS) $(LDFLAGS_EXTRA)

JFLAGS :=
JFLAGS_ALL := $(JFLAGS) $(JFLAGS_EXTRA)

C_SRCS := $(C_SRCS_EXTRA)
C_OBJS := $(addprefix build/obj/, $(notdir $(C_SRCS:.c=.o)))

CXX_SRCS := $(COMMON_DIR)/src/jni.cpp $(CXX_SRCS_EXTRA)
CXX_OBJS := $(addprefix build/obj/, $(notdir $(CXX_SRCS:.cpp=.o)))

ALL := build/$(BUNDLE_NAME).apk

-include $(COMMON_DIR)/vars-extra.mk

all: $(ALL)

build/$(BUNDLE_NAME).apk: build/gen/$(BUNDLE_NAME).aligned.apk $(KEY_STORE)
	$(APKSIGNER) sign --ks $(KEY_STORE) --ks-key-alias $(KEY_ALIAS) --ks-pass pass:$(STORE_PASS) --key-pass pass:$(KEY_PASS) --out $@ build/gen/$(BUNDLE_NAME).aligned.apk

build/gen/$(BUNDLE_NAME).aligned.apk: build/gen/$(BUNDLE_NAME).unsigned.apk
	$(ZIPALIGN) -f -p 4 $^ $@

build/gen/$(BUNDLE_NAME).unsigned.apk: build/apk/classes.dex build/apk/lib/armeabi-v7a/lib$(BUNDLE_NAME).so $(DATA_DIR)/data/AndroidManifest.xml build/assets/index.html | build/gen
	$(AAPT) package -f -M $(DATA_DIR)/data/AndroidManifest.xml -A build/assets $(foreach jar,$(JARS),-I $(jar)) -F $@ build/apk

build/apk/classes.dex: build/apk/my_classes.jar
	cd build/apk && $(D8) --min-api $(MIN_API) ../../$^ $(JARS) && cd ../..

build/apk/my_classes.jar: $(foreach class,$(CLASSES),build/obj/$(CLASSES_PATH)/$(class).class) | build/apk
	@echo $(CLASSES_PATH)
	$(D8) $(foreach class,$(CLASSES),'build/obj/$(CLASSES_PATH)/$(class).class') --min-api $(MIN_API) --output $@ --no-desugaring

build/obj/$(CLASSES_PATH)/MainActivity.class: $(DATA_DIR)/src/MainActivity.java | build/obj
	$(JC) $(JFLAGS_ALL) -classpath "$(subst $() $(),:,$(JARS))" -d build/obj $^

build/apk/lib/armeabi-v7a/lib$(BUNDLE_NAME).so: $(C_OBJS) $(CXX_OBJS) | build/apk/lib/armeabi-v7a
	$(CXX) $^ -o $@ $(CFLAGS_ALL) $(CXXFLAGS_ALL) $(LDFLAGS_ALL)

build/assets/index.html: $(DATA_DIR)/src/index.html | build/assets
	cp $^ $@

build/gen build/apk build/obj build/apk/lib/armeabi-v7a build/assets:
	mkdir -p $@

clean:
	rm -fr build

install: build/$(BUNDLE_NAME).apk
	[ -n "`$(ADB) shell pm list packages | grep ^package:$(JAVA_PACKAGE_NAME)`" ] && $(ADB) uninstall $(JAVA_PACKAGE_NAME); exit 0
	$(ADB) install $^

-include $(COMMON_DIR)/rules-extra.mk

.PHONY: all clean install

.SECONDEXPANSION:

PERCENT := %

$(C_OBJS): build/obj/%.o: $$(filter $$(PERCENT)/$$(basename $$(notdir $$@)).c,$$(C_SRCS)) | build/obj
	$(CC) $^ -o $@ -c $(CFLAGS_ALL)

$(CXX_OBJS): build/obj/%.o: $$(filter $$(PERCENT)/$$(basename $$(notdir $$@)).cpp,$$(CXX_SRCS)) | build/obj
	$(CXX) $^ -o $@ -c $(CXXFLAGS_ALL)

-include $(COMMON_DIR)/rules-secondexp-extra.mk
