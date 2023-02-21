/* mipslabwork.c

   This file written 2015 by F Lundevall
   Updated 2017-04-21 by F Lundevall

   This file should be changed by YOU! So you must
   add comment(s) here with your name(s) and date(s):

   This file modified 2017-04-31 by Ture Teknolog 

   For copyright and licensing, see file COPYING */

#include <stdint.h>   /* Declarations of uint_32 and the like */
#include <stdio.h>
#include <pic32mx.h>  /* Declarations of system-specific addresses etc */
#include "mipslab.h"  /* Declatations for these labs */

int mytime = 0x5957;
int prime = 0;
int timeoutcount = 0;

char textstring[] = "text, more text, and even more text!";

/* Interrupt Service Routine */
void user_isr( void )
{	
	timeoutcount++;
	
	if(timeoutcount % 10 == 0)
	{
		time2string( textstring, mytime );
		display_string( 3, textstring );
		display_update();
		tick( &mytime );
		
		PORTE += 1; //Lights
	}

	IFS(0) &= ~0x100; //Reset flag
}

/* Lab-specific initialization goes here */
void labinit( void )
{
	TRISE &= ~0xff; //Set port to output
	TRISD |= 0xfe0; //Set port to input
	
	T2CON = 0x8070; //Prescaling 1:256
	PR2 = 0x7a12; //31250
	
	IEC(0) |= 0x100; //Timer 2 interrupt enable
	IPC(2) |= 0x1f; //Sets highest priorities
	enable_interrupt(); //Enable interrupts globally
	return;
}

/* This function is called repetitively from the main program */
void labwork( void )
{
	prime = nextprime(prime);
	display_string(0, itoaconv(prime));
	display_update();
}
