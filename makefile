src_dir_local := "$(PWD)/src"
data_dir_local := "$(PWD)/data"
scripts_dir_local := "$(PWD)/scripts"

src_dir_container := /usr/src
data_dir_container := /usr/data
scripts_dir_container := /usr/scripts

build_dir_volume := ldg-build
build_dir_container := /usr/build

container_name := ldg-compiler:1.0
src_dir_bind := -v $(src_dir_local):$(src_dir_container)
data_dir_bind := -v $(data_dir_local):$(data_dir_container)
scripts_dir_bind := -v $(scripts_dir_local):$(scripts_dir_container)
build_dir_bind := -v $(build_dir_volume):$(build_dir_container)

build-all: build-container build-cmake compile

build-container:
	docker volume create $(build_dir_volume)
	docker build --tag $(container_name) .

build-cmake:
	docker run --rm $(src_dir_bind) $(build_dir_bind) $(container_name) cmake -S $(src_dir_container)

build: build-cmake

compile:
	docker run --rm $(src_dir_bind) $(build_dir_bind) $(container_name) cmake --build . -j4

run:
	docker run --rm $(src_dir_bind) $(build_dir_bind) $(container_name) $(build_dir_container)/ldg_core

bash:
	docker run -it --rm $(src_dir_bind) $(build_dir_bind) $(container_name) bash

test_script:
	docker run -it --rm \
		$(src_dir_bind) \
		$(data_dir_bind) \
		$(scripts_dir_bind) \
 		$(build_dir_bind) \
		$(container_name) bash $(SCRIPT) $(data_dir_container)/input $(data_dir_container)/output $(build_dir_container)/ldg_core
