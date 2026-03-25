#include "../include/efi.h"
#include "console.h"
#include "hal.h"

void print(CHAR16* str) {
    hal_console_write(str);
}

void println(CHAR16* str) {
    print(str); print(L"\r\n");
}

// Print 8-bit ASCII text
void print_ascii(UINT8* str) {
    CHAR16 buf[512];
    UINTN i = 0;
    while (str[i] && i < 510) {
        buf[i] = (CHAR16)str[i];
        i++;
    }
    buf[i] = 0;
    print(buf);
}

void clear(void) {
    hal_console_clear();
}

void set_color(UINTN fg, UINTN bg) {
    hal_console_set_color(fg, bg);
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
        hal_console_read_key(&key);
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

// Convert CHAR16 string to raw bytes (8-bit ASCII/UTF-8)
UINTN char16_to_bytes(CHAR16* src, UINT8* dst, UINTN maxlen) {
    UINTN i = 0;
    while (src[i] && i < maxlen - 1) {
        if (src[i] < 128) {
            dst[i] = (UINT8)src[i];
        } else {
            dst[i] = '?';  // Replace non-ASCII with ?
        }
        i++;
    }
    dst[i] = 0;
    return i;
}
