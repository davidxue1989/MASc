// ParallelPort.h : defines functions for reading and writing to parallel port

#pragma once
#include <stdio.h>
#include <conio.h>
#include <windows.h>

#define PPORT_BASE ((short) 0x378)


//INPUTS
#define FIRSTOPTION			119
#define SECONDOPTION		 95
#define THIRDOPTION			255

#define WETHANDS_BUTTON		119
#define GOTSOAP_BUTTON		111
#define SCRUBBED_BUTTON		 95
#define RINSED_BUTTON		 63
#define DRIEDHANDS_BUTTON	255

#define SOAPDISPENSER		 79
#define TOWELDISPENSER		 31

//OUTPUTS
#define REWARD1BUTTON		  1
#define REWARD2BUTTON		  8
#define REWARD3BUTTON		128

#define WETHANDS_LIGHT		  1
#define GOTSOAP_LIGHT		  2
#define SCRUBBED_LIGHT		  8
#define RINSED_LIGHT		 32
#define DRIEDHANDS_LIGHT	128


/* Definitions in the build of inpout32.dll are:
 *   short _stdcall Inp32(short PortAddress);
 *   void _stdcall Out32(short PortAddress, short data);
 */

/* prototype (function typedef) for DLL function Inp32: */

typedef short (_stdcall *inpfuncPtr)(short portaddr);
typedef void (_stdcall *oupfuncPtr)(short portaddr, short datum);


// Prototypes for Test functions
void test_read8(void);
void test_write(void);
void test_write_datum(short datum);

//short GetInputs(void);
//void WriteOutput(short datum);


/* After successful initialization, these 2 variables
   will contain function pointers.
 */
 
inpfuncPtr inp32fp;
oupfuncPtr oup32fp;


/* Wrapper functions for the function pointers
    - call these functions to perform I/O.
 */
short  Inp32 (short portaddr)
{
     return (inp32fp)(portaddr);
}

void  Out32 (short portaddr, short datum)
{
     (oup32fp)(portaddr,datum);
}

void test_read8(void) {

     short x;
     short i;

     /* Try to read 0x378..0x37F, LPT1:  */

     for (i=PPORT_BASE; (i<(PPORT_BASE+8)); i++) {

          x = Inp32(i);

          printf("Port read (%04X)= %04X\n",i,x);
     }

}

/*
   TEST:  Write constant 0x77 to PORT_BASE (Data register)
 */
void test_write(void) {
     short x;
     short i;

     /*****  Write the data register */

     i=PPORT_BASE;
     x=0x75;

     /*****  Write the data register */
     Out32(i,x);

     printf("Port write to 0x%X, datum=0x%2X\n" ,i ,x);

     /***** And read back to verify  */
     x = Inp32(i);
     printf("Port read (%04X)= %04X\n",i,x);


     /*****  Set all bits high */
     x=0xFF;
     Out32(i,x);

     /*****  Now, set bi-directional and read again */
     Out32(PPORT_BASE+2,0x20);     // Activate bi-directional
     x = Inp32(i);
     printf("Set Input, read (%04X)= %04X\n",i,x);

     Out32(PPORT_BASE+2,0x00);     // Set Output-only again
     x = Inp32(i);
     printf("Reset Ouput, read (%04X)= %04X\n",i,x);

}

/*
   TEST:  Write data from parameter
 */
void test_write_datum(short datum) {
     short x;
     short i;

     i=PPORT_BASE;
     x = datum;

     /*****  Write the data register */
     Out32(i,x);

     printf("Port write to 0x%X, datum=0x%2X\n" ,i ,x);

     /***** And read back to verify  */
     x = Inp32(i);
     printf("Port read (%04X)= %04X\n",i,x);
}

//short GetInputs(void)
//{
//	short x;
//	short i;
//
//	//Status port address = port base address + 1
//	i=PPORT_BASE + 1;
//	//Input from status port
//	x = Inp32(i);
//
//	return x;
//}
//
//
//void WriteOutput(short datum){
//	
//	Out32(PPORT_BASE,datum);

//}

	/*	Active Low Inputs - Pull up Resistors with Momentary Push Buttons

					InputPin	Decimal		Binary		Hexadecimal		Input - output value
						None	127			01111111			7F		None
						15		119			01110111			77		Button1 - Pin 2/Data0 = 1
						13		111			01101111			6F		Button2 - Pin 3/Data1 = 2
						12		 95			01011111			5F		Button3 - Pin 5/Data3 = 8
						10		 63			00111111			3F		Button4 - Pin 7/Data5 = 32
						11		255			11111111			FF		Button5 - Pin 9/Data7 = 128

						//15,13	103			01100111			67		
						12,10	 31			00011111			1F		Towel Dispenser 
						
						15,12	 87										
						15,10	 55					 					
						13,12	 79										Soap Dispenser

	
	`	++++++++ */
