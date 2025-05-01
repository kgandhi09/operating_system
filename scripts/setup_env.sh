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
sudo mkdir -pv $LFS/cross_compile
sudo mkdir -pv $LFS/tools

# Add a temperory unpriviliged LFS user
sudo groupadd jk
sudo useradd -s /bin/bash -g jk -m -k /dev/null jk

sudo passwd jk

# Grant jk user full access to all the directories under $LFS by making jk the owner
sudo chown -v jk $LFS/{usr{,/*},lib,var,etc,bin,sbin,tools,cross_compile}
case $(uname -m) in
    x86_64)
        sudo chown -v jk $LFS/lib64 ;;
esac

sudo -u jk bash << 'EOF'
export LFS=$LFS
echo "Inside jk: LFS is \$LFS"

# set up bash profile
cat > ~/.bash_profile << "EOPROFILE"
exec env -i HOME=$HOME TERM=$TERM PS1='\u:\w\$ ' /bin/bash
EOPROFILE

cat > ~/.bashrc << "EOBASHRC"
set +h
umask 022
LFS=/mnt/lfs
LC_ALL=POSIX
LFS_TGT=$(uname -m)-lfs-linux-gnu
PATH=/usr/bin
if [ ! -L /bin ]; then PATH=/bin:$PATH; fi
PATH=$LFS/tools/bin:$PATH
CONFIG_SITE=$LFS/usr/share/config.site
export LFS LC_ALL LFS_TGT PATH CONFIG_SITE
EOBASHRC

export MAKEFLAGS=-j$(nproc)
echo "Inside jk: MAKEFLAGS is $MAKEFLAGS"

source ~/.bash_profile

EOF
