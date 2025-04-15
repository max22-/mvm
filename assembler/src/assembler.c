#include <stdio.h>
#include <mvm.h>
#include "assembler.h"
#define SV_IMPLEMENTATION
#include "sv.h"

typedef struct assembler {
    uint32_t pc, pc_max;
    const char *file_name;
    const char *source;
    uint8_t *rom;
    size_t rom_size;
    int success;
    sv s;
} assembler;

static assembler assembler_new(const char *file_name, const char *source, uint8_t *rom, size_t rom_size) {
    assembler a = {
        .pc = 0,
        .pc_max = 0,
        .file_name = file_name,
        .source = source,
        .rom = rom,
        .rom_size = rom_size,
        .success = 1,
        .s = sv_from_cstr(source),
    };
    return a;
}

void assembler_error(assembler *a, const char *err_msg) {
    a->success = 0;
    size_t offset = (uintptr_t)a->s.data - (uintptr_t)a->source;
    size_t line = 1, column = 1;
    for(size_t i = 0; i < offset; i++) {
        if(a->source[i] == '\n') {
            line++;
            column = 1;
        } else {
            column++;
        }
    }
    fprintf(stderr, "%s:%lu:%lu: error: %s\n", a->file_name, line, column, err_msg);
}

static void set_pc(assembler *a, uint32_t pc) {
    if(!a->success) return;
    a->pc = pc;
    if(a->pc_max < pc)
        a->pc_max = pc;
}

static void emit_u8(assembler *a, uint8_t b) {
    if(!a->success) return;
    if(a->pc >= a->rom_size) {
        assembler_error(a, "trying to write code after rom limit");
        return;
    }
    a->rom[a->pc] = b;
    set_pc(a, a->pc + 1);
}



void org(assembler *a) {
    int success;
    a->s = sv_skipspace(a->s);
    sv sv_addr = sv_tok(a->s);
    uint32_t addr = sv_u32_hex(sv_addr, &success);
    if(!success) {
        assembler_error(a, "expected hex number");
        return;
    }
    set_pc(a, addr);
    a->s = sv_chop_tok(a->s);
}

ssize_t assemble(const char *file_name, const char *source, uint8_t *rom, size_t rom_size) {
    char buf[1024];

    assembler a = assembler_new(file_name, source, rom, rom_size);
    
    while(!sv_is_empty(a.s) && a.success) {
        a.s = sv_skipspace(a.s);
        sv tok = sv_tok(a.s);
        if(sv_eq(tok, sv_from_cstr("org"))) {
            a.s = sv_chop_tok(a.s);
            org(&a);
        }
        else {
            assembler_error(&a, "unexpected token");
        }
    }

    printf("result: pc = %d\n", a.pc);
    return 0;
}