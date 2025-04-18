.org $40

push 0

:start
,xor_pattern call
ovr
,pixel call
,blit call

push 1 add
dup ,width lw ,height lw mul gteu ,end cjmp
,start jmp

:end
brk

:pixel
    push $80000000
    add
    sh
    ret

:blit
    push 1
    push 0
    out
    ret

:xor_pattern
    dup ,width lw remu
    ovr ,width lw divu
    xor push 9 remu
    push 0 eq ,xor_pattern/black cjmp
    push $ffff
    ,xor_pattern/end jmp
    :xor_pattern/black
    push 0
    :xor_pattern/end
    ret

:width .word 320
:height .word 240