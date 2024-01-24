include vars.mk

CC = gcc

CFLAGS = -I${TINYWAV_DIR} -fPIC -Wall -Wpedantic -Wextra -Wno-unused-parameter

all: build/${BUNDLE_NAME}

build/${BUNDLE_NAME}: src/main.c ${TINYWAV_DIR}/tinywav.c | build
	${CC} $^ -o $@ ${CFLAGS} ${CFLAGS_EXTRA} ${LDFLAGS_EXTRA}

build:
	mkdir -p $@

clean:
	rm -fr build

.PHONY: all clean
