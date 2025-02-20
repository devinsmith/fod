; A rough disassembly of FOD
; Mostly appears to be C based using cdecl caller conventions
; However there is some aggressive optimization that as well.

bits 16

; Initial startup at
;01EF:1AEC

start:
  ; Get DOS version
  mov ah, 0x30
  int 0x21
  cmp al, 02             ; See if this is DOS 2.x
  jnc .loc_1AF6
  int 20                 ; Terminate (this is DOS 2 or lower)

  ; Ok we have DOS 2.x or higher (DOSBox returns 5.0)
  ; 0x1AF6
  .loc_1AF6:
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

; call 1B9E (setup interrupt vector)
  push ss
  pop ds

; call 1EFA
; call 1D6C
; 1191 is likely main and not game loop, as it receives argc, argv[], env
; call 1191 - Game loop.

; 01EF:1B6D

  xor bp, bp
; Call main
  push word [0x1EA4] ; environment array
  push word [0x1EA2] ; argument array (argv)
  push word [0x1EA0] ; arg count (argc)
  call main


; 01EF:1191 (CS)
main:
  push bp
  mov bp, sp
  sub sp, 0x0010 ; Allocate space for 0x10 bytes of local variables

  push si
  mov word [bp-0x10], 0x0000 ; zero initialize
  mov byte [bp-0x0A], 0x00

  ; Reads DISK1 (initializes new game if not a saved game)
  ; A return value of 0 means this is a new game
  ; A return value of 1 means it's an existing saved game
  ; The contents of DISK1 will be loaded in memory at DS:[231E]
  call sub_0105

  mov [bp-0x0E], al  ; Is this a new game or saved game?
  mov al, [0x2324]   ; Data 1 buffer[6]
  mov [0x0424], al   ; Overwrite memory in ds:[0x424] ?

  ; ?
  mov ax, 0x03E8
  push ax
  call sub_2FE4
  add sp, 0x0002

  mov [0x3E66], ax
  mov al, [bp-0x0E] ; saved game
  sub ah, ah
  push ax
  call sub_02E5 ; Reads BORDERS
  add sp, 2
  call sub_1439 ; Switches to mode 13, opens FONT

  mov word [0x231C], 0x0000
  mov word [0x35E4], 0xFFFF
  mov word [0x33DE], 0x0000

  ; Arguments for sub_18B8
  mov ax, 0x8000
  push ax
  mov ax, 0x01C6 ; ds:[0x01C6] = "TPICT"
  push ax
  call dos_open_file ; Opens TPICT
  add sp, 0x0004
  mov [bp-0x06], ax ; File handle for TPICT

  push ax
  call dos_get_file_size
  add sp, 0x0002
  mov [bp-0x04], ax ; file size low
  mov [bp-0x02], dx ; file size high

  push word [0x0296]        ; Buffer
  push word [0x0294]        ; Buffer index
  push dx                   ; File size (high)
  push ax                   ; File size (low)
  push word [bp-06]         ; File Handle
  call read_file_to_buffer
  add sp, 0x000a

  push word [bp-0x06] ; file handle
  call dos_close_file ; Closes file

  cmp byte [bp-0x0E], 0   ; New or saved game?
  je .loc_1222

  ; This is a saved game
  mov  byte [bp-0x0A],01
  jmp  0x000013D7

.loc_1222

  ; Push some buffers
  push word [0x042D] ; Destination segment
  push word [0x042B] ; Destination offset
  push word [0x0296] ; Source segment
  push word [0x0294] ; Source offset

  call sub_1AC7 ; Sets up VGA?
  add sp, 0x0008

  mov ax, 0x029E
  push ax
  call 0x14D5 ; Show title screen? TPICT?
  add sp, 0x0002

call sub_1548

call 0x141A ; ?

call 0x39FE ; inner loop?

; sub_090 at 01EF:0090
sub_090:

; sub_0105 at 01EF:0105
sub_0105:
  push bp
  mov bp, sp
  sub sp, 0x000C

  push si
  sub ax, ax
  push ax

  mov ax, 0x009F ; DS:[0x9F] = "DISK1"
  push ax
  call dos_open_file
  add sp, 0x0004 ; cleanup for dos_open_file
  mov [bp-06], ax ; save in local variable
  or ax, ax
  jne 0x000123
  call sub_081
