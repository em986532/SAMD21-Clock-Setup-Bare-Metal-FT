/*
 * main.cpp
 *
 * Created: 12/5/2020 
 * Author : Forcetronics
 *
 */


#include "samd21j18a.h"
#include "Clock_Control.h" //class for setting up and changing system clock
#include "Definitions.h" //definitions for settings

Clock_Control clocks; //declare object to access clock control class


extern "C" int main(void)
{
	clocks.Clock_Init(); //sets up 48MHz clock in closed loop mode as the default system clock
	clocks.Change_Clock(CLOCK_8MHZ); //changes the system clock in real time, see Definitions.h for clock options
	
    while (1) 
    {
    }
    
    return 0;
}
