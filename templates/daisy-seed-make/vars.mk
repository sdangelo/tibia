BUNDLE_NAME := {{=it.product.bundleName}}
CFLAGS_EXTRA := {{=it.make && it.make.cflags ? it.make.cflags : ""}} {{=it.daisy_seed_make.cflags ? it.daisy_seed_make.cflags : ""}}
LDFLAGS_EXTRA := {{=it.make && it.make.ldflags ? it.make.ldflags : ""}} {{=it.daisy_seed_make.ldflags ? it.daisy_seed_make.ldflags : ""}}
LIBDAISY_DIR := {{=it.daisy_seed_make.libdaisyDir}}
