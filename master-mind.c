/*
 * MasterMind implementation: template; see comments below on which parts need to be completed
 * CW spec: https://www.macs.hw.ac.uk/~hwloidl/Courses/F28HS/F28HS_CW2_2022.pdf
 * This repo: https://gitlab-student.macs.hw.ac.uk/f28hs-2021-22/f28hs-2021-22-staff/f28hs-2021-22-cwk2-sys

 * Compile:
 gcc -c -o lcdBinary.o lcdBinary.c
 gcc -c -o master-mind.o master-mind.c
 gcc -o master-mind master-mind.o lcdBinary.o
 * Run:
 sudo ./master-mind

 OR use the Makefile to build
 > make all
 and run
 > make run
 and test
 > make test

 ***********************************************************************
 * The Low-level interface to LED, button, and LCD is based on:
 * wiringPi libraries by
 * Copyright (c) 2012-2013 Gordon Henderson.
 ***********************************************************************
 * See:
 *	https://projects.drogon.net/raspberry-pi/wiringpi/
 *
 *    wiringPi is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU Lesser General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    wiringPi is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public License
 *    along with wiringPi.  If not, see <http://www.gnu.org/licenses/>.
 ***********************************************************************
*/

/* ======================================================= */
/* SECTION: includes                                       */
/* ------------------------------------------------------- */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>

#include <unistd.h>
#include <string.h>
#include <time.h>

#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/ioctl.h>
//#include "lcdBinary.c"
#include "test-hardware.c"

/* --------------------------------------------------------------------------- */
/* Config settings */
/* you can use CPP flags to e.g. print extra debugging messages */
/* or switch between different versions of the code e.g. //digitalWrite() in Assembler */
#define DEBUG
#undef ASM_CODE

// =======================================================
// Tunables
// PINs (based on BCM numbering)
// For wiring see CW spec: https://www.macs.hw.ac.uk/~hwloidl/Courses/F28HS/F28HS_CW2_2022.pdf
// GPIO pin for green LED
#define LED 13

// GPIO pin for red LED
#define LED2 5

// GPIO pin for button
#define BUTTON 19
// =======================================================
// delay for loop iterations (mainly), in ms
// in mili-seconds: 0.2s
#define DELAY 200
// in micro-seconds: 3s
#define TIMEOUT 3000000
// =======================================================
// APP constants   ---------------------------------
// number of colours and length of the sequence
#define COLS 3
#define SEQL 3
// =======================================================

// generic constants

#ifndef TRUE
#define TRUE (1 == 1)
#define FALSE (1 == 2)
#endif

#define PAGE_SIZE (4 * 1024)
#define BLOCK_SIZE (4 * 1024)

#define INPUT 0
#define OUTPUT 1

#define LOW 0
#define HIGH 1

#define BLINK_DELAY 200

// =======================================================
// Wiring (see inlined initialisation routine)

#define STRB_PIN 24
#define RS_PIN 25
#define DATA0_PIN 23
#define DATA1_PIN 10
#define DATA2_PIN 27
#define DATA3_PIN 22

/* ======================================================= */
/* SECTION: constants and prototypes                       */
/* ------------------------------------------------------- */


/* Constants */

static const int colors = COLS;
static const int seqlen = SEQL;

static char *color_names[] = {"red", "green", "blue"};

static int *theSeq = NULL;

static int *seq1, *seq2, *cpy1, *cpy2;


/* ***************************************************************************** */
/* INLINED fcts from wiringPi/devLib/lcd.c: */
// HD44780U Commands (see Fig 11, p28 of the Hitachi HD44780U datasheet)

#define LCD_CLEAR 0x01
#define LCD_HOME 0x02
#define LCD_ENTRY 0x04
#define LCD_CTRL 0x08
#define LCD_CDSHIFT 0x10
#define LCD_FUNC 0x20
#define LCD_CGRAM 0x40
#define LCD_DGRAM 0x80

// Bits in the entry register

#define LCD_ENTRY_SH 0x01
#define LCD_ENTRY_ID 0x02

// Bits in the control register

#define LCD_BLINK_CTRL 0x01
#define LCD_CURSOR_CTRL 0x02
#define LCD_DISPLAY_CTRL 0x04

// Bits in the function register

#define LCD_FUNC_F 0x04
#define LCD_FUNC_N 0x08
#define LCD_FUNC_DL 0x10

#define LCD_CDSHIFT_RL 0x04

// Mask for the bottom 64 pins which belong to the Raspberry Pi
//	The others are available for the other devices

#define PI_GPIO_MASK (0xFFFFFFC0)

static unsigned int gpiobase;
static uint32_t *gpio;

