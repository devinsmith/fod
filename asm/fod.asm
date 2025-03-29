; A rough disassembly of FOD
; Mostly appears to be C based using cdecl caller conventions
; However there is some inline assembly that is invoked as well.

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
  mov ax, 0x03E8   ; 1000 bytes
  push ax
  call malloc ; possibly memory allocation
  add sp, 0x0002

  mov [0x3E66], ax  ; memory allocation pointer
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

  ; 0x29E is an offset to a struct in the EXE
  mov ax, 0x029E
  push ax
  call sub_14D5 ; Show title screen? TPICT?
  add sp, 0x0002

  call sub_56B ; Wait for key press (and does some other things) ?
  call sub_1548

  mov ax, 0x029E
  push ax
  call sub_14D5  ; Shows a screen (border screen)?
  add sp, 0x0002

  sub ax, ax
  push ax
  mov ax 0x0302 ; ?
  push ax
  call sub_155E
  add sp, 0x0004

  jmp .loc_13D7

call 0x141A ; ?

call 0x39FE ; inner loop?


; sub_090 at 01EF:0090
sub_090:
  push bp
  mov  bp,sp
  sub  sp,0x0008
  sub  ax,ax
  push ax
  mov  ax, 0x009A    ; DS:[0x9A] = "FONT"
  push ax
  call dos_open_file
  add  sp,0004
  mov  [bp-08],ax    ; file handle
  or   ax,ax
  jne  000000AD ($+3)         (down)
  call 00000081 ($-2c)
  lea  ax,[bp-02]             ss:[4826]=2485
  push ss
  push ax
  mov ax, 0002    ; reads 2 bytes of FONT
  cwd
  push dx
  push ax
  push word [bp-08] ; file handle
  call sub_1902
  add sp, 0x000A

  mov ax, [bp-02]  ; read first 2 bytes.
  mov cl, 0x05
  shl ax, cl
  push ax
  call sub_2FE4 ; memory allocate
  add sp, 2



  ; 3bfa



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
  call read_file_to_buffer
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
  call sub_19A2 ; seek to position
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
  call sub_2FE4    ; memory allocation
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
  add  sp, 0x000A

  push word [bp-06]
  call dos_close_file
  add  sp,0002

  ; Check buffer of file read 
  mov  bx,[0x37E6]              ; data of ARCHTYPE
  mov  ax,[bx+04]   ; 4th byte in ARCHTYPE
  add  ax,bx
  mov  [0x3C84],ax    ; First ARCHTYPE record ?

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

  mov  word [bp-04],0x049C  ; file size of HDSPCT
  mov  word [bp-02],0000    ; high value
  mov  ax, 0x37E8
  push ds
  push ax
  mov  ax, 0x049C
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

sub_44E:
  push bp
  mov bp, sp
  sub sb, 0x0006
  mov word [0x2310], 0x031E
  mov ax, [0x35E4]
  cmp [bp+0x04], ax
  je .loc_4E0

  mov ax, [bp+0x04]
  mov [0x35E4], ax
  mov ax, 0x8000
  push ax
  mov ax, 0x00C3  ; 'GANI'
  push ax
  call dos_open_file
  add sp, 0x0004
  mov [bp-0x06], ax ; file handle for GANI
  push ax
  call sub_19BE  ; get GANI file size
  add sp, 0x0002
  mov [bp-0x04], ax
  mov [bp-0x02], dx

  ; Read into buffer
  push word [0x0292]
  push word [0x0290]
  push dx
  push ax
  push word [bp-0x06]
  call sub_1902
  add sp, 0x000A

  push word [bp-0x06]
  call sub_18F1
  add sp, 0x0002

; 0x01ED:04A2

  push word [029A]            ds:[029A]=1E81
  push word [0298]            ds:[0298]=0000
  push word [0x0292]  ; Segment
  push word [0x0290]  ; offset of Gani
  call sub_1AC7       ; Decompress !
  add  sp, 0x0008

