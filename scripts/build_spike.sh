#!/bin/bash

if [ -z "$1" ]; then
	echo "Usage: $0 <build_dir>"
	echo
	exit 1
fi

BUILD_DIR="$1"

mkdir -p $BUILD_DIR

# ubuntu的release 不使能commitlog 以提升性能
if [[ $BUILD_DIR =~ "ubuntu" ]]
then
    cd $BUILD_DIR && ../configure --with-isa=rv64gcv_zfh --with-varch=vlen:256,elen:32,slen:256 --enable-misaligned --prefix=$PWD/../install && make -j`nproc` && cd -
else
    cd $BUILD_DIR && ../configure --with-isa=rv64gcv_zfh --with-varch=vlen:256,elen:32,slen:256 --enable-commitlog --enable-misaligned --prefix=$PWD/../install && make -j`nproc` && cd -
fi

