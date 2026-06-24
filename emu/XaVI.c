/*
**	XaVI bits (16 bits en numeros romanos) procesador de 16 bits
**
*/
/*
    Este codigo es parte de "XaVI"
    Copyright (C) 2026  Marcelo Reina Aguilar

    Este programa es software libre: usted puede redistribuirlo y/o modificarlo
    bajo los términos de la Licencia Pública General de GNU publicada por
    la Fundación para el Software Libre, ya sea la versión 3 de la Licencia,
    o (a su elección) cualquier versión posterior.

    Este programa se distribuye con la esperanza de que sea útil, pero
    SIN NINGUNA GARANTÍA; ni siquiera la garantía implícita de
    COMERCIABILIDAD o IDONEIDAD PARA UN FIN DETERMINADO. Consulte la
    Licencia Pública General de GNU para más detalles.

    Debería haber recibido una copia de la Licencia Pública General de GNU
    junto con este programa. En caso contrario, consulte <https://www.gnu.org/licenses/>.
*/


#include "define.h"
#include "struct.h"
#include "tools.h"


#define r0 (reg[0])
#define r1 (reg[1])
#define r2 (reg[2])
#define r3 (reg[3])
#define r4 (reg[4])
#define r5 (reg[5])
#define r6 (reg[6])
#define r7 (reg[7])

#define s0 (seg[0])
#define s1 (seg[1])
#define s2 (seg[2])
#define s3 (seg[3])
#define s4 (seg[4])
#define s5 (seg[5])
#define s6 (seg[6])
#define s7 (seg[7])

#define ss0 (sseg[0])
#define ss1 (sseg[1])
#define ss2 (sseg[2])
#define ss3 (sseg[3])
#define ss4 (sseg[4])
#define ss5 (sseg[5])
#define ss6 (sseg[6])
#define ss7 (sseg[7])

#define s64k 0xfffe
#define s32k 0x7ffe
#define s16k 0x3ffe
#define s8k  0x1ffe

#define zero		1
#define carry		2
#define sign		4
#define overflow	8
#define interrupt	16
#define privilege	2
#define virtual	1

#define TimeScreen  100000
void initScreen (void *);
void updateScreen ();

void inst_mul ();
void inst_div ();
void inst_umul ();
void inst_udiv ();
void inst_ret ();
void inst_reti ();
void inst_neg ();
void inst_not ();
void inst_load_reg ();
void inst_load_reg_seg ();
void inst_save_reg_seg ();
void inst_int ();
void inst_jmp_i10 ();
void inst_inport ();
void inst_outport ();
void inst_load_seg ();
void inst_load_2reg ();
void inst_load ();
void inst_save_seg ();
void inst_save_2reg ();
void inst_save ();
void inst_add ();
void inst_add_2reg ();
void inst_adc ();
void inst_adc_2reg ();
void inst_sub ();
void inst_sub_2reg ();
void inst_sbb ();
void inst_sbb_2reg ();
void inst_cmp ();
void inst_and ();
void inst_or ();
void inst_xor ();
void inst_shl ();
void inst_shl_i ();
void inst_shr ();
void inst_shr_i ();
void inst_sar ();
void inst_sar_i ();

void inst_jb ();
void inst_jbe ();
void inst_je ();
void inst_jne ();
void inst_ja ();
void inst_jae ();
void inst_jl ();
void inst_jle ();
void inst_jg ();
void inst_jge ();

void inst_load_u16 ();

void inst_cli ();
void inst_sti ();
void inst_push ();
void inst_pop ();
void inst_jmps ();
void inst_call_2reg ();
void inst_call ();
void inst_calls ();
void inst_jo ();
void inst_jno ();
void inst_rets ();
void inst_save_seg7 ();
void inst_save_segr7 ();
void inst_call_u16 ();

void inst_ilegal ();
void inst_virtual ();

uint16
  reg[8],
  seg[8],sseg[8],
  slimit[4]={s64k,s32k,s16k,s8k},
  ioport[16];
  
int16
  state,
  *o0,
  *o1,
  *o2,
  *o3,
  *pmem,
  *paux0,
  daux,
  code;
int32 aux0,aux1;

int8
  memory[16*1024*1024],
  ri0,ri1,ri2;

