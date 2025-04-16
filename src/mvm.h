#ifndef MVM_H
#define MVM_H

#include <stdint.h>

#define MVM_RAM_SIZE 0x10000
#define MVM_INTERRUPT_TABLE_SIZE 0x10
#define MVM_ENTRY_POINT                                                        \
    (MVM_INTERRUPT_TABLE_SIZE *                                                \
     sizeof(uint32_t)) // the entry point is just after the interrupt table

// Generated enums start

enum MVM_OPCODE {
    OP_BRK,
    OP_PUSH_U8,
    OP_PUSH_U16,
    OP_PUSH32,
    OP_ADD,
    OP_SUB,
    OP_MUL,
    OP_DIV,
    MVM_OPCODE_COUNT,
};

enum mvm_error {
    MVM_NO_ERROR,
    MVM_SEGMENTATION_FAULT,
    MVM_STACK_OVERFLOW,
    MVM_STACK_UNDERFLOW,
    MVM_RETURN_STACK_OVERFLOW,
    MVM_RETURN_STACK_UNDERFLOW,
    MVM_INVALID_INSTRUCTION,
    MVM_DIVISION_BY_ZERO,
};


// Generated enums end

typedef struct mvm {
    uint32_t pc, sp, rsp;
    uint32_t stk[256], rstk[256];
    uint8_t *ram;
    uint8_t is_running;
    enum mvm_error error;
} mvm;

void mvm_init(mvm *vm, uint8_t *ram);
void mvm_run(mvm *vm, uint32_t limit);
int mvm_opcode_from_name(const char *name);
const char *mvm_current_instruction_name(mvm *vm);
void mvm_dump(mvm *vm);

#ifdef MVM_IMPLEMENTATION

#include <string.h>

// Generated strings arrays start

const char *mvm_op_name[] = {
    "brk",
    "push_u8",
    "push_u16",
    "push32",
    "add",
    "sub",
    "mul",
    "div",
};

const char *mvm_error_name[] = {
    "",
    "segmentation fault",
    "stack overflow",
    "stack underflow",
    "return stack overflow",
    "return stack underflow",
    "invalid instruction",
    "division by zero",
};

// Generated strings arrays end

#define MVM_ARRAYSIZE(x) (sizeof(x) / sizeof((x)[0]))

void mvm_init(mvm *vm, uint8_t *ram) {
    memset(vm, 0, sizeof(mvm));
    vm->pc = MVM_ENTRY_POINT;
    vm->ram = ram;
    vm->is_running = 1;
}

uint32_t mvm_load8(mvm *vm, uint32_t addr, uint8_t *success) {
    if(addr >= MVM_RAM_SIZE) {
        vm->error = MVM_SEGMENTATION_FAULT;
        vm->is_running = 0;
        *success = 0;
        return 0;
    }
    *success = 1;
    return vm->ram[addr];
}

void mvm_push(mvm *vm, uint32_t x, uint8_t *success) {
    if(vm->sp >= MVM_ARRAYSIZE(vm->stk)) {
        *success = 0;
        vm->error = MVM_STACK_OVERFLOW;
        return;
    }
    vm->stk[vm->sp++] = x;
    *success = 1;
}

uint32_t mvm_pop(mvm *vm, uint32_t x, uint8_t *success) {
    if(vm->sp == 0) {
        *success = 0;
        vm->error = MVM_STACK_UNDERFLOW;
        return 0;
    }
    *success = 1;
    return vm->stk[--vm->sp];
}

#define MVM_BINOP_CHECKED(binop, check) \
    do { \
        b = mvm_pop(vm, b, &success);   \
        if(!success)    \
            return; \
        a = mvm_pop(vm, b, &success);   \
        if(!success)    \
            return; \
        check \
        mvm_push(vm, a binop b, &success); \
    } while(0)

#define MVM_BINOP(binop) MVM_BINOP_CHECKED(binop, /* dummy arg */)

void mvm_run(mvm *vm, uint32_t limit) {
    uint8_t success;
    uint32_t a, b;
    while(limit-- && vm->is_running) {
        uint8_t op = mvm_load8(vm, vm->pc++, &success);
        if(!success)
            return;
        switch(op) {
        case OP_BRK:
            vm->is_running = 0;
            break;
        case OP_PUSH_U8:
            a = mvm_load_u8(vm, vm->pc, &success);
            if(!success)
                return;
            vm->pc++;
            mvm_push(vm, a, &success);
            break;
        case OP_PUSH_U16:
            a = mvm_load_u16(vm, vm->pc, &success);
            if(!success)
                return;
            vm->pc += 2;
            mvm_push(vm, a, &success);
            break;
        case OP_PUSH32:
            a = mvm_load32(vm, vm->pc, &success);
            if(!success)
                return;
            vm->pc += 4;
            mvm_push(vm, a, &success);
            break;
        case OP_ADD:
            MVM_BINOP(+);
            break;
        case OP_SUB:
            MVM_BINOP(-);
            break;
        case OP_MUL:
            MVM_BINOP(*);
            break;
        case OP_DIV:
            MVM_BINOP_CHECKED(/,
                if(b == 0) {
                    vm->error = MVM_DIVISION_BY_ZERO;
                    vm->is_running = 0;
                    return;
                }
            );
        default:
            vm->error = MVM_INVALID_INSTRUCTION;
            vm->is_running = 0;
            return;
        }
    }
}

static int str_eq(const char *s1, const char *s2) {
    while(*s1 && *s2) {
        if(*s1 != *s2)
            return 0;
        s1++;
        s2++;
    }
    return *s1 == 0 && *s2 == 0;
}

int mvm_opcode_from_name(const char *name) {
    for(size_t i = 0; i < MVM_ARRAYSIZE(mvm_op_name); i++) {
        if(str_eq(name, mvm_op_name[i]))
            return i;
    }
    return -1;
}

const char *mvm_current_instruction_name(mvm *vm) {
    if(vm->pc < MVM_RAM_SIZE) {
        uint8_t op = vm->ram[vm->pc];
        if(op < MVM_OPCODE_COUNT)
            return mvm_op_name[op];
    }
    return "";
}

void mvm_dump(mvm *vm) {
    printf("\n\nmvm\n");
    printf("    stk: sp=0x%x\n", vm->sp);
    printf("    ");
    for(uint32_t i = 0; i < vm->sp; i++) {
        printf("%08x ", vm->stk[i]);
        if((i + 1) % 16 == 0)
            printf("\n    ");
    }
    printf("\n");
}

#endif
#endif