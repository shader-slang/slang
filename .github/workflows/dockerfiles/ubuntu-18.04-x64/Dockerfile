FROM ubuntu:18.04
RUN apt update
RUN apt install -y software-properties-common wget git build-essential zip libx11-dev zlib1g-dev
RUN add-apt-repository ppa:ubuntu-toolchain-r/test -y
RUN apt update
RUN apt install -y gcc-9 g++-9
RUN update-alternatives \
    --install /usr/bin/gcc gcc /usr/bin/gcc-9 100 \
    --slave /usr/bin/g++ g++ /usr/bin/g++-9 \
    --slave /usr/bin/gcc-ar gcc-ar /usr/bin/gcc-ar-9 \
    --slave /usr/bin/gcc-nm gcc-nm /usr/bin/gcc-nm-9 \
    --slave /usr/bin/gcc-ranlib gcc-ranlib /usr/bin/gcc-ranlib-9 \
    --slave /usr/bin/gcov gcov /usr/bin/gcov-9 \
    --slave /usr/bin/gcov-dump gcov-dump /usr/bin/gcov-dump-9 \
    --slave /usr/bin/gcov-tool gcov-tool /usr/bin/gcov-tool-9