01ED:04B8  A19802              mov  ax,[0298]              ds:[0298]=0000
01ED:04BB  8B169A02            mov  dx,[029A]              ds:[029A]=1E81
01ED:04BF  A31423              mov  [2314],ax              ds:[2314]=0000
01ED:04C2  89161623            mov  [2316],dx              ds:[2316]=0000
01ED:04C6  C41E9802            les  bx,[0298]              ds:[0298]=0000
01ED:04CA  268A6702            mov  ah,es:[bx+02]          es:[0007]=4D00
01ED:04CE  2AC0                sub  al,al
01ED:04D0  268A4F01            mov  cl,es:[bx+01]          es:[0001]=1089
01ED:04D4  2AED                sub  ch,ch
01ED:04D6  03C1                add  ax,cx                                      
01ED:04D8  A31223              mov  [2312],ax              ds:[2312]=0000
01ED:04DB  C6061A2301          mov  byte [231A],01         ds:[231A]=0000
01ED:04E0  C7061C230100        mov  word [231C],0001       ds:[231C]=0000
01ED:04E6  8BE5                mov  sp,bp
01ED:04E8  5D                  pop  bp
  ret

; 0x4EA (takes 1 argument)
sub_4EA:
01ED:04EA  55                  push bp
01ED:04EB  8BEC                mov  bp,sp
01ED:04ED  83EC0E              sub  sp,000E
01ED:04F0  A11423              mov  ax,[2314]              ds:[2314]=0000
01ED:04F3  8B161623            mov  dx,[2316]              ds:[2316]=1E81
01ED:04F7  8946FA              mov  [bp-06],ax             ss:[4830]=4836
01ED:04FA  8956FC              mov  [bp-04],dx             ss:[4832]=073C
01ED:04FD  03061223            add  ax,[2312]              ds:[2312]=1089
01ED:0501  8946F4              mov  [bp-0C],ax             ss:[482A]=0005
01ED:0504  8956F6              mov  [bp-0A],dx             ss:[482C]=046A



; 0x56B - Wait for key
sub_56B:
  push bp
  mov bp, sp
  sub sb, 0x0006
  mov byte [bp-06], 0x00

  jmp short .loc_57A
.loc_577:
  call sub_1729
.loc_57A:
  call sub_330C   ; Check keyboard input buffer
  or ax, ax
  je .loc_577
  call sub_3320  ; Read key
  mov [bp-0x04], al  ; key read.
  cbw
  or ax, ax
  je .loc_59B
  cmp ax, 0x000D ; Enter key?
  je .loc_603
  cmp ax, 0x001B ; Backspace?
  je .loc_5FD
  mov [bp-0x06], al
  jmp short .loc_607
.loc_59B:   ; Enter key was pressed
  ; XXX Document

.loc_607:
  cmp byte [bp-0x06] 0x00 ; No key?
  jne .loc_610
  jmp .loc_67A
  mov al, [bp-0x06] ; al contains key
  cbw
  mov sp, bp
  pop bp
  ret

sub_71B:
  push bp
  mov bp, sp
  mov ax, 0x0001
  push ax
  mov ax, 0x031E
  push ax
  call sub_155E
  add sp, 0x0004

  push word [bp+0x04]
  call sub_44E
  add sp, 0x0002

  mov ax, 0x0001
  push ax
  call sub_4EA
  add sp, 0x0002

sub_1262:
  mov ax, 0x01CC
  push ax
  mov ax, 0x0033
  push ax
  call sub_71B
  add sp, 0x0004

01ED:12CA  E89EF2              call 0000056B ($-d62)
01ED:12CD  8846F8              mov  [bp-08],al       ; key pressed
01ED:12D0  98                  cbw                                             
01ED:12D1  8BD8                mov  bx,ax
01ED:12D3  F687272002          test byte [bx+2027],02      ds:[2027]=2020
01ED:12D8  7404                je   000012DE ($+4)         (no jmp)
01ED:12DA  2C20                sub  al,20     ; convert to upper case
01ED:12DC  EB03                jmp  short 000012E1 ($+3)   (down)

