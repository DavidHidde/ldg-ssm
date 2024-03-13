FROM gcc:12.3

RUN apt-get update && apt install -y \
    ssh \
    valgrind \
    build-essential \
    libomp-dev \
    libc++-dev \
    libc++abi-dev \
    libx11-dev \
    cmake \
    gdb \
    clang \
    git

WORKDIR /usr/app