.loc_123:
  push word [bp-06]
  call dos_get_file_size
  add, 0002

  mov [bp-04], ax
  mov [bp-02], dx
  mov ax, 0x231E
  push ds
  push ax
  push dx
  push word [bp-04]
  push word [bp-06]

  add sp, 0x000A  ; clean up 5 arguments

  push word [bp-06]
  call dos_close_file ; close file
  add sp, 0002

  ; Determine if this is a saved game.
  cmp word [0x2320], 0000 ; second word in buffer
  jne .loc_015B ; is word not 0?
  cmp word [0x231E], 0000 ; first word in buffer.
  je .loc_0161
.loc_015B:
  mov ax, 0x0001 ; return code "return 1;"
  jmp .loc_02E0

.loc_0161:
  ; This code executes if DISK1 first word is 0x0000 (not a saved game)
  mov word [bp-08], 0000
  mov ax, 0x014C
  imul word [bp-08]
  add ax, 0x2358
  mov [bp-0C], ax
  mov bx, [bp-08]
  shl bx, 1
  mov [bx+0x3D88], ax
  mov bx, [bp-08]
  mov al, [bp-08]
  mov [bx+0x2350], al
  mov bx, [bp-0x0C]
  mov byte [bx], 0x00   ; 0x2358 (offset 3a, 58) (0 out)
  mov bx, [bp-0x0C]
  mov byte [bx+0x52], 0x06 ; 6 parties?
  mov word [bp-0x0a], 0000

  ; Loop 32 times.
.loc_0197:
  mov ax, 0x0006
  imul word [bp-0x0a]
  mov si, ax
  mov bx, [bp-0x0c]
  mov byte [bx+si+0x62], 0xFF
  mov ax, 0x0006
  imul word [bp-0x0A]
  mov si, ax
  mov bx, [bp-0x0c]
  mov byte [bx+si+0x64], 0
  mov ax, 0x0006
  imul word [bp-0x0A]

  mov si, ax
  mov bx, [bp-0x0c]
  mov byte [bx+si+0x63], 0
  mov ax, 0x0006
  imul word [bp-0x0A]

  ; 0x1CA
  mov si, ax
  mov bx, [bp-0x0c]
  mov byte [bx+si+0x65], 0
  inc word [bp-0x0A]

  cmp word [bp-0x0A], 0x0020
  jl .loc_0197

  ; 0x1DC
  mov bx, [bp-0x0c]
  mov byte [bx+0x62], 0x02
  mov bx, [bp-0x0c]
  mov byte [bx+0x64], 0x09
  mov bx, [bp-0x0c]
  mov byte [bx+0x68], 0x0C
  mov bx, [bp-0x0c]
  mov byte [bx+0x6E], 0x0C
  mov bx, [bp-0x0c]
  mov byte [bx+0x74], 0x20
  mov bx, [bp-0x0c]
  mov byte [bx+0x7A], 0x29
  mov bx, [bp-0x0c]
  mov byte [bx+0x0080], 0x37
  mov bx, [bp-0x0c]
  mov byte [bx+0x4E], 0x01
  mov bx, [bp-0x0c]
  mov word [bx+0x5E], 0x05DC
  mov word [bx+0x60], 0x0000
  mov bx, [bp-0x0c]
  mov byte [bx+0x4A], 0xFF
  mov word [bp-0x0A], 0x0000
.loc_022E:
  mov si, [bp-0x0A]
  mov bx, [bp-0x0c]
  mov byte [bx+si+0x4B], 0xFF
  inc word [bp-0x0A]
  cmp word [bp-0x0A], 0x0003
  jl .loc_022E








.loc_02E0:
  pop si
  mov sp, bp
  pop bp
  ret

sub_02E5:
  push bp
  mov bp, sp
  sub sp, 0x0006 ; 6 bytes of local variables
  sub ax, ax ; 0
  push ax
  mov ax, 0x00A5 ; DS:[0x00A5] = "BORDERS"
  push ax
  call dos_open_file
  add sp, 0x0004
  mov [bp-06], ax ; Save handle

  sub ax, ax
  push ax
  mov ax, 0x1388 ; offset inside "BORDERS"
  cwd
  push dx
  push ax
  push word [bp-0x06] ; handle
  call sub_19A2 ; seek
  add sp, 0x0008

  push ds
  push word [0x3E66]
  mov ax, 0x3E8 ; Read 0x3E8 bytes (1000 bytes)
  cwd
  push dx
  push ax
  push word [bp-0x6] ; handle
  call read_file_to_buffer
  add sp, 0x000A

  push word [bp-0x06]
  call dos_close_file
  add sp, 0x0002

  cmp byte [bp+0x04], 00
  je .loc_0333
  jmp .loc_03DC

