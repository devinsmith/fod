bits 16

; Initial startup at
;01EF:1AEC

; Get DOS version
mov ah, 0x30
int 0x21
cmp al, 02 ; See if this is DOS 2.x
jnc 0x1AF6 ;
int 20;   ; Terminate

; Ok we have DOS 2.x or higher (DOSBox returns 5.0)
; 0x1AF6
mov di, 0x06B2
mov si, [0x0002]  ; ds:[0002] = 0x9FFF
sub si, di
cmp si, 0x1000
jc 0x1B08
mov si, 0x1000
cli
mov ss, di
; 0x1B0B
add sp, 0x406E
sti
jnc 0x1B22

; 1B22
and sp, 0xFFFE

01EF:1B25  368926121E          mov  ss:[1E12],sp           ss:[1E12]=0000
01EF:1B2A  3689260E1E          mov  ss:[1E0E],sp           ss:[1E0E]=0000
01EF:1B2F  8BC6                mov  ax,si
01EF:1B31  B104                mov  cl,04
01EF:1B33  D3E0                shl  ax,cl
01EF:1B35  48                  dec  ax
01EF:1B36  36A30C1E            mov  ss:[1E0C],ax           ss:[1E0C]=0000
01EF:1B3A  03F7                add  si,di
01EF:1B3C  89360200            mov  [0002],si              ds:[0002]=9FFF

; Memory allocate (realloc) ?
; Block size = 0x14D3

01EF:1B46  B44A                mov  ah,4A
01EF:1B48  CD21                int  21

; Zero out memory

; call 1B9E
; call 1EFA
; call 1D6C
; call 1191 - Game loop.


; 01EF:1191 (CS)


call 0x105 ; Reads DISK1
call 0x2FE4 ; ?

call 0x2E5 ; Reads BORDERS
add sp, 2
call 0x1439 ; Switches to Mode 13, opens FONT
call 0x18B8 ; Opens TPICT
call 0x19BE ;  ?
call 0x1902 ; ?

call 0x18F1 ; ?

jmp 0x13D7

call 0x1AC7 ; Sets up VGA?

call 0x14D5 ; Show title screen? TPICT?
call 0x141A ; ?

call 0x39FE ; inner loop?



