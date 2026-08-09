; this program generates first 10 iterms of the fib sequence
; and put it on memory, starting from address 0

CLR x1 ; counter
CLR x2 ; a
CLR x3
CLR x5
CLR x7
CLR x10

LI x3, 1 ; b
LI x5, 1 ; "1"
LI x7, 10 ; "10"
LL x8, x15, loop ; load label "loop" to x8
LI x10, 2 ; "2"
LL x11, x15, result

loop:
ADD x4, x2, x3 ; c = a + b

; calculate store address
MOV x9, x1 ; addr = counter
ADD x9, x9, x11
STR x4, x9 ; M[addr] = c

MOV x2, x3 ; a = b
MOV x3, x4 ; b = c
ADD x1, x1, x5 ; counter += 1
SLT x6, x1, x7 ; x6 = counter < 10
BAL x6, x8, x0

HLT

result:
