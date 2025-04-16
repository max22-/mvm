#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#define MVM_IMPLEMENTATION
#include "mvm.h"
#include "util.h"

int main(int argc, char *argv[]) {
    if(argc != 2) {
        FATAL("usage: %s file.rom", argv[0]);
        return 1;
    }
    const char *rom_path = argv[1];
    FILE *f = fopen(rom_path, "rb");
    uint8_t *ram = (uint8_t *)malloc(MVM_RAM_SIZE);
    if(!ram) {
        FATAL("failed to allocate memory");
        return 1;
    }
    if(!f) {
        FATAL("failed to open %s", rom_path);
        return 1;
    }

    fseek(f, 0, SEEK_END);
    const long rom_size = ftell(f);
    if(rom_size > MVM_RAM_SIZE) {
        fclose(f);
        FATAL("rom file is too big to fit in ram");
        return 1;
    }
    fseek(f, 0, SEEK_SET);
    size_t n = fread(ram, rom_size, 1, f);
    fclose(f);
    if(n != 1) {
        FATAL("failed to load the rom file");
        return 1;
    }

    mvm vm;
    mvm_init(&vm, ram);
    while(vm.status == MVM_RUNNING)
        mvm_run(&vm, 1000);
    if(vm.status != MVM_HALTED)
        printf("status: %s\n", mvm_status_name[vm.status]);
    mvm_dump(&vm);

    free(ram);
    return 0;
}