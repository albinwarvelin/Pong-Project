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

int ballX;
int ballY;
int ballDirX = 0; //0 Left 1 Right
int ballDirY = 0; //0 Up 1 Down
const int ballSpeedX = 1;
const int ballSpeedY = 1;
const int ballSize = 2;
int win = 0; //1 = left win, 2 = right win
int firstWin = 0; //Used as a flag to not reset all bits when won if no need for it

/* Interrupt Service Routine, runs game at 30fps */
void user_isr( void )
{	
	if(!win)
	{
		matrix_to_array();
		display_game(0, displayArray);
		
		int btns = (PORTD & 0x000000e0) >> 5;
		int btn1 = (PORTF & ~0x01) >> 1;
		
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
		}
		if((btn1 & 0x1) != 0) //Button 1
		{
			move_r_paddle_down();
		}
		
		ball_update();
		
		IFS(0) &= ~0x100; //Reset flag
	}	
	else 
	{
		if(firstWin)
		{
			for(x = 0; x < 128; x++)
			{
				for(y = 0; y < 32; y++)
				{
					display[x][y] = 0;
				}
			}
			matrix_to_array();
			display_game(0, displayArray);
			firstWin = 0;
		}
		if(win == 1)
		{
			display_string(2, "Left wins!");
		}
		if(win == 2)
		{
			display_string(2, "Right wins!");
		}
		display_update();
	}
}

/* Initialization here, run before game */
void game_init( void )
{
	TRISD |= 0xfe0; //Set port to input
	
	T2CON = 0x8070; //Prescaling 1:256
	PR2 = 10417; //30 interrupts/second (frames per second on display)
	
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
	
	/* Initializes game mechanics */
	paddle_l_init();
	paddle_r_init();
	ball_init();
	return;
}

/* Original code by F.Lundevall rewritten by Albin Warvelin in order to handle whole screen image (128*32) */
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

/* Writes matrix data to single row array, utilizes global variables as they are used throughout the game.
   Display is updated in 8 bit columns along four rows, hence method. */
void matrix_to_array()
{
	for(i = 0; i < 4; i++) //4 Segment rows
	{
		for(j = 0; j < 128; j++) //128 Columns
		{
			int col = 255;
			for(k = 0; k < 8; k++) //8 bits per column
			{
				col -= display[j][(i * 8) + k] * power(2, k); //On display high = pixel off and low = pixel on, therefore k power of 2 is subtracted from value if pixel shall be on.
				
				displayArray[(128 * i) + j] = col; //Adds to 1d array
			}
		}
	}
}

/* Math.h did not work, function returns a to the power of b */
int power(int a, int b)
{
	int toReturn = 1;
	
	for(c = 0; c < b; c++)
	{
		toReturn *= a;
	}
	
	return toReturn;
}

/* Initializes left paddle */
void paddle_l_init()
{
	paddleLY = (32 / 2) - (paddleSizeY / 2); //Start pos
	
	for(x = paddleDistance; x < paddleDistance + paddleSizeX; x++) //Draw paddle
	{
		for(y = paddleLY; y < paddleLY + paddleSizeY; y++)
		{
			display[x][y] = 1;
		}
	}
}
 /* Initializes right paddle */
void paddle_r_init()
{
	paddleRY = (32 / 2) - (paddleSizeY / 2); //Start pos
	
	for(x = 127 - paddleDistance - paddleSizeX; x < 127 - paddleDistance; x++) //Draw paddle
	{
		for(y = paddleRY; y < paddleRY + paddleSizeY; y++)
		{
			display[x][y] = 1;
		}
	}
}

/* Moves left paddle up by 1 */
void move_l_paddle_up()
{
	if(paddleLY > 0)
	{		
		for(x = paddleDistance; x < paddleDistance + paddleSizeX; x++)
		{
			display[x][paddleLY] = 1; //Sets pixels on top row high (on)
			display[x][paddleLY + paddleSizeY] = 0;  //Sets pixels on bottom row low (off)
		}
		paddleLY--;
	}	
}