int8 *string_inst[]={
  "mul","div","umul","udiv",
  "ret","reti","neg","not",
  
  "load_reg","load_reg_seg","save_reg_seg",
  "int","jmp_i10","inport","outport",
  "load_seg","load/jmp_2reg","load/jmp",
  "save_seg","save_2reg","save",
  
  "add","add_2reg","adc","adc_2reg",
  "sub","sub_2reg","sbb","sbb_2reg",

  "cmp","and","or","xor",
  "shl","shl_i","shr","shr_i",
  "sar","sar_i",

  "jb","jbe","je","jne",
  "ja","jae","jl","jle",
  "jg","jge",
  
  "load/jmp u16","cli","sti",
  "push","pop","call_2reg",

  "call","jmps","save_seg7","calls",
  "jo","jno","rets","save_segr7",
  "call_u",
};

void *inst[]={
  inst_mul,inst_div,inst_umul,inst_udiv,
  inst_ret,inst_reti,inst_neg,inst_not,
  
  inst_load_reg,inst_load_reg_seg,inst_save_reg_seg,
  inst_int,inst_jmp_i10,inst_inport,inst_outport,
  inst_load_seg,inst_load_2reg,inst_load,
  inst_save_seg,inst_save_2reg,inst_save,
  
  inst_add,inst_add_2reg,inst_adc,inst_adc_2reg,
  inst_sub,inst_sub_2reg,inst_sbb,inst_sbb_2reg,

  inst_cmp,inst_and,inst_or,inst_xor,
  inst_shl,inst_shl_i,inst_shr,inst_shr_i,
  inst_sar,inst_sar_i,

  inst_jb,inst_jbe,inst_je,inst_jne,
  inst_ja,inst_jae,inst_jl,inst_jle,
  inst_jg,inst_jge,
  
  inst_load_u16,inst_cli,inst_sti,
  inst_push,inst_pop,inst_call_2reg,

  inst_call,inst_jmps,inst_save_seg7,inst_calls,
  inst_jo,inst_jno,inst_rets,
  inst_save_segr7,inst_call_u16
};


void initCPU () {
  s0=0xfc00;
  s1=0x0010;
  s2=s0;s3=s0;s4=s0;s5=s0;s6=s0;s7=s0;
  ss0=s64k;ss1=s64k;ss2=s64k;ss3=s64k;
  ss4=s64k;ss5=s64k;ss6=s64k;ss7=s64k;
  r0=0;r2=r0;r3=r0;r4=r0;r5=r0;r6=r0;r7=r0;
  r1=0xfffe;
}

void writeState () {
  uint16 c;
  
  string_write (STDOUT,"CPU State\n");
  
  if (state&zero) string_write (STDOUT,"   Z ");
  else  string_write (STDOUT,"  NZ ");
  if (state&carry) string_write (STDOUT," C ");
  else  string_write (STDOUT,"NC ");
  if (state&sign) string_write (STDOUT," S ");
  else  string_write (STDOUT,"NS ");
  if (state&interrupt) string_write (STDOUT," I ");
  else  string_write (STDOUT,"NI ");
  if (state&privilege) string_write (STDOUT," P ");
  else  string_write (STDOUT,"NP  ");
  c=*(uint16 *)(memory+(((s0&0xfff0)<<8)+(r0&ss0)));
  string_write (STDOUT,"HEX: ");
  string_write (STDOUT,string_hex (c));
  string_write (STDOUT," ");
  string_write (STDOUT,string_hex (*(uint16 *)(memory+(((s0&0xfff0)<<8)+((r0+2)&ss0)))));
  string_write (STDOUT," CODE: ");
  string_write (STDOUT,string_inst[c>>10]);
  string_write (STDOUT,"\n");

  string_write (STDOUT,"  r0(cp): ");
  string_write (STDOUT,string_hex (r0));
  string_write (STDOUT,"  r1(sp): ");
  string_write (STDOUT,string_hex (r1));
  string_write (STDOUT,"\n");
  string_write (STDOUT,"  r2    : ");
  string_write (STDOUT,string_hex (r2));
  string_write (STDOUT,"  r3    : ");
  string_write (STDOUT,string_hex (r3));
  string_write (STDOUT,"\n");
  string_write (STDOUT,"  r4    : ");
  string_write (STDOUT,string_hex (r4));
  string_write (STDOUT,"  r5    : ");
  string_write (STDOUT,string_hex (r5));
  string_write (STDOUT,"\n");
  string_write (STDOUT,"  r6    : ");
  string_write (STDOUT,string_hex (r6));
  string_write (STDOUT,"  r7    : ");
  string_write (STDOUT,string_hex (r7));
  string_write (STDOUT,"\n");

  string_write (STDOUT,"  s0(sp): ");
  string_write (STDOUT,string_hex (s0));
  string_write (STDOUT,"  s1(ss): ");
  string_write (STDOUT,string_hex (s1));
  string_write (STDOUT,"\n");
  string_write (STDOUT,"  s2(sd): ");
  string_write (STDOUT,string_hex (s2));
  string_write (STDOUT,"  s3    : ");
  string_write (STDOUT,string_hex (s3));
  string_write (STDOUT,"\n");
  string_write (STDOUT,"  s4    : ");
  string_write (STDOUT,string_hex (s4));
  string_write (STDOUT,"  s5    : ");
  string_write (STDOUT,string_hex (s5));
  string_write (STDOUT,"\n");
  string_write (STDOUT,"  s6    : ");
  string_write (STDOUT,string_hex (s6));
  string_write (STDOUT,"  s7    : ");
  string_write (STDOUT,string_hex (s7));
  string_write (STDOUT,"\n");
}

