#!/bin/bash

export LFS=/mnt/lfs

# Make the limited directory layout for the os
sudo mkdir -pv $LFS/{etc,var} $LFS/usr/{bin,lib,sbin}

# Make symlinks
for i in bin lib sbin; do
    sudo ln -sv $LFS/usr/$i $LFS/$i
done

case $(uname -m) in
    x86_64)
        sudo mkdir -pv $LFS/lib64 ;;
esac

# dir for cross compiler toolchain
sudo mkdir -pv $LFS/tools
