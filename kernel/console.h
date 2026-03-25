#ifndef CONSOLE_H
#define CONSOLE_H

#include "../include/efi.h"

void  print(CHAR16* str);
void  println(CHAR16* str);
void  print_ascii(UINT8* str);
void  clear(void);
void  set_color(UINTN fg, UINTN bg);
void  print_uint(UINT64 n);
void  read_line(CHAR16* buf, UINTN max);
int   streq(CHAR16* a, CHAR16* b);
UINTN strlen16(CHAR16* s);
int   startswith(CHAR16* str, CHAR16* prefix);
void  strcpy16(CHAR16* dst, CHAR16* src);
UINTN char16_to_bytes(CHAR16* src, UINT8* dst, UINTN maxlen);

#endif
