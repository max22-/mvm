instructions = [
    "brk",
    "push_u8",
    "push_u16",
    "push32",
    "dup",
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
    "call",
    "ret",
]

status = [
    "running",
    "halted",
    "segmentation fault",
    "stack overflow",
    "stack underflow",
    "return stack overflow",
    "return stack underflow",
    "invalid instruction",
    "division by zero"
]

class Generator:
    def __init__(self, f, marker_start, marker_end):
        self.f = f
        self.marker_start = marker_start
        self.marker_end = marker_end

    def emit(self, line):
        self.f.write(line + "\n")

    def generate(self):

        self.emit(self.marker_start)
        self.emit("")
        self.gen()
        self.emit("")
        self.emit(self.marker_end)

class EnumsGenerator(Generator):
    def __init__(self, f):
        super().__init__(f, "// Generated enums start", "// Generated enums end")

    def gen(self):
        self.emit("enum MVM_OPCODE {")
        for i in instructions:
            self.emit(f"    OP_{i.upper()},")
        self.emit("    MVM_OPCODE_COUNT,")
        self.emit("};")
        self.emit("")

        self.emit("enum mvm_status {")
        for s in status:
            enum_entry = s.replace(" ", "_").upper()
            self.emit(f"    MVM_{enum_entry},")
        self.emit("};")


class StringsArraysGenerator(Generator):
    def __init__(self, f):
        super().__init__(f, "// Generated strings arrays start", "// Generated strings arrays end")

    def gen(self):
        self.emit("const char *mvm_op_name[] = {")
        for i in instructions:
            self.emit(f'    "{i}",')
        self.emit("};")
        self.emit("")
        self.emit("const char *mvm_status_name[] = {")
        for s in status:
            self.emit(f'    "{s}",')
        self.emit("};")

class LoadStoreGenerator(Generator):
    def __init__(self, f):
        super().__init__(f, "// Generated load/store start", "// Generated load/store end")

    def gen(self):
            types = [(sign, size) for sign in ["u", "i"] for size in [8, 16, 32]]
            type_map = {"i": "int", "u": "uint"}
            for t in types:
                sign, size = t
                type_name = f"{type_map[sign]}{size}_t"
                self.emit(f"{type_map[sign]}32_t mvm_load_{sign}{size}(mvm *vm, uint32_t addr, uint8_t *success) {{")
                self.emit(f"    if(addr > MVM_RAM_SIZE - sizeof({type_name})) {{")
                self.emit("        vm->status = MVM_SEGMENTATION_FAULT;")
                self.emit("        *success = 0;")
                self.emit("        return 0;")
                self.emit("    }")
                self.emit("    *success = 1;")
                self.emit(f"    return MVM_BITCAST({type_name}, vm->ram[addr]);")
                self.emit("}")
                self.emit("")



with open("src/mvm.h", "r") as f:
    source = f.readlines()

f = open("src/mvm.h", "w")

gens = [EnumsGenerator(f), StringsArraysGenerator(f), LoadStoreGenerator(f)]

inside_block = False

for l in source:
    if inside_block:
        if l.strip() in [g.marker_end for g in gens]:
            inside_block = False
            continue
    for g in gens:
        if l.strip() == g.marker_start:
            g.generate()
            inside_block = True
    if not inside_block:
        f.write(l)

if inside_block:
    raise RuntimeError("block not closed")

f.close()


