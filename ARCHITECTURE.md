# TestOS Architecture

# $${\color{red}WARNING! \space SUBJECT \space TO \space BE \space UPDATED! }$$

## Overview

TestOS is a minimal UEFI-based shell operating system. It demonstrates a bare-metal C implementation without standard library dependencies, communicating directly with UEFI firmware APIs.

## Execution Flow

```
UEFI Firmware
    ↓
[BOOTX64.EFI] ← boot/bootloader.c (efi_main)
    ↓
Initialize global handles (ST, IH)
    ↓
Print banner
    ↓ 
shell() → command loop
    ↓
    Parse input → Dispatch command → Execute
    ↓
Loop until shutdown
```

## Module Architecture

### Core Modules

#### `kernel/console.c` / `console.h`
**Purpose**: I/O and text rendering

**Key Functions**:
- `print(CHAR16* str)` — Output UTF-16 string via UEFI
- `println(CHAR16* str)` — Print with newline
- `read_line(CHAR16* buf, UINTN max)` — Blocking input (handles backspace)
- `set_color(UINTN fg, UINTN bg)` — Set text color
- `clear()` — Clear screen
- `print_uint(UINT64 n)` — Print integer

**Global State**:
- `EFI_SYSTEM_TABLE* ST` — UEFI system table (used by all modules)
- `EFI_HANDLE IH` — UEFI image handle

---

#### `kernel/fs.c` / `fs.h`
**Purpose**: Filesystem operations and file commands

**Key Functions**:
- `open_root()` — Get EFI_FILE_PROTOCOL for root directory
- `build_path(CHAR16* path, CHAR16* name, CHAR16* result)` — Construct absolute path from `cwd`
- `cmd_dir()` — List directory contents
- `cmd_cd(CHAR16* input)` — Change directory
- `cmd_type(CHAR16* input)` — Display file contents
- `cmd_edit(CHAR16* input)` — Line-based file editor
- `cmd_mkdir()`, `cmd_del()`, `cmd_ren()`, `cmd_copy()`, `cmd_move()`, `cmd_rmdir()`, `cmd_more()`, `cmd_tree()` — File/directory management

**Global State**:
- `CHAR16 cwd[256]` — Current working directory path

**Static Buffers**:
```c
static CHAR16 lines[32][64];  // Editor buffer
static UINT8 buf[2048];        // File read buffer
static UINT8 out[2048];        // File write buffer
```

---

#### `kernel/commands.c` / `commands.h`
**Purpose**: Built-in system commands

**Commands**:
- `cmd_help()` — List available commands
- `cmd_ver()` — Display OS version
- `cmd_echo(CHAR16* input)` — Echo text
- `cmd_color(CHAR16* input)` — Set text color

---

#### `kernel/shell.c` / `kernel/shell.h`
**Purpose**: Main shell loop and command dispatcher

**Main Function**:
```c
void shell(void) {
    while (1) {
        print(cwd); print("> ");
        read_line(input, 256);
        // Parse and dispatch based on input
        if (streq(input, "help")) cmd_help();
        else if (startswith(input, "cd ")) cmd_cd(input);
        // ... more commands
    }
}
```

---

#### `boot/bootloader.c`
**Purpose**: UEFI entry point

**Key Function**:
```c
EFI_STATUS efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE* SystemTable) {
    ST = SystemTable;        // Store global handles
    IH = ImageHandle;
    
    #include "../kernel/console.c"   // Unity build pattern
    #include "../kernel/fs.c"
    #include "../kernel/commands.c"
    #include "../kernel/shell.c"
    
    clear();
    println(L"TestOS...");
    shell();
    return EFI_SUCCESS;
}
```

---

### Utility Functions (`kernel/console.c`)

**String utilities**:
```c
int streq(CHAR16* a, CHAR16* b)           // String equality
UINTN strlen16(CHAR16* s)                 // String length
int startswith(CHAR16* str, CHAR16* pre)  // Prefix check
void strcpy16(CHAR16* dst, CHAR16* src)   // Copy string
```

---

## Data Structures

### EFI System Table
```c
typedef struct {
    EFI_TABLE_HEADER Hdr;
    CHAR16* FirmwareVendor;
    UINT32 FirmwareRevision;
    EFI_HANDLE ConsoleInHandle;
    EFI_SIMPLE_TEXT_INPUT_PROTOCOL* ConIn;
    EFI_HANDLE ConsoleOutHandle;
    EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL* ConOut;
    EFI_HANDLE StandardErrorHandle;
    EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL* StdErr;
    EFI_RUNTIME_SERVICES* RuntimeServices;
    EFI_BOOT_SERVICES* BootServices;
    // ... more fields
} EFI_SYSTEM_TABLE;
```