; 134C - 'R'emove a member
01ED:134C  803E4F2300          cmp  byte [234F],00         ds:[234F]=0000
01ED:1351  7408                je   0000135B ($+8)         (down)
01ED:1353  FF76F0              push word [bp-10]           ss:[4840]=0000
01ED:1356  E82AFD              call 00001083 ($-2d6)
01ED:1359  EBE3                jmp  short 0000133E ($-1d)  (up)
01ED:135B  B82702              mov  ax,0227
01ED:135E  50                  push ax
01ED:135F  E802FE              call 00001164 ($-1fe)
01ED:1362  83C402              add  sp,0002
01ED:1365  EB70                jmp  short 000013D7 ($+70)  (down)


; 1391 - 'P'lay the game
.loc_1391:
  cmp  byte [234F],00         ds:[234F]=0000      
  je   0000139E ($+6)         (down)
01ED:1398  E845F0              call 000003E0 ($-fbb)
01ED:139B  E97DFE              jmp  0000121B ($-183)       (up)
.loc_139E:
  call 00001593 
  mov  ax,0001
  push ax
  mov  ax,0233   ; 'It's tough out there!'
  push ax
  call 00000010 ($-139c)
  add sp, 0x0004
  mov ax, 0x0003
  push ax
  mov ax, 0x0249   ; 'You should take somebody with you'
  add sp, 0x0004
  call sub_1631
  call sub_56B   ; Wait key?
  jmp short .loc_13D7


loc_13D7:
  cmp byte [bp-0x0A], 0x00
  jne .loc_13E0
  jmp sub_1262
.loc_13E0:

loc_1424:
  cli
  mov al, 3
  mov ah, 0
  int 0x10  ; VIDEO - Set Video Mode (text)

  mov dx, 0x340 ;  "You do not have enough memory to run Fountain of Dreams.$"
  mov ah, 9
  int 0x21      ; DOS - Print String
                ; DS:DX -> string terminated by "$".

  mov al, 0     ; Exit code
  mov ah, 0x4c
  int 0x21      ; DOS - 2+ - Quit with exit code (EXIT)
                ; AL = exit code
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
  ; Allocate around 1 MB
  mov ah, 0x48
  mov bx, 0xFFFF ; 64k paragraphs (65535 * 16) = 1048560 bytes
  int 0x21

  cmp byte [0x0424], 0x01 ; ?
  jne .loc_1453

.loc_1453:
; Allocate again, this time as much memory as possible.
  mov ah, 0x48
  int 0x21
  jc loc_1424 ; if we still couldn't allocate

  mov [0x0296], ax ; pointer to memory allocated?
  cmp byte [0x0424], 0x01
  jne .loc_146C

.loc_146C:
  push es
  mov ax, [0x0296] ; allocated memory pointer
  mov es, ax
  mov ah, 0x49
  int 0x21 ;   free memory
           ;   ES = segment address of area to be freed

  pop es
  mov ah, 0x48
  mov bx, 0x45F5 ; 286544 bytes
  int 0x21 ; allocate again
  jc loc_1424

  ; Save some offsets that are used later.
  mov [0x042D], ax ; allocated memory pointer
  add ax, 0x07D0   ; 2000 into allocated memory
  mov [0x029A], ax ; save this offset
  add ax, 0x043A   ; 1082 into the allocated memory
  mov [0x0292], ax ; save offset
  add ax, 0x01CA   ; 458 into allocated memory
  mov [0x0296], ax ; save offset
  call seg001:0014 ; Video hardware?

  call sub_0090

  ; 0 out 16000 bytes
  mov  cx, 0x3E80 ; 16000
  xor  di,di
  mov  es,[0x042D]
  xor  ax,ax
  repe stosw
  pop  bp
  pop  es
  pop  ds
  pop  dx
  pop cx
  pop bx
  pop di
  pop si
  ret

