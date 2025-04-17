#!/bin/bash
set -xe

python code_gen/code_gen.py
make
make -C assembler
make -C debugger -j`nproc`
./assembler/bin/mvmasm assembler/examples/example.asm assembler/bin/example.rom
./debugger/bin/mvmdbg assembler/bin/example.rom
