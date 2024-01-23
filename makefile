build-all: build-container build-cmake compile

build-container:
	docker volume create ldg-build
	docker build --tag ldg-compiler:1.0 .

build-cmake:
	docker run --rm -v "$(PWD)/src":/usr/src/ -v ldg-build:/usr/build ldg-compiler:1.0 cmake \
 		-S /usr/src/

build: build-cmake

compile:
	docker run --rm -v "$(PWD)/src":/usr/src/ -v ldg-build:/usr/build ldg-compiler:1.0 cmake \
		--build . \
		-j4

run:
	docker run --rm -v "$(PWD)/src":/usr/src/ -v ldg-build:/usr/build ldg-compiler:1.0 /usr/build/ldg_core

bash:
	docker run -it --rm -v "$(PWD)/src":/usr/src/ -v ldg-build:/usr/build ldg-compiler:1.0 bash