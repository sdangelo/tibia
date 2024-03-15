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

C_SRCS_EXTRA := {{=it.make && it.make.cSrcs ? it.make.cSrcs : ""}} {{=it.ios_make && it.ios_make.cSrcs ? it.ios_make.cSrcs : ""}}
CXX_SRCS_EXTRA := {{=it.make && it.make.cxxSrcs ? it.make.cxxSrcs : ""}} {{=it.ios_make && it.ios_make.cxxSrcs ? it.ios_make.cxxSrcs : ""}}
SRCS_EXTRA := {{=it.ios_make && it.ios_make.srcsExtra ? it.ios_make.srcsExtra : ""}}

COMMON_DIR := {{=it.ios_make && it.ios_make.commonDir ? it.ios_make.commonDir : (it.make && it.make.commonDir ? it.make.commonDir : "")}}
DATA_DIR := {{=it.ios_make && it.ios_make.dataDir ? it.ios_make.dataDir : (it.make && it.make.dataDir ? it.make.dataDir : "")}}
PLUGIN_DIR := {{=it.ios_make && it.ios_make.pluginDir ? it.ios_make.pluginDir : (it.make && it.make.pluginDir ? it.make.pluginDir : "")}}
