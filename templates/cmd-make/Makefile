include vars.mk

ifeq ($(OS), Windows_NT)
	EXE_SUFFIX = .exe
else
	UNAME_S = $(shell uname -s)
	EXE_SUFFIX =
	PREFIX = /usr/local
	BINDIR = ${PREFIX}/bin
endif

COMMON_DIR := $(or $(COMMON_DIR),.)
DATA_DIR := $(or $(DATA_DIR),.)
PLUGIN_DIR := $(or $(PLUGIN_DIR),src)

CC = gcc

CFLAGS = -O3 -Wall -Wpedantic -Wextra
CFLAGS_ALL = -I${DATA_DIR}/src -I${PLUGIN_DIR} -I${TINYWAV_DIR} -I${MIDI_PARSER_DIR}/include -fPIC ${CFLAGS} ${CFLAGS_EXTRA}
ifeq ($(UNAME_S), Darwin)
CFLAGS_ALL := ${CFLAGS_ALL} -arch arm64 -arch x86_64
endif

LDFLAGS =
LDFLAGS_ALL = ${LDFLAGS} ${LDFLAGS_EXTRA}

PROGRAM = ${BUNDLE_NAME}${EXE_SUFFIX}

all: build/${PROGRAM}

build/${PROGRAM}: ${COMMON_DIR}/src/main.c ${DATA_DIR}/src/data.h ${PLUGIN_DIR}/plugin.h ${TINYWAV_DIR}/tinywav.c ${MIDI_PARSER_DIR}/src/midi-parser.c | build
	${CC} ${COMMON_DIR}/src/main.c ${TINYWAV_DIR}/tinywav.c ${MIDI_PARSER_DIR}/src/midi-parser.c -o $@ ${CFLAGS_ALL} ${LDFLAGS_ALL}

build:
	mkdir -p $@

clean:
	rm -fr build

ifeq ($(OS), Windows_NT)

.PHONY: all clean

else

install: all
	mkdir -p -m 0755 ${BINDIR}
	install -m 0755 build/${PROGRAM} ${BINDIR}

.PHONY: all clean install

endif
