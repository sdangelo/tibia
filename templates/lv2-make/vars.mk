BUNDLE_NAME := {{=it.product.bundleName}}
CFLAGS_EXTRA := {{=it.make && it.make.cflags ? it.make.cflags : ""}} {{=it.lv2_make && it.lv2_make.cflags ? it.lv2_make.cflags : ""}}
LIBS_EXTRA := {{=it.make && it.make.libs ? it.make.libs : ""}} {{=it.lv2_make && it.lv2_make.libs ? it.lv2_make.libs : ""}}
