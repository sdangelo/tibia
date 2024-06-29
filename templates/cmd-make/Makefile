#
# Tibia
#
# Copyright (C) 2024 Orastron Srl unipersonale
#
# Tibia is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, version 3 of the License.
#
# Tibia is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Tibia.  If not, see <http://www.gnu.org/licenses/>.
#
# File author: Stefano D'Angelo
#

include vars.mk

ifeq ($(OS), Windows_NT)
	EXE_SUFFIX := .exe
else
	UNAME_S := $(shell uname -s)
	EXE_SUFFIX :=
	PREFIX := /usr/local
	BINDIR := $(PREFIX)/bin
endif

COMMON_DIR := $(or $(COMMON_DIR),.)
DATA_DIR := $(or $(DATA_DIR),.)
PLUGIN_DIR := $(or $(PLUGIN_DIR),src)

CC := gcc
CXX := g++

CFLAGS := -O3 -Wall -Wpedantic -Wextra
CFLAGS_ALL := -I$(DATA_DIR)/src -I$(PLUGIN_DIR) -I$(TINYWAV_DIR) -I$(MIDI_PARSER_DIR)/include -fPIC $(CFLAGS) $(CFLAGS_EXTRA)

LDFLAGS :=
LDFLAGS_ALL := $(LDFLAGS) $(LDFLAGS_EXTRA)

CXXFLAGS := $(CFLAGS)
CXXFLAGS_ALL := -I$(DATA_DIR)/src -I$(PLUGIN_DIR) -I$(TINYWAV_DIR) -I$(MIDI_PARSER_DIR)/include -fPIC $(CXXFLAGS) $(CXXFLAGS_EXTRA)

ifeq ($(UNAME_S), Darwin)
CFLAGS_ALL := $(CFLAGS_ALL) -arch arm64 -arch x86_64
LDFLAGS_ALL := $(LDFLAGS_ALL) -arch arm64 -arch x86_64
CXXFLAGS_ALL := $(CXXFLAGS_ALL) -arch arm64 -arch x86_64
endif

PROGRAM := $(BUNDLE_NAME)$(EXE_SUFFIX)

C_SRCS := $(COMMON_DIR)/src/main.c $(C_SRCS_EXTRA)
ifeq ($(HAS_MIDI_IN), yes)
	C_SRCS := $(C_SRCS) $(MIDI_PARSER_DIR)/src/midi-parser.c
endif
C_OBJS := $(addprefix build/obj/, $(notdir $(C_SRCS:.c=.o)))

CXX_SRCS := $(CXX_SRCS_EXTRA)
CXX_OBJS := $(addprefix build/obj/, $(notdir $(CXX_SRCS:.cpp=.o)))

ALL := build/$(PROGRAM)

-include $(COMMON_DIR)/vars-extra.mk

all: $(ALL)

ifeq ($(CXX_OBJS),)
build/$(PROGRAM): $(C_OBJS) build/obj/tinywav.o | build
	$(CC) $^ -o $@ $(CFLAGS_ALL) $(LDFLAGS_ALL)
else
build/$(PROGRAM): $(C_OBJS) $(CXX_OBJS) build/obj/tinywav.o | build
	$(CXX) $^ -o $@ $(CFLAGS_ALL) $(CXXFLAGS_ALL) $(LDFLAGS_ALL)
endif

build build/obj:
	mkdir -p $@

clean:
	rm -fr build

ifeq ($(OS), Windows_NT)
.PHONY: all clean
else
install: all
	mkdir -m 0755 -p $(BINDIR)
	install -m 0755 build/$(PROGRAM) $(BINDIR)

.PHONY: all clean install
endif

-include $(COMMON_DIR)/rules-extra.mk

.SECONDEXPANSION:

PERCENT := %

build/obj/tinywav.o: $(TINYWAV_DIR)/tinywav.c
	$(CC) $^ -o $@ -c $(CFLAGS_ALL) -Wno-unused-result

$(C_OBJS): build/obj/%.o: $$(filter $$(PERCENT)/$$(basename $$(notdir $$@)).c,$$(C_SRCS)) | build/obj
	$(CC) $^ -o $@ -c $(CFLAGS_ALL)

$(CXX_OBJS): build/obj/%.o: $$(filter $$(PERCENT)/$$(basename $$(notdir $$@)).cpp,$$(CXX_SRCS)) | build/obj
	$(CXX) $^ -o $@ -c $(CXXFLAGS_ALL)

-include $(COMMON_DIR)/rules-secondexp-extra.mk
