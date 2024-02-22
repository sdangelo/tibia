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

CFLAGS_EXTRA := {{=it.make && it.make.cflags ? it.make.cflags : ""}} {{=it.web_make && it.web_make.cflags ? it.web_make.cflags : ""}}
LDFLAGS_EXTRA := {{=it.make && it.make.ldflags ? it.make.ldflags : ""}} {{=it.web_make && it.web_make.ldflags ? it.web_make.ldflags : ""}}
CXXFLAGS_EXTRA := {{=it.make && it.make.cxxflags ? it.make.cxxflags : ""}} {{=it.web_make && it.web_make.cxxflags ? it.web_make.cxxflags : ""}}

C_SRCS_EXTRA := {{=it.make && it.make.cSrcs ? it.make.cSrcs : ""}} {{=it.web_make && it.web_make.cSrcs ? it.web_make.cSrcs : ""}}
CXX_SRCS_EXTRA := {{=it.make && it.make.cxxSrcs ? it.make.cxxSrcs : ""}} {{=it.web_make && it.web_make.cxxSrcs ? it.web_make.cxxSrcs : ""}}

COMMON_DIR := {{=it.web_make && it.web_make.commonDir ? it.web_make.commonDir : (it.make && it.make.commonDir ? it.make.commonDir : "")}}
DATA_DIR := {{=it.web_make && it.web_make.dataDir ? it.web_make.dataDir : (it.make && it.make.dataDir ? it.make.dataDir : "")}}
PLUGIN_DIR := {{=it.web_make && it.web_make.pluginDir ? it.web_make.pluginDir : (it.make && it.make.pluginDir ? it.make.pluginDir : "")}}

HAS_MIDI_IN := {{=it.product.buses.filter(x => x.type == "midi" && x.direction == "input").length > 0 ? "yes" : "no"}}
