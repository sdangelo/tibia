BUNDLE_NAME := {{=it.product.bundleName}}
CFLAGS_EXTRA := {{=it.make && it.make.cflags ? it.make.cflags : ""}} {{=it.daisy_seed_make.cflags ? it.daisy_seed_make.cflags : ""}}
LDFLAGS_EXTRA := {{=it.make && it.make.ldflags ? it.make.ldflags : ""}} {{=it.daisy_seed_make.ldflags ? it.daisy_seed_make.ldflags : ""}}
COMMON_DIR := {{=it.daisy_seed_make && it.daisy_seed_make.commonDir ? it.daisy_seed_make.commonDir : (it.make && it.make.commonDir ? it.make.commonDir : "")}}
DATA_DIR := {{=it.daisy_seed_make && it.daisy_seed_make.dataDir ? it.daisy_seed_make.dataDir : (it.make && it.make.dataDir ? it.make.dataDir : "")}}
PLUGIN_DIR := {{=it.daisy_seed_make && it.daisy_seed_make.pluginDir ? it.daisy_seed_make.pluginDir : (it.make && it.make.pluginDir ? it.make.pluginDir : "")}}
LIBDAISY_DIR := {{=it.daisy_seed_make.libdaisyDir}}
