@ This ARM Assembler code should implement a matching function, for use in the MasterMind program, as
@ described in the CW2 specification. It should produce as output 2 numbers, the first for the
@ exact matches (peg of right colour and in right position) and approximate matches (peg of right
@ color but not in right position). Make sure to count each peg just once!
	
@ Example (first sequence is secret, second sequence is guess):
@ 1 2 1
@ 3 1 3 ==> 0 1
@ You can return the result as a pointer to two numbers, or two values
@ encoded within one number
@
@ -----------------------------------------------------------------------------

.text
@ this is the matching fct that should be called from the C part of the CW	
.global         matches
@ use the name `main` here, for standalone testing of the assembler code
@ when integrating this code into `master-mind.c`, choose a different name
@ otw there will be a clash with the main function in the C code
.global         main1
main1: 
	LDR  R2, =secret	@ pointer to secret sequence
	LDR  R3, =guess		@ pointer to guess sequence


	@ you probably need to initialise more values here

	@ ... COMPLETE THE CODE BY ADDING YOUR CODE HERE, you should use sub-routines to structure your code

exit:	@MOV	 R0, R4		@ load result to output register
	MOV 	 R7, #1		@ load system call code
	SWI 	 0		@ return this value

@ -----------------------------------------------------------------------------
@ sub-routines

@ this is the matching fct that should be callable from C	
matches:			@ Input: R0, R1 ... ptr to int arrays to match 
					@ Output: R0 ... exact matches (10s) and approx matches (1s) of base COLORS

	@ --------------------------------------------------------------------------------------------
	@ The arm assembler sub-routine for checking matches and approxes is as follows:
	@ --------------------------------------------------------------------------------------------
	@ * Check secret digit 1 against guess digit 1, if it matches erase that secret digit and skip to the next secret digit.
	@ * Check secret digit 1 against guess digit 2, if it matches erase that secret digit and skip to the next secret digit.
	@ * Check secret digit 1 against guess digit 3, if it matches erase that secret digit and ect.
	@ * Repeat the same, check secret digit 2 against guess digit 2, erase secret digit if it matches, skip to next secret etc.
	@ * check secret digit 2 against guess digit 1 etc
	@ * check secret digit 2 against guess digit 3 etc
	@ * check secret digit 3 against guess digit 3 etc
	@ * check secret digit 3 against guess digit 1 etc
	@ * check secret digit 3 against guess digit 2 etc
	@ * store the number of approxes and matches into a single register using bitwise operations, and return to the c code.
	@ --------------------------------------------------------------------------------------------

	@ We're going to need all these registers, push the existing data to a stack for now.
	push {r2-r8}

	@ Since we're going to be branching in the matches statement, save the
	@ return address so we can return to our c code once this whole function
	@ is done.
	mov r10, lr
	
	@ r2, r3, and r4 will contain the numbers of the secret.
	ldm r0!, {r2-r4}
	
	@ r5, r6, and r7 will contain the numbers of the guess.
	ldm r1!, {r5-r7}
	
	@ We no longer need the addresses of the numbers so
	@ we can use r0 and r1 for other perposes now.
	@ For now we'll use r0 to keep co
	mov r0, #0
	mov r1, #0
	
	@ Each time we check each digit from the secret,
	@ we may need to skip ahead to the next digit if a
	@ matching digit is found in a branching statement.
	@ To jump to that next digit, store the code address of
	@ the code that reads the next digit.
	ldr r8, =di2
	
	
	@ check secret digit 1 and guess digit 1
	cmp r2, r5
	
	@ Clear the digit in case it is a match,
	@ as we don't want the rest of the algorithm
	@ to detect it if it matches.
	mov r9, r5
	mov r5, #0
	beq correctNum
	
	@ If it matches, this line will be skipped, and
	@ the guess digit in that register will be erased.
	@ If not, then the digit is restored to the register.
	mov r5, r9
	
	@ Now to check the other digits.
	@ Wash, rinse, repeat, except since r2 and r6 are not digits
	@ in the same pos, we will skip to "approxNum"
	@ instead if they match.
	@ Compare 1st secret digit to 2nd guess digit
	cmp r2, r6
	mov r9, r6
	mov r6, #0
	beq approxNum
	mov r6, r9
	
	@ Same deal here.
	@ Compare 1st secret digit to 3rd guess digit
	cmp r2, r7
	mov r9, r7
	mov r7, #0
	beq approxNum
	mov r7, r9

	b di2