static int timed_out = 0;

/* ------------------------------------------------------- */
// misc prototypes

int failure(int fatal, const char *message, ...);

/* ********************************************************** */
/* COMPLETE the code for all of the functions in this SECTION */
/* Either put them in a separate file, lcdBinary.c, and use   */
/* inline Assembler there, or use a standalone Assembler file */
/* You can also directly implement them here (inline Asm).    */
/* ********************************************************** */

extern int matches(int* secret, int* guess);

/* ======================================================= */
/* SECTION: game logic                                     */
/* ------------------------------------------------------- */
/* AUX fcts of the game logic */

/* ********************************************************** */
/* COMPLETE the code for all of the functions in this SECTION */
/* Implement these as C functions in this file                */
/* ********************************************************** */

/* initialise the secret sequence; by default it should be a random sequence */

// Define array to store the randomly generated number sequence
int numberSequence[3];

// Generate random number sequnce
void initSeq()
{

  // Use time as the seed for the random number
  srand(time(NULL));

  // Define the int to store the random number generated
  int randomNumber;

  // Loop to generate 3 random numbers
  for (int i = 0; i < 3; i++)
  {
    // Generate a random number between 1 and 3 inclusive
    randomNumber = (rand() % 3 + 1);

    // Set the posotion in the array to the generated number
    numberSequence[i] = randomNumber;
  }

  // Set the generated sequnce to the global variable
  theSeq = numberSequence;
}

/* display the sequence on the terminal window, using the format from the sample run in the spec */
void showSeq(int *seq)
{
  // Print out the number sequnce
  printf("Printing sequence\n");
  for (int i = 0; i < 3; i++)
  {
    printf("The sequence is:  %d\n", *(seq + i));
  }
}

#define NAN1 8
#define NAN2 9

/* counts how many entries in seq2 match entries in seq1 */
/* returns exact and approximate matches, either both encoded in one value, */
/* or as a pointer to a pair of values */
int /* or int* */ countMatches(int *seq1, int *seq2)
{
  int numOfMatches = 0;

  // Count the number of matches between the generated sequnce and the user enetered sequence
  for (int i = 0; i < 3; i++)
  {
    if (seq1[i] == seq2[i])
    {
      numOfMatches++;
    }
  }

  // Return the number of matches
  return numOfMatches;
}

/* show the results from calling countMatches on seq1 and seq2 */
void showMatches(int /* or int* */ code, /* only for debugging */ int *seq1, int *seq2, /* optional, to control layout */ int lcd_format)
{

  // Loop through the matches printing them out
  for (int i = 0; i < 3; i++)
  {
    if (seq1[i] == seq2[i])
    {
      printf("There was a match between %d in sequnce 1 and %d in sequence 2", *(seq1 + i), *(seq2 + i));
    }
  }
}

/* parse an integer value as a list of digits, and put them into @seq@ */
/* needed for processing command-line with options -s or -u            */
void readSeq(int *seq, int val)
{

  // Setup variables to split up the inputted number
  int temp = 0;
  int i = 0;

  // Loop through the inputted number
  while (val > 0)
  {
    // Split the number into it's individual digits
    temp = val % 10;
    val = val / 10;
    seq[i] = temp;

    i++;
  }
}

/* read a guess sequence from stdin and store the values in arr */
/* only needed for testing the game logic, without button input */
int readNum(int max)
{

  int number1, number2, number3;

  printf("Enter the first number");
  scanf("%d", &number1);

  printf("Enter the second number");
  scanf("%d", &number2);

  printf("Enter the third number");
  scanf("%d", &number3);

  return number1, number2, number3;
}

/* ======================================================= */
/* SECTION: TIMER code                                     */
/* ------------------------------------------------------- */
/* TIMER code */

/* timestamps needed to implement a time-out mechanism */
static uint64_t startT, stopT;

/* ********************************************************** */
/* COMPLETE the code for all of the functions in this SECTION */
/* Implement these as C functions in this file                */
/* ********************************************************** */

/* you may need this function in timer_handler() below  */
/* use the libc fct gettimeofday() to implement it      */
uint64_t timeInMicroseconds()
{
  // Get current time in microseconds
  struct timeval current_time;
  gettimeofday(&current_time, NULL);
  printf("seconds : %ld\nmicro seconds : %ld",
         current_time.tv_sec, current_time.tv_usec);
  return current_time.tv_usec;
}


/* ======================================================= */
/* SECTION: Aux function                                   */
/* ------------------------------------------------------- */
/* misc aux functions */

