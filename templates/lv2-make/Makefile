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

COMMON_DIR := $(or $(COMMON_DIR),.)
DATA_DIR := $(or $(DATA_DIR),.)
PLUGIN_DIR := $(or $(PLUGIN_DIR),src)

CFLAGS = -O3 -Wall -Wpedantic -Wextra
CFLAGS_ALL = -I${DATA_DIR}/src -I${PLUGIN_DIR} -fPIC ${CFLAGS} ${CFLAGS_EXTRA}

LDFLAGS =
LDFLAGS_ALL = -shared ${LDFLAGS} ${LDFLAGS_EXTRA}

ifeq ($(UNAME_S), Darwin)
CFLAGS_ALL := ${CFLAGS_ALL} -arch arm64 -arch x86_64
LDFLAGS_ALL := ${LDFLAGS_ALL} -arch arm64 -arch x86_64
endif

BUNDLE_DIR = ${BUNDLE_NAME}.lv2

DLL_FILE = ${BUNDLE_NAME}${DLL_SUFFIX}

all: build/${BUNDLE_DIR}/manifest.ttl build/${BUNDLE_DIR}/${DLL_FILE}

build/${BUNDLE_DIR}/manifest.ttl: ${DATA_DIR}/data/manifest.ttl.in | build/${BUNDLE_DIR}
	cat $^ | sed s/@DLL_SUFFIX@/${DLL_SUFFIX}/g > $@

build/${BUNDLE_DIR}/${DLL_FILE}: ${COMMON_DIR}/src/lv2.c ${DATA_DIR}/src/data.h ${PLUGIN_DIR}/plugin.h | build/${BUNDLE_DIR}
	${CC} ${COMMON_DIR}/src/lv2.c -o $@ ${CFLAGS_ALL} ${LDFLAGS_ALL}

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
