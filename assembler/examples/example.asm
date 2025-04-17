org $40

push 0

:start
push $ffff
ovr
,pixel call
,blit call

push 1 add
dup push 320 push 240 mul gteu ,end cjmp
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