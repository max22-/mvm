
org $40

:start
    push 2
    ,square call
    ,inc call
    ,fac call
brk

:inc
    push 1
    add
    ret

:fac
    dup
    push 0 neq ,fac/rec cjmp 
    pop push 1 
    ret
:fac/rec
    dup push 1 sub ,fac call mul
    ret

org $100

:square
    dup mul ret