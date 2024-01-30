include vars.mk

ifeq ($(OS), Windows_NT)
	DLL_SUFFIX = .vst3
	PLATFORM = x86_64-win
	VST3DIR = $(shell echo '${COMMONPROGRAMFILES}' | sed 's:\\:/:g')/VST3
	VST3DIR_USER = $(shell echo '${LOCALAPPDATA}' | sed 's:\\:/:g')/Programs/Common/VST3
	CC = gcc
else
	UNAME_S = $(shell uname -s)
	ifeq ($(UNAME_S), Darwin)
		DLL_SUFFIX =
		PLATFORM = MacOS
		VST3DIR = /Library/Audio/Plug-Ins/VST3
		VST3DIR_USER = ${HOME}/Library/Audio/Plug-Ins/VST3
		CC = clang
	else
		DLL_SUFFIX = .so
		PLATFORM = $(shell uname -m)-linux
		VST3DIR = /usr/local/lib/vst3
		VST3DIR_USER = ${HOME}/.vst3
		CC = gcc
	endif
endif

CFLAGS = -O3 -fPIC -Wall -Wpedantic -Wextra -Wno-unused-parameter
LDFLAGS = -shared -lm

BUNDLE_DIR = ${BUNDLE_NAME}.vst3

DLL_DIR = Contents/${PLATFORM}
DLL_FILE = ${DLL_DIR}/${BUNDLE_NAME}${DLL_SUFFIX}

ifeq ($(UNAME_S), Darwin)

all: build/${BUNDLE_DIR}/${DLL_FILE} build/${BUNDLE_DIR}/Contents/Info.plist

build/${BUNDLE_DIR}/Contents/Info.plist: data/Info.plist | build/${BUNDLE_DIR}/Contents
	cp $^ $@

build/${BUNDLE_DIR}/${DLL_FILE}: build/tmp/x86_64 build/tmp/arm64 | buindle/${BUNDLE_DIR}/Contents
	lipo -create -output $@ $^

build/tmp/x86_64: src/vst3.c src/data.h src/plugin.h | build/tmp
	${CC} src/vst3.c -o $@ ${CFLAGS} ${CFLAGS_EXTRA} ${LDFLAGS} ${LDFLAGS_EXTRA} -arch x86_64

build/tmp/arm64: src/vst3.c src/data.h src/plugin.h | build/tmp
	${CC} src/vst3.c -o $@ ${CFLAGS} ${CFLAGS_EXTRA} ${LDFLAGS} ${LDFLAGS_EXTRA} -arch arm64

build/tmp:
	mkdir -p $@

else

all: build/${BUNDLE_DIR}/${DLL_FILE}

build/${BUNDLE_DIR}/${DLL_FILE}: src/vst3.c src/data.h | build/${BUNDLE_DIR}/${DLL_DIR}
	${CC} src/vst3.c -o $@ ${CFLAGS} ${CFLAGS_EXTRA} ${LDFLAGS} ${LDFLAGS_EXTRA}

endif

build/${BUNDLE_DIR}/Contents build/${BUNDLE_DIR}/${DLL_DIR}:
	mkdir -p $@

clean:
	rm -fr build

ifeq ($(UNAME_S), Darwin)

install: all
	mkdir -p -m 0755 "${VST3DIR}/${BUNDLE_DIR}/${DLL_DIR}"
	install -m 0755 build/${BUNDLE_DIR}/${DLL_FILE} "${VST3DIR}/${BUNDLE_DIR}/${DLL_DIR}"
	install -m 0644 build/${BUNDLE_DIR}/Contents/Info.plist "${VST3DIR}/${BUNDLE_DIR}/Contents/Info.plist"

install-user: all
	mkdir -p -m 0755 "${VST3DIR_USER}/${BUNDLE_DIR}/${DLL_DIR}"
	install -m 0755 build/${BUNDLE_DIR}/${DLL_FILE} "${VST3DIR_USER}/${BUNDLE_DIR}/${DLL_DIR}"
	install -m 0644 build/${BUNDLE_DIR}/Contents/Info.plist "${VST3DIR_USER}/${BUNDLE_DIR}/Contents/Info.plist"

else

install: all
	mkdir -p -m 0755 "${VST3DIR}/${BUNDLE_DIR}/${DLL_DIR}"
	install -m 0755 build/${BUNDLE_DIR}/${DLL_FILE} "${VST3DIR}/${BUNDLE_DIR}/${DLL_DIR}"

install-user: all
	mkdir -p -m 0755 "${VST3DIR_USER}/${BUNDLE_DIR}/${DLL_DIR}"
	install -m 0755 build/${BUNDLE_DIR}/${DLL_FILE} "${VST3DIR_USER}/${BUNDLE_DIR}/${DLL_DIR}"

endif

.PHONY: all clean install install-user
