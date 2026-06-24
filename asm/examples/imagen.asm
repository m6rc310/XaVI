; Esto es un comentario asm XaVI bits


load r2,0x7e00
save r2,s3
load r2,image
load r3,0
load r4,0
label for
  cmp r4,2
  je fin
  load r4,[r2]
  save r4,[s3:r3]
  add r2,r2+2
  jnz for_r2
    load r5,s2
    add r5,r5+0x0100
    save r5,s2
  label for_r2
  add r3,r3+2
  jnz for_r3
    add r4,r4+1
    load r5,s3
    add r5,r5+0x0100
    save r5,s3
  label for_r3
jmp for
label fin
jmp fin

label image
include 'carta-ajuste.asm'
