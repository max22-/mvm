instructions = [
    "brk",
    "lit_u8",
    "lit_u16",
    "lit_u32",
    "add",
    "sub",
    "mul",
    "div",
]

errors = [
    "",
    "segmentation fault",
    "stack overflow",
    "stack underflow",
    "return stack overflow",
    "return stack underflow",
    "invalid instruction",
]

code = ""

def emit(x):
    global code
    code += f"{x}\n"

emit("enum MVM_OPCODE {")
for i in instructions:
    emit(f"    OP_{i.upper()},")
emit("};")
emit("")
emit("const char *mvm_op_name[] = {")
for i in instructions:
    emit(f'    "{i}",')
emit("};")

emit("")

emit("enum mvm_error {")
for e in errors:
    if e == "":
        enum_entry = "NO_ERROR"
    else:
        enum_entry = e.replace(" ", "_").upper()
    emit(f"    MVM_{enum_entry},")
emit("};")
emit("")
emit("const char *mvm_error_name[] = {")
for e in errors:
    emit(f'    "{e}",')
emit("};")

marker_start = "// Generated code start\n"
marker_end = "// Generated code end\n"

with open("src/mvm.h", "r") as f:
    source = f.read()

source = source.split(marker_start)
source[1] = source[1].split(marker_end)
source = [source[0], source[1][0], source[1][1]]

with open("src/mvm.h", "w") as f:
    f.write(source[0])
    f.write(marker_start + "\n")
    f.write(code)
    f.write("\n" + marker_end)
    f.write(source[2])