; Takes a pointer as an argument
sub_14B3:
  push bp
  mov  bp,sp
  push si
  push di
  push ds
  push es
  push bp

  ; Sets up various offsets based on inputs
  mov  si,[bp+04]        ; First argument (pointer to a strucut)
  mov  ax,[si]           ; First word of struct
  mov  di,[si+04]        ; 3rd word of struct (0xA0)
  mov  cx,[si+06]        ; 4th word of struct (0xC8)
  mov  si,[si+02]        ; 2nd word of struct
  call seg001:00B0

  pop bp
  pop es
  pop ds
  pop di
  pop si
  pop bp
  ret

; 0x14D5
; Takes 1 argument
sub_14D5:
  push bp
  mov  bp,sp
  push si
  push di
  push ds
  push es
  push bp
  mov  si,[bp+04]  ; first argument (29E) ?
  push si
  call sub_14B3
  add sp, 0x0002
  call seg001:0x034D
  pop  bp
  pop  es
  pop  ds
  pop  di
  pop  si
  pop  bp
  ret

; sub_14FF
; something 40x25   (0x28 by 0x19)
; cpu.ax is an input (usually 0?)
sub_14FF:
  push di
  push si
  push es
  mov si. [0x3E66] ; data from "Borders" file?
  add si, ax       ; offset

  mov ax. [0x35E0]
  push ax
  mov ax [0x35E2]
  push ax
  mov word [0x35E2], 0x0000
  mov word [0x35E0], 0x0000
.loc_151C:
  lodsb     ; al = ds:[si]  si++;
  or al, al  ; is al = 0?
  je .loc_1526
  push si
  call sub_1778 ; todo
  pop si
.loc_1526:
  inc word [0x35E0]
  cmp word [0x35E0], 0x0028 ; 40
  jne .loc_151C

  inc word [0x35E2]
  cmp word [0x35E2], 0x0019 ; 25
  jne .loc_1516

  pop ax
  mov [0x3E52], ax
  pop ax
  mov [0x3E50], ax
  pop es
  pop si
  pop di
  ret

; sub_1548 (no arguments)
sub_1548:
  push si
  push di
  push bx
  push cx
  push dx
  push ds
  push es
  push bp
  xor ax, ax
  call sub_14FF
  pop bp
  pop es
  pop ds
  pop dx
  pop cx
  pop bx
  pop di
  pop si
  ret

; 0x155E
; 2 arguments
sub_155E:
  push bp
  mov bp, sp
  push si
  push di
  push bx
  push cx
  push dx
  push ds
  push es
  push bp
  mov si, [bp+0x04] ; First argument
  mov [0x029C], si
  cmp word [bp+0x06], 0x0000 ; second argument
  je .loc_1589

  push si
  mov word [si+0x18], 0x0000
  call sub_1593
  pop si
  cmp word [si+0x16], 0x0000
  je .loc_1589
  call near word [si+0x16] ; function pointer

.loc_1589:
  pop bp
  pop es
  pop ds
  pop dx
  pop cx
  pop bx
  pop di
  pop si
  pop bp
  ret

; 0x1593
sub_1593:
  mov si, [0x029C] ; first argument from earlier?
  add si, 0x000C
  call sub_17C4
  ret

; sub_1729
sub_1729:
  push si
  push di
  push bx
  push cx
  push dx
  push ds
  push es
  push bp
  call sub_173D
  pop bp
  pop es
  pop ds
  pop dx
  pop cx
  pop bx
  pop di
  pop si
  ret


; sub_173D
; Shuffles bytes around?
sub_173D:
  lea si, [0x27E]
  mov cx, 0x000F
  mov dl, [si+0x0F]
  mov bx, 0x000E
  clc
.loc_174B:
  mov al, [bx+si]
  adc dl, al
  mov [bx+si], dl
  dec bx
  loop .loc_174B

  mov cx, 0x0010
  mov bx, 0x0010
.loc_175A:
  dec bx
  inc byte [bx+si]
  loopne .loc_175A

  mov al, [si]
  ret

