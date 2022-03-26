/* ***************************************************************************** */
/* You can use this file to define the low-level hardware control fcts for       */
/* LED, button and LCD devices.                                                  */ 
/* Note that these need to be implemented in Assembler.                          */
/* You can use inline Assembler code, or use a stand-alone Assembler file.       */
/* Alternatively, you can implement all fcts directly in master-mind.c,          */  
/* using inline Assembler code there.                                            */
/* The Makefile assumes you define the functions here.                           */
/* ***************************************************************************** */

//For include guard.
#define LCD_BINARY_FILE

#ifndef	TRUE
#  define	TRUE	(1==1)
#  define	FALSE	(1==2)
#endif

#define	INPUT			 0
#define	OUTPUT			 1

#define	LOW			 0
#define	HIGH			 1

#define IN  0
#define OUT 1


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

//Declair a volatile structure, used to store our arguments for
//going into our assembly program, which will be stored into
//the registers. Therefore we only need to pass the address
//of this structure.


// -----------------------------------------------------------------------------
int failure (int fatal, const char *message, ...);

// -----------------------------------------------------------------------------
// Functions to implement here (or directly in master-mind.c)

/* this version needs gpio as argument, because it is in a separate file */
int static inline digitalWrite (uint32_t *gpio, int pin, int value) {

  //Prepare the arguments in a volatile struct
  //so that we can access them from memory, only
  //needing to pass the address of the arguments
  //in a single register.
    volatile struct AsmArgs {
      uint32_t pin;
      uint32_t* gpio;
      uint32_t value;
    } asmArgs;

    //Prepare the arguments.
    asmArgs.gpio = gpio;
    asmArgs.pin = pin;
    asmArgs.value = value;

    int outval = 0;
    
  //Inline assembly code sub-routine
    asm volatile (
        
        //Set the parameters
      "\t          mov r5, %[argsPointer] \n"   //Args memory location
        		
				//Get the arguments.
      "\t          ldr r0, [r5, #0] \n"     //store pin number into register
      "\t          add r5, #4 \n" 
      "\t          ldr r1, [r5, #0] \n"     //store gpio pointer into register
      "\t			     add r5, #4  \n"
      "\t			     ldr r4, [r5, #0] \n"     //store value into register
        
        		//Prepare the bit.
      "\t          mov r2, #1 \n"
      "\t          lsl r2, r0 \n"
        		
				//start off with 40 from the gpio address
      "\t  		add r1, #40 \n"
				//Depending if value is 1 or 0, we will either get
				//12 or 0.
			"\t	mov r3, #12 \n"
			"\t	mul r5, r4, r3 \n"
				//Sub that multiplied value from r5. If value was one,
				//this will result us in accessing gpio+7 (4*7=28)
				//otherwise if 0 we will access gpio+10 (4*10=40)
			"\t	sub r1, r5 \n"
				
				//Write the bit to the gpio register.
			"\t	str r2, [r1] \n"

      :  [result] "=r" (outval)
      :  [argsPointer] "r" (&asmArgs)
      :  "r0", "r1", "r2", "r3", "r4", "r5", "cc"
    );
}

