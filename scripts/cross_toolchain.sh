#!/bin/bash

sudo -u jk bash << 'EOF'
export LFS=/mnt/lfs
echo "Inside jk: LFS is $LFS"
SOURCE_DIR=$LFS/sources
WORK_DIR=$LFS/cross_compile

# echo "Building BinUtils"
# cd $LFS/cross_compile
# tar -xf $LFS/sources/binutils-*.tar.xz

# cd binutils-*/
# mkdir -v build
# cd build

# ../configure --prefix=$LFS/tools \
#              --with-sysroot=$LFS \
#              --target=$LFS_TGT \
#              --disable-nls \
#              --enable-gprofng=no \
#              --disable-werror \
#              --enable-new-dtags \
#              --enable-default-hash-style=gnu
# make
# make install

echo "Building GCC"
cd $LFS/cross_compile
tar -xf $LFS/sources/gcc-*.tar.xz
mv -v gcc-* gcc

case $(uname -m) in
    x86_64)
        sed -e '/m64=/s/lib64/lib/' \
            -i.orig $LFS/cross_compile/gcc/gcc/config/i386/t-linux64 ;;
esac

cd $LFS/cross_compile/gcc

tar -xf $LFS/sources/mpfr-*.tar.xz
mv -v mpfr-* mpfr
tar -xf $LFS/sources/gmp-*.tar.xz
mv -v gmp-* gmp
tar -xf $LFS/sources/mpc-*.tar.gz
mv -v mpc-* mpc
mkdir -v build
cd build

../configure    --target=$LFS_TGT \
                --prefix=$LFS/tools \
                --with-glibc-version=2.40 \
                --with-sysroot=$LFS \
                --with-newlib \
                --without-headers \
                --enable-default-pie \
                --enable-default-ssp \
                --disable-nls \
                --disable-shared \
                --disable-multilib \
                --disable-threads \
                --disable-libatomic \
                --disable-libgomp \
                --disable-libquadmath \
                --disable-libssp \
                --disable-libvtv \
                --disable-libstdcxx \
                --enable-languages=c,c++

# make
# make install

EOF
