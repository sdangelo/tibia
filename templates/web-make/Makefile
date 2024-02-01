include vars.mk

COMMON_DIR := $(or $(COMMON_DIR),.)
DATA_DIR := $(or $(DATA_DIR),.)
PLUGIN_DIR := $(or $(PLUGIN_DIR),src)

CC = clang

CFLAGS = -Ofast -Wall -Wpedantic -Wextra
CFLAGS_ALL = -I${COMMON_DIR}/src -I${DATA_DIR}/src -I${PLUGIN_DIR} --target=wasm32 -flto -fvisibility=hidden ${CFLAGS} ${CFLAGS_EXTRA}

LDFLAGS_ALL = \
	-Wl,--allow-undefined \
	-Wl,--no-entry \
	-Wl,--lto-O3 \
	-Wl,-strip-all \
	-Wl,--export-table \
	-Wl,--export=processor_new \
	-Wl,--export=processor_free \
	-Wl,--export=processor_get_x_buf \
	-Wl,--export=processor_get_y_buf \
	-Wl,--export=processor_get_out_params \
	-Wl,--export=processor_process \
	-Wl,--export=processor_set_parameter \
	-Wl,-z,stack-size=$$((8*1024*1024)) \
	-nostdlib
ifeq (${HAS_MIDI_IN}, yes)
LDFLAGS_ALL += -Wl,--export=processor_midi_msg_in
endif
LDFLAGS_ALL += ${LDFLAGS} ${LDFLAGS_EXTRA}

ALL = build/${BUNDLE_NAME}.wasm build/${BUNDLE_NAME}_processor.js build/${BUNDLE_NAME}.js

default: all

-include ${COMMON_DIR}/demo.mk

all: ${ALL}

build/${BUNDLE_NAME}.wasm: ${COMMON_DIR}/src/memset.h ${COMMON_DIR}/src/walloc.h ${COMMON_DIR}/src/processor.c ${DATA_DIR}/src/data.h ${PLUGIN_DIR}/plugin.h | build
	${CC} ${COMMON_DIR}/src/processor.c -o $@ ${CFLAGS_ALL} ${LDFLAGS_ALL}

build/${BUNDLE_NAME}_processor.js: ${DATA_DIR}/src/processor.js | build
	cp $^ $@

build/${BUNDLE_NAME}.js: ${DATA_DIR}/src/module.js | build
	cp $^ $@

build:
	mkdir -p $@

clean:
	rm -fr build

.PHONY: all clean