; Process bytes (another form of decompression?)
sub_1778:
  push si
  push di
  push ds
  push es
  push bp
  push ax

  mov ax, [0x35E2]

  shl ax, 1
  shl ax, 1
  shl ax, 1
  shl ax, 1

  mov di, ax
  mov di, [di+0x05BF] ; lookup table

  mov ax, [0x35E0]

  shl ax, 1
  shl ax, 1

  add di, ax

  mov es, [0x042D] ; destination location

  pop ax  ; byte

  xor ah, ah
  shl ax, 1
  shl ax, 1
  shl ax, 1
  shl ax, 1
  shl ax, 1

  mov si, [0x3C86] ; ?
  add si, ax

  mov bx, 0x0008
.loc_17B2:
  mov cx, 0x0002

  repe movsw ; copy words from ds:si to es:di (2 times) 
  add di, 0x009C
  dec bx

  jne .loc_17B2

  pop bp
  pop es
  pop ds
  pop di
  pop si

  ret

sub_17C4:
  push si
  push di
  push es
  push bp
  mov es, [0x042D]
  mov di, [si+0x02]
  shl di, 1
  mov di, [di+0x05BF]
  add di, [si]
  xor ax, ax
  mov cx, [si+0x06]
.loc_17DC:
  push cx
  push di
  mov cx, [si+0x04]
  sar cx, 1

  repe stosw  ; CX times store AX at address ES:DI
  pop di
  add di, 0x00A0 ; 160
  pop cx
  loop .loc_17DC
  pop bp
  pop es
  pop di
  pop si
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



; Takes 1 argument, this might be a malloc or memory allocation
; function
; 0x2FE4
malloc:
  push bp
  mov bp, sp
  push si
  push di

  ; What's at data segment 0x2000
  mov bx, 0x2000
  cmp word [bx], 0x0000
  jne .loc_301A

  push ds
  pop es
  mov ax, 0x0005
  call sub_316C
  jne .loc_3000
  xor ax, ax
  cwd
  jmp short .loc_3024
.loc_3000:
  inc  ax
  and  al,FE
  mov  [0x2000],ax
  mov  [0x2002],ax
  xchg si,ax
  mov  word [si],0001     ; Mark allocated?
  add si, 4
  mov [si-02], 0xFFFE
  mov [0x2006], si
.loc_301A:
  mov cx, [bp+04] ; first argument (amount allocated?)
  mov ax, ds
  mov es, ax
  call sub_302D
  pop di
  pop si
  mov sb, bp
  pop bp
  ret

sub_302D:
  inc cx
  je .loc_302A
  and cl, 0xFE
  cmp cx, 0xFFEE
  jnc .loc_302A
  mov si, [bx+2] ;
  cld
  lodsw   ; ax = ds:[si]  si += 2;
  mov di, si
  test al, 0x01
  je .loc_3085
  dec ax
  cmp ax, cx    ; memory desired
  jnc .loc_305D
  mov dx, ax
  add si, ax
  lodsw   ;
  test al, 0x01
  je .loc_3085

.loc_3085:
  mov byte es:[0x2014], 2
.loc_308B:
  cmp ax, 0xFFFE
  je .loc_30B5

.loc_30B5:
  mov  ax,[bx+08]             ds:[2008]=0000
  or   ax,ax
  je   .loc_30C0
  mov  ds,ax
  jmp  short 000030D4 ($+14)  (down)
.loc_30C0:
  dec  byte es:[2014]         es:[2014]=0002
  je   .loc_30D8
  mov  ax,ds
  mov  di,ss
  cmp  ax,di
  je .loc_30D4
  mov ds, es:[0x200A]
.loc_30D4:
  mov si, [bx]
  jmp short .loc_3094
.loc_30D8:
  mov si, [bx+06]
  xor ax, ax
  call sub_314A
  cmp ax, si
  je .loc_30F1
  and al, 0x01
  inc ax
  inc ax
  cbw
  call sub_314A
  je .loc_30FB
  dec byte [di-02]
  call sub_3110

  01EF:30F4  7405                je   000030FB ($+5)         (no jmp)
01EF:30F6  96                  xchg si,ax
01EF:30F7  4E                  dec  si
01EF:30F8  4E                  dec  si
01EF:30F9  EB99                jmp  short 00003094 ($-67)  (up)
01EF:30FB  8CD8                mov  ax,ds
01EF:30FD  8CD1                mov  cx,ss