void inst_mmul () {
  r7=aux0>>16;
  r6=aux0&0xffff;
  r0+=2;
}

void inst_mul () {
  aux0=r6*r7;
  
  inst_mmul ();
}

void inst_umul () {
  aux0=((uint16)r6)*((uint16)r7);
  
  inst_mmul ();
}

void inst_mdiv () {

  if (aux1) {
	  aux1=aux0/aux1;
    if (aux1&0xffff0000) state|=overflow;
    else state&=~overflow;
    r7=aux0%r5;
    r6=aux1;
  } else state|=overflow;
  r0+=2;
}

void inst_div () {
  aux0=(r7<<16)|r6;
  aux1=r5;

  inst_mdiv ();
}

void inst_udiv () {
  aux0=(((uint16)r7)<<16)|((uint16)r6);
  aux1=(uint16)r5;

  inst_mdiv ();
}

void inst_ret () {
	r0=*(int16 *)(memory+((s1&0xfff0)<<8)+(r1&ss1));
	r1-=2;
}

void inst_reti () {
  if (s0&privilege) inst_ilegal ();
  else {
	  paux0=(int16 *)(memory+((s1&0xfff0)<<8)+(r1&ss1));
	  s0=*paux0;
	  r1-=2;
	  paux0--;
	  r0=*paux0;
	  r1-=2;
  }
}

void updateZ () {
  if (!(reg[ri0]&0xffff)) state|=zero;
  else state&=~zero;
}

void updateFlags () {
  if (!(aux0&0xffff)) state|=zero;
  else state&=~zero;
  if (aux0&0x8000) state|=sign;
  else state&=~sign;
  if (aux0&0xffff0000) state|=carry;
  else state&=~carry;
}

void neg_not (int32 r) {
  if (r&0xffff) state&=~zero;
  else state|=zero;
  if (r&0x8000) state|=sign;
  else state&=~sign;
  r0+=2;
}

void inst_neg () {
  ri0=code&0x7;
  
  aux0=-(int32)reg[ri0];
  updateFlags ();
  reg[ri0]=aux0;
}

void inst_not () {
  ri0=code&0x7;
  
  aux0=~(int32)reg[ri0];
  updateFlags ();
  reg[ri0]=aux0;
}

void inst_load_reg_seg () {
  ri0=code&0x7;ri1=(code>>3)&0x7;

  reg[ri0]=seg[ri1];
  r0+=2;
}

void inst_save_reg_seg () {
  if (s0&privilege) inst_ilegal ();
  else {
    ri0=code&0x7;ri1=(code>>3)&0x7;

    seg[ri1]=reg[ri0];
    sseg[r1]=slimit[(reg[ri0]>>2)&0x3];
    r0+=2;
  }
}

