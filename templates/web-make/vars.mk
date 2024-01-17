BUNDLE_NAME := {{=it.product.bundleName}}
CFLAGS_EXTRA := {{=it.make && it.make.cflags ? it.make.cflags : ""}} {{=it.web_make && it.web_make.cflags ? it.web_make.cflags : ""}}
LIBS_EXTRA := {{=it.make && it.make.libs ? it.make.libs : ""}} {{=it.web_make && it.web_make.libs ? it.web_make.libs : ""}}
