#! /bin/bash
set -ex

fmt_version="8.0.1"
spdlog_version="1.9.2"
boost_version="1.79.0"

install_path=/usr/local

POSITIONAL=()
while [[ $# -gt 0 ]]; do
  key="$1"

  case $key in
    --install-path)
      install_path="$2"
      shift # past argument
      shift # past value
      ;;
    *)    # unknown option
      POSITIONAL+=("$1") # save it in an array for later
      shift # past argument
      ;;
  esac
done

if [ ${install_path} ]; then
    if [ ! -d "${install_path}" ]; then
        mkdir -p ${install_path}
    fi
    install_path="$(cd ${install_path}; pwd)"
fi

if [[ "$OSTYPE" == "linux-gnu" ]]; then
    CPUS=$(nproc)
elif [[ "$OSTYPE" == "darwin"* ]]; then
    CPUS=$(sysctl -n hw.logicalcpu)
fi

function setup_boost {
    root=$(pwd)
    temp_folder=$(mktemp -d)
    cd ${temp_folder}
    boost_version_string=${boost_version//[.]/_}
    wget https://boostorg.jfrog.io/artifactory/main/release/${boost_version}/source/boost_${boost_version_string}.tar.gz
    tar xf boost_${boost_version_string}.tar.gz
    cd boost_${boost_version_string}
    ./bootstrap.sh --prefix=${install_path} --with-libraries="headers"
    ./b2 --prefix=${install_path} install

    cd ${root}
    rm -rf ${temp_folder}
}

function setup_fmt {
    root=$(pwd)
    temp_folder=$(mktemp -d)
    cd ${temp_folder}

    wget https://github.com/fmtlib/fmt/archive/${fmt_version}.tar.gz
    tar xf ${fmt_version}.tar.gz
    cd fmt-${fmt_version}
    mkdir build
    cd build
    cmake .. \
        -DCMAKE_CXX_FLAGS="-fPIC -fvisibility=hidden" \
        -DFMT_DOC=OFF \
        -DFMT_TEST=OFF \
        -DCMAKE_INSTALL_PREFIX=${install_path} \
        -DCMAKE_BUILD_TYPE=Release
    cmake --build . -j ${CPUS}
    cmake --install .
    cd ${root}
    rm -rf ${temp_folder}
}

function setup_spdlog {
    root=$(pwd)
    temp_folder=$(mktemp -d)
    cd ${temp_folder}

    wget https://github.com/gabime/spdlog/archive/v${spdlog_version}.tar.gz
    tar xf v${spdlog_version}.tar.gz
    cd spdlog-${spdlog_version}
    mkdir build
    cd build
    cmake .. \
        -DCMAKE_CXX_FLAGS="-fPIC -fvisibility=hidden" \
        -DSPDLOG_BUILD_BENCH=OFF \
        -DSPDLOG_BUILD_TESTS=OFF \
        -DSPDLOG_FMT_EXTERNAL=ON \
        -DCMAKE_PREFIX_PATH=${install_path} \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_INSTALL_PREFIX=${install_path}
    cmake --build . --target install -- -j${CPUS}

    cd ${root}
    rm -rf ${temp_folder}
}

setup_boost
setup_fmt
setup_spdlog