// adapted from setPinMode
int static inline pinMode(uint32_t *gpio, int pin, int mode /*, int fSel, int shift */) {

  //Prepare the arguments in a volatile struct
  //so that we can access them from memory, only
  //needing to pass the address of the arguments
  //in a single register.
  volatile struct AsmArgs {
    uint32_t wordIndex;
    uint32_t bitIndex;
    uint32_t* gpio;
    uint32_t mode;
  } asmArgs;

  //Prepare the arguments.
  asmArgs.wordIndex = (pin/10)*4;
  asmArgs.bitIndex  = (pin%10)*3;
  asmArgs.gpio      = gpio;
  asmArgs.mode      = mode;
  
  //Inline assembly code sub-routine
    int outval = 0;
    asm volatile (
      //r0-  the word index
      //r1-  gpio registers location
      //r2-  temp bit that holds the on value of pin in register.
      //r3-  existing register from data.
      //r4-  the pin mode
      //r5-  the address of the parameters
      //r6-  the bit index
      
      //Set the parameters
      "\t          mov r5, %[argsPointer] \n"   //Args memory location
      
      //Load all the parameters into the registers.
      "\t          ldr r0, [r5, #0] \n"     //word index
      "\t          add r5, #4 \n"
      "\t          ldr r6, [r5, #0] \n"     //bit index
      "\t          add r5, #4 \n"
      "\t          ldr r1, [r5, #0] \n"     //gpio
      "\t          add r5, #4 \n"
      "\t          ldr r4, [r5, #0] \n"     //mode
      
      //Prepare the bit to write to register in memory.
      "\t          mov r2, #1 \n"
      "\t          lsl r2, r6 \n"
      
      //Get the existing register from memory.
      //Access gpio
      "\t          add r1, r0 \n"
      "\t          ldr r3, [r1, #0] \n"
      
      //Condition: if the mode is set to OUT (i.e. 1), go to modeOut,
      //else go to modeIn.
      "\t          cmp r4, #1 \n"
      "\t          beq modeOut \n"
      "\t          b modeIn \n"
      //Perform or bitwise operations to enable the pin.
      //This leaves any other bits intact.
      "modeOut:    orr r3, r2 \n"
      
      //Store the gpio register back into memory.
      "\t          str r3, [r1, #0] \n"
      
      "\t          b end2 \n"

      //We need to inverse the register holding the temp bit
      //so we can set it to 0 in the gpio register.
      //Perform an xor operation on a word full of 1's.
      //This should result in the active bit being set to 0
      //while the rest of the bits are 1's.
      "modeIn:     mov r0, #0xFFFFFFFF \n"
      "\t          eor r2, r0 \n"
      
      //bitwises and so that we set the right bit while
      //leaving the rest of the bits intact.
      "\t          and r3, r2 \n"
      
      //Store the gpio register back into memory.
      "\t          str r3, [r1, #0] \n"
      
      "\t          b end2 \n"
    "end2: \n"

      :  [result] "=r" (outval)
      :  [argsPointer] "r" (&asmArgs)
      :  "r0", "r1", "r2", "r3", "r4", "r5", "r6", "cc"
    );
}

void writeLED(uint32_t *gpio, int led, int value) {
  digitalWrite(gpio, led, value);
}

int static inline readButton(uint32_t *gpio, int button) {

  //Prepare the arguments in a volatile struct
  //so that we can access them from memory, only
  //needing to pass the address of the arguments
  //in a single register.
  volatile struct AsmArgs {
    uint32_t* gpio;
    uint32_t button;
  } asmArgs;

  //Prepare the arguments.
  asmArgs.gpio   = gpio;
  asmArgs.button = button;

  int outval = 0;

  //Inline assembly code sub-routine
  asm volatile (
    "\t          mov r0, %[argsPointer] \n"   //Args memory location
    
    //Load all the parameters into the registers.
    "\t          ldr r1, [r0, #0] \n"     //gpio address
    "\t          add r0, #4\n"
    "\t          ldr r2, [r0, #0] \n"     //button pin.
    
    //Get the button input gpio+13 register.
    //13*4=52
    "\t          add r1, #52 \n"
    "\t          ldr r3, [r1, #0] \n"
    
    //Bitshift the gpio register by the pin number,
    //and perform bitwise and to get state of bit at
    //the pin number.
    "\t          lsr r3, r2 \n"
    "\t          and r3, #1 \n"

    //result should be in r3 now.
    "\t          mov %[result], r3 \n"

      :  [result] "=r" (outval)
      :  [argsPointer] "r" (&asmArgs)
      :  "r0", "r1", "r2", "r3", "cc"
  );

  return outval;
}

//Run a while loop that simply does nothing (other than a sleep statement to prevent
//100% cpu time doing nothing) until the button is pressed.
void waitForButton(uint32_t *gpio, int button) {
  while (!readButton(gpio, button)) {
    usleep(1000);
  }
}
