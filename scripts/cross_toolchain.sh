#!/bin/bash

sudo -u jk bash << 'EOF'
export LFS=/mnt/lfs
echo "Inside jk: LFS is $LFS"
SOURCE_DIR=$LFS/sources
WORK_DIR=$LFS/cross_compile

echo "Building BinUtils"
cd $LFS/cross_compile
tar -xf $LFS/sources/binutils-*.tar.xz

cd binutils-*/
mkdir -v build
cd build

../configure --prefix=$LFS/tools \
             --with-sysroot=$LFS \
             --target=$LFS_TGT \
             --disable-nls \
             --enable-gprofng=no \
             --disable-werror \
             --enable-new-dtags \
             --enable-default-hash-style=gnu
make
make install

EOF
