#
# Tibia
#
# Copyright (C) 2023, 2024 Orastron Srl unipersonale
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
	DLL_SUFFIX := .vst3
	PLATFORM := x86_64-win
	VST3DIR := $(shell echo '$(COMMONPROGRAMFILES)' | sed 's:\\:/:g')/VST3
	VST3DIR_USER := $(shell echo '$(LOCALAPPDATA)' | sed 's:\\:/:g')/Programs/Common/VST3
	CC := gcc
	CXX := g++
else
	UNAME_S := $(shell uname -s)
	ifeq ($(UNAME_S), Darwin)
		DLL_SUFFIX :=
		PLATFORM := MacOS
		VST3DIR := /Library/Audio/Plug-Ins/VST3
		VST3DIR_USER := $(HOME)/Library/Audio/Plug-Ins/VST3
		CC := clang
		CXX := clang++
	else
		DLL_SUFFIX := .so
		PLATFORM := $(shell uname -m)-linux
		VST3DIR := /usr/local/lib/vst3
		VST3DIR_USER := $(HOME)/.vst3
		CC := gcc
		CXX := g++
	endif
endif

COMMON_DIR := $(or $(COMMON_DIR),.)
DATA_DIR := $(or $(DATA_DIR),.)
PLUGIN_DIR := $(or $(PLUGIN_DIR),src)

CFLAGS := -O3 -Wall -Wpedantic -Wextra
CFLAGS_ALL := -I$(DATA_DIR)/src -I$(PLUGIN_DIR) -fPIC $(CFLAGS) $(CFLAGS_EXTRA)

LDFLAGS :=
LDFLAGS_ALL := -shared -lm $(LDFLAGS) $(LDFLAGS_EXTRA)

CXXFLAGS := $(CFLAGS)
CXXFLAGS_ALL := -I$(DATA_DIR)/src -I$(PLUGIN_DIR) -fPIC $(CXXFLAGS) $(CXXFLAGS_EXTRA)

ifeq ($(UNAME_S), Darwin)
CFLAGS_ALL := $(CFLAGS_ALL) -arch arm64 -arch x86_64
LDFLAGS_ALL := $(LDFLAGS_ALL) -arch arm64 -arch x86_64
CXXFLAGS_ALL := $(CXXFLAGS_ALL) -arch arm64 -arch x86_64
 ifeq ($(HAS_UI), yes)
LDFLAGS_ALL := $(LDFLAGS_ALL) -Wl,-framework,Foundation -Wl,-framework,Cocoa -Wl,-framework,Corevideo
 endif
endif

ifeq ($(UNAME_S), Linux)
CFLAGS_ALL := $(CFLAGS_ALL) -D_GNU_SOURCE
LDFLAGS_ALL := $(LDFLAGS_ALL) -ldl
 ifeq ($(HAS_UI), yes)
CFLAGS_ALL := $(CFLAGS_ALL) $(shell pkg-config --cflags x11)
LDFLAGS_ALL := $(LDFLAGS_ALL) $(shell pkg-config --libs x11)
CXXFLAGS_ALL := $(CXXFLAGS_ALL) $(shell pkg-config --cflags x11)
 endif
endif

BUNDLE_DIR := $(BUNDLE_NAME).vst3

DLL_DIR := Contents/$(PLATFORM)
DLL_FILE := $(DLL_DIR)/$(BUNDLE_NAME)$(DLL_SUFFIX)

C_SRCS := $(COMMON_DIR)/src/vst3.c $(C_SRCS_EXTRA)
C_OBJS := $(addprefix build/obj/, $(notdir $(C_SRCS:.c=.o)))

CXX_SRCS := $(CXX_SRCS_EXTRA)
CXX_OBJS := $(addprefix build/obj/, $(notdir $(CXX_SRCS:.cpp=.o)))

ALL := build/$(BUNDLE_DIR)/$(DLL_FILE)

ifeq ($(UNAME_S), Darwin)
ALL := $(ALL) build/$(BUNDLE_DIR)/Contents/Info.plist build/$(BUNDLE_DIR)/Contents/PkgInfo
endif

-include $(COMMON_DIR)/vars-extra.mk

all: $(ALL)

ifeq ($(CXX_OBJS),)
build/$(BUNDLE_DIR)/$(DLL_FILE): $(C_OBJS) | build/$(BUNDLE_DIR)/$(DLL_DIR)
	$(CC) $^ -o $@ $(CFLAGS_ALL) $(LDFLAGS_ALL)
else
build/$(BUNDLE_DIR)/$(DLL_FILE): $(C_OBJS) $(CXX_OBJS) | build/$(BUNDLE_DIR)/$(DLL_DIR)
	$(CXX) $^ -o $@ $(CFLAGS_ALL) $(CXXFLAGS_ALL) $(LDFLAGS_ALL)
endif

ifeq ($(UNAME_S), Darwin)
build/$(BUNDLE_DIR)/Contents/%: $(DATA_DIR)/data/% | build/$(BUNDLE_DIR)/Contents
	cp $^ $@
endif

build/$(BUNDLE_DIR)/Contents build/$(BUNDLE_DIR)/$(DLL_DIR) build/obj:
	mkdir -p $@

clean:
	rm -fr build

install: all
	@for d in `find build/$(BUNDLE_DIR) -type d`; do \
		d=`echo $$d | sed 's:^build/::'` ; \
		echo mkdir -m 0755 -p "$(VST3DIR)/$$d"; \
		mkdir -m 0755 -p "$(VST3DIR)/$$d"; \
	done
	@for f in `find build/$(BUNDLE_DIR) -type f`; do \
		m=`[ -x $$f ] && echo 0755 || echo 0644`; \
		d=`echo $$f | sed 's:^build/::'` ; \
		d=`dirname $$d`; \
		echo install -m $$m $$f "$(VST3DIR)/$$d"; \
		install -m $$m $$f "$(VST3DIR)/$$d"; \
	done

install-user: all
	@for d in `find build/$(BUNDLE_DIR) -type d`; do \
		d=`echo $$d | sed 's:^build/::'` ; \
		echo mkdir -m 0755 -p "$(VST3DIR_USER)/$$d"; \
		mkdir -m 0755 -p "$(VST3DIR_USER)/$$d"; \
	done
	@for f in `find build/$(BUNDLE_DIR) -type f`; do \
		m=`[ -x $$f ] && echo 0755 || echo 0644`; \
		d=`echo $$f | sed 's:^build/::'` ; \
		d=`dirname $$d`; \
		echo install -m $$m $$f "$(VST3DIR_USER)/$$d"; \
		install -m $$m $$f "$(VST3DIR_USER)/$$d"; \
	done

-include $(COMMON_DIR)/rules-extra.mk

.PHONY: all clean install install-user

.SECONDEXPANSION:

PERCENT := %

$(C_OBJS): build/obj/%.o: $$(filter $$(PERCENT)/$$(basename $$(notdir $$@)).c,$$(C_SRCS)) | build/obj
	$(CC) $^ -o $@ -c $(CFLAGS_ALL)

$(CXX_OBJS): build/obj/%.o: $$(filter $$(PERCENT)/$$(basename $$(notdir $$@)).cpp,$$(CXX_SRCS)) | build/obj
	$(CXX) $^ -o $@ -c $(CXXFLAGS_ALL)

-include $(COMMON_DIR)/rules-secondexp-extra.mk
