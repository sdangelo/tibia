include vars.mk

ifeq ($(OS), Windows_NT)
	DLL_SUFFIX = .dll
	LV2DIR = $(shell echo '${COMMONPROGRAMFILES}' | sed 's:\\:/:g')/LV2
	LV2DIR_USER = $(shell echo '${APPDATA}' | sed 's:\\:/:g')/LV2
	CC = gcc
else
	UNAME_S = $(shell uname -s)
	ifeq ($(UNAME_S), Darwin)
		DLL_SUFFIX = .dylib
		LV2DIR = /Library/Audio/Plug-Ins/LV2
		LV2DIR_USER = ${HOME}/Library/Audio/Plug-Ins/LV2
		CC = clang
	else
		DLL_SUFFIX = .so
		PREFIX = /usr/local
		LV2DIR = ${PREFIX}/lib/lv2
		LV2DIR_USER = ${HOME}/.lv2
		CC = gcc
	endif
endif

CFLAGS = -O3 -Wall -Wpedantic -Wextra
CFLAGS_ALL = -fPIC ${CFLAGS} ${CFLAGS_EXTRA}

LDFLAGS =
LDFLAGS_ALL = -shared ${LDFLAGS} ${LDFLAGS_EXTRA}

BUNDLE_DIR = ${BUNDLE_NAME}.lv2

DLL_FILE = ${BUNDLE_NAME}${DLL_SUFFIX}

all: build/${BUNDLE_DIR}/manifest.ttl build/${BUNDLE_DIR}/${DLL_FILE}

build/${BUNDLE_DIR}/manifest.ttl: data/manifest.ttl.in | build/${BUNDLE_DIR}
	cat $^ | sed s/@DLL_SUFFIX@/${DLL_SUFFIX}/g > $@

ifeq ($(UNAME_S), Darwin)

build/${BUNDLE_DIR}/${DLL_FILE}: build/tmp/x86_64 build/tmp/arm64 | build/${BUNDLE_DIR}
	lipo -create -output $@ $^

build/tmp/x86_64: src/lv2.c src/data.h src/plugin.h | build/tmp
	${CC} src/lv2.c -o $@ ${CFLAGS_ALL} ${LDFLAGS_ALL} -arch x86_64

build/tmp/arm64: src/lv2.c src/data.h src/plugin.h | build/tmp
	${CC} src/lv2.c -o $@ ${CFLAGS_ALL} ${LDFLAGS_ALL} -arch arm64

build/tmp:
	mkdir -p $@

else

build/${BUNDLE_DIR}/${DLL_FILE}: src/lv2.c src/data.h | build/${BUNDLE_DIR}
	${CC} src/lv2.c -o $@ ${CFLAGS_ALL} ${LDFLAGS_ALL}

endif

build/${BUNDLE_DIR}:
	mkdir -p $@

clean:
	rm -fr build

install: all
	mkdir -p -m 0755 "${LV2DIR}/${BUNDLE_DIR}"
	install -m 0644 build/${BUNDLE_DIR}/manifest.ttl "${LV2DIR}/${BUNDLE_DIR}"
	install -m 0755 build/${BUNDLE_DIR}/${DLL_FILE} "${LV2DIR}/${BUNDLE_DIR}"

install-user: all
	mkdir -p -m 0755 "${LV2DIR_USER}/${BUNDLE_DIR}"
	install -m 0644 build/${BUNDLE_DIR}/manifest.ttl "${LV2DIR_USER}/${BUNDLE_DIR}"
	install -m 0755 build/${BUNDLE_DIR}/${DLL_FILE} "${LV2DIR_USER}/${BUNDLE_DIR}"

.PHONY: all clean install install-user
