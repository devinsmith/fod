; Code Segment: 0x05B0

sub_0014:
  push ax
  push bx
  push cx
  push dx
  push si
  push di
  push ds
  push es
  mov  word [0x0CCE],0x074F       ds:[0CCE]=074F
  mov  word [0x0CCC],0x0000       ds:[0CCC]=0000
  mov  ax,0x06B2
  mov  es,ax


  mov  di, 0x05BF  ; Lookup table?

  ; Store sequence of +160, 200 times
  ; Lookup table?
  xor  ax,ax
  mov  cx, 0x00C8 ; 200
.loc_35
  stosw ; put ax into ds:di, advance di by 2.
  add  ax,0x00A0 ; 160
  loop .loc_35

  xor bx, bx
  mov bl, [0x0424]   ; 0x0003   ; Saved game value?
  shl bl, 1
  mov bx, [bx+0x041C]     ; ds:0x02A9
  call near bx
  pop es
  pop ds
  pop di
  pop si
  pop dx
  pop cx
  pop bx
  pop ax
  retf

sub_B0:
  mov  bx,[0x0CCE]     ; pointer to struct
  add  word [0x0CCE], 0x000E       ds:[0CCE]=074F
  inc  word [0CCC]     ; counter
  xor  ah,ah
  test ax,0001
  je   .loc_C5
  inc  di
.loc_C5:
  test di,0001
  je   .loc_CC         (no jmp)
  inc  di
.loc_CC:
  mov  dx,00A0
  sub  dx,di
  mov  [bx+0C],dx             ds:[075B]=0000
  and  ax,00FE

  sar  ax,1
  sar  di,1
  mov  [bx+08],cx             ds:[0757]=0000
  mov  [bx+0A],di             ds:[0759]=0000

05B0:00E1  894702              mov  [bx+02],ax             ds:[0751]=0000      
05B0:00E4  D1E6                shl  si,1
05B0:00E6  BF2F04              mov  di,042F
05B0:00E9  03FE                add  di,si
05B0:00EB  893F                mov  [bx],di                ds:[074F]=0000      
05B0:00ED  BFBF05              mov  di,05BF
05B0:00F0  03FE                add  di,si
05B0:00F2  8B35                mov  si,[di]                ds:[042F]=0000
05B0:00F4  D1E0                shl  ax,1
05B0:00F6  03F0                add  si,ax
05B0:00F8  897704              mov  [bx+04],si             ds:[0753]=0000
05B0:00FB  8B7F0A              mov  di,[bx+0A]             ds:[0759]=0050
05B0:00FE  D1E7                shl  di,1
05B0:0100  8B857A03            mov  ax,[di+037A]           ds:[03CA]=082C
05B0:0104  894706              mov  [bx+06],ax             ds:[0755]=0000
05B0:0107  CB                  retf



; Seems to be setting up the video hardware, as well as setting up some video
; support tables
; 0x2A9
sub_02A9:
  mov word cs:[0x0353], 0x0136 ; Self modifying code
  ; Switch to mode 13
  mov ax, 0x0013
  int 0x10
  cli

  mov  ax,0x0040
  mov  es,ax
  mov  ax,es:[0x0010]
  and  al,CF
  or   al,0x20
  mov  es:[0x0010],ax
  sti
  mov  ax,0x06B2
  mov es, ax
  mov si, 0x0CD2 ; color palette source
  mov cx, 0x0010 ; 16 colors
  xor bx, bx     ; Start at index 0
.loc_2D5:
  mov di, 0x0D02
  lodsb ; load byte at ds:si into al (inc si by 1)
  mov dh, al
  lodsw ; load word at ds:si into ax (inc si by 2)
  push cx
  mov cx, 0x0010

  ; Copy over to ES:DI (16 times)
