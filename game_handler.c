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
#include "ponggame.h"

/* Display related defines */
#define DISPLAY_CHANGE_TO_COMMAND_MODE (PORTFCLR = 0x10)
#define DISPLAY_CHANGE_TO_DATA_MODE (PORTFSET = 0x10)

#define DISPLAY_ACTIVATE_RESET (PORTGCLR = 0x200)
#define DISPLAY_DO_NOT_RESET (PORTGSET = 0x200)

#define DISPLAY_ACTIVATE_VDD (PORTFCLR = 0x40)
#define DISPLAY_ACTIVATE_VBAT (PORTFCLR = 0x20)

#define DISPLAY_TURN_OFF_VDD (PORTFSET = 0x40)
#define DISPLAY_TURN_OFF_VBAT (PORTFSET = 0x20)

/* Game variables */
uint8_t display[128][32]; //Matrix represents display in x and y coordinates pixel by pixel.
uint8_t displayArray[512]; //Single array for display input.
int i, j, k, c, x, y; //Loop variables
int paddleLY, paddleRY; //Y positions for paddles, initialized
const int paddleDistance = 3; //Paddle distance from edge of display
const int paddleSizeY = 8;
const int paddleSizeX = 2;

/* Interrupt Service Routine, runs game at 60fps */
void user_isr( void )
{	

	matrix_to_array();
	display_game(0, displayArray);
	
	int btns = (PORTD & 0x000000e0) >> 5;
	int btn1 = (PORTF & 0x00000001);
	
	if((btns & 0x4) != 0) //Button 4
	{
		move_l_paddle_up();
	}
	if((btns & 0x2) != 0) //Button 3
	{
		move_l_paddle_down();
	}
	if((btns & 0x1) != 0) //Button 2
	{
		move_r_paddle_up();
		move_l_paddle_up();
	}
	if((btn1) != 0) //Button 1
	{
		move_r_paddle_down();
		move_l_paddle_up();
	}
	
	IFS(0) &= ~0x100; //Reset flag
}

/* Initialization here, run before game */
void game_init( void )
{
	TRISD |= 0xfe0; //Set port to input
	
	T2CON = 0x8070; //Prescaling 1:256
	PR2 = 10417; //30 interrupts/second
	
	IEC(0) |= 0x100; //Timer 2 interrupt enable
	IPC(2) |= 0x1f; //Sets highest priorities
	enable_interrupt(); //Enable interrupts globally
	
	/* initialize display to 0 (all pixels off) */
	for(i = 0; i < 128; i++)
	{
		for(j = 0; j < 32; j++)
		{
			display[i][j] = 0;
		}
	}
	
	paddle_l_init();
	paddle_r_init();
	return;
}

/* Original code rewritten by Albin Warvelin in order to handle whole screen image (128*32) */
void display_game(int x, const uint8_t *data) 
{
	int i, j;
	
	for(i = 0; i < 4; i++) {
		DISPLAY_CHANGE_TO_COMMAND_MODE;

		spi_send_recv(0x22);
		spi_send_recv(i);
		
		spi_send_recv(x & 0xF);
		spi_send_recv(0x10 | ((x >> 4) & 0xF));
		
		DISPLAY_CHANGE_TO_DATA_MODE;
		
		for(j = 0; j < 128; j++)
			spi_send_recv(~data[i*128 + j]);
	}
}

/* Writes matrix data to single row array, utilizes global variables as they are used throughout the game */
void matrix_to_array()
{
	for(i = 0; i < 4; i++) //4 Segment rows
	{
		for(j = 0; j < 128; j++) //128 Columns
		{
			int col = 255;
			for(k = 0; k < 8; k++) //8 bits columns
			{
				col -= display[j][(i * 8) + k] * power(2, k);
				displayArray[(128 * i) + j] = col;
			}
		}
	}
}

/* Called repetitively from main, used for code that isn't updated each frame but continuosly */
void game_run( void )
{
	
}

/* Math.h did not work */
int power(int a, int b)
{
	int toReturn = 1;
	
	for(c = 0; c < b; c++)
	{
		toReturn *= a;
	}
	
	return toReturn;
}

void paddle_l_init()
{
	paddleLY = (32 / 2) - (paddleSizeY / 2);
	
	for(x = paddleDistance; x < paddleDistance + paddleSizeX; x++)
	{
		for(y = paddleLY; y < paddleLY + paddleSizeY; y++)
		{
			display[x][y] = 1;
		}
	}
}

void paddle_r_init()
{
	paddleRY = (32 / 2) - (paddleSizeY / 2);
	
	for(x = 127 - paddleDistance - paddleSizeX; x < 127 - paddleDistance; x++)
	{
		for(y = paddleRY; y < paddleRY + paddleSizeY; y++)
		{
			display[x][y] = 1;
		}
	}
}

void move_l_paddle_up()
{
	if(paddleLY > 0)
	{		
		for(x = paddleDistance; x < paddleDistance + paddleSizeX; x++)
		{
			display[x][paddleLY - 1] = 1;
			display[x][paddleLY + paddleSizeY - 1] = 0;
		}
		paddleLY--;
	}	
}

void move_l_paddle_down()
{
	if(paddleLY < 31 - paddleSizeY)
	{		
		for(x = paddleDistance; x < paddleDistance + paddleSizeX; x++)
		{
			display[x][paddleLY - 1] = 0;
			display[x][paddleLY + paddleSizeY - 1] = 1;
		}
		paddleLY++;
	}
}

void move_r_paddle_up()
{
	if(paddleRY > 0)
	{		
		for(x = 127 - paddleDistance - paddleSizeX; x < paddleDistance; x++)
		{
			display[x][paddleRY - 1] = 1;
			display[x][paddleRY + paddleSizeY - 1] = 0;
		}
		paddleRY--;
	}
}

void move_r_paddle_down()
{
	if(paddleRY < 31 - paddleSizeY)
	{		
		for(x = 127 - paddleDistance - paddleSizeX; x < paddleDistance; x++)
		{
			display[x][paddleRY - 1] = 0;
			display[x][paddleRY + paddleSizeY - 1] = 1;
		}
		paddleRY++;
	}
}