#include "../include/efi.h"
#include "console.h"
#include "shell.h"
#include "kernel.h"

// ============================================================================
// Show welcome screen with loading animation
// ============================================================================

void show_welcome_screen(void) {
    clear();
    set_color(EFI_YELLOW, EFI_BGBLACK);
    println(L"        ,----,                                                      ");
    println(L"      ,/   .`|                                ,----..               ");
    println(L"    ,`   .'  :                      ___      /   /   \\   .--.--.    ");
    println(L"  ;    ;     /                    ,--.'|_   /   .     : /  /    '.  ");
    println(L".'___,/    ,'                     |  | :,' .   /   ;.  \\  :  /`. /  ");
    println(L"|    :     |            .--.--.   :  : ' :.   ;   /  ` ;  |  |--`   ");
    println(L";    |.';  ;   ,---.   /  /    '.;__,'  / ;   |  ; \\ ; |  :  ;_     ");
    println(L"`----'  |  |  /     \\ |  :  /`./|  |   |  |   :  | ; | '\\  \\    `.  ");
    println(L"    '   :  ; /    /  ||  :  ;_  :__,'| :  .   |  ' ' ' : `----.   \\ ");
    println(L"    |   |  '.    ' / | \\  \\    `. '  : |__'   ;  \\; /  | __ \\  \\  | ");
    println(L"    '   :  |'   ;   /|  `----.   \\|  | '.'|\\   \\  ',  / /  /`--'  / ");
    println(L"    ;   |.' '   |  / | /  /`--'  /;  :    ; ;   :    / '--'.     /  ");
    println(L"    '---'   |   :    |'--'.     / |  ,   /   \\   \\ .'    `--'---'   ");
    println(L"             \\   \\  /   `--'---'   ---`-'     `---`                 ");
    println(L"              `----'                                                ");
    println(L"                                                                    ");
    set_color(EFI_WHITE, EFI_BGBLACK);
    println(L"");
    println(L"                 Welcome to TestOS!");
    println(L"");
    println(L"             Initializing system...");
    println(L"");
    
    // Loading bar - approximately 5 seconds (50 iterations x ~100ms each)
    // This shit is cringe and doesn't work lol, timer is based on core speed (?)
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
    show_welcome_screen();
    shell();
}

