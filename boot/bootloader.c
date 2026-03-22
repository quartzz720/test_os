#include "../include/efi.h"

EFI_SYSTEM_TABLE* ST;
EFI_HANDLE        IH;

// #include "../kernel/console.h"
// #include "../kernel/shell.h"

#include "../kernel/console.c"
#include "../kernel/fs.c"
#include "../kernel/commands.c"
#include "../kernel/shell.c"



EFI_STATUS efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE* SystemTable) {
    ST = SystemTable;
    IH = ImageHandle;

    clear();
    set_color(EFI_YELLOW, EFI_BGBLACK);
    println(L"     _   ___                      ____  _____");
    println(L"    / | / (_)___ _____ _____ _   / __ \\/ ___/");
    println(L"   /  |/ / / __ `/ __ `/ __ `/  / / / /\\__ \\ ");
    println(L"  / /|  / / /_/ / /_/ / /_/ /  / /_/ /___/ / ");
    println(L" /_/ |_/_/\\__, /\\__, /\\__,_/   \\____//____/  ");
    println(L"         /____//____/                        ");
    set_color(EFI_WHITE, EFI_BGBLACK);
    println(L"");
    println(L"Type 'help' for available commands.");
    println(L"");

    shell();
    return EFI_SUCCESS;
}
