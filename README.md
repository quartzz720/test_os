# TestOS 🐧

A simple UEFI bootable OS shell written in pure C, no stdlib.

## Features

- **Pure C implementation** — no standard library, minimal dependencies
- **UEFI bootable** — runs directly on UEFI firmware
- **File system support** — FAT filesystem interaction via UEFI
- **Shell commands** — 20+ built-in commands for file/directory management
- **Cross-platform build** — compiles on MSYS2 (Windows)

## Requirements

- MSYS2 with UCRT64 environment
- `gcc` (mingw-w64-ucrt-x86_64)
- `make`
- QEMU with EDK2 firmware support

### Install Dependencies (MSYS2)

```bash
pacman -S mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-make mingw-w64-ucrt-x86_64-qemu
```

## Build & Run

### Build [MSYS2]
```bash
make
```
### Create a virtual disk [WSL]
```bash
dd if=/dev/zero of=esp.img bs=1M count=32
mkfs.vfat esp.img
```
### Put a compiled BOOTX64.EFI into a virtual disk [WSL]
```bash
mkdir -p /tmp/esp_mount
sudo mount -o loop esp.img /tmp/esp_mount
sudo mkdir -p /tmp/esp_mount/EFI/BOOT
sudo cp /location/to/BOOTX64.EFI /tmp/esp_mount/EFI/BOOT/
sudo umount /tmp/esp_mount
```
### Run [MSYS2 / WSL]
```bash
qemu-system-x86_64 \
  -drive if=pflash,format=raw,readonly=on,file="/ucrt64/share/qemu/edk2-x86_64-code.fd" \
  -drive if=pflash,format=raw,file="/ucrt64/share/qemu/edk2-i386-vars.fd" \
  -drive file=esp.img,format=raw \
  -net none
```

## Commands

| Command | Arguments | Description |
|---------|-----------|-------------|
| `help` | — | Show available commands |
| `cls` | — | Clear screen |
| `ver` | — | OS version info |
| `echo` | `[message]` | Print text |
| `color` | `[fg] [bg]` | Change text color (0-15) |
| `dir` / `ls` | — | List files in current directory |
| `type` | `[filename]` | Print file contents |
| `more` | `[filename]` | View file with pagination |
| `mkdir` | `[dirname]` | Create directory |
| `cd` | `[path]` | Change directory (or `cd ..` for parent) |
| `write` | `[filename] [text]` | Write text to file |
| `edit` | `[filename]` | Edit file (commands: `a`, `e <n>`, `d <n>`, `w`, `q`) |
| `del` | `[filename]` | Delete file |
| `ren` | `[oldname] [newname]` | Rename file |
| `rmdir` | `[dirname]` | Delete directory |
| `rmdir /s` | `[dirname]` | Recursively delete directory |
| `copy` | `[src] [dst]` | Copy file |
| `move` | `[src] [dst]` | Move file (copy + delete) |
| `tree` | — | Display directory tree |

## Editor (`edit` command)

Simple line-based editor mode:

```
Z:\> edit myfile.txt
=== edit mode ===
1: hello world
2: this is line 2
(empty)
Commands: a (append), e <n>, d <n>, w (save), q (cancel)
```

### Editor Commands

- `a` — Append new line
- `e <n>` — Edit line number `n`
- `d <n>` — Delete line number `n`
- `w` — Save file (prompts for filename if new file)
- `q` — Cancel and exit

## Project Structure

```
myos/
├── include/
│   └── efi.h              — EFI types, structs, GUIDs
├── kernel/
│   ├── console.c / console.h     — I/O & colors (print, read_line, set_color)
│   ├── fs.c / fs.h               — Filesystem commands (dir, cd, edit, etc)
│   ├── commands.c / commands.h   — Basic commands (help, ver, echo, color)
│   ├── shell.c / shell.h         — Main shell loop & command dispatch
│   └── fs.h                      — Helper functions (build_path, open_root)
├── boot/
│   ├── bootloader.c              — EFI entry point (efi_main)
│   └── efi/boot/BOOTX64.EFI      — Compiled bootloader (generated)
├── Makefile                      — Build configuration
├── linker.ld                     — Linker script (placeholder)
└── README.md                     — This file
```

## Implementation Notes

### Unity Build

Due to mingw `-shared` linker limitations, all kernel `.c` files are `#include`-d into `boot/bootloader.c` rather than compiled separately. This simplifies dependency management in a freestanding environment.

### Global Variables

- `EFI_SYSTEM_TABLE* ST` — Global system table handle
- `EFI_HANDLE IH` — Global image handle
- `CHAR16 cwd[256]` — Current working directory (updated by `cd` command)

### No Standard Library

The project uses `-ffreestanding -nostdlib` and provides minimal helper functions:
- `strcpy16()`, `strlen16()`, `streq()` — String utilities
- `print()`, `println()`, `print_uint()` — Basic I/O
- `set_color()`, `clear()` — Console control
- `open_root()`, `build_path()` — Filesystem helpers

### Stack Optimization

Large local buffers (in `cmd_edit`) are declared `static` to avoid stack overflow and `___chkstk_ms` linker errors. Compile flags include `-fno-stack-check` to prevent stack probing instrumentation.

## Color Codes (not implemented yet)

Colors are specified as numbers 0–15 (standard UEFI palette):

```
0=Black       8=Gray
1=Blue        9=Light Blue
2=Green      10=Light Green
3=Cyan       11=Light Cyan
4=Red        12=Light Red
5=Magenta    13=Light Magenta
6=Yellow     14=Light Yellow
7=White      15=Light White
```

Example: `color 14 0` sets yellow text on black background.

## Troubleshooting

### Build Error: `undefined reference to ___chkstk_ms`

Ensure `-fno-stack-check` is in `CFLAGS` in the Makefile:

```makefile
CFLAGS  = -ffreestanding -fno-stack-protector -fshort-wchar -mno-red-zone \
          -fno-builtin -fno-stack-check -Wall -Wextra -std=c11 -I include -I kernel
```

<!-- ### QEMU: "Could not open disk image"

Make sure the FAT image path is correct:
```bash
-drive "file=fat:rw:$(cygpath -m $(pwd)/boot/efi),format=raw"
```

On MSYS2, `$(pwd)` returns POSIX path; `cygpath -m` converts it to Windows path. -->

---

**Author**: quartzz720 
**Language**: C  
**Build Tool**: Makefile (GCC)  
**Target**: UEFI x86_64
