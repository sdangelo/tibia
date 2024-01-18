ALL += build/index.html

build/index.html: src/index.html | build
	cp $^ $@
