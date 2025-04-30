#!/bin/bash

export LFS=/mnt/lfs

# Make the limited directory layout for the os
mkdir -pv $LFS/{etc,var} $LFS/usr/{bin,lib,sbin}
