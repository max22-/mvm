instructions = [
    "brk",
    "push_u8",
    "push_u16",
    "push32",
    "add",
    "sub",
    "mul",
    "div",
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

code = ""

def emit(x):
    global code
    code += f"{x}\n"

emit("enum MVM_OPCODE {")
for i in instructions:
    emit(f"    OP_{i.upper()},")
emit("    MVM_OPCODE_COUNT,")
emit("};")
emit("")

emit("enum mvm_status {")
for s in status:
    enum_entry = s.replace(" ", "_").upper()
    emit(f"    MVM_{enum_entry},")
emit("};")
emit("")

enums_code = code
code = ""

emit("const char *mvm_op_name[] = {")
for i in instructions:
    emit(f'    "{i}",')
emit("};")

emit("")


emit("const char *mvm_status_name[] = {")
for s in status:
    emit(f'    "{s}",')
emit("};")

strings_arrays_code = code

marker_enums_start = "// Generated enums start\n"
marker_enums_end = "// Generated enums end\n"

marker_strings_arrays_start = "// Generated strings arrays start\n"
marker_strings_arrays_end = "// Generated strings arrays end\n"

with open("src/mvm.h", "r") as f:
    source = f.read()

source = source.split(marker_enums_start)
source[1] = source[1].split(marker_enums_end)
source = [source[0], source[1][0], source[1][1]]
source[2] = source[2].split(marker_strings_arrays_start)
source[2][1] = source[2][1].split(marker_strings_arrays_end)
source = [source[0], source[1], source[2][0], source[2][1][0], source[2][1][1]]

with open("src/mvm.h", "w") as f:
    f.write(source[0])
    f.write(marker_enums_start + "\n")
    f.write(enums_code)
    f.write("\n" + marker_enums_end)
    f.write(source[2])
    f.write(marker_strings_arrays_start + "\n")
    f.write(strings_arrays_code)
    f.write("\n" + marker_strings_arrays_end)
    f.write(source[4])