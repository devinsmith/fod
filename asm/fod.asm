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
; 1191 is likely main and not game loop, as it receives argc, argv[], env
; call 1191 - Game loop.

; 01EF:1B6D

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

  call 0x18B8 ; Opens TPICT
call 0x19BE ;  ?
call 0x1902 ; ?

call 0x18F1 ; Closes file

jmp 0x13D7

call 0x1AC7 ; Sets up VGA?

call 0x14D5 ; Show title screen? TPICT?

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

.loc_194D:


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
