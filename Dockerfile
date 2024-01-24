#FROM ubuntu:22.04
FROM gcc:12.3

RUN apt-get update
RUN apt install -y \
    build-essential \
    libcairo2-dev \
    libomp-dev \
    libc++-dev \
    libc++abi-dev \
    cimg-dev \
    bzip2 \
    cmake \
    git

WORKDIR /usr/build