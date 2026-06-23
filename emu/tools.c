/*
**	Definiciones de funciones utiles para GNUby
**
**	Marcelo Reina Aguilar 2001
*/

#include "define.h"
#include "struct.h"
#include "tools.h"

extern int64 syscall ();

float64 pe10 (float64 m,int64 e) {
  float64 r,d=10;
  int64 i;
  
  r=m;
  if (e<0) {e=-e;d=0.1;}
  for (i=0;i<e;i++) r=r*d;
  return r;
}


int64 system_write (int64 f,void *b,int64 n) {

    return syscall (1,f,b,n);
}


int64 system_read (int64 f,void *b,int64 n) {

    return syscall (0,f,b,n);
}

int64 system_open (void *n,int64 a,int64 m) {

    return syscall (2,n,a,m);
}

int64 system_close (int64 f) {

    return syscall (3,f);
}

int64 system_exit (int64 f) {

    return syscall (60,f);
}

int8 *string_hexadecimal_digit (int64 h,int8 d) {
  static int8 *digit="0123456789abcdef",s[20];
  int64 i;
  
  s[0]='0';s[1]='x';s[d+2]=0;
  for (i=d+1;i>=2;i--,h>>=4) {
    s[i]=digit[0xf & h];
  }
  return s;
}

int8 *string_hexadecimal (int64 h) {
  return string_hexadecimal_digit (h,16);
}

int8 *string_hex (int64 h) {
  return string_hexadecimal_digit (h,4);
}

void reverse (int8 *s) {
  int8 *p;
  int8 a;
  
  p=s+string_length (s);
  for (;;) {
    if (p==s) break;
    p=p-1;
    if (p==s) break;
    a=*p;
    *p=*s;
    *s=a;
    s=s+1;
  }
}

int8* string_decimal (int64 c) {
  int64 r;
  static int8 s[256];
  int8 *p;
  
  p=s;
  if (c<0) {
    c=-c;
    *p='-';
    p=p+1;
  }
  for (;;) {
    r=c%10;
    c=c/10;
    *p='0'+r;
    p=p+1;
    if (c==0) break;
  }
  *p=0;
  if (*s=='-') reverse (s+1);
  else reverse (s);
  return s;
}

void string_write (int64 f,int8 *c) {
  system_write (f,c,string_length (c));
}

int64 string_length (int8 *s) {
  int64 i;

  for (i=0;s[i];i++) ;
  return i;
}

int8 *string_copy (int8 *d,int8 *s) {
  int8 *r;

  r=d;
  for (;;s++,d++) {
    *d=*s;
    if (!*s) break;
  }
  return r;
}

int8 *string_concat (int8 *d,int8 *s) {
  int8 *r;

  r=d;
  string_copy (d+string_length (d),s);
  return r;
}

int64 string_compare (int8 *d,int8 *s) {
  for (;;d++,s++) {
    if (*d>*s) 
      return 1;
    else if (*d<*s)
      return -1;
    else if (!*d) return 0;
  }
}

int64 string_indexOfChar (int8 *s,int8 c) {
  int64 i;

  for (i=0;s[i]!=0;i++)
    if (s[i]==c) return i;
  return -1;
}

