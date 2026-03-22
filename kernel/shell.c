#include "../include/efi.h"
#include "console.h"
#include "commands.h"
#include "fs.h"
#include "shell.h"

void shell(void) {
    CHAR16 input[256];
    while (1) {
        set_color(EFI_GREEN, EFI_BGBLACK);
        print(L"Z:"); print(cwd); print(L"> ");
        set_color(EFI_WHITE, EFI_BGBLACK);
        read_line(input, 256);
        if      (streq(input, L"cls"))          clear();
        else if (streq(input, L"help"))         cmd_help();
        else if (streq(input, L"ver"))          cmd_ver();
        // else if (streq(input, L"color") || startswith(input, L"color ")) cmd_color(input);
        else if (streq(input, L"dir") || streq(input, L"ls"))          cmd_dir();
        else if (startswith(input, L"echo "))   cmd_echo(input);
        else if (startswith(input, L"type "))   cmd_type(input);
        else if (startswith(input, L"mkdir "))  cmd_mkdir(input);
        else if (startswith(input, L"write "))  cmd_write(input);
        else if (startswith(input, L"cd ") || streq(input, L"cd")) cmd_cd(input);
        else if (startswith(input, L"del "))    cmd_del(input);
        else if (startswith(input, L"ren "))    cmd_ren(input);
        else if (startswith(input, L"rmdir "))  cmd_rmdir(input);
        else if (startswith(input, L"copy "))   cmd_copy(input);
        else if (startswith(input, L"move "))   cmd_move(input);
        else if (startswith(input, L"more "))   cmd_more(input);
        else if (streq(input, L"tree"))         cmd_tree(input);
        else if (streq(input, L"edit") || startswith(input, L"edit ")) cmd_edit(input);
        else if (input[0] == 0) { /* empty */ }
        else {
            set_color(EFI_RED, EFI_BGBLACK);
            print(L"Unknown command: "); println(input);
            set_color(EFI_WHITE, EFI_BGBLACK);
        }
    }
}