di2:	
	@ Prepare the address of the code that checks the next secret digit
	@ so we can jump to it should we find an approx or match.
	ldr r8, =di3

	@ Same as before, we're just comparing the rest
	@ of the secret digits to the guess digits.
	@ Compare 2nd secret digit to 2nd guess digit
	cmp r3, r6
	mov r9, r6
	mov r6, #0
	beq correctNum
	mov r6, r9
	
	@ Compare 2nd secret digit to 1st guess digit
	cmp r3, r5
	mov r9, r5
	mov r5, #0
	beq approxNum
	mov r5, r9
	
	@ Compare 2nd secret digit to 3rd guess digit
	cmp r3, r7
	mov r9, r7
	mov r7, #0
	beq approxNum
	mov r7, r9

	b di3
	
di3:	
	@ Prepare the address of the code that checks the next secret digit
	@ so we can jump to it should we find an approx or match.
	ldr r8, =did

	@ Compare 3rd secret digit to 3rd guess digit
	cmp r4, r7
	mov r9, r7
	mov r7, #0
	beq correctNum
	mov r7, r9
	
	@ Compare 3rd secret digit to 1st guess digit
	cmp r4, r5
	mov r9, r5
	mov r5, #0
	beq approxNum
	mov r5, r9
	
	@ Compare 3rd secret digit to 2nd guess digit
	cmp r4, r6
	mov r9, r6
	mov r6, #0
	beq approxNum
	mov r6, r9

	@ At this point we should've compared all the numbers now.
	b did
	
did:
    @ Now "combine" the matches and aprroxes into a single register.
	lsl r1, #2
	orr r0, r1

	@ Store all of our old registers back from the stack now that we're
	@ done with them.
	pop {r2-r8}

	@ Return to our c code. 
    bx r10
	

approxNum:
	@ Increment the number of approx matches by 1
	@ and go to the next secret digit.
	add r0, #1
	bx r8
	

correctNum:
	@ Increment the number of correct matches by 1
	@ and go to the next secret digit.
	add r1, #1
	bx r8



@ show the sequence in R0, use a call to printf in libc to do the printing, a useful function when debugging 
showseq: 			@ Input: R0 = pointer to a sequence of 3 int values to show
	@ COMPLETE THE CODE HERE (OPTIONAL)
	
	
@ =============================================================================

.data

@ constants about the basic setup of the game: length of sequence and number of colors	
.equ LEN, 3
.equ COL, 3
.equ NAN1, 8
.equ NAN2, 9

@ a format string for printf that can be used in showseq
f4str: .asciz "Seq:    %d %d %d\n"

@ a memory location, initialised as 0, you may need this in the matching fct
n: .word 0x00
	
@ INPUT DATA for the matching function
.align 4
secret: .word 1 
	.word 2 
	.word 1 

.align 4
guess:	.word 3 
	.word 1 
	.word 3 

@ Not strictly necessary, but can be used to test the result	
@ Expect Answer: 0 1
.align 4
expect: .byte 0
	.byte 1

.align 4
secret1: .word 1 
	 .word 2 
	 .word 3 

.align 4
guess1:	.word 1 
	.word 1 
	.word 2 

@ Not strictly necessary, but can be used to test the result	
@ Expect Answer: 1 1
.align 4
expect1: .byte 1
	 .byte 1

.align 4
secret2: .word 2 
	 .word 3
	 .word 2 

.align 4
guess2:	.word 3 
	.word 3 
	.word 1 

@ Not strictly necessary, but can be used to test the result	
@ Expect Answer: 1 0
.align 4
expect2: .byte 1
	 .byte 0
