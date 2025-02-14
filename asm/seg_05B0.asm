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
05B0:02C8  B8B206              mov  ax,06B2




