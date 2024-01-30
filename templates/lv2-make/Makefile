include vars.mk

ifeq ($(OS), Windows_NT)
	DLL_SUFFIX = .dll
	COMMONPROGRAMFILES = $(shell echo '${COMMONPROGRAMFILES}' | sed 's:\\:/:g')
	APPDATA = $(shell echo '${APPDATA}' | sed 's:\\:/:g')
	LV2DIR = ${COMMONPROGRAMFILES}/LV2
	LV2DIR_USER = ${APPDATA}/LV2
else
	UNAME_S = $(shell uname -s)
	ifeq ($(UNAME_S), Darwin)
		DLL_SUFFIX = .dylib
		LV2DIR = /Library/Audio/Plug-Ins/LV2
		LV2DIR_USER = ${HOME}/Library/Audio/Plug-Ins/LV2
	else
		DLL_SUFFIX = .so
		PREFIX = /usr/local
		LV2DIR = ${PREFIX}/lib/lv2
		LV2DIR_USER = ${HOME}/.lv2
	endif
endif

CC = gcc

CFLAGS = -fPIC -Wall -Wpedantic -Wextra -Wno-unused-parameter
LDFLAGS = -shared

BUNDLE_DIR = ${BUNDLE_NAME}.lv2

DLL_FILE = ${BUNDLE_NAME}${DLL_SUFFIX}

all: build/${BUNDLE_DIR}/manifest.ttl build/${BUNDLE_DIR}/${DLL_FILE}

build/${BUNDLE_DIR}/manifest.ttl: data/manifest.ttl.in | build/${BUNDLE_DIR}
	cat $^ | sed s/@DLL_SUFFIX@/${DLL_SUFFIX}/g > $@

build/${BUNDLE_DIR}/${DLL_FILE}: src/lv2.c | build/${BUNDLE_DIR}
	${CC} $^ -o $@ ${CFLAGS} ${CFLAGS_EXTRA} ${LDFLAGS} ${LDFLAGS_EXTRA}

build/${BUNDLE_DIR}:
	mkdir -p $@

clean:
	rm -fr build

install: all
	mkdir -p -m 0755 ${LV2DIR}/${BUNDLE_DIR}
	install -m 0644 build/${BUNDLE_DIR}/manifest.ttl ${LV2DIR}/${BUNDLE_DIR}
	install -m 0755 build/${BUNDLE_DIR}/${DLL_FILE} ${LV2DIR}/${BUNDLE_DIR}

install-user: all
	mkdir -p -m 0755 ${LV2DIR_USER}/${BUNDLE_DIR}
	install -m 0644 build/${BUNDLE_DIR}/manifest.ttl ${LV2DIR_USER}/${BUNDLE_DIR}
	install -m 0755 build/${BUNDLE_DIR}/${DLL_FILE} ${LV2DIR_USER}/${BUNDLE_DIR}

.PHONY: all clean install install-user