/* Moves paddle down by 1 */
void move_l_paddle_down()
{
	if(paddleLY < 31 - paddleSizeY)
	{		
		for(x = paddleDistance; x < paddleDistance + paddleSizeX; x++)
		{
			display[x][paddleLY] = 0; //Sets pixels on top row low (off)
			display[x][paddleLY + paddleSizeY] = 1; //Sets pixels on bottom row high (on)
		}
		paddleLY++;
	}
}

/* Moves paddle up by 1 */
void move_r_paddle_up()
{
	if(paddleRY > 0)
	{		
		for(x = 127 - paddleDistance - paddleSizeX; x < 127 - paddleDistance; x++)
		{
			display[x][paddleRY] = 1; //Sets pixels on top row high (on)
			display[x][paddleRY + paddleSizeY] = 0; //Sets pixels on bottom row low (off)
		}
		paddleRY--;
	}
}

/* Moves paddle down by 1 */ 
void move_r_paddle_down()
{
	if(paddleRY < 31 - paddleSizeY)
	{		
		for(x = 127 - paddleDistance - paddleSizeX; x < 127 - paddleDistance; x++)
		{
			display[x][paddleRY] = 0; //Sets pixels on top row low (off)
			display[x][paddleRY + paddleSizeY] = 1; //Sets pixels on bottom row high (on)
		}
		paddleRY++;
	}
}

/* Initializes ball */ 
void ball_init()
{	
	ballX = (128 / 2) - (ballSize / 2) - 2;
	ballY = (32 / 2) - (ballSize / 2) + 4;
}

/* Updates ball position and checks if win, when win variables is updated */
void ball_update()
{
	if(ballX <= paddleDistance + paddleSizeX) //Left paddle
	{
		if(ballY >= paddleLY && ballY <= paddleLY + paddleSizeY)
		{
			ballDirX ^= 1; //Invert direction
		}
	}
	else if(ballX >= 127 - paddleDistance - paddleSizeX) //Right paddle
	{
		if(ballY >= paddleRY && ballY <= paddleRY + paddleSizeY)
		{
			ballDirX ^= 1; //Invert direction
		}
	}
	
	if(ballY <= ballSpeedY) //Top bounce
	{
		ballDirY ^= 1;
	}
	if(ballY >= 31 - ballSpeedY) //Bottom bounce
	{
		ballDirY ^= 1;
	}
	
	if(ballX <= 0) //Right win
	{
		win = 2;
		firstWin = 1;
	}
	if(ballX >= 127) //Left win
	{
		win = 1;
		firstWin = 1;
	}
	
	/* Move ball */
	if(ballDirX) //Move right
	{
		ballX += ballSpeedX;
	}
	else //Move left
	{
		ballX -= ballSpeedX;
	}
	
	if(ballDirY) //Move down
	{
		ballY += ballSpeedY;
	}
	else //Move up
	{
		ballY -= ballSpeedY;
	}
	
	for(x = ballX - ballSpeedX; x < ballX + ballSize + ballSpeedX; x++) //Clear all pixels surrounding ball and where ball might have been
	{
		for(y = ballY - ballSpeedY; y < ballY + ballSize + ballSpeedY; y++)
		{
			if(!(y >= paddleLY && y < paddleLY + paddleSizeY && x >= paddleDistance && x < paddleDistance + paddleSizeX) //Exempts left paddle pixels
				&& !(y >= paddleRY && y < paddleRY + paddleSizeY && x >= 127 - paddleDistance - paddleSizeX && x < 127 - paddleDistance)) //Exempts right paddle pixels
			{
				display[x][y] = 0;
			}
		}
	}
	
	for(x = ballX; x < ballX + ballSize; x++) //Set pixels of ball high
	{
		for(y = ballY; y < ballY + ballSize; y++)
		{
			display[x][y] = 1;
		}
	}
}