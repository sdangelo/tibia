BUNDLE_NAME := {{=it.product.bundleName}}
CFLAGS_EXTRA := {{=it.make && it.make.cflags ? it.make.cflags : ""}} {{=it.cmd_make.cflags ? it.cmd_make.cflags : ""}}
LDFLAGS_EXTRA := {{=it.make && it.make.ldflags ? it.make.ldflags : ""}} {{=it.cmd_make.ldflags ? it.cmd_make.ldflags : ""}}
TINYWAV_DIR := {{=it.cmd_make.tinywavDir}}
MIDI_PARSER_DIR := {{=it.cmd_make.midiParserDir}}
