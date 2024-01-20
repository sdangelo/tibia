ALL += build/index.html build/cert.pem build/key.pem

build/index.html: src/index.html | build
	cp $^ $@

build/key.pem: build/cert.pem

build/cert.pem: | build
	yes "" | openssl req -x509 -newkey rsa:2048 -keyout build/key.pem -out build/cert.pem -days 365 -nodes 2>/dev/null
