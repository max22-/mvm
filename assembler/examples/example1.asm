
org $40

:start
    push 2
    ,square call
    push 1
    ,inc call
    add
brk

:inc
    push 1
    add
    ret

org $100

:square
    dup mul ret