int failure(int fatal, const char *message, ...)
{
  va_list argp;
  char buffer[1024];

  if (!fatal) //  && wiringPiReturnCodes)
    return -1;

  va_start(argp, message);
  vsnprintf(buffer, 1023, message, argp);
  va_end(argp);

  fprintf(stderr, "%s", buffer);
  exit(EXIT_FAILURE);

  return 0;
}

/*
 * waitForEnter:
 *********************************************************************************
 */

void waitForEnter(void)
{
  printf("Press ENTER to continue: ");
  (void)fgetc(stdin);
}

/*
 * delay:
 *	Wait for some number of milliseconds
 *********************************************************************************
 */

void delay(unsigned int howLong)
{
  struct timespec sleeper, dummy;

  sleeper.tv_sec = (time_t)(howLong / 1000);
  sleeper.tv_nsec = (long)(howLong % 1000) * 1000000;

  nanosleep(&sleeper, &dummy);
}



/* Check if the sequence is correct and print relivant statement*/

int checkIfSequnceCorrect(int numGuess, int found)
{

  // Check if the number of matches is 3
  if (countMatches(theSeq, numGuess) == 3)
  {
    printf("The sequnce was correct\n");

    // End loop
    found = 1;
  }
  else
  {

    printf("Sequnce incorrect\n");
  }
  return found;
}

// Light sequence for acknowleding button input
void endOfInputLights(int buttonPressCount)
{
  for (int i = 0; i < buttonPressCount; i++)
  {
    digitalWrite(gpio, LED, HIGH);
    delay(BLINK_DELAY);
    digitalWrite(gpio, LED, LOW);
    delay(BLINK_DELAY);
  }
}

void communicateGuessAccuracy(int match, int approx)
{
  printf("Guess accuracy: matches: %d, approx: %d\n", match, approx);
  // Blink green LED for exact matches
  for (int i = 0; i < match; i++)
  {
    digitalWrite(gpio, LED, HIGH);
    delay(BLINK_DELAY);
    digitalWrite(gpio, LED, LOW);
    delay(BLINK_DELAY);
  }

  // Red LED seperator
  delay(BLINK_DELAY);

  digitalWrite(gpio, LED2, HIGH);
  delay(BLINK_DELAY);
  digitalWrite(gpio, LED2, LOW);

  delay(BLINK_DELAY);

  // Blink green LED for approxomite matches
  for (int i = 0; i < approx; i++)
  {
    digitalWrite(gpio, LED, HIGH);
    delay(BLINK_DELAY);
    digitalWrite(gpio, LED, LOW);
    delay(BLINK_DELAY);
  }
}

/* ======================================================= */
/* SECTION: main fct                                       */
/* ------------------------------------------------------- */

