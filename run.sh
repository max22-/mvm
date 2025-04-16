#!/bin/bash
set -xe

python code_gen/code_gen.py
make
make -C assembler
make -C debugger -j`nproc`
./assembler/bin/mvmasm assembler/examples/example1.asm assembler/bin/example1.rom
./debugger/bin/mvmdbg assembler/bin/example1.rom
