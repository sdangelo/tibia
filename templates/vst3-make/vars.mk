BUNDLE_NAME := {{=it.product.bundleName}}
CFLAGS_EXTRA := {{=it.make && it.make.cflags ? it.make.cflags : ""}} {{=it.vst3_make && it.vst3_make.cflags ? it.vst3_make.cflags : ""}}
LDFLAGS_EXTRA := {{=it.make && it.make.ldflags ? it.make.ldflags : ""}} {{=it.vst3_make && it.vst3_make.ldflags ? it.vst3_make.ldflags : ""}}
COMMON_DIR := {{=it.vst3_make && it.vst3_make.commonDir ? it.vst3_make.commonDir : (it.make && it.make.commonDir ? it.make.commonDir : "")}}
DATA_DIR := {{=it.vst3_make && it.vst3_make.dataDir ? it.vst3_make.dataDir : (it.make && it.make.dataDir ? it.make.dataDir : "")}}
PLUGIN_DIR := {{=it.vst3_make && it.vst3_make.pluginDir ? it.vst3_make.pluginDir : (it.make && it.make.pluginDir ? it.make.pluginDir : "")}}
