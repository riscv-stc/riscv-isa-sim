#!/bin/bash

if [ -z "$1" ]; then
	echo "Usage: $0 <build_dir>"
	echo
	exit 1
fi

BUILD_DIR="$1"

export PATH=$CI_PROJECT_DIR/$BUILD_DIR:$RISCV/bin:$PATH
export LC_ALL=C.UTF-8
export LANG=C.UTF-8
export PIP_INDEX_URL=https://nexus3.in/repository/pypi-aliyun/simple/
export PIP_TRUSTED_HOST=nexus3.in
export JAX_PLATFORM_NAME=cpu

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

rvpvp gen --basic --nproc `nproc` --failing-info
rvpvp run --nproc `nproc` --failing-info
