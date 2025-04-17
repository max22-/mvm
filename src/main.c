#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#define MVM_IMPLEMENTATION
#include "mvm.h"
#include "util.h"

#define FRAMEBUFFER_WIDTH 320
#define FRAMEBUFFER_HEIGHT 240

uint32_t port_in(uint32_t port) {
    return 0;
}

void port_out(uint32_t port, uint32_t value) {
    if(port == 0 && value == 1) {
        // blit the frame buffer
    }
}

uint32_t mmio_read8(mvm *vm, uint32_t addr) {
    vm->status = MVM_SEGMENTATION_FAULT;
    return 0;
}

uint32_t mmio_read16(mvm *vm, uint32_t addr) {
    vm->status = MVM_SEGMENTATION_FAULT;
    return 0;
}

uint32_t mmio_read32(mvm *vm, uint32_t addr) {
    vm->status = MVM_SEGMENTATION_FAULT;
    return 0;
}

void mmio_write8(mvm *vm, uint32_t addr, uint8_t value) {
    vm->status = MVM_SEGMENTATION_FAULT;
}

void mmio_write16(mvm *vm, uint32_t addr, uint16_t value) {
    const uint32_t base_fb_addr = 0x80000000;
    if(addr >= base_fb_addr && addr < base_fb_addr + FRAMEBUFFER_WIDTH * FRAMEBUFFER_HEIGHT * sizeof(uint16_t)) {
        // put a pixel inside the frame buffer
    } else {
        vm->status = MVM_SEGMENTATION_FAULT;
    }
}

void mmio_write32(mvm *vm, uint32_t addr, uint32_t value) {
    vm->status = MVM_SEGMENTATION_FAULT;
}

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