.loc_0333:
  sub ax, ax
  push ax
  mov ax, 0x00AD ;
  mov ax, 0x00AD ; DS:[0x00AD] = "ARCHTYPE"
  call dos_open_file
  add sp, 0x0004
  mov [bp-06], ax ; Save handle
  or ax, ax
  jne .loc_034A
  call sub_081 ; ? error handler?

.loc_034A:
  push word [bp-0x06]
  call dos_get_file_size
  add sp, 0x0002

  ; save file size
  mov [bp-0x04], ax
  mov [bp-0x02], dx
  push ax
  call sub_2FE4
  add sp, 0x0002

  mov [0x37E6], ax  ; ds:[0x37E6] = return from sub_2FE4
  or ax, ax
  jne .loc_0375

.loc_375:
  push ds
  push word [0x37E6] ;            ds:[37E6]=4CB0
  push word [bp-02] ; File size (high)
  push word [bp-04] ; File size (low)
  push word [bp-06] ; File Handle
  call read_file_to_buffer
  add  sp,000A

  push word [bp-06]
  call dos_close_file
  add  sp,0002

  ; Check buffer of file read 
  mov  bx,[37E6]              ; ds:[37E6]=4CB0
  mov  ax,[bx+04]   ; 4th byte in ARCHTYPE
  add  ax,bx
  mov  [3C84],ax    ; Put it in another buffer

  sub  ax,ax
  push ax
  mov ax, 0x00B6 ; "HDSPCT"
  call dos_open_file
  add sp, 0x0004
  mov [bp-06], ax ; Save handle
  or ax, ax
  jne .loc_03B5
  call sub_081 ; ? error handler?
.loc_03B5:

  mov  word [bp-04],049C      ss:[4826]=049C
  mov  word [bp-02],0000      ss:[4828]=0000
  mov  ax,37E8
  push ds
  push ax
  mov  ax,049C
  cwd
  push dx
  push ax
  push word [bp-06] ; handle
  call read_file_to_buffer
  add sp, 0x000A
  push word [bp-06] ; handle
  call dos_close_file
  add sp, 0x0002
  mov sp, bp
  pop bp
  ret

sub_1439:
  push si
  push di
  push bx
  push cx
  push dx
  push ds
  push es
  push bp
  ; Allocate 64k ?
  mov ah, 0x48
  mov bx, 0xFFFF
  int 0x21

  cmp byte [0x0424], 0x01 ; ?
  jne .loc_1453

.loc_1453:
; Allocate again, this time as much memory as possible.
  mov ah, 0x48
  int 0x21
  jc .loc_1424 ; if we still couldn't allocate

  mov [0x0296], ax ; pointer to memory allocated?
  cmp byte [0x0424], 0x01
  jne .loc_146C

.loc_146C:
  push es
  mov ax, [0x0296] ; allocated memory pointer
  mov es, ax
  mov ah, 0x49
  int 0x21 ;   free memory
  pop es
  mov ah, 0x48
  mov bx, 0x45F5 ; 
  int 0x21 ; allocate again
  jc .loc_1424

  mov [0x042D], ax ; allocated memory pointer
  add ax, 0x07D0
  mov [0x029A], ax
  add ax, 0x043A
  mov [0x0292], ax
  add ax, 0x01CA
  mov [0x0296], ax
  call 0x05B0:0014 ; Video hardware?

  call sub_0090

; Takes a pointer as an argument
sub_14B3:
  push bp
  mov  bp,sp
  push si
  push di
  push ds
  push es
  push bp
  mov  si,[bp+04]        ; First argument (pointer)
  mov  ax,[si]           ; First word of pointer
  mov  di,[si+04]        ; 3rd word of pointer
  mov  cx,[si+06]        ; 4th word of pointer
  mov  si,[si+02]        ; 2nd word of pointer
  call 0x05B0:00B0
  pop bp
  pop es
  pop  ds
  pop  di
  pop  si
  pop  bp
  ret

