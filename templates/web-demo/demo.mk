ALL += build/web/index.html build/web/cert.pem build/web/key.pem

build/web/index.html: ${DATA_DIR}/src/index.html | build
	cp $^ $@

build/web/key.pem: build/web/cert.pem

build/web/cert.pem: | build
	yes "" | openssl req -x509 -newkey rsa:2048 -keyout build/web/key.pem -out build/web/cert.pem -days 365 -nodes 2>/dev/null
