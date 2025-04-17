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
    OP_DUP,
    OP_POP,
    OP_ADD,
    OP_SUB,
    OP_MUL,
    OP_DIV,
    OP_DIVU,
    OP_EQ,
    OP_NEQ,
    OP_LT,
    OP_GTE,
    OP_LTU,
    OP_GTEU,
    OP_LB,
    OP_LH,
    OP_LW,
    OP_LBU,
    OP_LHU,
    OP_SB,
    OP_SH,
    OP_SW,
    OP_JMP,
    OP_CJMP,
    OP_CALL,
    OP_RET,
    MVM_OPCODE_COUNT,
};

enum mvm_status {
    MVM_RUNNING,
    MVM_HALTED,
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
    enum mvm_status status;
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
    "dup",
    "pop",
    "add",
    "sub",
    "mul",
    "div",
    "divu",
    "eq",
    "neq",
    "lt",
    "gte",
    "ltu",
    "gteu",
    "lb",
    "lh",
    "lw",
    "lbu",
    "lhu",
    "sb",
    "sh",
    "sw",
    "jmp",
    "cjmp",
    "call",
    "ret",
};

const char *mvm_status_name[] = {
    "running",
    "halted",
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
    vm->status = MVM_RUNNING;
}

#define MVM_BITCAST(t, x) (*(t *)(&(x)))

// Generated load/store start

uint32_t mvm_load_u8(mvm *vm, uint32_t addr, uint8_t *success) {
    if(addr > MVM_RAM_SIZE - sizeof(uint8_t)) {
        vm->status = MVM_SEGMENTATION_FAULT;
        *success = 0;
        return 0;
    }
    *success = 1;
    return MVM_BITCAST(uint8_t, vm->ram[addr]);
}

uint32_t mvm_load_u16(mvm *vm, uint32_t addr, uint8_t *success) {
    if(addr > MVM_RAM_SIZE - sizeof(uint16_t)) {
        vm->status = MVM_SEGMENTATION_FAULT;
        *success = 0;
        return 0;
    }
    *success = 1;
    return MVM_BITCAST(uint16_t, vm->ram[addr]);
}

uint32_t mvm_load_u32(mvm *vm, uint32_t addr, uint8_t *success) {
    if(addr > MVM_RAM_SIZE - sizeof(uint32_t)) {
        vm->status = MVM_SEGMENTATION_FAULT;
        *success = 0;
        return 0;
    }
    *success = 1;
    return MVM_BITCAST(uint32_t, vm->ram[addr]);
}

int32_t mvm_load_i8(mvm *vm, uint32_t addr, uint8_t *success) {
    if(addr > MVM_RAM_SIZE - sizeof(int8_t)) {
        vm->status = MVM_SEGMENTATION_FAULT;
        *success = 0;
        return 0;
    }
    *success = 1;
    return MVM_BITCAST(int8_t, vm->ram[addr]);
}

int32_t mvm_load_i16(mvm *vm, uint32_t addr, uint8_t *success) {
    if(addr > MVM_RAM_SIZE - sizeof(int16_t)) {
        vm->status = MVM_SEGMENTATION_FAULT;
        *success = 0;
        return 0;
    }
    *success = 1;
    return MVM_BITCAST(int16_t, vm->ram[addr]);
}

int32_t mvm_load_i32(mvm *vm, uint32_t addr, uint8_t *success) {
    if(addr > MVM_RAM_SIZE - sizeof(int32_t)) {
        vm->status = MVM_SEGMENTATION_FAULT;
        *success = 0;
        return 0;
    }
    *success = 1;
    return MVM_BITCAST(int32_t, vm->ram[addr]);
}


// Generated load/store end

void mvm_push(mvm *vm, uint32_t x, uint8_t *success) {
    if(vm->sp >= MVM_ARRAYSIZE(vm->stk)) {
        *success = 0;
        vm->status = MVM_STACK_OVERFLOW;
        return;
    }
    vm->stk[vm->sp++] = x;
    *success = 1;
}

uint32_t mvm_pop(mvm *vm, uint8_t *success) {
    if(vm->sp == 0) {
        *success = 0;
        vm->status = MVM_STACK_UNDERFLOW;
        return 0;
    }
    *success = 1;
    return vm->stk[--vm->sp];
}

void mvm_rpush(mvm *vm, uint32_t x, uint8_t *success) {
    if(vm->rsp >= MVM_ARRAYSIZE(vm->rstk)) {
        *success = 0;
        vm->status = MVM_RETURN_STACK_OVERFLOW;
        return;
    }
    vm->rstk[vm->rsp++] = x;
    *success = 1;
}

