;unit string 


label string.length
label l0x0000 ;begin
  load r2,1
  push r2
  load r2,[s1:r1]
  neg r2
  save r2,[s1:r1]
  pop r2
  save r2,[s1:r1-0x0004]
label l0x0002 ;begin
  load r2,[s1:r1-0x0002]
  push r2
  pop r3
  load r2,r3
  and r2,r2&r2&1
  load r2,[r3]
  jz +2
  shr r2,r2>8
  and r2,r2&r2&0xff
  load r7,r2
  and r7,r7&r7&0x80
  jz +4
  or r2,r2|r2|0xff00
  push r2
  load r2,0
  push r2
  pop r2
  pop r3
  cmp r3,r2
  jne l0x0007 ;cond
label l0x0008 ;orand
  jmp l0x0004 ;if
label l0x0007 ;orand
  jmp l0x0005 ;else
label l0x0004 ;if
  jmp l0x0003 ;end
label l0x0005 ;else
label l0x0006 ;fi
  load r2,[s1:r1-0x0004]
  push r2
  load r2,1
  push r2
  pop r2
  load r7,[s1:r1]
  add r7,r7+r2
  save r7,[s1:r1]
  pop r2
  save r2,[s1:r1-0x0004]
  load r2,[s1:r1-0x0002]
  push r2
  load r2,1
  push r2
  pop r2
  load r7,[s1:r1]
  add r7,r7+r2
  save r7,[s1:r1]
  pop r2
  save r2,[s1:r1-0x0002]
  jmp l0x0002 ;begin
label l0x0003 ;end
label l0x0001 ;end
