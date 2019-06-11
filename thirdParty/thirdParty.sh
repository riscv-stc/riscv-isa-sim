#!/bin/bash

# when modify in thirdPart/, version number should be updated
thirdpart_ver="${RISCV}/thridpart_v1"

if [ -f "${thirdpart_ver}" ]; then
	exit 0
fi

thirdparty_path="$1/"

cares_package="c-ares-cares-1_15_0"
protobuf_package="protobuf-cpp-3.7.1"
grpc_package="grpc-1.21.0"

rm -rf ${cares_package} ${protobuf_package} ${grpc_package}

pushd . &&
tar xvf ${thirdparty_path}/${cares_package}.tar.gz  && 
cd ${cares_package} && 
mkdir build && 
cd build/ && 
cmake ../ -DCMAKE_INSTALL_PREFIX=${RISCV} && 
make -j`nproc` && 
make install && 
popd && 
pushd .  && 
mkdir -p  ${protobuf_package} &&
tar xvf ${thirdparty_path}/${protobuf_package}.tar.gz -C ${protobuf_package} && 
cd ${protobuf_package}/protobuf-3.7.1 && 
./configure  --prefix=${RISCV} && 
make -j`nproc` && 
make install && 
popd && 
pushd . && 
tar xvf ${thirdparty_path}/${grpc_package}.tar.gz && 
cd ${grpc_package} && 
sed -i 's/-Werror//g' Makefile  && 
make  PERFTOOLS_CHECK_CMD=/bin/true HAS_SYSTEM_PERFTOOLS=false -j`nproc` prefix=${RISCV}  && 
make install prefix=${RISCV} && 
popd && touch ${thirdpart_ver} || exit 1
