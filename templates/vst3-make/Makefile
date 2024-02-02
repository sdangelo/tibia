include vars.mk

ifeq ($(OS), Windows_NT)
	DLL_SUFFIX = .vst3
	PLATFORM = x86_64-win
	VST3DIR = $(shell echo '${COMMONPROGRAMFILES}' | sed 's:\\:/:g')/VST3
	VST3DIR_USER = $(shell echo '${LOCALAPPDATA}' | sed 's:\\:/:g')/Programs/Common/VST3
	CC = gcc
	CXX = g++
else
	UNAME_S = $(shell uname -s)
	ifeq ($(UNAME_S), Darwin)
		DLL_SUFFIX =
		PLATFORM = MacOS
		VST3DIR = /Library/Audio/Plug-Ins/VST3
		VST3DIR_USER = ${HOME}/Library/Audio/Plug-Ins/VST3
		CC = clang
		CXX = clang++
	else
		DLL_SUFFIX = .so
		PLATFORM = $(shell uname -m)-linux
		VST3DIR = /usr/local/lib/vst3
		VST3DIR_USER = ${HOME}/.vst3
		CC = gcc
		CXX = g++
	endif
endif

COMMON_DIR := $(or $(COMMON_DIR),.)
DATA_DIR := $(or $(DATA_DIR),.)
PLUGIN_DIR := $(or $(PLUGIN_DIR),src)

CFLAGS = -O3 -Wall -Wpedantic -Wextra
CFLAGS_ALL = -I${DATA_DIR}/src -I${PLUGIN_DIR} -fPIC ${CFLAGS} ${CFLAGS_EXTRA}

LDFLAGS =
LDFLAGS_ALL = -shared -lm ${LDFLAGS} ${LDFLAGS_EXTRA}

CXXFLAGS = ${CFLAGS}
CXXFLAGS_ALL = -I${DATA_DIR}/src -I${PLUGIN_DIR} -fPIC ${CXXFLAGS} ${CXXFLAGS_EXTRA}

ifeq ($(UNAME_S), Darwin)
CFLAGS_ALL := ${CFLAGS_ALL} -arch arm64 -arch x86_64
LDFLAGS_ALL := ${LDFLAGS_ALL} -arch arm64 -arch x86_64
CXXFLAGS_ALL := ${CXXFLAGS_ALL} -arch arm64 -arch x86_64
endif

BUNDLE_DIR = ${BUNDLE_NAME}.vst3

DLL_DIR = Contents/${PLATFORM}
DLL_FILE = ${DLL_DIR}/${BUNDLE_NAME}${DLL_SUFFIX}

C_SRCS = ${COMMON_DIR}/src/vst3.c ${C_SRCS_EXTRA}
C_OBJS = $(addprefix build/obj/, $(notdir $(C_SRCS:.c=.o)))

CXX_SRCS = ${CXX_SRCS_EXTRA}
CXX_OBJS = $(addprefix build/obj/, $(notdir $(CXX_SRCS:.cpp=.o)))

ifeq ($(UNAME_S), Darwin)
all: build/${BUNDLE_DIR}/${DLL_FILE} build/${BUNDLE_DIR}/Contents/Info.plist

build/${BUNDLE_DIR}/Contents/Info.plist: ${DATA_DIR}/data/Info.plist | build/${BUNDLE_DIR}/Contents
	cp $^ $@
else
all: build/${BUNDLE_DIR}/${DLL_FILE}
endif

ifeq ($(CXX_OBJS),)
build/${BUNDLE_DIR}/${DLL_FILE}: ${C_OBJS} | build/${BUNDLE_DIR}/${DLL_DIR}
	${CC} $^ -o $@ ${CFLAGS_ALL} ${LDFLAGS_ALL}
else
build/${BUNDLE_DIR}/${DLL_FILE}: ${C_OBJS} ${CXX_OBJS} | build/${BUNDLE_DIR}/${DLL_DIR}
	${CXX} $^ -o $@ ${CFLAGS_ALL} ${CXXFLAGS_ALL} ${LDFLAGS_ALL}
endif

build/${BUNDLE_DIR}/${DLL_DIR} build/obj:
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

.SECONDEXPANSION:

PERCENT := %

$(C_OBJS): build/obj/%.o: $$(filter $$(PERCENT)/$$(basename $$(notdir $$@)).c,$$(C_SRCS)) | build/obj
	${CC} $^ -o $@ -c ${CFLAGS_ALL}

$(CXX_OBJS): build/obj/%.o: $$(filter $$(PERCENT)/$$(basename $$(notdir $$@)).cpp,$$(CXX_SRCS)) | build/obj
	${CXX} $^ -o $@ -c ${CXXFLAGS_ALL}