sub_14D5:
  push bp
  mov  bp,sp
  push si
  push di
  push ds
  push es
  push bp
  mov  si,[bp+04]  ; 29E ?
  push si
  call sub_14B3
  add sp, 0x0002
  call 0x05B0:0x034D
  pop  bp
  pop  es
  pop  ds
  pop  di
  pop  si
  pop  bp
  ret

; 18B8 - Opens a file and returns the file handle (int) into AX
dos_open_file:
  push bp    ; save base pointer
  mov bp, sp ; establish new stack frame

  mov ah, 0x3D ; Open File using Handle
  mov al, [bp+06] ; Open Access mode (00 - read only, 01 - write only, 02 - read/write)
  mov dx, [bp+04] ; DS:DX pointer to ASCIIZ file name
  int 21

  jnc .loc_18C9   ; No error
  xor ax, ax      ; return 0 into AX
.loc_18C9:
  pop bp
  ret

; 19A2 (takes multiple arguments)
sub_19A2:
  push bp
  mov bp, sp

  mov ah, 0x42
  mov al, [bp+0x0A] // 5th argument?
  mov bx, [bp+0x04] // 1st argument (file handle)
  mov cx, [bp+0x08] // Seek high
  mov dx, [bp+0x06] // Seek low
  int 0x21   ; Seeks to CX:DX in file handle starting at AL

  jnc .loc_19BC ; no error occurred
  mov ax, 0xFFFF
  mov dx, ax
.loc_19BC:
  pop bp
  ret

; 19BE (takes 1 argument, file handle)
; Gets file size.
dos_get_file_size:
  push bp
  mov bp, sp

  ; Move file pointer using handle
  mov bx, [bp+04]
  sub cx, cx ; (high) number of bytes to move
  sub dx, dx ; (low) number of bytes to move
  mov ax, 0x4202   ; SEEK_END
  int 21

  ; AX = error code if CF set
  ; DX:AX = new pointer location if CF not set
  push dx
  push ax
  sub dx, dx
  mov ax, 0x4200 ; SEEK_SET
  int 0x21
  pop ax
  pop bx
  pop bp
  ret

; 01EF:18F1 ; close file
dos_close_file:
  push bp
  mov bp, sp

  mov ah, 0x3E ; close file
  mov bx, [bp+04] ; first argument
  int 0x21

  xor ax, ax
  jc .loc_1900
  inc ax
.loc_1900:
  pop bp
  ret


; 01EF:1902
read_file_to_buffer:
  push bp
  mov bp, sp
  push ds

  mov bx, [bp+04] ; first argument
  ; Pointer to buffer, set to ds:dx
  mov ax, [bp+0x0C] ; segment
  mov ds, ax  ;
  mov dx, [bp+0x0A] ; offset
  mov cx, [bp+6] ; second argument (file size)
  cmp word [bp+8], 0000   ; ?
  je .loc_191D
  mov cx, 0xFFFF ; Read max of 64k
.loc_191D:
  mov ah, 0x3F
  int 0x21  ; Read file into DS:DX
  jc .loc_194D ; Error occurred?
  cmp cx, ax ; Does the count match?
  jne .loc_194D ; count does not match
  clc
  sub [bp+06], cx
  sbb word [bp+08], 0000

  add dx, cx ; 
  jnc .loc_193B ; no issues?

  mov ax, ds
  add ax, 0x1000
  mov ds, ax

.loc_193B:
  cmp word [bp+08], 0000
  jne .loc_1911 ; read again?
  cmp word [bp+06], 0000
  jne .loc_1911 ; read again?

  mov ax, 0x0001
  pop ds
  pop bp
  ret

.loc_194D:
; error handling? unknown

; Decodes TPICT (decompression)
; This is very similar to the decompression used in fod_install
sub_19DA:
  cld   ; Clear direction flag
  mov ax, 0x6B2 ; Switch out extra segment to this
  mov es, ax
  push ds ;
  mov ds, [0x0DF6] ; switch DS to source segment
  xor si, si
  mov bx, [si]      ;   bx = ds:[si]      0x7D00
  mov dx, [si+0x02] ;   dx = ds:[si+2]    0x0000
  pop ds
  mov [0xDF2], bx
  mov [0xDF4], dx

  mov di, 0x0DFB
  mov cx, 0x0808
  mov ax, 0x2020

  ; Store 0x2020 (0x808 times) starting at ES:DI (0x6B2:0xDFB)
  repe stosw  ; 0x808 times store AX at address ES:DI

  mov si, 0x0004
  xor di, di
  mov es, [0x0DF8] ; Switch ES to destination segment
  mov cx, [0x0DF2] ; Count
  mov dx, [0x0DF4] ; ?

  mov [0x0DEE], cx
  mov [0x0DF0], dx
  mov bx, 0x0FEE
  mov byte [0x0DFA], 0x01

  ; Window is 64x64?
