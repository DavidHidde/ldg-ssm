FROM gcc:12.3

RUN apt-get update && apt install -y \
    ssh \
    valgrind \
    build-essential \
    libcairo2-dev \
    libomp-dev \
    libc++-dev \
    libc++abi-dev \
    libeigen3-dev \
    cimg-dev \
    libx11-dev \
    bzip2 \
    cmake \
    gdb \
    clang \
    git

WORKDIR /usr/app