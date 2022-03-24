#include <stdio.h>
#include <stdlib.h>
#include "lcdBinary.c"

//set this to 1 to enable tests in the terminal.
#define TEST_HARDWARE 1

void printGPIO(uint32_t *gpio) {
  for (int i = 0; i < 15; i++) {
    printf("%d:", i);
    int bee = *(gpio+i);
    for (int bitindex = 0; bitindex < 32; bitindex++) {
      int bit = (bee << bitindex) & 0b10000000000000000000000000000000;

      //i.e. is true.
      if (bit != 0x0) {
        printf("1");
      }
      //i.e. is false.
      else {
        printf("0");
      }
    }

    printf("\n");
  }
}

void testDigital(uint32_t *gpio) {
    printf("digitalWrite test!\n");
    printf("Turning LED off and waiting 2 secs...\n");
    digitalWrite(gpio, 13, 0);
    sleep(2);
    printf("LED on...\n");
    digitalWrite(gpio, 5, 1);
    sleep(1);
    printf("LED off...\n");
    digitalWrite(gpio, 5, 0);

    printf("LED on...\n");
    digitalWrite(gpio, 13, 1);
    sleep(1);
    printf("LED off...\n");
    digitalWrite(gpio, 13, 0);

    sleep(1);
    printf("LED on...\n");
    digitalWrite(gpio, 13, 1);
    digitalWrite(gpio, 5, 1);
    sleep(1);
    printf("LED off...\n");
    digitalWrite(gpio, 13, 0);

    sleep(1);
    digitalWrite(gpio, 5, 0);

}

void pinModeTest(uint32_t *gpio) {
    printf("pinMode test!\n");

    printf("\nsetting pin 13 to INPUT\n");
    pinMode(gpio, 13, INPUT);
    printGPIO(gpio);

    printf("\nsetting pin 13 to OUTPUT\n");
    pinMode(gpio, 13, OUTPUT);
    printGPIO(gpio);
    
    printf("\nsetting pin 15 to OUTPUT\n");
    pinMode(gpio, 15, OUTPUT);
    printGPIO(gpio);

    printf("\n\n");
}

void readButtonTest(uint32_t *gpio) {
  printf("button test!\n");
  printf("I'm gonna assume pinmode is working.\n");
  pinMode(gpio, 19, INPUT);

  for (int i = 0; i < 3; i++) {
      printf("reading button on pin 19..!\n");
      sleep(1);
      if (readButton(gpio, 19)) {
          printf("button PRESSED\n");
      }
      else {
          printf("...not pressed\n");
      }
      sleep(1);
  }
  

}

void testWaitButton(uint32_t *gpio) {
    printf("I'm gonna assume the button works on pin 19.\n");
    printf("Press the button to continue...\n");
    waitForButton(gpio, 19);
    printf("Yay!\n");

}


void argsTest() {
  volatile struct AsmArgs {
      uint32_t pin;
      uint32_t gpio;
      uint32_t value;
  } asmArgs;

  asmArgs.gpio = 1;
  asmArgs.pin = 2;
  asmArgs.value = 3;

  printf("args test..\n");
  printf("%d    %d,%d,%d\n", &asmArgs, &asmArgs.pin, &asmArgs.gpio, &asmArgs.value);
}


void testHardware(uint32_t *gpio) {
    if (TEST_HARDWARE) {
        printf("\033[33m ===Teo's whacky testing code===\n");

        pinMode(gpio, 13, 1);
        pinMode(gpio, 5, 1);

        //pinModeTest(gpio);
        //readButtonTest(gpio);

        testDigital(gpio);
        testWaitButton(gpio);
        printf("End tests.\033[0m\n");
    }
}