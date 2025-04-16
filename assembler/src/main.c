#include <stdio.h>
#include <stdlib.h>
#include <mvm.h>
#include <util.h>
#include "assembler.h"

int main(int argc, char *argv[]) {
    char *source = NULL;
    uint8_t *rom = NULL;
    FILE *f = NULL;

    int rc = 0;
    if(argc != 3) {
        FATAL("usage: %s source.asm output.rom", argv[0]);
        rc = 1;
        goto cleanup;
    }

    const char *source_path = argv[1];
    const char *rom_path = argv[2];

    rom = (uint8_t *)malloc(MVM_RAM_SIZE);
    if(!rom) {
        FATAL("failed to allocate memory");
        rc = 1;
        goto cleanup;
    }

    f = fopen(source_path, "rb");
    if(!f) {
        FATAL("failed to open `%s`", source_path);
        rc = 1;
        goto cleanup;
    }
    fseek(f, 0, SEEK_END);
    const long source_size = ftell(f);
    fseek(f, 0, SEEK_SET);

    source = (char *)malloc(source_size + 1);
    if(!source) {
        fclose(f);
        FATAL("failed to allocate memory");
        rc = 1;
        goto cleanup;
    }

    size_t n = fread(source, source_size, 1, f);
    fclose(f);
    if(n != 1) {
        FATAL("failed to load source in memory");
        rc = 1;
        goto cleanup;
    }
    source[source_size] = 0;

    f = fopen(rom_path, "wb");
    if(!f) {
        FATAL("failed to open output file `%s`\n", rom_path);
        rc = 1;
        goto cleanup;
    }

    printf("assembling...\n");
    size_t bytes_to_write = assemble(source_path, source, rom, MVM_RAM_SIZE);
    if(bytes_to_write != -1)
        fwrite(rom, bytes_to_write, 1, f);
    else
        rc = 1;

    fclose(f);

cleanup:
    if(rom)
        free(rom);
    if(source)
        free(source);
    return rc;
}
