#include "../include/efi.h"
#include "console.h"

void print(CHAR16* str) {
    ST->ConOut->OutputString(ST->ConOut, str);
}

void println(CHAR16* str) {
    print(str); print(L"\r\n");
}

void clear(void) {
    ST->ConOut->ClearScreen(ST->ConOut);
}

void set_color(UINTN fg, UINTN bg) {
    ST->ConOut->SetAttribute(ST->ConOut, fg | (bg << 4));
}

void print_uint(UINT64 n) {
    if (n == 0) { print(L"0"); return; }
    CHAR16 buf[20];
    int i = 19;
    buf[i] = 0;
    while (n > 0) { buf[--i] = L'0' + (n % 10); n /= 10; }
    print(&buf[i]);
}

void read_line(CHAR16* buf, UINTN max) {
    UINTN i = 0;
    while (1) {
        EFI_INPUT_KEY key;
        while (ST->ConIn->ReadKeyStroke(ST->ConIn, &key) != EFI_SUCCESS);
        if (key.UnicodeChar == L'\r') {
            buf[i] = 0; print(L"\r\n"); return;
        } else if (key.UnicodeChar == 8) {
            if (i > 0) { i--; print(L"\b \b"); }
        } else if (key.UnicodeChar >= 32 && i < max - 1) {
            buf[i++] = key.UnicodeChar;
            CHAR16 ch[2] = { key.UnicodeChar, 0 };
            print(ch);
        }
    }
}

int streq(CHAR16* a, CHAR16* b) {
    while (*a && *b) { if (*a != *b) return 0; a++; b++; }
    return *a == *b;
}

UINTN strlen16(CHAR16* s) {
    UINTN n = 0; while (s[n]) n++; return n;
}

int startswith(CHAR16* str, CHAR16* prefix) {
    while (*prefix) { if (*str++ != *prefix++) return 0; }
    return 1;
}

void strcpy16(CHAR16* dst, CHAR16* src) {
    while (*src) { *dst++ = *src++; } *dst = 0;
}
