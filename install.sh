export GPPPARAMS="-m64 -fno-use-cxa-atexit -nostdlib -fno-builtin -fno-rtti -fno-exceptions -fno-leading-underscore"
export ASPARAMS="--64"
export LDPARAMS="-melf_x86_64"

# kernel.cpp -> kernel.o
g++ ${GPPPARAMS} -c kernel.cpp -o kernel.o

# loader.s --> loader.o
as ${ASPARAMS} loader.s -o loader.o

# linking loader.o and kernel.o
# ld ${LDPARAMS} -T linker.ld -o kernel.bin loader.o kernel.o

# # Generating iso using bin
# mkdir iso
# mkdir iso/boot
# mkdir iso/boot/grub
# cp kernel.bin ./iso/boot/
# cp grub.cfg ./iso/boot/grub/
# grub-mkrescue --output=gandhi_os.iso iso
# rm -rf iso
