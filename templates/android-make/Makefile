include vars.mk

ifeq (${HAS_MIDI_IN}, yes)
MIN_API := 29
else
MIN_API := 26
endif

JC = javac
CXX = ${ANDROID_NDK_DIR}/toolchains/llvm/prebuilt/linux-x86_64/bin/armv7a-linux-androideabi${MIN_API}-clang++

APKSIGNER = ${BUILD_TOOLS_DIR}/apksigner
ZIPALIGN = ${BUILD_TOOLS_DIR}/zipalign
AAPT = ${BUILD_TOOLS_DIR}/aapt
D8 = ${BUILD_TOOLS_DIR}/d8
ADB = ${ANDROID_SDK_DIR}/platform-tools/adb

JARS := \
	${ANDROID_JAR_FILE} \
	${ANDROIDX_CORE_FILE} \
	${ANDROIDX_LIFECYCLE_COMMON_FILE} \
	${ANDROIDX_VERSIONEDPARCELABLE_FILE} \
	${KOTLIN_STDLIB_FILE} \
	${KOTLINX_COROUTINES_CORE_FILE} \
	${KOTLINX_COROUTINES_CORE_JVM_FILE}

CLASSES_PATH := $(subst .,/,$(JAVA_PACKAGE_NAME))

CLASSES := \
	MainActivity \
	MainActivity$$WebAppInterface
ifeq (${HAS_MIDI_IN}, yes)
	CLASSES += MainActivity$$WebAppInterface$$MidiDeviceCallback MainActivity$$WebAppInterface$$1
endif

JFLAGS =

CXXFLAGS = -O3 -Wall -Wextra -Wpedantic
CXXFLAGS_ALL = -fPIC -std=c++11 ${CXXFLAGS} ${CXXFLAGS_EXTRA}

LDFLAGS =
LDFLAGS_ALL = -shared -static-libstdc++ -landroid
ifeq (${HAS_MIDI_IN}, yes)
	LDFLAGS += -lamidi
endif
LDFLAGS_ALL += ${LDFLAGS} ${LDFLAGS_EXTRA}

all: build/${BUNDLE_NAME}.apk

build/${BUNDLE_NAME}.apk: build/gen/${BUNDLE_NAME}.aligned.apk ${KEY_STORE}
	${APKSIGNER} sign --ks ${KEY_STORE} --ks-key-alias ${KEY_ALIAS} --ks-pass pass:${STORE_PASS} --key-pass pass:${KEY_PASS} --out $@ build/gen/${BUNDLE_NAME}.aligned.apk

build/gen/${BUNDLE_NAME}.aligned.apk: build/gen/${BUNDLE_NAME}.unsigned.apk
	${ZIPALIGN} -f -p 4 $^ $@

build/gen/${BUNDLE_NAME}.unsigned.apk: build/apk/classes.dex build/apk/lib/armeabi-v7a/lib${BUNDLE_NAME}.so data/AndroidManifest.xml build/assets/index.html | build/gen
	${AAPT} package -f -M data/AndroidManifest.xml -A build/assets $(foreach jar,$(JARS),-I $(jar)) -F $@ build/apk

build/apk/classes.dex: build/apk/my_classes.jar
	cd build/apk && ${D8} --min-api ${MIN_API} ../../$^ ${JARS} && cd ../..

build/apk/my_classes.jar: $(foreach class,$(CLASSES),build/obj/$(CLASSES_PATH)/$(class).class) | build/apk
	@echo ${CLASSES_PATH}
	${D8} $(foreach class,$(CLASSES),'build/obj/$(CLASSES_PATH)/$(class).class') --min-api ${MIN_API} --output $@ --no-desugaring

build/obj/${CLASSES_PATH}/MainActivity.class: src/MainActivity.java | build/obj
	${JC} ${JFLAGS} -classpath "$(subst $() $(),:,$(JARS))" -d build/obj $^

build/apk/lib/armeabi-v7a/lib${BUNDLE_NAME}.so: src/jni.cpp | build/apk/lib/armeabi-v7a
	${CXX} $^ ${CXXFLAGS_ALL} ${LDFLAGS_ALL} -o $@

build/assets/index.html: src/index.html | build/assets
	cp $^ $@

build/gen build/apk build/obj build/apk/lib/armeabi-v7a build/assets:
	mkdir -p $@

clean:
	rm -fr build

install: build/${BUNDLE_NAME}.apk
	[ -n "`${ADB} shell pm list packages | grep ^package:${JAVA_PACKAGE_NAME}`" ] && ${ADB} uninstall ${JAVA_PACKAGE_NAME}; exit 0
	${ADB} install $^

.PHONY: all clean install
