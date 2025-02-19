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
  mov  di,0x05BF
  xor  ax,ax
  mov  cx,0x00C8
.loc_35
  stosw
  add  ax,0x00A0
  loop .loc_35

  xor bx, bx
  mov bl, [0x0424]   ; 0x0003
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


; Seems to be setting up the video hardware, as well as setting up some video support tables
; sub_02A9
  mov word cs:[0x0353], 0x0136
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
  mov si, 0x0CD2
  mov cx, 0x0010
  xor bx, bx
.loc_2D5:
  mov di, 0x0D02
  lodsb ; load byte at ds:si into al (inc si by 1)
  mov dh, al
  lodsw ; load word at ds:si into ax (inc si by 2)
  push cx
  mov cx, 0x0010
.loc_2e0:
  mov es:[di], dh
  inc di
  stosw ; put ax into ds:di, advance di by 2.
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
  mov ax, 0x0B74 ; 
.loc_315:
  stosw
  sub ax, 0x0015
  loop .loc_315

  mov ax, cs
  mov ds, ax
  mov es, ax

  mov si, 0x0338
  mov di, 0x04E4
  mov cx, 0x0015
  repe movsb ; copy 0x15 byte string from DS:SI to ES:DI

  mov cx, 0x067B
  mov si, 0x04E4
  repe movsb ; copy 0x67B byte string from ds:si to es:di

  mov al, 0xC3
  stosb ; put al into ds:di
  ret

