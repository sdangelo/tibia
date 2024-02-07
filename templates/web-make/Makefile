include vars.mk

COMMON_DIR := $(or $(COMMON_DIR),.)
DATA_DIR := $(or $(DATA_DIR),.)
PLUGIN_DIR := $(or $(PLUGIN_DIR),src)

CC = clang
CXX = clang++

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
	-Wl,--export=processor_get_x \
	-Wl,--export=processor_get_zero_buf \
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

CXXFLAGS = ${CFLAGS}
CXXFLAGS_ALL = -I${COMMON_DIR}/src -I${DATA_DIR}/src -I${PLUGIN_DIR} --target=wasm32 -flto -fvisibility=hidden ${CXXFLAGS} ${CXXFLAGS_EXTRA}

C_SRCS = ${COMMON_DIR}/src/processor.c ${COMMON_DIR}/src/walloc.c ${COMMON_DIR}/src/memset.c
C_OBJS = $(addprefix build/obj/, $(notdir $(C_SRCS:.c=.o)))

ifeq ($(CXX_SRCS_EXTRA),)
CXX_SRCS =
CXX_OBJS =
else
CXX_SRCS = ${COMMON_DIR}/src/new.cpp ${CXX_SRCS_EXTRA}
CXX_OBJS = $(addprefix build/obj/, $(notdir $(CXX_SRCS:.cpp=.o)))
endif

ALL = build/web/${BUNDLE_NAME}.wasm build/web/${BUNDLE_NAME}_processor.js build/web/${BUNDLE_NAME}.js

default: all

-include ${COMMON_DIR}/demo.mk

all: ${ALL}

ifeq ($(CXX_OBJS),)
build/web/${BUNDLE_NAME}.wasm: ${C_OBJS} | build/web
	${CC} $^ -o $@ ${CFLAGS_ALL} ${LDFLAGS_ALL}
else
build/web/${BUNDLE_NAME}.wasm: ${C_OBJS} ${CXX_OBJS} | build/web
	${CXX} $^ -o $@ ${CFLAGS_ALL} ${CXXFLAGS_ALL} ${LDFLAGS_ALL}
endif

build/web/${BUNDLE_NAME}_processor.js: ${DATA_DIR}/src/processor.js | build/web
	cp $^ $@

build/web/${BUNDLE_NAME}.js: ${DATA_DIR}/src/module.js | build/web
	cp $^ $@

build/web build/obj:
	mkdir -p $@

clean:
	rm -fr build

.PHONY: all clean

.SECONDEXPANSION:

PERCENT := %

$(C_OBJS): build/obj/%.o: $$(filter $$(PERCENT)/$$(basename $$(notdir $$@)).c,$$(C_SRCS)) | build/obj
	${CC} $^ -o $@ -c ${CFLAGS_ALL}

$(CXX_OBJS): build/obj/%.o: $$(filter $$(PERCENT)/$$(basename $$(notdir $$@)).cpp,$$(CXX_SRCS)) | build/obj
	${CXX} $^ -o $@ -c ${CXXFLAGS_ALL}
