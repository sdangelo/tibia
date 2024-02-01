BUNDLE_NAME := {{=it.product.bundleName}}
CFLAGS_EXTRA := {{=it.make && it.make.cflags ? it.make.cflags : ""}} {{=it.cmd_make.cflags ? it.cmd_make.cflags : ""}}
LDFLAGS_EXTRA := {{=it.make && it.make.ldflags ? it.make.ldflags : ""}} {{=it.cmd_make.ldflags ? it.cmd_make.ldflags : ""}}
MAIN_DIR := {{=it.cmd_make && it.cmd_make.mainDir ? it.cmd_make.mainDir : (it.make && it.make.mainDir ? it.make.mainDir : "")}}
DATA_DIR := {{=it.cmd_make && it.cmd_make.dataDir ? it.cmd_make.dataDir : (it.make && it.make.dataDir ? it.make.dataDir : "")}}
PLUGIN_DIR := {{=it.cmd_make && it.cmd_make.pluginDir ? it.cmd_make.pluginDir : (it.make && it.make.pluginDir ? it.make.pluginDir : "")}}
TINYWAV_DIR := {{=it.cmd_make.tinywavDir}}
MIDI_PARSER_DIR := {{=it.cmd_make.midiParserDir}}