void inst_int () {
  ri0=code&0x1f;
  
  r1+=2;
  paux0=(int16 *)(memory+((s1&0xfff0)<<8)+(r1&ss1));
  *paux0=r0;
  paux0++;
  *paux0=s0;
  r1+=2;
  paux0=(int16 *)(memory+(ri0<<1));
  r0=*paux0;
  paux0++;
  s0=*paux0;
}

void inst_jmp_i10 () {
  daux=code&0x3ff;

  if (daux&0x200) daux|=0xfc00;
  r0+=daux;
}

void inst_inport () {
  if (s0&privilege) inst_ilegal ();
  else {
    daux=0;ri0=code&0x7;ri1=(code>>3)&0x7;
  
    if (code&0x40) {
      r0+=2;
      daux=*(int16 *)(memory+((s0&0xfff0)<<8)+(r0&ss0));
    }
    if (code&0x80) daux+=reg[ri1];
    reg[ri0]=ioport[daux&0xf];
    r0+=2;
  }
}

void inst_outport () {
  if (s0&privilege) inst_ilegal ();
  else {
    daux=0;ri0=code&0x7;ri1=(code>>3)&0x7;
  
    if (code&0x40) {
      r0+=2;
      daux=*(int16 *)(memory+((s0&0xfff0)<<8)+(r0&ss0));
    }
    if (code&0x80) daux+=reg[ri1];
    ioport[daux&0xf]=reg[ri0];
    r0+=2;
  }
}

void inst_load_seg () {
  ri0=code&0x7;ri1=(code>>3)&0x7;ri2=(code>>6)&0x7;

  r0+=2;
  daux=*(int16 *)(memory+((s0&0xfff0)<<8)+(r0&ss0));
  r0+=2;
  if (code&0x200) daux+=reg[ri2];
  reg[ri0]=*(int16 *)(memory+((seg[ri1]&0xfff0)<<8)+(daux&sseg[ri1]));
}

void inst_save_seg () {
  ri0=code&0x7;ri1=(code>>3)&0x7;ri2=(code>>6)&0x7;

  r0+=2;
  daux=reg[ri2];
  if (code&0x200) {
    daux+=*(int16 *)(memory+((s0&0xfff0)<<8)+(r0&ss0));
    r0+=2;
  }
  *(int16 *)(memory+((seg[ri1]&0xfff0)<<8)+(daux&sseg[ri1]))=reg[ri0];
}

void inst_save_seg7 () {
  ri0=code&0x7;ri1=(code>>3)&0x7;
  
  daux=seg[ri1];
  seg[ri1]=seg[ri0];
  seg[ri0]=daux;
  daux=sseg[ri1];
  sseg[ri1]=sseg[ri0];
  sseg[ri0]=daux;
  r0+=2;
}

void inst_save_segr7 () {
  ri0=code&0x7;ri1=(code>>3)&0x7;
  
  ri0=reg[ri0]&0x7;
  daux=seg[ri1];
  seg[ri1]=seg[ri0];
  seg[ri0]=daux;
  daux=sseg[ri1];
  sseg[ri1]=sseg[ri0];
  sseg[ri0]=daux;
  r0+=2;
}

int8 load_save_2reg () {
  daux=0;ri0=code&0x7;
  ri1=(code>>3)&0x7;ri2=(code>>6)&0x7;

  if (s2&virtual) {
    inst_virtual ();
    return 0;
  }  
  if (code&0x200) {
	  r0+=2;
    daux=*(int16 *)(memory+((s0&0xfff0)<<8)+(r0&ss0));
  }
  r0+=2;
  paux0=(int16 *)(memory+((s2&0xfff0)<<8)+
    ((reg[ri1]+reg[ri2]+(uint16)daux)&ss2));
  return -1;
}

void inst_load_2reg () {
  if (load_save_2reg ()) reg[ri0]=*paux0;
}

void inst_save_2reg () {
  if (load_save_2reg ()) *paux0=reg[ri0];
}

void inst_call_2reg () {
  if (load_save_2reg ()) {
    r1+=2;
    *(uint16 *)(memory+((s1&0xfff0)<<8)+(r1&ss1))=r0;
    r0=*paux0;
  }
}