.loc_1A21:
  cmp si, 0x0040
  jc .loc_1A2E

  sub si, 0x0040
  add word [0x0DF6], 0x0004 ; ? source segment

.loc_1A2E:
  cmp di, 0x0040
  jc .loc_1A3F

  sub di, 0x0040
  add word [0x0DF8], 0x0004 ; ? dest segment
  mov es, [0x0DF8]

.loc_1A3F
  dec byte [0x0DFA]    ; See above
  jnz .loc_1A53

  push ds
  mov ds, [0x0DF6] ; source segment
  lodsb    ; al = ds:[si]  si++;

  mov ah, al
  pop ds

  mov byte [0x0DFA], 0x08 ; New counter

.loc_1A53:
  shr ah, 1    ; ah = ah >> 1;
  jnc .loc_1A78

  push ds
  mov ds, [0x0DF6] ; source segment
  lodsb     ; al = ds:[si] si++;
  stosb     ; es:[di] = al
  pop ds

  sub cx, 0x0001 ; cx = cx - 1 (did we underflow?)
  sbb dx, 0x0000 ; dx = dx - cf
  or dx, dx
  jne .loc_1A6D

  or cx, cx
  jz .loc_1AC6   ; Exit

.loc_1A6D:
  mov [bx+0x0DFB], al ; Store in dictionary?
  inc bx
  and bx, 0x0FFF
  jmp short .loc_1A21

.loc_1A78:
  mov bp, cx
  push ds
  mov ds, [0x0DF6] ; Source segment
  lodsb     ; al = ds:[si] si++;
  mov cl, al
  lodsb     ; al = ds:[si] si++;
  pop ds
  push si
  mov ch, al
  and al, 0x0f
  add al, 0x03

  shr ch, 1
  shr ch, 1
  shr ch, 1
  shr ch, 1

  mov si, cx
  mov cl, al
  xor ch, ch

.loc_1A99:
  mov al, [si+0x0DFB]  ; dictionary lookup?
  stosb     ; es:[di] = al
  inc si
  and si, 0x0FFF
  sub bp, 0x0001
  sbb dx, 0x0000
  or dx, dx
  jne .loc_1AB5
  or bp, bp
  jne .loc_1AB5

  pop si
  jmp short .loc_1AC6
  nop

.loc_1AB5:
  mov [bx+0x0DFB], al
  inc bx
  and bx, 0x0FFF
  loop .loc_1A99
  mov cx, bp
  pop si
  jmp .loc_1A21

.loc_1AC6:
  ret

sub_1AC7:
  push bp
  mov  bp,sp
  push si
  push di
  push bx
  push cx
  push dx
  push ds
  push es
  push bp
  mov  ax,[bp+06]    ; Source segment
  mov  [0x0DF6],ax   ; Save it
  mov  ax,[bp+0x0A]  ; Destination segment
  mov  [0x0DF8],ax   ; Save it
  call sub_19DA
  pop  bp
  pop  es
  pop  ds
  pop  dx
  pop  cx
  pop  bx
  pop  di
  pop  si
  pop  bp
  ret




;
sub_1B9E:
; Get MS-DOS Version Number
  mov  ah,0x30
  int  0x21
  mov  [0x1E85],ax  ; Store MS DOS version

  ; Get interrupt vector (and save it)
  mov  ax,0x3500
  int  0x21
  mov  [0x1E71],bx              ; ds:[1E71]=0000
  mov  [0x1E73],es              ; ds:[1E73]=0000

  ; Set interrupt vector
  push cs
  pop  ds
  mov  ax,0x2500
  mov  dx,0x1B80
  int  0x21
  push ss
  pop ds
  mov cx [0x2152]




sub_2FE4:
  push bp
  mov bp, sp
  push si
  push di
  mov bx, 0x2000
  cmp word [bx], 0x0000
  jne .loc_301A

  push ds
  pop es
  mov ax, 0x0005
  call sub_316C

.loc_301A:
