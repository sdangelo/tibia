include vars.mk

CC = gcc
CFLAGS = -fPIC -Wall -Wpedantic -Wextra -Wno-unused-parameter

BUNDLE_DIR = ${BUNDLE_NAME}.lv2

SO_FILE := ${BUNDLE_NAME}.so

all: build/${BUNDLE_DIR}/manifest.ttl build/${BUNDLE_DIR}/${SO_FILE}

build/${BUNDLE_DIR}/manifest.ttl: data/manifest.ttl | build/${BUNDLE_DIR}
	cp $^ $@

build/${BUNDLE_DIR}/${SO_FILE}: src/lv2.c | build/${BUNDLE_DIR}
	${CC} $^ -o $@ ${CFLAGS} -shared

build/${BUNDLE_DIR}:
	mkdir -p $@

clean:
	rm -fr build

.PHONY: clean
