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

# Add a temperory unpriviliged LFS user
sudo groupadd jk
sudo useradd -s /bin/bash -g jk -m -k /dev/null jk

sudo passwd jk

# Grant jk user full access to all the directories under $LFS by making jk the owner
sudo chown -v jk $LFS/{usr{,/*},lib,var,etc,bin,sbin,tools}
case $(uname -m) in
    x86_64)
        sudo chown -v jk $LFS/lib64 ;;
esac

sudo -u jk bash << 'EOF'
export LFS=$LFS
echo "Inside jk: LFS is \$LFS"
EOF
