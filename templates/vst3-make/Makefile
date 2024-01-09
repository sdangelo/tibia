include vars.mk

CC = gcc
CFLAGS = -fPIC -Wall -Wpedantic -Wextra -Wno-unused-parameter

BUNDLE_DIR = ${BUNDLE_NAME}.vst3

SO_DIR = $(shell uname -m)-linux
SO_FILE = ${SO_DIR}/${BUNDLE_NAME}.so

all: build/${BUNDLE_DIR}/Contents/Info.plist build/${BUNDLE_DIR}/Contents/${SO_FILE}

build/${BUNDLE_DIR}/Contents/Info.plist: data/Info.plist | build/${BUNDLE_DIR}/Contents
	cp $^ $@

build/${BUNDLE_DIR}/Contents/${SO_FILE}: src/vst3.c | build/${BUNDLE_DIR}/Contents/${SO_DIR}
	${CC} $^ -o $@ ${CFLAGS} ${CFLAGS_EXTRA} ${LIBS_EXTRA} -shared

build/${BUNDLE_DIR}/Contents build/${BUNDLE_DIR}/Contents/${SO_DIR}:
	mkdir -p $@

clean:
	rm -fr build

.PHONY: all clean
