#include "../include/efi.h"
#include "../kernel/hal.h"
#include "../kernel/kernel.h"

// Include kernel modules
#include "../kernel/hal.c"
#include "../kernel/console.c"
#include "../kernel/fs.c"
#include "../kernel/commands.c"
#include "../kernel/shell.c"
#include "../kernel/kernel.c"

EFI_STATUS efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE* SystemTable) {
    // Initialize HAL with UEFI
    hal_init(SystemTable, ImageHandle);

    // Enter kernel
    kernel_main();
    return EFI_SUCCESS;
}