.loc_2e0:
  mov es:[di], dh
  inc di
  stosw ; put ax into es:di, advance di by 2.
  loop .loc_2e0

  mov ax, 0x1012 ; AL = 12  set block of DAC color registers
                 ; BX = first color register to set
  mov cx, 0x0010 ; CX = number of color registers to set
  mov dx, 0x0D02 ; ES:DX = pointer to table of color values to set

  ; Set palette
  push bx
  push si
  push di
  int 0x10
  pop di
  pop si
  pop bx
  add bx, 0x0010
  pop cx
  loop .loc_2D5

  ; Putting line offsets into DS:0x042F
  mov di, 0x042F
  mov cx, 0x00C8 ; 200
  xor ax, ax
.loc_306:
  stosw ; put ax into ds:di, advance di by 2.
  add ax, 0x140 ; 320 pixels?
  loop .loc_306

  ; setting up another lookup table?
  mov di, 0x037A
  mov cx, 0x0051
  mov ax, 0x0B74 ; Starting value
.loc_315:
  stosw
  sub ax, 0x0015
  loop .loc_315

  mov ax, cs
  mov ds, ax
  mov es, ax

  ; copy code from CS:338 (see below) to cs:4E4
  mov si, 0x0338
  mov di, 0x04E4
  mov cx, 0x0015
  repe movsb ; copy 0x15 byte string from DS:SI to ES:DI

  ; Copy those 21 bytes (above), 1659 (0x67B) times
  ; Basically 79 additional copies of above 21 bytes
  mov cx, 0x067B
  mov si, 0x04E4
  repe movsb ; copy 0x67B byte string from ds:si to es:di

  mov al, 0xC3  ; Finally add a RET instruction.
  stosb ; put al into es:di
  ret

; 0x338
; This fragment of code (which is about 21 bytes) is copied to 0x4E4 80 times
  lodsw                 ; AX = DS:SI  SI += 2
  mov  ch,ah            ;
  mov  bx,ax
  rol  bx,1
  rol  bx,1
  rol  bx,1
  rol  bx,1
  mov  ah,bl
  stosw                ; store AX at address ES:(E)DI; di += 2;
  mov al, ch
  mov ah, bh
  stosw                ; store AX at address ES:(E)DI; di += 2;
  

sub_34D:
  push si
  push di
  push es
  push ds
  push bp
  call 0000048B ($+136)
  pop  bp
  pop  ds
  pop  es
  pop  di
  pop  si

sub_48B:
  mov  word [0x0CD0], 0x074F

  ; Prepare for VGA
  mov  ax,A000
  mov  es,ax
  cmp  word [0x0CCC],0000      ; ds:[0CCC]=0001
  jne  .loc_49E
  ret
.loc_49E:
  dec  word [0x0CCC]
  mov  bx,[0x0CD0]              ds:[0CD0]=074F
  mov  si,[bx+0x04]             ds:[0753]=0000
  mov  dx,[bx+0x0C]             ds:[075B]=0000
  mov  cx,[bx+0x08]             ds:[0757]=00C8   ; line count?

05B0:04AF  8B4706              mov  ax,[bx+06]             ds:[0755]=04E4
05B0:04B2  A3C70C              mov  [0CC7],ax              ds:[0CC7]=0000
05B0:04B5  8B4702              mov  ax,[bx+02]             ds:[0751]=0000
05B0:04B8  D1E0                shl  ax,1

05B0:04BA  D1E0                shl  ax,1
05B0:04BC  8B1F                mov  bx,[bx]                ds:[074F]=042F
05B0:04BE  8B2F                mov  bp,[bx]                ds:[074F]=042F
05B0:04C0  03E8                add  bp,ax
.loc_4C2:
05B0:04C2  8BFD                mov  di,bp
05B0:04C4  81C54001            add  bp,0140
05B0:04C8  A1C70C              mov  ax,[0CC7]              ds:[0CC7]=04E4
05B0:04CB  8E1E2D04            mov  ds,[042D]              ds:[042D]=16B3

  call near ax      ; 0x4E4 (line pointer)

  mov  ax,0x06B2
  mov  ds,ax
  add  si,dx   ; Skip pixel count (probably 0)
  dec  cl      ; line count ?
  jne 0x4C2

