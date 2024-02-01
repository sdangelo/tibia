BUNDLE_NAME := {{=it.product.bundleName}}
CFLAGS_EXTRA := {{=it.make && it.make.cflags ? it.make.cflags : ""}} {{=it.web_make && it.web_make.cflags ? it.web_make.cflags : ""}}
LDFLAGS_EXTRA := {{=it.make && it.make.ldflags ? it.make.ldflags : ""}} {{=it.web_make && it.web_make.ldflags ? it.web_make.ldflags : ""}}
COMMON_DIR := {{=it.web_make && it.web_make.commonDir ? it.web_make.commonDir : (it.make && it.make.commonDir ? it.make.commonDir : "")}}
DATA_DIR := {{=it.web_make && it.web_make.dataDir ? it.web_make.dataDir : (it.make && it.make.dataDir ? it.make.dataDir : "")}}
PLUGIN_DIR := {{=it.web_make && it.web_make.pluginDir ? it.web_make.pluginDir : (it.make && it.make.pluginDir ? it.make.pluginDir : "")}}
HAS_MIDI_IN := {{=it.product.buses.filter(x => x.type == "midi" && x.direction == "input").length > 0 ? "yes" : "no"}}
