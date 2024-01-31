include vars.mk

ifeq ($(OS), Windows_NT)
	EXE_SUFFIX = .exe
else
	UNAME_S = $(shell uname -s)
	EXE_SUFFIX =
	PREFIX = /usr/local
	BINDIR = ${PREFIX}/bin
endif

CC = gcc

CFLAGS = -O3 -Wall -Wpedantic -Wextra
CFLAGS_ALL = -I${TINYWAV_DIR} -I${MIDI_PARSER_DIR}/include -fPIC ${CFLAGS} ${CFLAGS_EXTRA}

LDFLAGS =
LDFLAGS_ALL = ${LDFLAGS} ${LDFLAGS_EXTRA}

PROGRAM = ${BUNDLE_NAME}${EXE_SUFFIX}

all: build/${PROGRAM}

ifeq ($(UNAME_S), Darwin)

build/${PROGRAM}: build/tmp/x86_64 build/tmp/arm64
	lipo -create -output $@ $^

build/tmp/x86_64: src/main.c src/data.h src/plugin.h ${TINYWAV_DIR}/tinywav.c ${MIDI_PARSER_DIR}/src/midi-parser.c | build
	${CC} src/main.c ${TINYWAV_DIR}/tinywav.c ${MIDI_PARSER_DIR}/src/midi-parser.c -o $@ ${CFLAGS_ALL} ${LDFLAGS_ALL} -arch x86_64

build/tmp/arm64: src/main.c src/data.h src/plugin.h ${TINYWAV_DIR}/tinywav.c ${MIDI_PARSER_DIR}/src/midi-parser.c | build
	${CC} src/main.c ${TINYWAV_DIR}/tinywav.c ${MIDI_PARSER_DIR}/src/midi-parser.c -o $@ ${CFLAGS_ALL} ${LDFLAGS_ALL} -arch arm64

build/tmp:
	mkdir -p $@

else

build/${PROGRAM}: src/main.c src/data.h src/plugin.h ${TINYWAV_DIR}/tinywav.c ${MIDI_PARSER_DIR}/src/midi-parser.c | build
	${CC} src/main.c ${TINYWAV_DIR}/tinywav.c ${MIDI_PARSER_DIR}/src/midi-parser.c -o $@ ${CFLAGS_ALL} ${LDFLAGS_ALL}

endif

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
