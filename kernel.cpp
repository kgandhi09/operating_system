void printf(char *str) {
    unsigned short * videoMemoryVGA = (unsigned short *)0xb8000;

    for (int i = 0; str[i] != '\0'; ++i) {
        videoMemoryVGA[i] = (videoMemoryVGA[i] & 0xFF00) | str[i];
    }
}

void kernelMain(void *multiboot_structure, unsigned int magicnumber) {
    printf("Hello from the Kernel!");
    while (1)
        ;
}
