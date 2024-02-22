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

BUNDLE_NAME := {{=it.product.bundleName}}

CFLAGS_EXTRA := {{=it.make && it.make.cflags ? it.make.cflags : ""}} {{=it.lv2_make && it.lv2_make.cflags ? it.lv2_make.cflags : ""}}
LDFLAGS_EXTRA := {{=it.make && it.make.ldflags ? it.make.ldflags : ""}} {{=it.lv2_make && it.lv2_make.ldflags ? it.lv2_make.ldflags : ""}}
CXXFLAGS_EXTRA := {{=it.make && it.make.cxxflags ? it.make.cxxflags : ""}} {{=it.lv2_make && it.lv2_make.cxxflags ? it.lv2_make.cxxflags : ""}}

C_SRCS_EXTRA := {{=it.make && it.make.cSrcs ? it.make.cSrcs : ""}} {{=it.lv2_make && it.lv2_make.cSrcs ? it.lv2_make.cSrcs : ""}}
CXX_SRCS_EXTRA := {{=it.make && it.make.cxxSrcs ? it.make.cxxSrcs : ""}} {{=it.lv2_make && it.lv2_make.cxxSrcs ? it.lv2_make.cxxSrcs : ""}}

COMMON_DIR := {{=it.lv2_make && it.lv2_make.commonDir ? it.lv2_make.commonDir : (it.make && it.make.commonDir ? it.make.commonDir : "")}}
DATA_DIR := {{=it.lv2_make && it.lv2_make.dataDir ? it.lv2_make.dataDir : (it.make && it.make.dataDir ? it.make.dataDir : "")}}
PLUGIN_DIR := {{=it.lv2_make && it.lv2_make.pluginDir ? it.lv2_make.pluginDir : (it.make && it.make.pluginDir ? it.make.pluginDir : "")}}
