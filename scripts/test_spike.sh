#!/bin/bash

export LC_ALL=C.UTF-8
export LANG=C.UTF-8
export PIP_INDEX_URL=https://nexus3.in/repository/pypi-aliyun/simple/
export PIP_TRUSTED_HOST=nexus3.in
export JAX_PLATFORM_NAME=cpu

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

curl http://releases.streamcomputing.com/host-sdk/hpe/hpe2.x-latest | bash
curl http://releases.streamcomputing.com/toolchain/llvm-12/latest | bash

git clone --recursive  http://gitlab-ci-token:${CI_JOB_TOKEN}@code.streamcomputing.com/verification/rvpvp-npuv2.git

which spike
python3 -m pip install --upgrade pip && pip install virtualenv

cd rvpvp-npuv2
virtualenv -p python3 .env
source .env/bin/activate

pip install -U pip
pip install -r requirements.txt
pip install --editable rvpvp

rvpvp gen --nproc `nproc` --failing-info $SPEC_LIST $EXTRA_OPTS
rvpvp run --nproc `nproc` --failing-info
