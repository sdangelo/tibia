BUNDLE_NAME := {{=it.product.bundleName}}

C_SRCS_EXTRA := {{=it.make && it.make.cSrcs ? it.make.cSrcs : ""}} {{=it.ios_make && it.ios_make.cSrcs ? it.ios_make.cSrcs : ""}}
CXX_SRCS_EXTRA := {{=it.make && it.make.cxxSrcs ? it.make.cxxSrcs : ""}} {{=it.ios_make && it.ios_make.cxxSrcs ? it.ios_make.cxxSrcs : ""}}

COMMON_DIR := {{=it.ios_make && it.ios_make.commonDir ? it.ios_make.commonDir : (it.make && it.make.commonDir ? it.make.commonDir : "")}}
DATA_DIR := {{=it.ios_make && it.ios_make.dataDir ? it.ios_make.dataDir : (it.make && it.make.dataDir ? it.make.dataDir : "")}}
PLUGIN_DIR := {{=it.ios_make && it.ios_make.pluginDir ? it.ios_make.pluginDir : (it.make && it.make.pluginDir ? it.make.pluginDir : "")}}
