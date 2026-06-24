; funcion putpixel (x,y)

  load r7,160
  push r7
  load r7,100
  push r7
  load r7,0xffff
  push r7
  call putpixel
  sub r1,r1-6

label fin
  jmp fin
  

label hello db "Hola Mundo !"
  
define  WIDTH   320
define  HEIGHT  200

label backgroundColor
  dw  0

label foregroundColor
  dw  0xffff


  ;0    x=160
  ;2    y=100
  ;4    color=0xffff
  ;6    direccion retorno call
  ;8    push r5
  ;10   push r6
  ;12   push r7
  ;14

label putpixel
  push r5
  push r6
  push r7
  load r5,[s1:r1-12]           ; comprobamos que x<WIDTH
  cmp r5,WIDTH
  jae putpixel_fin
  load r7,[s1:r1-10]           ; comprobamos que y<HEIGHT
  cmp r7,HEIGHT
  jae putpixel_fin  
  load r6,WIDTH               ; calculamos y*WIDTH+x
  mul                         ; y*WIDTH
  add r6,r6+r5                ; +x
  load r5,r6                  ; copiamos el resultado
  load r7,[s0:foregroundColor]  ; cargamos el color
  shl r6,r6<1                 ; el color ocupa 2 bytes
  and r5,r5&r5&0x8000         ; comprobamos si estamos en los primeros
  jnz putpixel_nseg           ; 64KB
    load r5,0x7e00            ; cargamos el segmento de pantalla
    jmp putpixel_save
  label putpixel_nseg
    load r5,0x7f00            ; cargamos el segmento de pantalla
  label putpixel_save
  save r5,s7
  save r7,[s7:r6]             ; guardamos el color
label putpixel_fin
  pop r7
  pop r6
  pop r5
  ret
  

  ;0  x=160
  ;2  y=100
  ;4  caracter='A'
  ;6  direccion retorno call
  ;8  push r2
  ;10 push r3
  ;12 push r4
  ;14 push r5
  ;16 push r6
  ;18 push r7
  ;20

label putchar
  push r2
  push r3
  push r4
  push r5
  push r6
  push r7
  load r7,[s1:r1-16]          ; leemos la letra
  shl r7,r7<3                 ; multiplicamos por 8
  load r6,r7                  ; copiamos en r6 para calcular el bucle
  add r6,r6+8                 ; sumamos 8, tenemos el limite del bucle
  label putchar_bchar
    cmp r6,r7                 ; r7 recorre los 8 bytes de la fuente
    je putchar_fin            ; terminamos de dibujar
    
    load r3,r7
    and r3,r3&r3&1
    load r4,[r7+font]         ; leemos el byte de la fuente
    jz putchar_char
      shr r4,r4>8
    label putchar_char
    
    load r2,[s1:r1-20]        ; leemos x
    load r5,0x80              ; mascara para hacer and y ver si el pixel se pinta
    label putchar_bmasc
      cmp r5,0                ; acabamos con la mascara, 8 bits
      je putchar_fmasc
      load r3,r4
      and r3,r3&r5            ; vemos si hay un pixel que dibujar
      jz putchar_np           ; no hay que dibujar
        push r2               ; guardamos x
        load r3,[s1:r1-20]    ; leemos y
        push r3               ; guardamos y
        call putpixel         ; llamamos a putpixel
        sub r1,r1-4           ; borramos los parametros
      label putchar_np
      shr r5,r5>1             ; desplazamos la mascara
      add r2,r2+1             ; añadimos 1 a x
    jmp putchar_bmasc         ; retornamos al bucle de la mascara
    label putchar_fmasc
    add r7,r7+1               ; pasamos al siguiente byte
    load r3,[s1:r1-18]        ; leemos y
    add r3,r3+1               ; añadimos 1 a y
    save r3,[s1:r1-18]        ; guardamos y
  jmp putchar_bchar           ; retornamos al bucle de byte
label putchar_fin
  pop r7
  pop r6
  pop r5
  pop r4
  pop r3
  pop r2
  ret                         ; retornamos de la funcion


  ;0  x=160
  ;2  y=100
  ;4  string
  ;6  direccion retorno call
  ;8    push r5
  ;10   push r6
  ;12   push r7
  ;14

label writeString
  push r5
  push r6
  push r7
  load r5,[s1:r1-10]             ; leemos la direccion del string
  label writeString_bu
    load r7,[r5]                ; leemos la letra
    and r7,r7&r7&0xff           ; comprobamos si es cero (borramos lo que sobra)
    jz writeString_fin
      load r6,[s1:r1-14]         ; leemos x
      push r6                   ; lo guardamos
      load r6,[s1:r1-14]         ; leemos y
      push r6                   ; lo guardamos
      push r7                   ; guardamos la letra
      call putchar              ; dibujamos la letra
      sub r1,r1-6               ; borramos los parametros
      load r7,[r5]
      shr r7,r7>8
      jz writeString_fin
      load r6,[s1:r1-14]         ; leemos x
      add r6,r6+8               ; sumamos 8 font8x8
      push r6                   ; lo guardamos
      load r6,[s1:r1-14]         ; leemos y
      push r6                   ; lo guardamos
      push r7
      call putchar              ; dibujamos la letra
      sub r1,r1-6               ; borramos los parametros
      load r6,[s1:r1-14]         ; leemos x
      add r6,r6+16              ; sumamos 8 font8x8
      save r6,[s1:r1-14]         ; guardamos x
      add r5,r5+2
  jmp writeString_bu
label writeString_fin
  pop r7
  pop r6
  pop r5
  ret

label font

include 'font.asm'
