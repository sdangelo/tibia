BUNDLE_NAME := {{=it.product.bundleName}}
CFLAGS_EXTRA := {{=it.make && it.make.cflags ? it.make.cflags : ""}} {{=it.vst3_make && it.vst3_make.cflags ? it.vst3_make.cflags : ""}}
LIBS_EXTRA := {{=it.make && it.make.libs ? it.make.libs : ""}} {{=it.vst3_make && it.vst3_make.libs ? it.vst3_make.libs : ""}}
