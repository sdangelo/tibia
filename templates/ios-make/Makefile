include vars.mk

COMMON_DIR := $(or $(COMMON_DIR),.)
DATA_DIR := $(or $(DATA_DIR),.)
PLUGIN_DIR := $(or $(PLUGIN_DIR),src)

SOURCES := \
	${DATA_DIR}/src/data.h \
	${DATA_DIR}/src/index.html \
	${COMMON_DIR}/src/app.swift \
	${COMMON_DIR}/src/native.mm \
	${COMMON_DIR}/src/app-Bridging-Header.h \
	${PLUGIN_DIR}/plugin.h \
	${C_SRCS_EXTRA} \
	${CXX_SRCS_EXTRA}
SOURCES_OUT = $(addprefix build/gen/src/, $(notdir $(SOURCES)))

all: build/gen/${BUNDLE_NAME}.xcodeproj

build/gen/${BUNDLE_NAME}.xcodeproj: ${SOURCES_OUT}
	xcodegen generate --spec project.yml
	mv ${BUNDLE_NAME}.xcodeproj build/gen/${BUNDLE_NAME}.xcodeproj
	mv Info.plist build/gen/Info.plist

build/gen/src:
	mkdir -p $@

clean:
	rm -fr build

.PHONY: all clean

.SECONDEXPANSION:

PERCENT := %

$(SOURCES_OUT): build/gen/src/%: $$(filter $$(PERCENT)/%,$$(SOURCES)) | build/gen/src
	cp $^ $@