int main(int argc, char *argv[])
{ // this is just a suggestion of some variable that you may want to use
  printf("Entering Main Program\n");

  int found = 0, attempts = 0, i, j, code;
  int c, d, buttonPressed, rel, foo;
  int *attSeq;

  int pinLED = LED, pin2LED2 = LED2, pinButton = BUTTON;
  int fSel, shift, pin, clrOff, setOff, off, res;
  int fd;

  int exact, contained;
  char str1[32];
  char str2[32];

  struct timeval t1, t2;
  int t;

  char buf[32];

  // variables for command-line processing
  char str_in[20], str[20] = "some text";
  int verbose = 0, debug = 0, help = 0, opt_m = 0, opt_n = 0, opt_s = 0, unit_test = 0, res_matches = 0;


  // -------------------------------------------------------
  // process command-line arguments

  // see: man 3 getopt for docu and an example of command line parsing
  printf("Entering something\n");
  { // see the CW spec for the intended meaning of these options
    int opt;
    while ((opt = getopt(argc, argv, "hvdus:")) != -1)
    {
      switch (opt)
      {
      case 'v':
        verbose = 1;
        break;
      case 'h':
        help = 1;
        break;
      case 'd':
        debug = 1;
        break;
      case 'u':
        unit_test = 1;
        break;
      case 's':
        opt_s = atoi(optarg);
        break;
      default: /* '?' */
        fprintf(stderr, "Usage: %s [-h] [-v] [-d] [-u <seq1> <seq2>] [-s <secret seq>]  \n", argv[0]);
        exit(EXIT_FAILURE);
      }
    }
  }
  printf("Exiting something\n");

  if (help)
  {
    printf("Entering help");
    fprintf(stderr, "MasterMind program, running on a Raspberry Pi, with connected LED, button and LCD display\n");
    fprintf(stderr, "Use the button for input of numbers. The LCD display will show the matches with the secret sequence.\n");
    fprintf(stderr, "For full specification of the program see: https://www.macs.hw.ac.uk/~hwloidl/Courses/F28HS/F28HS_CW2_2022.pdf\n");
    fprintf(stderr, "Usage: %s [-h] [-v] [-d] [-u <seq1> <seq2>] [-s <secret seq>]  \n", argv[0]);
    printf("Exiting help\n");
    exit(EXIT_SUCCESS);
  }

  if (unit_test && optind >= argc - 1)
  {
    fprintf(stderr, "Expected 2 arguments after option -u\n");
    exit(EXIT_FAILURE);
  }

  if (verbose && unit_test)
  {
    printf("1st argument = %s\n", argv[optind]);
    printf("2nd argument = %s\n", argv[optind + 1]);
  }

  if (verbose)
  {
    fprintf(stdout, "Settings for running the program\n");
    fprintf(stdout, "Verbose is %s\n", (verbose ? "ON" : "OFF"));
    fprintf(stdout, "Debug is %s\n", (debug ? "ON" : "OFF"));
    fprintf(stdout, "Unittest is %s\n", (unit_test ? "ON" : "OFF"));
    if (opt_s)
      fprintf(stdout, "Secret sequence set to %d\n", opt_s);
  }

  seq1 = (int *)malloc(seqlen * sizeof(int));
  seq2 = (int *)malloc(seqlen * sizeof(int));
  cpy1 = (int *)malloc(seqlen * sizeof(int));
  cpy2 = (int *)malloc(seqlen * sizeof(int));

  // check for -u option, and if so run a unit test on the matching function
  if (unit_test && argc > optind + 1)
  { // more arguments to process; only needed with -u
    strcpy(str_in, argv[optind]);
    opt_m = atoi(str_in);
    strcpy(str_in, argv[optind + 1]);
    opt_n = atoi(str_in);
    // CALL a test-matches function; see testm.c for an example implementation
    readSeq(seq1, opt_m); // turn the integer number into a sequence of numbers
    readSeq(seq2, opt_n); // turn the integer number into a sequence of numbers
    if (verbose)
      fprintf(stdout, "Testing matches function with sequences %d and %d\n", opt_m, opt_n);
    res_matches = countMatches(seq1, seq2);
    showMatches(res_matches, seq1, seq2, 1);
    exit(EXIT_SUCCESS);
  }

  if (geteuid() != 0)
    fprintf(stderr, "setup: Must be root. (Did you forget sudo?)\n");

  // init of guess sequence, and copies (for use in countMatches)
  attSeq = (int *)malloc(seqlen * sizeof(int));
  cpy1 = (int *)malloc(seqlen * sizeof(int));
  cpy2 = (int *)malloc(seqlen * sizeof(int));

  // -----------------------------------------------------------------------------
  // constants for RPi2
  gpiobase = 0x3F200000;

  // -----------------------------------------------------------------------------
  // memory mapping
  // Open the master /dev/memory device

  if ((fd = open("/dev/mem", O_RDWR | O_SYNC | O_CLOEXEC)) < 0)
    return failure(FALSE, "setup: Unable to open /dev/mem: %s\n", strerror(errno));

  // GPIO:
  gpio = (uint32_t *)mmap(0, BLOCK_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, gpiobase);
  if ((int32_t)gpio == -1)
    return failure(FALSE, "setup: mmap (GPIO) failed: %s\n", strerror(errno));

  // -------------------------------------------------------
  // Configuration of LED and BUTTON
  // -----------------------------------------------------------------------------
  // setting LED 1 the mode
  pinMode(gpio, LED, OUT);

  // setting LED 2 the mode
  pinMode(gpio, LED2, OUT);

  // setting BUTTON the mode
  pinMode(gpio, BUTTON, IN);

  int theValue;
  unsigned int howLong = DELAY;

  // -----------------------------------------------------------------------------

  // Start of game

  /* initialise the secret sequence */
  if (!opt_s)
  initSeq();
  if (debug)
  showSeq(theSeq);


  //testHardware(gpio);


  // -----------------------------------------------------------------------------
  // +++++ main loop
  // testHardware(gpio);
  printf("matches test\n");
  int ssss[] = {2,1,2};
  int gggg[] = {2,1,3};
  int answr = matches(&ssss, &gggg);
  printf("approx:%d, correct:%d\n", (answr & 0x03), ((answr >> 2) & 0x03));

  printf("Welcome to mastermind! Enter a number on the button to begin...\n");
  waitForButton(gpio, BUTTON);


  // Define variables needed in the game loop

  // Status of the button variable
  int isPressed = 0;

  // Count how many times the button has been pressed
  int buttonPressCount = 0;

  // Define the array to store the entered numbers in
  int numGuess[3];

  // Get the clock to check button intevals
  clock_t timer;

  // Store the calculation of each button interval
  int timeInterval = 0;

  // Store the lnegth of time between each button press
  int timeToSubtract = 0;

  // Store the number of numbers entered by the user
  int numberOfNumbersEnetered = 0;

  // Store if the button was pressed or not
  int wasPressed = 0;

  // Store the current round number of guesses and answers
  int roundNumber = 0;

  // Main game loop
  while (!found)
  {

    // Check if there has been 3 rounds and if so end the game
    if (roundNumber == 3)
    {
      printf("That's the game done\n");
      break;
    }

    // Calculate how long since the last button press
    timeInterval = clock() - timeToSubtract;

    // Check if the button has been pressed
    if (readButton(gpio, BUTTON) && !isPressed)
    {
      // Set the button tracking varibles
      isPressed = 1;
      wasPressed = 1;

      // Increase the number to be entered into the sequence
      buttonPressCount++;

      // Set the time to base the future calcutions of
      timeToSubtract = clock();

      // Sleep for a small amount time to make sure the button isn't registering more than 1 input
      usleep(2000);
    }

    // Check if the button isn't being pressed anymore
    if (!readButton(gpio, BUTTON) && isPressed)
    {
      // Set the button tracker variable to not pressed
      isPressed = 0;
    }

    // If 2 seconds have passed since the last button press then continue on with the game
    if ((timeInterval > 1000000) && wasPressed) // Set's time interval to 2 seconds
    {
      // Reset pressed variable to prepare for a new input
      wasPressed = 0;

      //Blink red LED once to acknoledge input of a number
      digitalWrite(gpio, LED2, HIGH);
      delay(BLINK_DELAY);
      digitalWrite(gpio, LED2, LOW);

      printf("The time between was greater than 2 seconds moving on to the next number\n");
      
      // Play the end of input light sequnce
      endOfInputLights(buttonPressCount);

      // Set the postion in the array to the inputted value
      numGuess[numberOfNumbersEnetered] = buttonPressCount;

      // Print the value of the number enetered
      printf("The number entered into the sequence was %d\n", numGuess[numberOfNumbersEnetered]);
      
      // Increase the counter for the number of the numbers entered
      numberOfNumbersEnetered++;

      // Check if the number of numbers entered is greater than 3 and if so move onto the next round
      if (numberOfNumbersEnetered >= 3) // Check the sequnce if 3 numbers entered
      {

        // Check the matches for aproxomate and exact
        int* numGuessPointr = numGuess;
        printf("%d, %d, %d\n", *(theSeq), *(theSeq+1), *(theSeq+2));
        printf("%d, %d, %d\n", *(numGuessPointr), *(numGuessPointr+1), *(numGuessPointr+2));
        int answr = matches(theSeq, numGuessPointr);
        int approx = (answr & 0x03);
        int match = ((answr >> 2) & 0x03);

        // Display end of round LED sequnce
        digitalWrite(gpio, LED2, HIGH);
        delay(BLINK_DELAY);
        digitalWrite(gpio, LED2, LOW);
        delay(BLINK_DELAY);
        digitalWrite(gpio, LED2, HIGH);
        delay(BLINK_DELAY);
        digitalWrite(gpio, LED2, LOW);

        // Set the found value
        found = (match == 3);

        delay(1000);

        // Run LED sequnce for the number of exact and aproxomate matches
        communicateGuessAccuracy(match, approx);

        // If the sequnce was not correct then proceed to the next round
        if (!found)
        {
          printf("Beginning new round\n");
          printf("Please enter number: \n");
          waitForButton(gpio, BUTTON);
          timeToSubtract = clock();
        }

        // Reset variables to begin next round
        numberOfNumbersEnetered = 0;
        roundNumber++;
      }
      else {

        // Ask user for button input
        printf("Please enter number: \n");
        waitForButton(gpio, BUTTON);
        timeToSubtract = clock();
      }

      // Reset the button press count
      buttonPressCount = 0;
    }

  }
  if (found)
  {
    printf("Sequence found!");

    // Display end of game LED sequence
    digitalWrite(gpio, LED2, HIGH);

    for (int i = 0; i < 3; i++)
    {
      digitalWrite(gpio, LED, HIGH);
      delay(BLINK_DELAY);
      digitalWrite(gpio, LED, LOW);
      delay(BLINK_DELAY);
    }

    digitalWrite(gpio, LED2, LOW);
  }
  else
  {
    fprintf(stdout, "Sequence not found\n");
  }
  return 0;
}
