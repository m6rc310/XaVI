; Suma de 32 bits a partir de primitivas de 16

label inicio
  load r7,0x1111
  push r7
  load r7,0x2222
  push r7
  load r7,0x4444
  push r7
  load r7,0x5555
  push r7
  call add32
  sub r1,r1-4
  pop r7
  pop r6
label fin
  jmp fin
  
  ;0   0x1111 0x2222
  ;4   0x4444 0x5555
  ;8   direccion retorno call 0x0000
  ;10
  
label add32
  load r6,[s1:r1-10]        ;-10
  load r7,[s1:r1-6]        ;-6
  add r6,r6+r7
  save r6,[s1:r1-10]        ;-10
  load r6,[s1:r1-8]        ;-8
  load r7,[s1:r1-4]        ;-4
  adc r6,r6+r7
  save r6,[s1:r1-8]        ;-8
  ret
  


