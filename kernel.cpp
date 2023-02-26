#include "types.h"

void printf(int8_t *str) {
    uint16_t *videoMemoryVGA = (uint16_t *)0xb8000;

    for (int32_t i = 0; str[i] != '\0'; ++i) {
        videoMemoryVGA[i] = (videoMemoryVGA[i] & 0xFF00) | str[i];
    }
}

typedef void (*constructor)();

extern "C" constructor start_ctors;
extern "C" constructor end_ctors;
extern "C" void callConstructors(){
    for(constructor* i = &start_ctors; i != &end_ctors; ++i){
        (*i)();
    }
}


extern "C" void kernelMain(void *multiboot_structure, uint32_t magicnumber) {
    printf("Hello from the kernel!");
    while (1)
        ;
}
