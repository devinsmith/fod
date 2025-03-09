; Takes 2 arguments
sub_10:
  push bp
  mov  bp,sp
  sub  sp,002A
  push si
  push word [bp+04]   ; First argument
  mov  ax,0070
  push ax
  lea  ax,[bp-2A]             ss:[4826]=4836
  push ax
  call 00003338 ($+3313)
  add sp, 0x0006

  mov bx, 

sub_14B3:
  

sub_14D5:
  01ED:14D5  55                  push bp                                         
01ED:14D6  8BEC                mov  bp,sp
01ED:14D8  56                  push si
01ED:14D9  57                  push di
01ED:14DA  1E                  push ds
01ED:14DB  06                  push es
01ED:14DC  55                  push bp
01ED:14DD  8B7604              mov  si,[bp+04]             ss:[4854]=0001
01ED:14E0  56                  push si
01ED:14E1  E8CFFF              call 000014B3 ($-31)


sub_1631:
01ED:1631  56                  push si
01ED:1632  57                  push di
01ED:1633  55                  push bp
01ED:1634  8B369C02            mov  si,[029C]              ds:[029C]=0302
01ED:1638  837C1A00            cmp  word [si+1A],0000      ds:[0328]=000C
01ED:163C  8D7C0C              lea  di,[si+0C]             ds:[031A]=0000
01ED:163F  7403                je   00001644 ($+3)         (no jmp)
01ED:1641  8B7C1A              mov  di,[si+1A]             ds:[0328]=000C
01ED:1644  57                  push di
01ED:1645  E88DFE              call 000014D5 ($-173)


sub_1D34:
  01ED:1D34  59                  pop  cx                                         
01ED:1D35  8BDC                mov  bx,sp
01ED:1D37  2BD8                sub  bx,ax
01ED:1D39  720A                jc   00001D45 ($+a)         (no jmp)
01ED:1D3B  3B1EB61E            cmp  bx,[1EB6]              ds:[1EB6]=4170
01ED:1D3F  7204                jc   00001D45 ($+4)         (no jmp)
01ED:1D41  8BE3                mov  sp,bx
01ED:1D43  FFE1                jmp  near cx
01ED:1D45  33C0                xor  ax,ax
01ED:1D47  E945FE              jmp  00001B8F ($-1bb)       (up)


sub_22B4:
  01ED:22B4  55                  push bp
01ED:22B5  8BEC                mov  bp,sp
01ED:22B7  B86401              mov  ax,0164
01ED:22BA  E877FA              call 00001D34 ($-589)                           
; .loc_22BD:
01ED:22BD  57                  push di
01ED:22BE  56                  push si
01ED:22BF  8B7606              mov  si,[bp+06]             ss:[47EA]=0070
01ED:22C2  8D869EFE            lea  ax,[bp-0162]           ss:[4682]=0000
01ED:22C6  A3FA22              mov  [22FA],ax              ds:[22FA]=467A
01ED:22C9  8B4608              mov  ax,[bp+08]             ss:[47EC]=4808
01ED:22CC  A3EA22              mov  [22EA],ax              ds:[22EA]=4802      
01ED:22CF  8B4604              mov  ax,[bp+04]             ss:[47E8]=47F4
01ED:22D2  A3DE22              mov  [22DE],ax              ds:[22DE]=47EC
01ED:22D5  C706F4220000        mov  word [22F4],0000       ds:[22F4]=0000
01ED:22DB  C706F2220000        mov  word [22F2],0000       ds:[22F2]=0007
01ED:22E1  E97C02              jmp  00002560 ($+27c)       (down)

01ED:2560  803C00              cmp  byte [si],00           ds:[0070]=7325
01ED:2563  7403                je   00002568 ($+3)         (no jmp)
01ED:2565  E97CFD              jmp  000022E4 ($-284)       (up)
01ED:2568  833EF22200          cmp  word [22F2],0000       ds:[22F2]=0000
01ED:256D  7559                jne  000025C8 ($+59)        (down)
01ED:256F  8B1EDE22            mov  bx,[22DE]              ds:[22DE]=47F4
01ED:2573  F6470620            test byte [bx+06],20        ds:[4686]=0000
01ED:2577  744F                je   000025C8 ($+4f)        (no jmp)
01ED:2579  B8FFFF              mov  ax,FFFF
01ED:257C  EB4D                jmp  short 000025CB ($+4d)  (down)

; calculating string length
01ED:27AE  46                  inc  si
01ED:27AF  C45EF2              les  bx,[bp-0E]             ss:[4668]=01DB
01ED:27B2  FF46F2              inc  word [bp-0E]           ss:[4668]=01DB
01ED:27B5  26803F00            cmp  byte es:[bx],00        es:[01DA]=6120
01ED:27B9  75F3                jne  000027AE ($-d)         (up)                


sub_2B54:
  01ED:2B54  55                  push bp                                         
01ED:2B55  8BEC                mov  bp,sp
01ED:2B57  83EC02              sub  sp,0002
01ED:2B5A  56                  push si
01ED:2B5B  BEF81F              mov  si,1FF8
01ED:2B5E  8A4E04              mov  cl,[bp+04]             ss:[47E8]=47F4
01ED:2B61  EB02                jmp  short 00002B65 ($+2)   (down)

01ED:2B64  46                  inc  si
01ED:2B65  803C00              cmp  byte [si],00           ds:[1FF9]=202D
01ED:2B68  740A                je   00002B74 ($+a)         (no jmp)
01ED:2B6A  3A0C                cmp  cl,[si]                ds:[1FF9]=202D
01ED:2B6C  75F6                jne  00002B64 ($-a)         (up)
01ED:2B6E  B80100              mov  ax,0001
01ED:2B71  EB03                jmp  short 00002B76 ($+3)   (down)



sub_3338:
  01ED:3338  55                  push bp                                         
01ED:3339  8BEC                mov  bp,sp
01ED:333B  83EC0E              sub  sp,000E
01ED:333E  57                  push di
01ED:333F  56                  push si
01ED:3340  8D46F4              lea  ax,[bp-0C]             ss:[482A]=06B0
01ED:3343  8BF8                mov  di,ax
01ED:3345  8D4608              lea  ax,[bp+08]             ss:[483E]=0082
01ED:3348  8946FE              mov  [bp-02],ax             ss:[4834]=06B0
01ED:334B  C6450642            mov  byte [di+06],42        ds:[4874
01ED:334F  8B4604              mov  ax,[bp+04]             ss:[4804]=480C
01ED:3352  894504              mov  [di+04],ax             ds:[47F8]=480C
01ED:3355  8905                mov  [di],ax                ds:[47F4]=000C
01ED:3357  C74502FF7F          mov  word [di+02],7FFF      ds:[47F6]=482E
01ED:335C  8D4608              lea  ax,[bp+08]             ss:[4808]=01D4
01ED:335F  50                  push ax
01ED:3360  FF7606              push word [bp+06]           ss:[4806]=0070
01ED:3363  57                  push di
01ED:3364  E84DEF              call 000022B4 ($-10b3)
01ED:3367  83C406              add  sp,0006
01ED:336A  8BF0                mov  si,ax
01ED:336C  FF4D02              dec  word [di+02]           ds:[47F6]=7FFF
01ED:336F  780B                js   0000337C ($+b)         (no jmp)
01ED:3371  2AC0                sub  al,al



