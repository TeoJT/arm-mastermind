#include <stdio.h>
#include <stdlib.h>
#include "lcdBinary.c"

void testWrite() {

}

void testDigital(uint32_t *gpio) {
    printf("\033[33m ===Teo's whacky testing code===\n");
    
    //digitalWrite(gpio, 13)
    printGPIO(gpio);
    printf("\033[0m\n");
}