01EF:3110  51                  push cx
01EF:3111  8B45FE              mov  ax,[di-02]             ds:[48C0]=0002
01EF:3114  A801                test al,01
01EF:3116  7403                je   0000311B ($+3)         (no jmp)
01EF:3118  2BC8                sub  cx,ax
01EF:311A  49                  dec  cx
01EF:311B  41                  inc  cx
01EF:311C  41                  inc  cx
01EF:311D  BAFF7F              mov  dx,7FFF
01EF:3120  263B161020          cmp  dx,es:[2010]           es:[2010]=2000


sub_314A:
  push dx
  push cx
  call sub_316C
  je   .loc_3169
  push di
  mov  di,si
  mov  si,ax
  add  si,dx
  mov  word [si-02],FFFE      ds:[48C0]=FFFE
  mov  [bx+06],si             ds:[2006]=48C2
  mov  dx,si
  sub  dx,di
  dec  dx
  mov  [di-02],dx             ds:[48C0]=FFFE
  pop  ax

.loc_3169:
  pop cx
  pop dx
  ret


sub_316C:
  push bx
  push ax
  xor  dx,dx
  push ds
  push dx
  push dx
  push ax
  mov  ax,0001
  push ax
  push es
  pop ds
  call sub_318C
  add sp, 0x0008
  cmp dx, 0xFFFF
  pop ds
  pop dx
  pop bx
  je .loc_318A
  or dx, dx
.loc_318A:
  ret


sub_318C:
  push bp
  mov  bp,sp
  push si
  push di
  push es
  cmp  word [bp+08],0000   ; 3rd argument
  jne  .loc_31D0
  mov  di,1E12
  mov  dx,[bp+06]          ; 2nd argument
  mov  ax,[bp+04]          ; 1st argument
  dec  ax
  jne  .loc_31AB
  call sub_31FA
  jc   .loc_31D0
  jmp  short .loc_31F3
.loc_31AB:

.loc_31F3:
  pop es
  pop di
  pop si
  mov sb, bp
  pop bp
  ret

sub_31FA:
  mov  cx,[bp+0C]
  mov  si,di
  cmp  [si+02],cx             ds:[0084]=0515
  je   00003210 ($+c)         (down)
01EF:3204  83C604              add  si,0004
01EF:3207  81FE621E            cmp  si,1E62
01EF:320B  75F2                jne  000031FF ($-e)         (no jmp)
01EF:320D  F9                  stc
01EF:320E  EB3F                jmp  short 0000324F ($+3f)  (down)
.loc_3210:
  mov  bx,dx
  add bx, [si]
  jc .loc_324F
  mov dx, bx
  mov es, cx
  cmp si, di
  jne .loc_3224
  cmp [0x1E0C], bx
  jnc .loc_324A
.loc_3224:
  add bx, 0x000F

.loc_324A:
  xchg dx, ax
  xchg [si], ax
  mov dx, cx
  ret



; 0x3320 - Checks input key buffer.
; Also checks memory at 0x2128 ?
sub_330C:
  mov ax, [0x2128] ; key buffer
  or ah, ah
  mov al, 0xFF
  je .loc_331B

  ; Check input key buffer
  ; After return AL register will contain 0x00 (empty) or 0xFF (not-empty)
  mov ah, 0x0B
  int 0x21
  mov ah, 0x00

.loc_331B:
  ret

; Reads a key with echo.
  mov dh, 1    ; With echo
  jmp short loc_3322

; Read a key (either without echo or with echo)
sub_3320:
  mov dh, 0x08  ; Without echo
loc_3322:
  mov ax, [0x2128] ; ?
  or ah, ah
  jne .loc_3331
  mov word [0x2128], 0xFFFF ; ?
  jmp short .loc_3336

  ; Read a key (echo depends on above), if no input, waits for a key.
  ; AL contains key code.
  xchg dx, ax
  int 0x21
  mov ah, 0x00
  ret

