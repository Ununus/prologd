FROM gcc:10.3.0-buster

# Install build system generator
ARG CMAKE_VERSION=3.22.2
RUN wget https://github.com/Kitware/CMake/releases/download/v${CMAKE_VERSION}/cmake-${CMAKE_VERSION}-Linux-x86_64.sh \
      -q -O /tmp/cmake-install.sh \
      && chmod u+x /tmp/cmake-install.sh \
      && mkdir /usr/bin/cmake \
      && /tmp/cmake-install.sh --skip-license --prefix=/usr/bin/cmake \
      && rm /tmp/cmake-install.sh

ENV PATH="/usr/bin/cmake/bin:${PATH}"

# Build prologd
COPY ./src /src
WORKDIR /src
RUN cmake -BUILD_GUI=OFF . && make && mkdir -m 777 /compiled
CMD mv ./prologd /compiled/ && chmod 777 /compiled/prologd