### EFI File Protocol
```c
typedef struct {
    EFI_STATUS (*Open)(
        EFI_FILE_PROTOCOL* This,
        EFI_FILE_PROTOCOL** NewHandle,
        CHAR16* FileName,
        UINT64 OpenMode,
        UINT64 Attributes
    );
    EFI_STATUS (*Read)(EFI_FILE_PROTOCOL* This, UINTN* Size, VOID* Buffer);
    EFI_STATUS (*Write)(EFI_FILE_PROTOCOL* This, UINTN* Size, VOID* Buffer);
    EFI_STATUS (*Delete)(EFI_FILE_PROTOCOL* This);
    EFI_STATUS (*GetInfo)(/* ... */);
    EFI_STATUS (*SetInfo)(/* ... */);
    EFI_STATUS (*Flush)(EFI_FILE_PROTOCOL* This);
    EFI_STATUS (*Close)(EFI_FILE_PROTOCOL* This);
} EFI_FILE_PROTOCOL;
```

---

## Build System (Makefile)

### Compilation Flags

```makefile
CFLAGS = -ffreestanding \           # No standard library
         -fno-stack-protector \     # No canaries
         -fshort-wchar \            # CHAR16 = 2 bytes
         -mno-red-zone \            # No red zone for interrupts
         -fno-builtin \             # No builtin functions
         -fno-stack-check \         # No __chkstk_ms
         -Wall -Wextra -std=c11
```

### Linker Flags

```makefile
LDFLAGS = -nostdlib \               # No C runtime
          -Wl,--subsystem,10 \      # UEFI subsystem
          -Wl,-e,efi_main \         # Entry point
          -shared                   # Shared library format
```

---

## Path Resolution

### Absolute vs Relative Paths

- Paths starting with `\` are **absolute** (from root)
- Other paths are **relative** (from current directory `cwd`)

### `build_path()` Logic

```c
void build_path(CHAR16* path, CHAR16* name, CHAR16* result) {
    if (name[0] == L'\\') {
        strcpy16(result, name);  // Already absolute
        return;
    }
    
    // Relative: combine path + name
    strcpy16(result, path);
    if (result[last] != L'\\') result[++last] = L'\\';
    strcat16(result, name);
}
```

### Current Directory (`cwd`)

- Initially: `L"\\"` (root)
- Updated by `cd` command
- Used by all file operations via `build_path()`

---

## I/O Model

### Keyboard Input

**Blocking model** (`read_line()`):
```c
while (1) {
    EFI_INPUT_KEY key;
    while (ST->ConIn->ReadKeyStroke(ST->ConIn, &key) != EFI_SUCCESS);
    
    if (key.UnicodeChar == L'\r') { /* done */ }
    else if (key.UnicodeChar == 8) { /* backspace */ }
    else if (key.UnicodeChar >= 32) { /* printable */ }
}
```

- **No interrupts** — Simple polling loop
- **UTF-16 support** — Full Unicode characters

### Screen Output

- Coordinates: **column, line** (0-based)
- Colors: **4-bit palette** (16 colors)
- Operations: `OutputString()`, `ClearScreen()`, `SetAttribute()`

---

## Memory Layout

### Stack Considerations

**Problem**: Large local arrays trigger `___chkstk_ms`:
```c
CHAR16 buf[1024];  // 2KB on stack → needs stack probe
```

**Solution**: Use `static` for large buffers:
```c
static CHAR16 buf[1024];  // Data section, not stack
```

### Estimated Sizes

- **Code**: ~20KB (all commands)
- **Data**: ~10KB (static buffers)
- **BSS**: ~1KB (uninitialized data)
- **Total**: < 50KB

---

## Limitations & Design Choices

| Aspect | Limitation | Reason |
|--------|-----------|--------|
| **Lines in editor** | 32 max | Stack/memory constraint |
| **Line length** | 64 chars (CHAR16) | FAT filesystem paths |
| **Buffer reads** | 2KB per call | UEFI firmware limitation |
| **No heap** | Static allocation only | `malloc` not available |
| **Unity build** | All `.c` in bootloader | Linker simplicity (-shared) |
| **No threads** | Single-threaded | Not needed for shell |
| **No disk I/O** | FAT only via UEFI | Portable to EFI systems |

---

## Extending TestOS

### Adding a New Command

1. Create function in `kernel/commands.c`:
```c
void cmd_mycommand(CHAR16* input) {
    CHAR16* arg = input + 10;  // Skip "mycommand "
    // ... implementation
}
```

2. Add declaration in `kernel/commands.h`:
```c
void cmd_mycommand(CHAR16* input);
```

3. Add dispatch in `kernel/shell.c`:
```c
else if (startswith(input, L"mycommand ")) cmd_mycommand(input);
```

4. Update help in `cmd_help()`:
```c
println(L"  mycommand     - description");
```

---

## Debugging

### Print Debugging

Use `print()` and `println()` anywhere in code:
```c
print(L"Debug: value = ");
print_uint(myvar);
println(L"");
```

### In QEMU

Monitor output appears in terminal. To attach debugger:
```bash
qemu-system-x86_64 ... -s -S
# In another terminal: gdb kernel.elf
```

---

## Performance Notes

- **No optimization flags** in current Makefile (could add `-O2`)
- **FAT reads** are buffered (2KB per call)
- **String operations** are O(n) but acceptable for small strings
- **Shell loop** is pure polling (no event-driven I/O)

---

**Last Updated**: March 2026  
**Architecture Version**: 1.0
