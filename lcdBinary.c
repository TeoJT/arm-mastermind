/* ***************************************************************************** */
/* You can use this file to define the low-level hardware control fcts for       */
/* LED, button and LCD devices.                                                  */ 
/* Note that these need to be implemented in Assembler.                          */
/* You can use inline Assembler code, or use a stand-alone Assembler file.       */
/* Alternatively, you can implement all fcts directly in master-mind.c,          */  
/* using inline Assembler code there.                                            */
/* The Makefile assumes you define the functions here.                           */
/* ***************************************************************************** */

#ifndef	TRUE
#  define	TRUE	(1==1)
#  define	FALSE	(1==2)
#endif
#define	PAGE_SIZE		(4*1024)
#define	BLOCK_SIZE		(4*1024)

#define	INPUT			 0
#define	OUTPUT			 1

#define	LOW			 0
#define	HIGH			 1


//https://cpulator.01xz.net/?sys=arm-de1soc&d_audio=48000

// APP constants   ---------------------------------

// Wiring (see call to lcdInit in main, using BCM numbering)
// NB: this needs to match the wiring as defined in master-mind.c

#define STRB_PIN 24
#define RS_PIN   25
#define DATA0_PIN 23
#define DATA1_PIN 10
#define DATA2_PIN 27
#define DATA3_PIN 22

// -----------------------------------------------------------------------------
// includes 
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/types.h>
#include <time.h>

// -----------------------------------------------------------------------------
// prototypes

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

int failure (int fatal, const char *message, ...);

// -----------------------------------------------------------------------------
// Functions to implement here (or directly in master-mind.c)

/* this version needs gpio as argument, because it is in a separate file */
int static inline digitalWrite (uint32_t *gpio, int pin, int value) {
  /* ***  COMPLETE the code here, using inline Assembler  ***  */
    // asm volatile (
    //     "\t     mov      r1,    %[asmintArr]    \n" // get address of array
    //     "\t     mov      r2,    $0              \n" // initialize R1 to 1
	  //     "\t     mov      r3,    $50             \n" // initialize R3 to 20
    //     "\t     bl       lmao                   \n"
    //     "lmao:  str      r2,    [r1,$0]         \n"
    //     "\t     add      r2,    r2,     $2      \n" // Add 1 to R2
    //     "\t     add      r1,    r1,     $4      \n" // Add 1 to R1
    //     "\t     cmp      r2,    r3;             \n" // compare R2 and R3
    //     "\t     bne      lmao;                  \n" // loop while not equal (R2 and R3)
    //   :  [result] "=r" (outval)
    //   :  [asmintArr] "r" (intArr)
    //   :  "r0", "r1", "r2", "r3", "r4", "r5", "cc"
    // );

    //lsl
    //lsr
}

// adapted from setPinMode
void pinMode(uint32_t *gpio, int pin, int mode /*, int fSel, int shift */) {
  /* ***  COMPLETE the code here, using inline Assembler  ***  */
}

void writeLED(uint32_t *gpio, int led, int value) {
  /* ***  COMPLETE the code here, using inline Assembler  ***  */
}

int readButton(uint32_t *gpio, int button) {
  /* ***  COMPLETE the code here, using inline Assembler  ***  */
}

void waitForButton(uint32_t *gpio, int button) {
  /* ***  COMPLETE the code here, just C no Assembler; you can use readButton ***  */
}
