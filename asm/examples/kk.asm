
  load r3,hello
  label bucle
  
    load r2,r3
    and r2,r2&1
    load r2,[r3]
    jz +2
      shr r2,r2>8
;    label
    and r2,r2&r2&0xff

    jz fin
      add r3,r3+1
      jmp bucle
  label fin
  jmp fin
  
label hello db "Hola Mundo !",0
