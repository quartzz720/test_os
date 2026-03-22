CC = gcc
CFLAGS  = -ffreestanding -fno-stack-protector -fshort-wchar -mno-red-zone \
          -fno-builtin -fno-stack-check -Wall -Wextra -std=c11 -I include -I kernel

LDFLAGS = -nostdlib -Wl,--subsystem,10 -Wl,-e,efi_main -shared

EFI_DIR = boot/efi/boot

SRCS = boot/bootloader.c

all: $(EFI_DIR)/BOOTX64.EFI

$(EFI_DIR)/BOOTX64.EFI: $(SRCS) kernel/console.c kernel/fs.c kernel/commands.c kernel/shell.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ boot/bootloader.c

clean:
	rm -f $(EFI_DIR)/BOOTX64.EFI
