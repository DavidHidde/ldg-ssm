app_dir_local := "$(PWD)/app"
data_dir_local := "$(PWD)/data"
scripts_dir_local := "$(PWD)/scripts"

app_dir_container := /usr/app
data_dir_container := /usr/data
scripts_dir_container := /usr/scripts

build_dir_volume := ldg-build
build_dir_container := $(app_dir_container)/make-build-default

container_name := ldg-compiler:1.0
app_dir_bind := -v $(app_dir_local):$(app_dir_container)
data_dir_bind := -v $(data_dir_local):$(data_dir_container)
scripts_dir_bind := -v $(scripts_dir_local):$(scripts_dir_container)
build_dir_bind := -v $(build_dir_volume):$(build_dir_container)

build-all: build-container build-cmake compile

build-container:
	docker build --tag $(container_name) .

build-cmake:
	docker run --rm $(app_dir_bind) $(container_name) cmake -S $(app_dir_container) -B $(build_dir_container)

build: build-cmake

compile:
	docker run --rm $(app_dir_bind) $(container_name) cmake --build $(build_dir_container) -j10

run:
	docker run -it --rm $(app_dir_bind) $(data_dir_bind) $(container_name) sh -c "cd $(build_dir_container) && ./new_ldg"

run-visualize: compile run
	docker run -it --rm \
		$(app_dir_bind) \
		$(data_dir_bind) \
		$(scripts_dir_bind) \
		$(container_name) bash /usr/scripts/run_visualize_ssm.sh $(data_dir_container)/input $(build_dir_container) $(data_dir_container)/output $(build_dir_container)/ldg_core

bash:
	docker run -it --rm $(app_dir_bind) $(data_dir_bind) $(scripts_dir_bind) $(container_name) bash

test_script:
	docker run -it --rm \
		$(app_dir_bind) \
		$(data_dir_bind) \
		$(scripts_dir_bind) \
		$(container_name) bash $(SCRIPT) $(data_dir_container)/input $(data_dir_container)/output $(build_dir_container)/ldg_core
