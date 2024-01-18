include vars.mk

CC = clang
CFLAGS = --target=wasm32 -flto -fvisibility=hidden -Ofast -Wall -Wextra -Wpedantic
LDFLAGS = \
	-Wl,--allow-undefined \
	-Wl,--no-entry \
	-Wl,--lto-O3 \
	-Wl,-strip-all \
	-Wl,--export-table \
	-Wl,--export=processor_new \
	-Wl,--export=processor_free \
	-Wl,--export=processor_get_ins \
	-Wl,--export=processor_get_outs \
	-Wl,--export=processor_get_param_values \
	-Wl,--export=processor_process \
	-Wl,--export=processor_set_parameter \
	-Wl,-z,stack-size=$$((8*1024*1024)) \
	-nostdlib

all: build/${BUNDLE_NAME}.wasm build/${BUNDLE_NAME}_processor.js

build/${BUNDLE_NAME}.wasm: src/data.h src/memset.h src/plugin.h src/walloc.h src/processor.c | build
	${CC} src/processor.c -o $@ ${CFLAGS} ${LDFLAGS} ${CFLAGS_EXTRA} ${LIBS_EXTRA}

build/${BUNDLE_NAME}_processor.js: src/processor.js | build
	cp $^ $@

build:
	mkdir -p $@

clean:
	rm -fr build

.PHONY: all clean