uint32_t mvm_rpop(mvm *vm, uint8_t *success) {
    if(vm->rsp == 0) {
        *success = 0;
        vm->status = MVM_RETURN_STACK_UNDERFLOW;
        return 0;
    }
    *success = 1;
    return vm->rstk[--vm->rsp];
}

#define MVM_BINOP_UNSIGNED(binop, block)                                       \
    do {                                                                       \
        ub = mvm_pop(vm, &success);                                            \
        if(!success)                                                           \
            return;                                                            \
        ua = mvm_pop(vm, &success);                                            \
        if(!success)                                                           \
            return;                                                            \
        block mvm_push(vm, ua binop ub, &success);                             \
    } while(0)

#define MVM_BINOP_SIGNED(binop, block)                                         \
    do {                                                                       \
        ub = mvm_pop(vm, &success);                                            \
        if(!success)                                                           \
            return;                                                            \
        ua = mvm_pop(vm, &success);                                            \
        if(!success)                                                           \
            return;                                                            \
        ia = MVM_BITCAST(int32_t, ua);                                         \
        ib = MVM_BITCAST(int32_t, ub);                                         \
        block ia = ia binop ib;                                                \
        ua = MVM_BITCAST(uint32_t, ia);                                        \
        mvm_push(vm, ua, &success);                                            \
    } while(0)

void mvm_run(mvm *vm, uint32_t limit) {
    uint8_t success;
    uint32_t ua, ub;
    int32_t ia, ib;
    while(limit-- && vm->status == MVM_RUNNING) {
        uint8_t op = mvm_load_u8(vm, vm->pc++, &success);
        if(!success)
            return;
        switch(op) {
        case OP_BRK:
            vm->status = MVM_HALTED;
            break;
        case OP_PUSH_U8:
            ua = mvm_load_u8(vm, vm->pc, &success);
            if(!success)
                return;
            vm->pc += sizeof(uint8_t);
            mvm_push(vm, ua, &success);
            break;
        case OP_PUSH_U16:
            ua = mvm_load_u16(vm, vm->pc, &success);
            if(!success)
                return;
            vm->pc += sizeof(uint16_t);
            mvm_push(vm, ua, &success);
            break;
        case OP_PUSH32:
            ua = mvm_load_u32(vm, vm->pc, &success);
            if(!success)
                return;
            vm->pc += sizeof(uint32_t);
            mvm_push(vm, ua, &success);
            break;
        case OP_DUP:
            ua = mvm_pop(vm, &success);
            if(!success)
                return;
            mvm_push(vm, ua, &success);
            if(!success)
                return;
            mvm_push(vm, ua, &success);
            break;
        case OP_POP:
            mvm_pop(vm, &success);
            break;
        case OP_ADD:
            MVM_BINOP_UNSIGNED(+, {});
            break;
        case OP_SUB:
            MVM_BINOP_UNSIGNED(-, {});
            break;
        case OP_MUL:
            MVM_BINOP_UNSIGNED(*, {});
            break;
        case OP_DIV:
            MVM_BINOP_SIGNED(/, {
                if(ib == 0) {
                    vm->status = MVM_DIVISION_BY_ZERO;
                    return;
                }
            });
            break;
        case OP_DIVU:
            MVM_BINOP_UNSIGNED(/, {
                if(ub == 0) {
                    vm->status = MVM_DIVISION_BY_ZERO;
                    return;
                }
            });
            break;
        case OP_EQ:
            MVM_BINOP_UNSIGNED(==, {});
            break;
        case OP_NEQ:
            MVM_BINOP_UNSIGNED(!=, {});
            break;
        case OP_LT:
            MVM_BINOP_SIGNED(<, {});
            break;
        case OP_GTE:
            MVM_BINOP_SIGNED(>=, {});
            break;
        case OP_LTU:
            MVM_BINOP_UNSIGNED(<, {});
            break;
        case OP_GTEU:
            MVM_BINOP_UNSIGNED(>=, {});
            break;
        case OP_JMP:
            ua = mvm_pop(vm, &success);
            if(!success)
                return;
            vm->pc = ua;
            break;
        case OP_CJMP:
            ub = mvm_pop(vm, &success);
            if(!success)
                return;
            ua = mvm_pop(vm, &success);
            if(!success)
                return;
            if(ua)
                vm->pc = ub;
            break;
        case OP_CALL:
            ua = mvm_pop(vm, &success);
            if(!success)
                return;
            mvm_rpush(vm, vm->pc, &success);
            vm->pc = ua;
            break;

        case OP_RET:
            ua = mvm_rpop(vm, &success);
            if(!success)
                return;
            vm->pc = ua;
            break;
        default:
            vm->status = MVM_INVALID_INSTRUCTION;
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