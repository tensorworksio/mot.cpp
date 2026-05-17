# Stage 1: Builder
FROM ubuntu:24.04 AS builder

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    build-essential \
    g++-14 \
    gcc-14 \
    cmake \
    git \
    ninja-build \
    pkg-config \
    libopencv-dev \
    libpng-dev \
    zlib1g-dev \
    libjpeg-dev \
    libgtest-dev \
    python3.12 \
    python3-pip \
    && rm -rf /var/lib/apt/lists/*

# meson from pip to ensure a version that supports C++23
RUN pip3 install --no-cache-dir --break-system-packages meson

# GCC 14 is needed for C++23 <print> support
RUN update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-14 100 \
    && update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-14 100 \
    && update-alternatives --install /usr/bin/cc  cc  /usr/bin/gcc-14 100 \
    && update-alternatives --install /usr/bin/c++ c++ /usr/bin/g++-14 100

# Build the project
WORKDIR /opt/mot.cpp
COPY . .

RUN meson subprojects update --reset \
    && meson setup build --wipe -Dbuildtype=release \
    && meson compile -C build

# Stage 2: Runtime
FROM ubuntu:24.04

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y \
    libopencv-dev \
    ffmpeg \
    python3.12 \
    python3-venv \
    && rm -rf /var/lib/apt/lists/*

COPY requirements.txt /tmp/
RUN python3 -m venv /opt/mot.cpp/venv \
    && /opt/mot.cpp/venv/bin/pip install --no-cache-dir -r /tmp/requirements.txt

COPY --from=builder /opt/mot.cpp/build/libmot.so /usr/local/lib/
COPY --from=builder /opt/mot.cpp/build/subprojects/tomlplusplus-3.4.0/src/libtomlplusplus.so.3.4.0 /usr/local/lib/
COPY --from=builder /opt/mot.cpp/build/app/mot   /usr/local/bin/mot
COPY --from=builder /opt/mot.cpp/app/config      /opt/mot.cpp/app/config
COPY --from=builder /opt/mot.cpp/mot-eval.sh     /opt/mot.cpp/mot-eval.sh

RUN ldconfig

WORKDIR /opt/mot.cpp

ENTRYPOINT ["mot"]
