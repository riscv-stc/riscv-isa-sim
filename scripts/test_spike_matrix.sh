#!/bin/bash
wget http://ops.streamcomputing.com/repo_list/ubuntu2004_sources.list -O /etc/apt/sources.list
mkdir -p ~/.pip && wget http://pypi.streamcomputing.com/repo_list/pip.conf -O ~/.pip/pip.conf
export LC_ALL=C.UTF-8
export LANG=C.UTF-8
export JAX_PLATFORM_NAME=cpu
export PIP_INDEX_URL=""
export PIP_TRUSTED_HOST=""
export PIP_INDEX_URL=https://nexus3.in/repository/pypi-aliyun/simple/
export PIP_TRUSTED_HOST=nexus3.in

apt update
apt-get install libffi-dev

BUILD_DIR=""
SPEC_LIST=""
EXTRA_OPTS=""

while getopts "s:e:b:" arg; do
    case $arg in
    s)
        SPEC_LIST="$OPTARG"
	    ;;
    e)
        EXTRA_OPTS="$OPTARG"
        ;;
    b)
        BUILD_DIR="$OPTARG"
        ;;
    ?)
        echo "Unkonw argument"
        ;;
    esac
done

if [ -z "$BUILD_DIR" ]; then
	echo "need -b <build_dir>"
	echo
	exit 1
fi

export PATH=$CI_PROJECT_DIR/$BUILD_DIR:$RISCV/bin:$PATH

# Set specs list
if [ -n "$SPEC_LIST" ]; then
    SPEC_LIST="--specs $SPEC_LIST"
fi

wget http://172.16.31.70/ci/spike_matrix_ci/riscv-gnu-toolchain.tar.gz
if [ -f "riscv-gnu-toolchain.tar.gz" ]; then
    tar zxvf riscv-gnu-toolchain.tar.gz
    pushd riscv-gnu-toolchain
    mkdir build && cd build
    ../configure --prefix=$RISCV
    make -j`nproc`
    popd
else
    echo "riscv-gnu-toolchain.tar.gz not exist, please check "
    exit 1
fi

# get toolchain
git clone --recursive git@code.streamcomputing.com:simulator/llvm-project.git && pushd llvm-project
git checkout matrix-0.3.1

mkdir -p build && cd build && cmake -DCMAKE_INSTALL_PREFIX=$RISCV \
    -DCMAKE_BUILD_TYPE=Release -DLLVM_OPTIMIZED_TABLEGEN=On \
    -DLLVM_ENABLE_PROJECTS="clang;compiler-rt;lld;clang-tools-extra" \
    -DLLVM_TARGETS_TO_BUILD="X86;RISCV" ../llvm

make -j`nproc` && make install

popd

# create python3 envriment
# load python3 package
wget http://172.16.31.70/pack/Python-3.10.13.tgz

# uncompress 
if [ -f "Python-3.10.13.tgz" ];then
    tar zxvf Python-3.10.13.tgz
    pushd Python-3.10.13

    mkdir build && cd build
    ../configure --enable-optimizations

    make altinstall -j`nproc`

    # update python3.10.13
    update-alternatives --install /usr/bin/python3 python3 /usr/local/bin/python3.10 1
    pip install "numpy<2"
    python3 --version
    popd

else
    echo "Python-3.10.13.tgz not exist, please check"
    exit 1
fi

git clone --recursive  http://gitlab-ci-token:${CI_JOB_TOKEN}@code.streamcomputing.com/verification/rvpvp-matrix.git && pushd rvpvp-matrix

git branch
git checkout matrix-0.3
git submodule init && git submodule update
pushd rvpvp
git branch
git checkout matrix-0.3 
pushd rvpvp/env
git submodule init && git submodule update
popd
which spike
python3 -m pip install --upgrade pip && pip install virtualenv

virtualenv -p python3 .env

source ./scripts/env.common 
pip install -U pip
pip3 install .
pip install "numpy<2"
popd
rvpvp gen --nproc `nproc` --failing-info $SPEC_LIST $EXTRA_OPTS
rvpvp run --nproc `nproc` --failing-info
