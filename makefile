app_dir_local := "$(PWD)/app"
data_dir_local := "$(PWD)/data"
scripts_dir_local := "$(PWD)/scripts"

app_dir_container := /usr/app
data_dir_container := /usr/data
scripts_dir_container := /usr/scripts
build_dir_container := $(app_dir_container)/make-build-default

container_name := ldg-ssm-compiler:1.0
app_dir_bind := -v $(app_dir_local):$(app_dir_container)
data_dir_bind := -v $(data_dir_local):$(data_dir_container)
scripts_dir_bind := -v $(scripts_dir_local):$(scripts_dir_container)

build-all: build-image build-cmake compile

build-image:
	docker build --tag $(container_name) .

build-cmake:
	docker run -it --rm $(app_dir_bind) $(container_name) cmake -S $(app_dir_container) -B $(build_dir_container)

compile:
	docker run -it --rm $(app_dir_bind) $(container_name) cmake --build $(build_dir_container) -j10

bash:
	docker run -it --rm $(app_dir_bind) $(data_dir_bind) $(scripts_dir_bind) $(container_name) bash