int8 load_save () {
  daux=0;ri0=code&0x7;ri1=(code>>3)&0x7;

  if (s2&virtual) {
    inst_virtual ();
    return 0;
  }  
  if (code&0x40) {
	  r0+=2;
    daux=*(int16 *)(memory+((s0&0xfff0)<<8)+r0);
  }
  if (code&0x80) daux+=reg[ri1];
  r0+=2;
  paux0=(int16 *)(memory+((s2&0xfff0)<<8)+(((uint16)daux)&ss2));
  return -1;
}

void inst_load_reg () {
 ri0=code&0x7;ri1=(code>>3)&0x7;

 reg[ri0]=reg[ri1];
 r0+=2;
}

void inst_load () {
  if (load_save ()) reg[ri0]=*paux0;
}

void inst_save () {
  if (load_save ()) *paux0=reg[ri0];
}

void inst_call () {
  if (load_save ()) {
    r1+=2;
    *(uint16 *)(memory+((s1&0xfff0)<<8)+(r1&ss1))=r0;
    r0=*paux0;
  }
}

void inst_add () {
  ri0=code&0x7;ri1=(code>>3)&0x7;
  
  r0+=2;
  aux0=reg[ri1]+*(int16 *)(memory+((s0&0xfff0)<<8)+(r0&ss0));
  r0+=2;
  updateFlags ();
  reg[ri0]=aux0;
}

void inst_add_2reg () {
  ri0=code&0x7;ri1=(code>>3)&0x7;ri2=(code>>6)&0x7;
  
  reg[ri0]=
  aux0=reg[ri1]+reg[ri2];
  if (code&0x200) {
    r0+=2;
    aux0+=*(int16 *)(memory+((s0&0xfff0)<<8)+(r0&ss0));
  }
  r0+=2;
  updateFlags ();
  reg[ri0]=aux0;
}

void inst_adc () {
  ri0=code&0x7;ri1=(code>>3)&0x7;
  
  r0+=2;
  aux0=reg[ri1]+*(int16 *)(memory+((s0&0xfff0)<<8)+(r0&ss0));
  if (state&carry) aux0+=1;
  r0+=2;
  updateFlags ();
  reg[ri0]=aux0;
}

void inst_adc_2reg () {
  ri0=code&0x7;ri1=(code>>3)&0x7;ri2=(code>>6)&0x7;
  
  aux0=reg[ri1]+reg[ri2];
  if (code&0x200) {
    r0+=2;
    aux0+=*(int16 *)(memory+((s0&0xfff0)<<8)+(r0&ss0));
  }
  if (state&carry) aux0+=1;
  r0+=2;
  updateFlags ();
  reg[ri0]=aux0;
}

void inst_sub () {
  ri0=code&0x7;ri1=(code>>3)&0x7;
  
  r0+=2;
  aux0=reg[ri1]-*(int16 *)(memory+((s0&0xfff0)<<8)+(r0&ss0));
  r0+=2;
  updateFlags ();
  reg[ri0]=aux0;
}

void inst_sub_2reg () {
  ri0=code&0x7;ri1=(code>>3)&0x7;ri2=(code>>6)&0x7;
  
  aux0=reg[ri1]-reg[ri2];
  if (code&0x200) {
    r0+=2;
    aux0-=*(int16 *)(memory+((s0&0xfff0)<<8)+(r0&ss0));
  }
  r0+=2;
  updateFlags ();
  reg[ri0]=aux0;
}

void inst_sbb () {
  ri0=code&0x7;ri1=(code>>3)&0x7;
  
  r0+=2;
  aux0=reg[ri1]-*(int16 *)(memory+((s0&0xfff0)<<8)+(r0&ss0));
  if (state&carry) aux0-=1;
  r0+=2;
  updateFlags ();
  reg[ri0]=aux0;
}

void inst_sbb_2reg () {
  ri0=code&0x7;ri1=(code>>3)&0x7;ri2=(code>>6)&0x7;
  
  aux0=reg[ri1]-reg[ri2];
  if (code&0x200) {
    r0+=2;
    aux0-=*(int16 *)(memory+((s0&0xfff0)<<8)+(r0&ss0));
  }
  if (state&carry) aux0-=1;
  r0+=2;
  updateFlags ();
  reg[ri0]=aux0;
}