; Line function pointer
; Input is stored in DS
; DS = 16B3
; Massive unrolled loop?
sub_4E4:
; Repeats 80x times (to 0xB5F)
  lodsw                 ; AX = DS:SI  SI += 2
  mov  ch,ah            ;
  mov  bx,ax
  rol  bx,1
  rol  bx,1
  rol  bx,1
  rol  bx,1
  mov  ah,bl
  stosw                ; store AX at address ES:(E)DI; di += 2;
  mov al, ch
  mov ah, bh
  stosw                ; store AX at address ES:(E)DI; di += 2;

  ; 0x4F9
  lodsw
  mov  ch,ah            ;
  mov  bx,ax
  rol  bx,1
  rol  bx,1
  rol  bx,1
  rol  bx,1
  mov  ah,bl
  stosw                ; store AX at address ES:(E)DI; di += 2;
  mov al, ch
  mov ah, bh
  stosw                ; store AX at address ES:(E)DI; di += 2;

  lodsw
  mov  ch,ah            ;
  mov  bx,ax
  rol  bx,1
  rol  bx,1
  rol  bx,1
  rol  bx,1

  mov  ah,bl
  stosw                ; store AX at address ES:(E)DI; di += 2;
  mov al, ch
  mov ah, bh
  stosw                ; store AX at address ES:(E)DI; di += 2;

  lodsw
  mov  ch,ah            ;
  mov  bx,ax
  rol  bx,1
  rol  bx,1
  rol  bx,1
  rol  bx,1

  mov  ah,bl
  stosw                ; store AX at address ES:(E)DI; di += 2;
  mov al, ch
  mov ah, bh
  stosw                ; store AX at address ES:(E)DI; di += 2;

  ; 0x538
  lodsw
  mov  ch,ah            ;
  mov  bx,ax
  rol  bx,1
  rol  bx,1
  rol  bx,1
  rol  bx,1
  mov  ah,bl
  stosw                ; store AX at address ES:(E)DI; di += 2;
  mov al, ch
  mov ah, bh
  stosw                ; store AX at address ES:(E)DI; di += 2;

 ; 0x54D
  lodsw
  mov  ch,ah            ;
  mov  bx,ax
  rol  bx,1
  rol  bx,1
  rol  bx,1
  rol  bx,1
  mov  ah,bl
  stosw                ; store AX at address ES:(E)DI; di += 2;
  mov al, ch
  mov ah, bh
  stosw                ; store AX at address ES:(E)DI; di += 2;

  ; 0x562
  lodsw
  mov  ch,ah            ;
  mov  bx,ax
  rol  bx,1
  rol  bx,1
  rol  bx,1
  rol  bx,1
  mov  ah,bl
  stosw                ; store AX at address ES:(E)DI; di += 2;
  mov al, ch
  mov ah, bh
  stosw                ; store AX at address ES:(E)DI; di += 2;

  ; 0x577
  lodsw
  mov  ch,ah            ;
  mov  bx,ax
  rol  bx,1
  rol  bx,1
  rol  bx,1
  rol  bx,1
  mov  ah,bl
  stosw                ; store AX at address ES:(E)DI; di += 2;
  mov al, ch
  mov ah, bh
  stosw                ; store AX at address ES:(E)DI; di += 2;

  ; 0x58C
  lodsw
  mov  ch,ah            ;
  mov  bx,ax
  rol  bx,1
  rol  bx,1
  rol  bx,1
  rol  bx,1
  mov  ah,bl
  stosw                ; store AX at address ES:(E)DI; di += 2;
  mov al, ch
  mov ah, bh
  stosw                ; store AX at address ES:(E)DI; di += 2;

  ; 0x5A1
  lodsw
  mov  ch,ah            ;
  mov  bx,ax
  rol  bx,1
  rol  bx,1
  rol  bx,1
  rol  bx,1
  mov  ah,bl
  stosw                ; store AX at address ES:(E)DI; di += 2;
  mov al, ch
  mov ah, bh
  stosw                ; store AX at address ES:(E)DI; di += 2;

  ; 0x5B6
  lodsw
  mov  ch,ah            ;
  mov  bx,ax
  rol  bx,1
  rol  bx,1
  rol  bx,1
  rol  bx,1
  mov  ah,bl
  stosw                ; store AX at address ES:(E)DI; di += 2;
  mov al, ch
  mov ah, bh
  stosw                ; store AX at address ES:(E)DI; di += 2;

  ; 0x5CB
  lodsw
  mov  ch,ah            ;
  mov  bx,ax
  rol  bx,1
  rol  bx,1
  rol  bx,1
  rol  bx,1
  mov  ah,bl
  stosw                ; store AX at address ES:(E)DI; di += 2;
  mov al, ch
  mov ah, bh
  stosw                ; store AX at address ES:(E)DI; di += 2;

  ; 0x5E0
  lodsw
  mov  ch,ah            ;
  mov  bx,ax
  rol  bx,1
  rol  bx,1
  rol  bx,1
  rol  bx,1
  mov  ah,bl
  stosw                ; store AX at address ES:(E)DI; di += 2;
  mov al, ch
  mov ah, bh
  stosw                ; store AX at address ES:(E)DI; di += 2;

  ; 0x5F5
  lodsw
  mov  ch,ah            ;
  mov  bx,ax
  rol  bx,1
  rol  bx,1
  rol  bx,1
  rol  bx,1
  mov  ah,bl
  stosw                ; store AX at address ES:(E)DI; di += 2;
  mov al, ch
  mov ah, bh
  stosw                ; store AX at address ES:(E)DI; di += 2;

  ; 0x60A
  lodsw
  mov  ch,ah            ;
  mov  bx,ax
  rol  bx,1
  rol  bx,1
  rol  bx,1
  rol  bx,1
  mov  ah,bl
  stosw                ; store AX at address ES:(E)DI; di += 2;
  mov al, ch
  mov ah, bh
  stosw                ; store AX at address ES:(E)DI; di += 2;

  ; 0x61F
  lodsw
  mov  ch,ah            ;
  mov  bx,ax
  rol  bx,1
  rol  bx,1
  rol  bx,1
  rol  bx,1
  mov  ah,bl
  stosw                ; store AX at address ES:(E)DI; di += 2;
  mov al, ch
  mov ah, bh
  stosw                ; store AX at address ES:(E)DI; di += 2;

  ; 0x634
  lodsw
  mov  ch,ah            ;
  mov  bx,ax
  rol  bx,1
  rol  bx,1
  rol  bx,1
  rol  bx,1
  mov  ah,bl
  stosw                ; store AX at address ES:(E)DI; di += 2;
  mov al, ch
  mov ah, bh
  stosw                ; store AX at address ES:(E)DI; di += 2;

  ; 0x649
  lodsw
  mov  ch,ah            ;
  mov  bx,ax
  rol  bx,1
  rol  bx,1
  rol  bx,1
  rol  bx,1
  mov  ah,bl
  stosw                ; store AX at address ES:(E)DI; di += 2;
  mov al, ch
  mov ah, bh
  stosw                ; store AX at address ES:(E)DI; di += 2;

  ; 0x65E
  lodsw
  mov  ch,ah            ;
  mov  bx,ax
  rol  bx,1
  rol  bx,1
  rol  bx,1
  rol  bx,1
  mov  ah,bl
  stosw                ; store AX at address ES:(E)DI; di += 2;
  mov al, ch
  mov ah, bh
  stosw                ; store AX at address ES:(E)DI; di += 2;

  ; 0x673
  lodsw
  mov  ch,ah            ;
  mov  bx,ax
  rol  bx,1
  rol  bx,1
  rol  bx,1
  rol  bx,1
  mov  ah,bl
  stosw                ; store AX at address ES:(E)DI; di += 2;
  mov al, ch
  mov ah, bh
  stosw                ; store AX at address ES:(E)DI; di += 2;

  ; 0x688
  lodsw
  mov  ch,ah            ;
  mov  bx,ax
  rol  bx,1
  rol  bx,1
  rol  bx,1
  rol  bx,1
  mov  ah,bl
  stosw                ; store AX at address ES:(E)DI; di += 2;
  mov al, ch
  mov ah, bh
  stosw                ; store AX at address ES:(E)DI; di += 2;

  ; 0x69D
  lodsw
  mov  ch,ah            ;
  mov  bx,ax
  rol  bx,1
  rol  bx,1
  rol  bx,1
  rol  bx,1
  mov  ah,bl
  stosw                ; store AX at address ES:(E)DI; di += 2;
  mov al, ch
  mov ah, bh
  stosw                ; store AX at address ES:(E)DI; di += 2;

  ; 0x6B2
  lodsw
  mov  ch,ah            ;
  mov  bx,ax
  rol  bx,1
  rol  bx,1
  rol  bx,1
  rol  bx,1
  mov  ah,bl
  stosw                ; store AX at address ES:(E)DI; di += 2;
  mov al, ch
  mov ah, bh
  stosw                ; store AX at address ES:(E)DI; di += 2;

  ; 0x6C7
  lodsw
  mov  ch,ah            ;
  mov  bx,ax
  rol  bx,1
  rol  bx,1
  rol  bx,1
  rol  bx,1
  mov  ah,bl
  stosw                ; store AX at address ES:(E)DI; di += 2;
  mov al, ch
  mov ah, bh
  stosw                ; store AX at address ES:(E)DI; di += 2;

  ; 0x6DC
  lodsw
  mov  ch,ah            ;
  mov  bx,ax
  rol  bx,1
  rol  bx,1
  rol  bx,1
  rol  bx,1
  mov  ah,bl
  stosw                ; store AX at address ES:(E)DI; di += 2;
  mov al, ch
  mov ah, bh
  stosw                ; store AX at address ES:(E)DI; di += 2;

  ; 0x6F1
  lodsw
  mov  ch,ah            ;
  mov  bx,ax
  rol  bx,1
  rol  bx,1
  rol  bx,1
  rol  bx,1
  mov  ah,bl
  stosw                ; store AX at address ES:(E)DI; di += 2;
  mov al, ch
  mov ah, bh
  stosw                ; store AX at address ES:(E)DI; di += 2;

  ; 0x706
  lodsw
  mov  ch,ah            ;
  mov  bx,ax
  rol  bx,1
  rol  bx,1
  rol  bx,1
  rol  bx,1
  mov  ah,bl
  stosw                ; store AX at address ES:(E)DI; di += 2;
  mov al, ch
  mov ah, bh
  stosw                ; store AX at address ES:(E)DI; di += 2;

  ; 0x71B
  lodsw
  mov  ch,ah            ;
  mov  bx,ax
  rol  bx,1
  rol  bx,1
  rol  bx,1
  rol  bx,1
  mov  ah,bl
  stosw                ; store AX at address ES:(E)DI; di += 2;
  mov al, ch
  mov ah, bh
  stosw                ; store AX at address ES:(E)DI; di += 2;

  ; 0x730
  lodsw
  mov  ch,ah            ;
  mov  bx,ax
  rol  bx,1
  rol  bx,1
  rol  bx,1
  rol  bx,1
  mov  ah,bl
  stosw                ; store AX at address ES:(E)DI; di += 2;
  mov al, ch
  mov ah, bh
  stosw                ; store AX at address ES:(E)DI; di += 2;

  ; 0x745
  lodsw
  mov  ch,ah            ;
  mov  bx,ax
  rol  bx,1
  rol  bx,1
  rol  bx,1
  rol  bx,1
  mov  ah,bl
  stosw                ; store AX at address ES:(E)DI; di += 2;
  mov al, ch
  mov ah, bh
  stosw                ; store AX at address ES:(E)DI; di += 2;

  ; 0x75A
  lodsw
  mov  ch,ah            ;
  mov  bx,ax
  rol  bx,1
  rol  bx,1
  rol  bx,1
  rol  bx,1
  mov  ah,bl
  stosw                ; store AX at address ES:(E)DI; di += 2;
  mov al, ch
  mov ah, bh
  stosw                ; store AX at address ES:(E)DI; di += 2;

  ; 0x9E6
  ; ...

  ; 0xA8D
  ; 0xAA2
  ; 0xAF6
  ; 0xB5F


  ; 0xB2BA -> (rotate left 4 times) 0x2BAB

  ; 0xB74
  ret
