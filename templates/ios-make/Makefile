include vars.mk

ROOT_DIR := $(shell dirname $(realpath $(firstword $(MAKEFILE_LIST))))


SOURCES := data.h index.html app.swift native.mm app-Bridging-Header.h platform.h plugin.h
SOURCES_IN := $(addprefix ${ROOT_DIR}/src/, ${SOURCES})
SOURCES_OUT := $(addprefix ${ROOT_DIR}/build/gen/src/, ${SOURCES})
EXT_SOURCES_IN := ${3RDPARTYFILES}
EXT_SOURCES_OUT := 



all: build/gen/${PROJECT_NAME}.xcodeproj

${SOURCES_OUT}: ${SOURCES_IN} ${EXT_SOURCES_IN} | build/gen/src
	cp $^ ${ROOT_DIR}/build/gen/src

${EXT_SOURCES_OUT}: ${EXT_SOURCES_IN}
	cp ${EXT_SOURCES_IN} ${ROOT_DIR}/build/gen/src/

build/gen/${PROJECT_NAME}.xcodeproj: ${SOURCES_OUT}
	xcodegen generate --spec project.yml
	mv ${PROJECT_NAME}.xcodeproj build/gen/${PROJECT_NAME}.xcodeproj
	mv Info.plist build/gen/Info.plist

build/gen build/gen/src:
	mkdir -p $@

clean:
	rm -fr build

install: 

.PHONY: all clean install
