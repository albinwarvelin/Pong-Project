#include <stdint.h>
#include <pic32mx.h>
#include "mipslab.h"

int getsw()
{
	return (PORTD & 0x00000f00) >> 8;
}

int getbtns()
{
	return (PORTD & 0x000000e0) >> 5;
}