#include "../include/efi.h"
#include "console.h"
#include "shell.h"
#include "kernel.h"

// ============================================================================
// Show welcome screen with loading animation (5 seconds)
// ============================================================================

void show_welcome_screen(void) {
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
    println(L"                 Welcome to TestOS!");
    println(L"");
    println(L"             Initializing system...");
    println(L"");
    
    // Loading bar - approximately 5 seconds (50 iterations x ~100ms each)
    print(L"  [");
    for (int i = 0; i < 50; i++) {
        print(L"=");
        // Delay loop - tune this for ~100ms per iteration
        for (volatile int j = 0; j < 5000000; j++);
    }
    println(L"] 100%");
    println(L"");
    println(L"Type 'help' for available commands.");
    println(L"");
}

void kernel_main(void) {
    // Show welcome screen
    show_welcome_screen();
    
    // Enter shell
    shell();
}