void inst_cmp () {
  ri0=code&0x7;ri1=(code>>3)&0x7;
  
  aux0=reg[ri0];
  if (code&0x40) {
    r0+=2;
    aux0-=*(uint16 *)(memory+((s0&0xfff0)<<8)+(r0&ss0));
  }
  if (code&0x80) aux0-=reg[ri1];
  r0+=2;
  updateFlags ();
}

void inst_and () {
  ri0=code&0x7;ri1=(code>>3)&0x7;ri2=(code>>6)&0x7;
  
  reg[ri0]=reg[ri1]&reg[ri2];
  if (code&0x200) {
    r0+=2;
    reg[ri0]&=*(int16 *)(memory+((s0&0xfff0)<<8)+(r0&ss0));
  }
  updateZ ();
  r0+=2;
}

void inst_or () {
  ri0=code&0x7;ri1=(code>>3)&0x7;ri2=(code>>6)&0x7;
  
  reg[ri0]=reg[ri1]|reg[ri2];
  if (code&0x200) {
    r0+=2;
    reg[ri0]|=*(int16 *)(memory+((s0&0xfff0)<<8)+(r0&ss0));
  }
  updateZ ();
  r0+=2;
}

void inst_xor () {
  ri0=code&0x7;ri1=(code>>3)&0x7;ri2=(code>>6)&0x7;
  
  reg[ri0]=reg[ri1]^reg[ri2];
  if (code&0x200) {
    r0+=2;
    reg[ri0]^=*(int16 *)(memory+((s0&0xfff0)<<8)+(r0&ss0));
  }
  updateZ ();
  r0+=2;
}

void inst_shl () {
  ri0=code&0x7;ri1=(code>>3)&0x7;ri2=(code>>6)&0x7;
  
  aux0=reg[ri1]<<reg[ri2];
  r0+=2;
  updateFlags ();
  reg[ri0]=aux0;
}

void inst_shl_i () {
  ri0=code&0x7;ri1=(code>>3)&0x7;
  
  aux0=reg[ri1]<<((code>>6)&0xf);
  r0+=2;
  updateFlags ();
  reg[ri0]=aux0;
}

void inst_shr () {
  ri0=code&0x7;ri1=(code>>3)&0x7;ri2=(code>>6)&0x7;
  
  aux0=reg[ri1]>>reg[ri2];
  r0+=2;
  updateFlags ();
  reg[ri0]=aux0;
}

void inst_shr_i () {
  ri0=code&0x7;ri1=(code>>3)&0x7;
  
  aux0=reg[ri1]>>((code>>6)&0xf);
  r0+=2;
  updateFlags ();
  reg[ri0]=aux0;
}

void inst_sar () {
  ri0=code&0x7;ri1=(code>>3)&0x7;ri2=(code>>6)&0x7;
  
  aux0=((int16)reg[ri1])>>reg[ri2];
  r0+=2;
  updateFlags ();
  reg[ri0]=aux0;
}

void inst_sar_i () {
  ri0=code&0x7;ri1=(code>>3)&0x7;
  
  aux0=((int16)reg[ri1])>>((code>>6)&0xf);
  r0+=2;
  updateFlags ();
  reg[ri0]=aux0;
}

void inst_jcc (int8 st) {

  if (st) {
    daux=code&0x3ff;
    if (daux&0x200) daux|=0xfc00;
    daux<<=1;
    r0+=daux;
  } else r0+=2;
}

void inst_jb () {
  inst_jcc (state&carry);
}

void inst_jbe () {
  inst_jcc (state&carry || state&zero);
}

void inst_je () {
  inst_jcc (state&zero);
}

void inst_jne () {
  inst_jcc (!(state&zero));
}

void inst_ja () {
  inst_jcc (!(state&carry) && !(state&zero));
}

void inst_jae () {
  inst_jcc (!(state&carry) || state&zero);
}

void inst_jl () {
  inst_jcc ((state&sign));
//  inst_jcc (!(state&carry) && state&sign ||
//    state&carry && !(state&sign));
}

void inst_jle () {
  inst_jcc ((state&zero) || (state&sign));
//  inst_jcc (state&zero ||
//    !(state&carry) && state&sign ||
//    state&carry && !(state&sign));
}

void inst_jg () {
  inst_jcc (!(state&zero) && !(state&sign));
}

