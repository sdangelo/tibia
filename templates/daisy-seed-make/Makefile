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

TARGET := $(BUNDLE_NAME)

COMMON_DIR := $(or $(COMMON_DIR),.)
DATA_DIR := $(or $(DATA_DIR),.)
PLUGIN_DIR := $(or $(PLUGIN_DIR),src)

CPP_SOURCES := $(COMMON_DIR)/src/main.cpp $(CXX_SRCS_EXTRA)

SYSTEM_FILES_DIR := $(LIBDAISY_DIR)/core

include $(SYSTEM_FILES_DIR)/Makefile

C_SOURCES += $(C_SRCS_EXTRA)

CFLAGS += -I$(DATA_DIR)/src -I$(PLUGIN_DIR) $(CFLAGS_EXTRA)
LDFLAGS += $(LDFLAGS_EXTRA)
CXXFLAGS += -I$(DATA_DIR)/src -I$(PLUGIN_DIR) $(CXXFLAGS_EXTRA)

-include $(COMMON_DIR)/extra.mk
