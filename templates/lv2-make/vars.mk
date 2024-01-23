BUNDLE_NAME := {{=it.product.bundleName}}
CFLAGS_EXTRA := {{=it.make && it.make.cflags ? it.make.cflags : ""}} {{=it.lv2_make && it.lv2_make.cflags ? it.lv2_make.cflags : ""}}
LDFLAGS_EXTRA := {{=it.make && it.make.ldflags ? it.make.ldflags : ""}} {{=it.lv2_make && it.lv2_make.ldflags ? it.lv2_make.ldflags : ""}}
