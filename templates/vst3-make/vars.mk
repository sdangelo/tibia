BUNDLE_NAME := {{=it.product.bundleName}}
CFLAGS_EXTRA := {{=it.make && it.make.cflags ? it.make.cflags : ""}} {{=it.vst3_make && it.vst3_make.cflags ? it.vst3_make.cflags : ""}}
LDFLAGS_EXTRA := {{=it.make && it.make.ldflags ? it.make.ldflags : ""}} {{=it.vst3_make && it.vst3_make.ldflags ? it.vst3_make.ldflags : ""}}
