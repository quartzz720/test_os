CC = gcc
CFLAGS  = -ffreestanding -fno-stack-protector -fshort-wchar -mno-red-zone \
          -fno-builtin -fno-stack-check -Wall -Wextra -std=c11 -I include -I kernel

LDFLAGS = -nostdlib -Wl,--subsystem,10 -Wl,-e,efi_main -shared

EFI_DIR = boot/efi/boot

all: $(EFI_DIR)/BOOTX64.EFI

$(EFI_DIR)/BOOTX64.EFI: boot/bootloader.c kernel/hal.c kernel/console.c kernel/console.h kernel/fs.c kernel/fs.h kernel/commands.c kernel/commands.h kernel/shell.c kernel/shell.h kernel/kernel.c kernel/kernel.h kernel/hal.h
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ boot/bootloader.c

clean:
	rm -f $(EFI_DIR)/BOOTX64.EFI
