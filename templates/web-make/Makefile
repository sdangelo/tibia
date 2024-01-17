include vars.mk

CC = clang
CFLAGS = --target=wasm32 -flto -fvisibility=hidden -Ofast -Wall -Wextra -Wpedantic
LDFLAGS = \
	-Wl,--allow-undefined \
	-Wl,--no-entry \
	-Wl,--lto-O3 \
	-Wl,-strip-all \
	-Wl,--export-table \
	-Wl,--export=wrapper_new \
	-Wl,--export=wrapper_free \
	-Wl,--export=wrapper_get_ins \
	-Wl,--export=wrapper_get_outs \
	-Wl,--export=wrapper_get_param_values \
	-Wl,--export=wrapper_process \
	-Wl,--export=wrapper_set_parameter \
	-Wl,-z,stack-size=$$((8*1024*1024)) \
	-nostdlib

all: build/${BUNDLE_NAME}.wasm

build/${BUNDLE_NAME}.wasm: src/data.h src/memset.h src/plugin.h src/walloc.h src/wrapper.c | build
	${CC} src/wrapper.c -o $@ ${CFLAGS} ${LDFLAGS} ${CFLAGS_EXTRA} ${LIBS_EXTRA}

build:
	mkdir -p $@

clean:
	rm -fr build

.PHONY: all clean
