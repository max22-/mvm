#include <stdio.h>
#define MVM_IMPLEMENTATION
#include <mvm.h>
#include "assembler.h"
#define SV_IMPLEMENTATION
#include "sv.h"

typedef struct assembler {
    uint32_t pc, pc_max;
    const char *file_name;
    const char *source;
    uint8_t *rom;
    size_t rom_capacity;
    int success;
    sv s;
} assembler;

static assembler assembler_new(const char *file_name, const char *source, uint8_t *rom, size_t rom_capacity) {
    assembler a = {
        .pc = 0,
        .pc_max = 0,
        .file_name = file_name,
        .source = source,
        .rom = rom,
        .rom_capacity = rom_capacity,
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
    if(a->pc >= a->rom_capacity) {
        assembler_error(a, "trying to write code after rom limit");
        return;
    }
    a->rom[a->pc] = b;
    set_pc(a, a->pc + 1);
}

uint32_t sv_u32(sv s, int *success) {
    int _success;
    uint32_t result;
    if(sv_starts_with(s, sv_from_cstr("$"))) {
        s = sv_chop_left(s, 1);
        result = sv_u32_hex(s, &_success);
        if(!_success) {
            *success = 0;
            return 0;
        }
    } else {
        result = sv_u32_dec(s, &_success);
        if(!_success) {
            *success = 0;
            return 0;
        }
    }
    return result;
}

void org(assembler *a) {
    int success;
    a->s = sv_skipspace(a->s);
    sv sv_addr = sv_tok(a->s);
    uint32_t addr = sv_u32(sv_addr, &success);
    if(!success) {
        assembler_error(a, "expected number");
        return;
    }
    set_pc(a, addr);
    a->s = sv_chop_tok(a->s);
}

void push(assembler *a) {
    int success;
    a->s = sv_skipspace(a->s);
    sv sv_lit = sv_tok(a->s);
    uint32_t lit = sv_u32(sv_lit, &success);
    if(!success) {
        assembler_error(a, "expected number");
        return;
    }
    if(lit < UINT8_MAX) {
        emit_u8(a, OP_PUSH_U8);
        emit_u8(a, (uint8_t)lit);
    } else if(lit < 65536) {
        // TODO
    } else {
        // TODO
    }
    a->s = sv_chop_tok(a->s);
}

void instruction(assembler *a, sv tok) {
    const char buf[32];
    sv_to_cstr(tok, buf, sizeof(buf));
    int opcode = mvm_opcode_from_name(buf);
    if(opcode == -1) {
        assembler_error(a, "unexpected token");
        return;
    }
    emit_u8(a, (uint8_t)opcode);
    a->s = sv_chop_tok(a->s);
}

ssize_t assemble(const char *file_name, const char *source, uint8_t *rom, size_t rom_capacity) {
    char buf[1024];

    assembler a = assembler_new(file_name, source, rom, rom_capacity);
    
    while(!sv_is_empty(a.s) && a.success) {
        a.s = sv_skipspace(a.s);
        sv tok = sv_tok(a.s);
        if(sv_eq(tok, sv_from_cstr("org"))) {
            a.s = sv_chop_tok(a.s);
            org(&a);
        } else if(sv_eq(tok, sv_from_cstr("push"))) {
            a.s = sv_chop_tok(a.s);
            push(&a);
        } else {
            instruction(&a, tok);
        }
    }

    if(a.success)
        return a.pc_max;
    else
        return -1;
}