void inst_jge () {
  inst_jcc (!(state&sign));
//  inst_jcc (state&carry && state&sign ||
//  !(state&carry) && !(state&sign));
}

void inst_load_u16 () {
  ri0=code&0x7;

  r0+=2;
  reg[ri0]=*(uint16 *)(memory+((s0&0xfff0)<<8)+(r0&ss0));
  if (ri0) r0+=2;
}

void inst_call_u16 () {
  r0+=2;
  r1+=2;
  *(uint16 *)(memory+((s1&0xfff0)<<8)+(r1&ss1))=r0+2;
  r0=*(uint16 *)(memory+((s0&0xfff0)<<8)+(r0&ss0));
}

void inst_cli () {
  if (s0&privilege) inst_ilegal ();
  else {
    r0+=2;
    state&=~interrupt;
  }
}

void inst_sti () {
  if (s0&privilege) inst_ilegal ();
  else {
    r0+=2;
    state|=interrupt;
  }
}

void inst_push () {
  ri0=code&0x7;
  
  r1+=2;
  *(uint16 *)(memory+((s1&0xfff0)<<8)+(r1&ss1))=reg[ri0];
  r0+=2;
}

void inst_pop () {
  ri0=code&0x7;
  
  reg[ri0]=*(uint16 *)(memory+((s1&0xfff0)<<8)+(r1&ss1));
  r1-=2;
  r0+=2;
}

void inst_nop () {
  r0+=2;
}

void inst_jmps () {
  daux=0;ri0=code&0x7;

  if (s2&virtual) {
    inst_virtual ();
    return;
  }  
  r0+=2;
  daux=*(int16 *)(memory+((s0&0xfff0)<<8)+(r0&ss0));
  s0=seg[ri0];
  r0=daux;
}

void inst_calls () {
  daux=0;ri0=code&0x7;

  if (s2&virtual) {
    inst_virtual ();
    return;
  }  
  r0+=2;
  daux=*(int16 *)(memory+((s0&0xfff0)<<8)+(r0&ss0));
  r0+=2;
  r1+=2;
  *(uint16 *)(memory+((s1&0xfff0)<<8)+(r1&ss1))=r0;
  r1+=2;
  *(uint16 *)(memory+((s1&0xfff0)<<8)+(r1&ss1))=ri0;
  r0=daux;
  daux=s0;
  s0=seg[ri0];
  seg[ri0]=daux;
}

void inst_jo () {
  inst_jcc (state&overflow);
}

void inst_jno () {
  inst_jcc (!(state&overflow));
}

void inst_rets () {
  ri0=(*(uint16 *)(memory+((s1&0xfff0)<<8)+(r1&ss1)))&0x7;
  r1-=2;
  daux=s0;
  s0=seg[ri0];
  seg[ri0]=daux;
  r0=*(uint16 *)(memory+((s1&0xfff0)<<8)+(r1&ss1));
  r1-=2;
}

void inst_virtual_ilegal (void *m) {  
  r1+=2;
  paux0=(int16 *)(memory+((s1&0xfff0)<<8)+(r1&ss1));
  *paux0=r0;
  paux0++;
  *paux0=s0;
  r1+=2;
  paux0=(int16 *)m;
  r0=*paux0;
  paux0++;
  s0=*paux0;
}

void inst_ilegal () {
  inst_virtual_ilegal (memory);
}

void inst_virtual () {
  inst_virtual_ilegal (memory+4);
}

int main (int argc,char *argv[]) {
  void (*p) ();
  int64 fileROM;
  int32 i;
  int8 c[4],t=0;
  
  if (argc==2) t=argv[1][0];
  initCPU ();
  fileROM=system_open ("rom.bin",O_RDONLY,0);
  for (i=((s0&0xfff0)<<8)+r0;;i++)
    if (!system_read (fileROM,memory+i,1)) break;
  initScreen (memory+0x7e0000);
  for (i=0;;i++) {
    if (i==TimeScreen) i=0;
    if (t=='t' || t=='w') writeState ();
    pmem=(void *)memory+((s0&0xfff0)<<8)+r0;
    code=*pmem;
    p=inst[(code&0xfc00)>>10];
    (*p) ();
    if (t=='t') {
      system_read (STDIN,c,1);
      if (c[0]=='q') break;
    }
  }
  return 0;
}
