#MAGIC: Universal Metrics for bootloader to recognize that kernelMain is a kernel
.set MAGIC, 0x1badb002
.set FLAGS, (1<<0 | 1<<1)
.set CHECKSUM, -(MAGIC + FLAGS)

.section .multiboot
    .long MAGIC
    .long FLAGS
    .long CHECKSUM

.section .text
.extern kernelMain
.global loader

loader:
    # Copy instruction pointer to kernel stack
    # param esp: ESP register in the cpu - stack pointer for the system stack
    #param kernel_stack: pointer where we set the esp register to
    mov $kernel_stack, %esp
    push %eax
    push %ebx
    call kernelMain

#Add another infinite loop
_stop:
    cli         #Clear Interrup Flag
    hlt         #Halt the CPU (only for x76 architecture)
    jmp _stop

# Block Starting symbol section - code that contains statically allocated variables 
.section .bss
# Since the stack writes to left - leaving 2 MB of bytes for the kernel. 
# Essentially to prevent overwriting of firmware and Grub memory location by the kernel stack
# Like this in the RAM --> |Firmware|..|Grub|..|..|   |   |<-- Kernel|
.space 2*1024*1024 # 2 MB
kernel_stack:


