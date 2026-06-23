/*
**	Definiciones de funciones utiles para GNUby
**
**	Marcelo Reina Aguilar 2001
*/

int64 system_write (int64,void *,int64);
int64 system_read (int64,void *,int64);
int64 system_open (void *,int64,int64);
int64 system_close (int64);
int64 system_exit (int64);
int8 *string_hexadecimal (int64);
int8 *string_hex (int64);
void reverse (int8 *);
int8 *string_decimal (int64);
void string_write (int64,int8 *);
int64 string_length (int8 *);
int8 *string_copy (int8 *,int8 *);
int8 *string_concat (int8 *,int8 *);
int64 string_compare (int8 *,int8 *);
int64 string_indexOfChar (int8 *,int8);

