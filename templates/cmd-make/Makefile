include vars.mk

PHONY_TARGETS = all clean
ifeq ($(OS), Windows_NT)
	EXE_SUFFIX = .exe
else
	EXE_SUFFIX =

	PHONY_TARGETS += install
	PREFIX = /usr/local
	BINDIR = ${PREFIX}/bin
endif

CC = gcc

CFLAGS = -I${TINYWAV_DIR} -I${MIDI_PARSER_DIR}/include -fPIC -Wall -Wpedantic -Wextra -Wno-unused-parameter

PROGRAM = ${BUNDLE_NAME}${EXE_SUFFIX}

all: build/${PROGRAM}

build/${PROGRAM}: src/main.c ${TINYWAV_DIR}/tinywav.c ${MIDI_PARSER_DIR}/src/midi-parser.c | build
	${CC} $^ -o $@ ${CFLAGS} ${CFLAGS_EXTRA} ${LDFLAGS_EXTRA}

build:
	mkdir -p $@

clean:
	rm -fr build

ifneq ($(OS), Windows_NT)
install: all
	mkdir -p -m 0755 ${BINDIR}
	install -m 0755 build/${PROGRAM} ${BINDIR}
endif

.PHONY: ${PHONY_TARGETS}
