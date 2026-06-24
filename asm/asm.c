/*
**	Ensamblador para Xavi bits
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

#define		MAXTOKEN    256
#define		MAXOBJECT 	1024
#define   MAXLINE     1024
#define   MAXCODE     (64*1024)
#define		MAXFILES    8

#define		SPACE		0
#define		NUMBER		1
#define		LETTER		2
#define		OTHER		3

#define   UNRESOLVED      0
#define   UNRESOLVED_10   1
#define   UNRESOLVED_NEG  2

#define		isIdentifier(a)	(type (a[0])==LETTER)
#define		isNumber(a)	(type (a[0])==NUMBER)
#define		isString(a)	(a[0]=='\"')
#define		isCharacter(a)	(a[0]=='\'')

int16 isDefine (int8 *);
int8 isRegister (int8 *);
int8 isSegment (int8 *);
int16 isLabel (int8 *);
int16 toInt16 ();
int8 constant (int16 *);
int16 hexToInt16 ();
int8 assemble_cmp (int16);
int8 assemble_r (int16);
int8 assemble_jmp_call (int16,int16,int16);
void syntaxError (int8 *);
void save_unresolved (int8);


int64
  fileSource,
  files[MAXFILES],
  filesLine[MAXFILES],
  fileIndex,
  fileBIN,
  lineNumber;

int8
  tk[MAXTOKEN],
  line[MAXLINE],
  label[MAXOBJECT][MAXTOKEN],
  unresolved[MAXOBJECT][MAXTOKEN],
  unresolvedCond[MAXOBJECT],
  unreadToken[1],
  unreadCharacter[1],
  ri0,ri1,ri2,
  eof;
int16
  itok,
  nlabel,
  nunresolved,
  dlabel[MAXOBJECT],
  dunresolved[MAXOBJECT],
  code[MAXCODE];
int32
  indexCode;

int8 type (int8 c)
{
  if ('0'<=c && c<='9') return NUMBER;
  if ('a'<=c && c<='z' || 'A'<=c && c<='Z' || c=='_' || c=='.')
    return LETTER;
  if (c==' ' || c=='\t' || c=='\r' || c=='\n') return SPACE;
  return OTHER;
}

int8 unread (int8 c,int8 *p)
{
  int8 b;
  
  b=*p;
  *p=c;
  return b;
}

int8 readCharacter ()
{
  int8 c;
  int16 l;

  c=unread (0,unreadCharacter);
  if (c) return c;
  system_read (fileSource,&c,1);
  if (c=='\n') {
    lineNumber++;
    line[0]=0;
  } else {
    if (!c) {
      fileIndex--;
      lineNumber=filesLine[fileIndex];
      system_close (files[fileIndex]);
      if (!fileIndex) eof=-1;
      else {
        fileSource=files[fileIndex-1];
      }
    }
    l=string_length (line);
    line[l]=c;
    line[l+1]=0;
  }
  return c;
}

void token ()
{
  int8 t,c;
  int16 i;

#ifdef DEBUG
    string_write (STDOUT,"Function token \'");
    string_write (STDOUT,tk);
    string_write (STDOUT,"\' line ");
    string_write (STDOUT,string_decimal (lineNumber));
    string_write (STDOUT,".\n");
#endif
  if (unread (0,unreadToken)) return;
  tokenMain:
  for (;;)
  {
    for (;;)
    {
      c=readCharacter ();
      if (type (c)!=SPACE) break;
    }
    if (c==';')
    {
        for (;c!='\n';c=readCharacter ());
	      goto tokenMain;
    } else goto tokenEnd;
  }
  tokenEnd:
  i=0;
  tk[i]=c;
  i++;
  if (c=='\"' || c=='\'')
  {
    for (;;)
    {
      c=readCharacter ();
      tk[i]=c;
      i++;
      if (c==tk[0]) break;
    }
  }
  else if (type (c)!=OTHER)
  {
    for (;;i++)
    {
      c=readCharacter ();
      tk[i]=c;
      t=type (c);
      if (tk[0]=='.' || t==SPACE || t==OTHER || type (tk[0])==NUMBER && t==LETTER)
      {
        unread (c,unreadCharacter);
	      break;
      }
    }
  }
  tk[i]=0;
#ifdef DEBUG
  string_write (STDOUT,"Function token return \'");
  string_write (STDOUT,tk);
  string_write (STDOUT,"\'\n");
#endif
}

int16 hexToInt16 () {
  int16 i,r=0;

  for (i=1;tk[i];i++) {
    r<<=4;
    if ('0'<=tk[i] && tk[i]<='9') r+=tk[i]-'0';
    else if ('a'<=tk[i] && tk[i]<='f') r+=tk[i]-'a'+10;
    else syntaxError ("toInt16");
  }
  return r;
}

int16 toInt16 () {
  int16 i,r=0;
  
  if (tk[0]=='0' && !tk[1]) {
    token ();
    if (tk[0]=='x') {
      return hexToInt16 ();
    } else {
      unread (-1,unreadToken);
      return 0;
    }
  }
  for (i=0;tk[i];i++)
    if ('0'<=tk[i] && tk[i]<='9') {
      r*=10;
      r+=tk[i]-'0';
    } else syntaxError ("toInt16");
  return r;
}

int8 constant (int16 *d) {
  if (type (tk[0])==NUMBER) {
      *d=toInt16 (tk);
      return -1;
  } else if (tk[0]=='\'' && tk[2]=='\'' && !tk[3]) {
    *d=tk[1];
    return -1;
  }
  *d=isLabel (tk);
  if (*d==-1) return 0;
  *d=dlabel[*d];
  return -1;
}

int8 isRegister (int8 *s) {
  if (s[0]=='r' && !s[2] && '0'<=s[1] && s[1]<='7') return s[1]-'0';
  return -1;
}

int8 isSegment (int8 *s) {
  if (s[0]=='s' && !s[2] && '0'<=s[1] && s[1]<='7') return s[1]-'0';
  return -1;
}

int16 isLabel (int8 *s) {
  int16 i;

  for (i=0;i<nlabel;i++)
      if (!string_compare (label[i],s)) return i;
  return -1;  
}

int8 assemble_label_define (int8 l) {
  int16 d;

  token ();
  if (type (tk[0])!=LETTER) return 0;
  string_copy (label[nlabel],tk);
  if (l) d=indexCode<<1;
  else {
    token ();
    if (!constant (&d)) return 0;
  }
  dlabel[nlabel]=d;
  nlabel++;
  return -1;
}

int8 assemble_ssave () {
  token ();
  ri0=isSegment (tk);
  if (ri0==-1) {
    if (string_compare (tk,"seg")) return 0;
    token ();
    if (string_compare (tk,"(")) return 0;
    token ();
    ri1=isRegister (tk);
    if (ri1==-1) return 0;
    token ();
    if (string_compare (tk,")")) return 0;
    token ();
    if (string_compare (tk,",")) return 0;
    token ();
    ri0=isSegment (tk);
    if (ri0==-1) return 0;
    code[indexCode]=0xf800|ri0|(ri1<<3);
    indexCode++;
  } else {
    token ();
    if (string_compare (tk,",")) return 0;
    token ();
    ri0=isSegment (tk);
    if (ri0==-1) return 0;
    code[indexCode]=0xe400|ri0;
    indexCode++;
  }
  return -1;
}

int8 assemble_cmp (int16 c) {
  int16 d;
  int8 s;

  token ();
  ri0=isRegister (tk);
  if (ri0==-1) return 0;
  token ();
  if (string_compare (tk,",")) return 0;
  token ();
  ri1=isRegister (tk);
  if (ri1==-1) {
    code[indexCode]=c|ri0|0x40;
    indexCode++;
    if (!constant (&d)) save_unresolved (UNRESOLVED);
    else code[indexCode]=d;
    indexCode++;
    return -1;
  }
  token ();
  s=tk[0];
  if (string_compare (tk,"+") && string_compare (tk,"-")) {
    code[indexCode]=c|ri0|(ri1<<3)|0x80;
    indexCode++;
    unread (-1,unreadToken);
    return -1;
  }
  code[indexCode]=c|ri0|(ri1<<3)|0xc0;
  indexCode++;
  if (s=='-') s=UNRESOLVED_NEG;
  else s=UNRESOLVED;
  if (!constant (&d)) save_unresolved (s);
  else {
    if (s==UNRESOLVED_NEG) d=-d;
    code[indexCode]=d;
  }
  indexCode++;
  return -1;
}

int8 assemble_v1 (int16 c,int8 *i) {
  int16 d;

  token ();
  ri0=isRegister (tk);
  if (ri0==-1) return 0;
  token ();
  if (string_compare (tk,",")) return 0;
  token ();
  ri1=isRegister (tk);
  if (ri1==-1) return 0;
  token ();
  if (string_compare (tk,i)) return 0;
  token ();
  ri2=isRegister (tk);
  if (ri2==-1) {
    if (i[0]='^') return 0;
    ri2=ri1;
  } else {
    token ();
    if (string_compare (tk,i)) {
      code[indexCode]=c|ri0|(ri1<<3)|(ri2<<6);
      indexCode++;
      unread (-1,unreadToken);
      return -1;
    }
  token ();
  }
  if (!constant (&d)) return 0;
  code[indexCode]=c|ri0|(ri1<<3)|(ri2<<6);
  if (d) {
    code[indexCode]|=0x200;
    indexCode++;
    code[indexCode]=d;
  }
  indexCode++;
  return -1;
}

int8 assemble_v2 (int16 c1,int16 c2,int8 s) {
  int16 d;

  token ();
  ri0=isRegister (tk);
  if (ri0==-1) return 0;
  token ();
  if (string_compare (tk,",")) return 0;
  token ();
  ri1=isRegister (tk);
  if (ri1==-1) return 0;
  token ();
  if (tk[0]!=s || tk[1]!=0) return 0;
  token ();
  ri2=isRegister (tk);
  if (ri2==-1) {
    code[indexCode]=c1|ri0|(ri1<<3);
    indexCode++;
    if (s=='-') s=UNRESOLVED_NEG;
    else s=UNRESOLVED;
    if (!constant (&d)) save_unresolved (s);
    else {
      code[indexCode]=d;
    }
    indexCode++;
    return -1;
  } else {
    token ();
    if (tk[0]!=s || tk[1]!=0) {
      code[indexCode]=c2|ri0|(ri1<<3)|(ri2<<6);
      indexCode++;
      unread (-1,unreadToken);
      return -1;
    }
    token ();
    code[indexCode]=c2|ri0|(ri1<<3)|(ri2<<6);
    if (!constant (&d)) {
      code[indexCode]|=0x200;
      indexCode++;
      save_unresolved (UNRESOLVED);
    } else if (d) {
      code[indexCode]|=0x200;
      indexCode++;
      code[indexCode]=d;
    }
    indexCode++;
  }
  return -1;
}

int8 assemble_shift (int16 c1,int16 c2) {
  int16 d;

  token ();
  ri0=isRegister (tk);
  if (ri0==-1) return 0;
  token ();
  if (string_compare (tk,",")) return 0;
  token ();
  ri1=isRegister (tk);
  if (ri1==-1) return 0;
  token ();
  if (string_compare (tk,"<") && string_compare (tk,">")) return 0;
  token ();
  ri2=isRegister (tk);
  if (ri2==-1) {
    if (!constant (&d)) return 0;
    code[indexCode]=c2|ri0|(ri1<<3)|((d&0xf)<<6);
    indexCode++;
  } else {
    code[indexCode]=c1|ri0|(ri1<<3)|(ri2<<6);
    indexCode++;
  }
  return -1;
}

int8 assemble_r (int16 c) {
  token ();
  ri0=isRegister (tk);
  if (ri0==-1) return 0;
  code[indexCode]=c|ri0;
  indexCode++;
}

int8 assemble_rr (int16 c) {
  token ();
  ri0=isRegister (tk);
  if (ri0==-1) return 0;
  token ();
  if (string_compare (tk,",")) return 0;
  token ();
  ri1=isRegister (tk);
  if (ri0==-1) return 0;
  code[indexCode]=c|ri0|(ri1<<3);
  indexCode++;
}

int8 assemble_jmps_calls (int16 c) {
  int16 d;
  
  token ();
  ri0=isSegment (tk);
  if (ri0==-1) return 0;
  token ();
  if (tk[0]!=':') return 0;
  token ();
  code[indexCode]=c|ri0;
  indexCode++;
  if (!constant (&d)) save_unresolved (UNRESOLVED);
  else code[indexCode]=d;
  indexCode++;
  return -1;
}

void assemble_string () {
  int32 i;
  int16 d,e;

  for (i=1;;i++) {
    if (tk[i]=='\"') break;
    if (i&1) e=tk[i]&0xff;
    else {
      d=tk[i]&0xff;
      code[indexCode]=(d<<8)|e;
      indexCode++;
    }
  }
  if (!(i&1)) {
    code[indexCode]=e;
    indexCode++;
  }
}

int8 assemble_db () {
  int32 i;
  int16 d,e;
  
  for (i=1;;i++) {
    token ();
    if (tk[0]=='\"') {
      assemble_string ();
      i=0;
      e=0;
    } else if (constant (&d)) {
      if (i&1) e=d&0xff;
      else {
        d&=0xff;
        code[indexCode]=(d<<8)|e;
        indexCode++;
      }
    } else return 0;
    token ();
    if (string_compare (tk,",")) {
      if (i&1) {
        code[indexCode]=e;
        indexCode++;
      }
      unread (-1,unreadToken);
      break;
    }
  }
  return -1;
}

int8 assemble_dw () {
  int16 d;
  
  for (;;) {
    token ();
    if (constant (&d)) {
      code[indexCode]=d;
      indexCode++;
    } else return 0;
    token ();
    if (string_compare (tk,",")) {
      unread (-1,unreadToken);
      break;
    }
  }
  return -1;
}

void save_unresolved (int8 u) {
  string_copy (unresolved[nunresolved],tk);
  dunresolved[nunresolved]=indexCode<<1;
  unresolvedCond[nunresolved]=u;
  nunresolved++;
}

int8 assemble_jcc (int16 c) {
  int16 s=0,d;
  
  token ();
  if (tk[0]=='+' || tk[0]=='-') {
    s=tk[0];
    token ();
  }
  if (!constant (&d)) {
    code[indexCode]=c;
    string_copy (unresolved[nunresolved],tk);
    dunresolved[nunresolved]=indexCode<<1;
    unresolvedCond[nunresolved]=UNRESOLVED_10;
    nunresolved++;
    indexCode++;
    return -1;
  }
  if (s=='+') s=d>>1;
  else if (s=='-') s=-d>>1;
  else s=(d>>1)-indexCode;
  if ((s&0xfe00)==0xfe00 || !(s&0xfe00)) code[indexCode]|=(0x3ff&s);
  else {
    syntaxError ("conditional jump >i10");
    return 0;
  }
  indexCode++;
  return -1;
}

int8 assemble_load_save_nseg (int16 c1,int16 c2) {
  int16 d;
  int8 s;

  ri1=isRegister (tk);
  if (ri1==-1) {
    if (!constant (&d)) return 0;
    code[indexCode]=c1|ri0|0x40;
    indexCode++;
    code[indexCode]=d;
    indexCode++;
    return -1;
  }
  token ();
  s=tk[0];
  if (string_compare (tk,"+") && string_compare (tk,"-")) {
    if (string_compare (tk,"]")) return 0;
    code[indexCode]=c1|ri0|(ri1<<3)|0x80;
    indexCode++;
    return -1;
  }
  token ();
  ri2=isRegister (tk);
  if (ri2==-1) {
    code[indexCode]=c1|ri0|(ri1<<3)|0xc0;
    indexCode++;
    if (s=='-') s=UNRESOLVED_NEG;
    else s=UNRESOLVED;
    if (!constant (&d)) save_unresolved (s);
    else {
      if (s==UNRESOLVED_NEG) d=-d;
      code[indexCode]=d;
    }
    indexCode++;
    token ();
    if (string_compare (tk,"]")) return 0;
    return -1;
  }
  token ();
  s=tk[0];
  if (string_compare (tk,"+") && string_compare (tk,"-")) {
    if (string_compare (tk,"]")) return 0;
    code[indexCode]=c2|ri0|(ri1<<3)|(ri2<<6);
    indexCode++;
    return -1;
  }
  if (!constant (&d)) return 0;
  code[indexCode]=c2|ri0|(ri1<<3)|(ri2<<6)|0x200;
  indexCode++;
  if (s=='-') d=-d;
  code[indexCode]=d;
  indexCode++;
  return -1;
}

int8 assemble_jmp_call (int16 c1,int16 c2,int16 c3) {
  int16 d,e;

  token ();
  if (string_compare (tk,"[")) {
    code[indexCode]=c3;
    indexCode++;
    if (!constant (&d)) save_unresolved (0);
    else {
      e=(indexCode-d-1)>>1;
      if (c3==0xc400 && (!(e&0xfe00) || (e&0xfe00)==0xfe00)) {
        code[indexCode-1]=0x3000|(e&0x3ff);
      } else {
        code[indexCode]=d;
      }
    }
    indexCode++;
    return -1;
  }
  token ();
  return assemble_load_save_nseg (c1,c2);
}

int8 assemble_load_save (int16 c1,int16 c2,int16 c3,int16 c4) {
  int16 d;
  int8 s;
  
  token ();
  ri0=isRegister (tk);
  if (ri0==-1) return 0;
  token ();
  if (string_compare (tk,",")) return 0;
  token ();
  if (!string_compare (tk,"[")) {
    token ();
    ri1=isSegment (tk);
    if (ri1==-1)
      return assemble_load_save_nseg (c1,c2);
    token ();
    if (string_compare (tk,":")) return 0;
    token ();
    ri2=isRegister (tk);
    if (ri2==-1) {
      if (!constant (&d)) save_unresolved (UNRESOLVED);
      else code[indexCode]=c3|ri0|(ri1<<3);
      indexCode++;
      if (s=='-') d=-d;
      code[indexCode]=d;
      indexCode++;
      token ();
      if (string_compare (tk,"]")) return 0;
      return -1;
    }
    token ();
    s=tk[0];
    if (string_compare (tk,"+") && string_compare (tk,"-")) {
      if (string_compare (tk,"]")) return 0;
      code[indexCode]=c3|ri0|(ri1<<3)|(ri2<<6)|0x200;
      indexCode++;
      code[indexCode]=0;
      indexCode++;
      return -1;
    }
    token ();
    if (!constant (&d)) save_unresolved (UNRESOLVED);
    else code[indexCode]=c3|ri0|(ri1<<3)|(ri2<<6)|0x200;
    indexCode++;
    if (s=='-') d=-d;
    code[indexCode]=d;
    indexCode++;
    token ();
    if (string_compare (tk,"]")) return 0;
    return -1;
  }
  ri1=isSegment (tk);
  if (ri1==-1) {
    if (c1==0x4400) {
      ri1=isRegister (tk);
      if (ri1!=-1) {
        code[indexCode]=0x2000|ri0|(ri1<<3);
        indexCode++;
      } else {
        code[indexCode]=0xc400|ri0;
        indexCode++;
        if (!constant (&d)) save_unresolved (UNRESOLVED);
        else code[indexCode]=d;
        indexCode++;
      }
      return -1;
    } else return 0;
  }
  code[indexCode]=c4|ri0|(ri1<<3);
  indexCode++;
}

int8 assemble_include () {
  int16 l;
  
  token ();
  if (fileIndex>=MAXFILES) syntaxError ("too many files open");
  l=string_length (tk)-1;
  
  if (l<=0 || tk[0]!='\'' || tk[l]!='\'') syntaxError ("include");
  tk[l]=0;
  files[fileIndex]=system_open (tk+1,O_RDONLY,0);
  fileSource=files[fileIndex];
  filesLine[fileIndex-1]=lineNumber;
  lineNumber=1;
  fileIndex++;
  return -1;
}

int8 syntaxASM ()
{
  for (;!eof;) {
    token ();
    switch (tk[0]) {
      case 0:
      break;
      case 'a':             // adc,add,and
        if (!string_compare (tk,"adc")) {
          if (!assemble_v2 (0x5c00,0x6000,'+')) return 0;
        } else if (!string_compare (tk,"add")) {
          if (!assemble_v2 (0x5400,0x5800,'+')) return 0;
        } else if (!string_compare (tk,"and")) {
          if (!assemble_v1 (0x7800,"&")) return 0;
        } else return 0;
      break;
      case 'c':             // call,calls,cli,cmp
        if (!string_compare (tk,"call")) {
          if (!assemble_jmp_call (0xd800,0xdc00,0xfc00)) return 0;
        } else if (!string_compare (tk,"calls")) {
          if (!assemble_jmps_calls (0xe800)) return 0;
        } else if (!string_compare (tk,"cli")) {
          code[indexCode]=0xc800;
          indexCode++;
        } else if (!string_compare (tk,"cmp")) {
          if (!assemble_cmp (0x7400)) return 0;
        } else return 0;
      break;
      case 'd':             // define,div
        if (!string_compare (tk,"define")) {
          if (!assemble_label_define (0)) return 0;
        } else if (!string_compare (tk,"db")) {
          if (!assemble_db ()) return 0;
        } else if (!string_compare (tk,"div")) {
          code[indexCode]=0x0400;
          indexCode++;
        } else if (!string_compare (tk,"dw")) {
          if (!assemble_dw ()) return 0;
        } else return 0;
      break;
      case 'i':             // inport
        if (!string_compare (tk,"inport")) {
          if (!assemble_cmp (0x3400)) return 0;
        } else if (!string_compare (tk,"include")) {
          if (!assemble_include ()) return 0;
        } else return 0;
      break;
      case 'j':             // ja,jae,jb,jbe,je,jg,jge,jl,jle,jmp,jmps,jne,jno,jo
        switch (tk[1]) {
          case 'a':             // ja,jae
            if (!string_compare (tk,"ja")) {
              if (!assemble_jcc (0xac00)) return 0;
            } else if (!string_compare (tk,"jae")) {
              if (!assemble_jcc (0xb000)) return 0;
            } else return 0;
          break;
          case 'b':             // jb,jbe
            if (!string_compare (tk,"jb")) {
              if (!assemble_jcc (0x9c00)) return 0;
            } else if (!string_compare (tk,"jbe")) {
              if (!assemble_jcc (0xa000)) return 0;
            } else return 0;
          break;
          case 'z':
          case 'e':             // je, jz
            if (!tk[2]) {
              if (!assemble_jcc (0xa400)) return 0;
            } else return 0;
          break;
          case 'g':             // jg,jge
            if (!string_compare (tk,"jg")) {
              if (!assemble_jcc (0xbc00)) return 0;
            } else if (!string_compare (tk,"jge")) {
              if (!assemble_jcc (0xc000)) return 0;
            } else return 0;
          break;
          case 'l':             // jl,jle
            if (!string_compare (tk,"jl")) {
              if (!assemble_jcc (0xb400)) return 0;
            } else if (!string_compare (tk,"jle")) {
              if (!assemble_jcc (0xb800)) return 0;
            } else return 0;
          break;
          case 'm':             // jmp,jmps
            if (!string_compare (tk,"jmp")) {
              if (!assemble_jmp_call (0x4000,0x4400,0xc400)) return 0;
            } else if (!string_compare (tk,"jmps")) {
          if (!assemble_jmps_calls (0xe000)) return 0;
            } else return 0;
          break;
          case 'n':             // jne,jno
            if (!string_compare (tk,"jne") || !string_compare (tk,"jnz")) {
              if (!assemble_jcc (0xa800)) return 0;
            } else if (!string_compare (tk,"jno")) {
              if (!assemble_jcc (0xf000)) return 0;
            } else return 0;
          break;
          case 'o':             // jo
            if (!string_compare (tk,"jo")) {
              if (!assemble_jcc (0xec00)) return 0;
            } else return 0;
          break;
          default:
            return 0;
        }
      break;
      case 'l':             // label,load
        if (!string_compare (tk,"label")) {
          if (!assemble_label_define (-1)) return 0;
        } else if (!string_compare (tk,"load")) {
          if (!assemble_load_save (0x4400,0x4000,0x3c00,0x2400)) return 0;
        } else return 0;
      break;
      case 'n':             // neg,nop,not
        if (!string_compare (tk,"neg")) {
          if (!assemble_r (0x1800)) return 0;
        } else if (!string_compare (tk,"nop")) {
          code[indexCode]=0x2012;
          indexCode++;
        } else if (!string_compare (tk,"not")) {
          if (!assemble_r (0x1c00)) return 0;
        } else return 0;
      break;
      case 'm':             // mul
        if (!string_compare (tk,"mul")) {
          code[indexCode]=0x0000;
          indexCode++;
        } else return 0;
      break;
      case 'o':             // or,outport
        if (!string_compare (tk,"or")) {
          if (!assemble_v1 (0x7c00,"|")) return 0;
        } else if (!string_compare (tk,"outport")) {
          if (!assemble_cmp (0x3800)) return 0;
        } else return 0;
      break;
      case 'p':             // pop,push
        if (!string_compare (tk,"pop")) {
          if (!assemble_r (0xd400)) return 0;
        } else if (!string_compare (tk,"push")) {
          if (!assemble_r (0xd000)) return 0;
        } else return 0;
      break;
      case 'r':             // ret,reti,rets
        if (!string_compare (tk,"ret")) {
          code[indexCode]=0x1000;
          indexCode++;
        } else if (!string_compare (tk,"reti")) {
          code[indexCode]=0x1400;
          indexCode++;
        } else if (!string_compare (tk,"rets")) {
          code[indexCode]=0xf400;
          indexCode++;
        } else return 0;
      break;
      case 's':             // save,sar,sbb,shl,shr,sti,sub
        if (!string_compare (tk,"save")) {
          if (!assemble_load_save (0x5000,0x4c00,0x4800,0x2800)) return 0;
        } else if (!string_compare (tk,"sar")) {
          if (!assemble_shift (0x9400,0x9800)) return 0;
        } else if (!string_compare (tk,"sbb")) {
          if (!assemble_v2 (0x6c00,0x7000,'-')) return 0;
        } else if (!string_compare (tk,"shl")) {
          if (!assemble_shift (0x8400,0x8800)) return 0;
        } else if (!string_compare (tk,"shr")) {
          if (!assemble_shift (0x8c00,0x9000)) return 0;
        } else if (!string_compare (tk,"ssave")) {
          if (!assemble_ssave ()) return 0;
        } else if (!string_compare (tk,"sti")) {
          code[indexCode]=0xcc00;
          indexCode++;
        } else if (!string_compare (tk,"sub")) {
          if (!assemble_v2 (0x6400,0x6800,'-')) return 0;
        } else return 0;
      break;
      case 'u':             // umul,udiv
        if (!string_compare (tk,"umul")) {
          code[indexCode]=0x0800;
          indexCode++;
        } else if (!string_compare (tk,"udiv")) {
          code[indexCode]=0x0c00;
          indexCode++;
        } else return 0;
      break;
      case 'x':             // xor
        if (!string_compare (tk,"xor")) {
          if (!assemble_v1 (0x8000,"^")) return 0;
        } else return 0;
      break;
      default:
        return 0;
    }
  }
  return -1;
}

void syntaxError (int8 *s) {
  string_write (STDOUT,"Syntax error '");
  string_write (STDOUT,s);
  string_write (STDOUT,"' line ");
  string_write (STDOUT,string_decimal (lineNumber));
  string_write (STDOUT,".\n-->");
  string_write (STDOUT,line);
  string_write (STDOUT,"<--\n");
  system_exit (2);
}

int main (int argc,char *argv[]) {
  int16 i,d,u;
  if (argc<3)
  {
    string_write (STDOUT,"usage: \'asm sourcefile binfile\'\n");
    return 1;
  }
  eof=0;
  indexCode=0;
  nlabel=0;
  nunresolved=0;
  fileIndex=0;
  files[fileIndex]=system_open (argv[1],O_RDONLY,0);
  fileSource=files[fileIndex];
  fileIndex++;
  fileBIN=system_open (argv[2],O_WRONLY|O_CREAT|O_TRUNC,0666);
  
  lineNumber=1;

  if (!syntaxASM ()) syntaxError (line);
  else {
    for (i=0;i<nunresolved;i++) {
      u=isLabel (unresolved[i]);
      if (u==-1) {
        string_write (STDOUT,"Unresolved label\n");
        syntaxError (unresolved[i]);
      }
      if (unresolvedCond[i]==UNRESOLVED_10) {
        d=(dlabel[u]-dunresolved[i])>>1;
        if (!(d&0xfe00) || (d&0xfe00)==0xfe00) code[dunresolved[i]>>1]|=(0x3ff&d);
        else {
          string_write (STDOUT,"Conditional jump >i10\n");
          syntaxError (unresolved[i]);
          return 0;
        }
      } else if (unresolvedCond[i]==UNRESOLVED_NEG)
        code[dunresolved[i]>>1]=-dlabel[u];
      else code[dunresolved[i]>>1]=dlabel[u];
    }
    if (indexCode)
      system_write (fileBIN,code,indexCode<<1);
  }
  system_close (fileBIN);
  system_close (fileSource);
  return 0;
}

