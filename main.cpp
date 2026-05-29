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

#define LED_PB30_MASK (1UL << 30)

Clock_Control clocks; //declare object to access clock control class

static inline void LED_PB30_Init(void)
{
    PORT_REGS->GROUP[1].PORT_PINCFG[30] = 0x0U;        // Disable peripheral functions
    PORT_REGS->GROUP[1].PORT_PMUX[15] = 0x0U;          // Clear MUX for GPIO mode
    PORT_REGS->GROUP[1].PORT_DIRSET = LED_PB30_MASK;   // Set as output
    PORT_REGS->GROUP[1].PORT_OUTCLR = LED_PB30_MASK;   // Clear (LED off initially)
}

static inline void LED_PB30_On(void)
{
    PORT_REGS->GROUP[1].PORT_OUTCLR = LED_PB30_MASK;
}

static inline void LED_PB30_Off(void)
{
    PORT_REGS->GROUP[1].PORT_OUTSET = LED_PB30_MASK;
}

extern "C" int main(void)
{
    int i = 0;
    LED_PB30_Init();
    LED_PB30_On();//LEDs work

	clocks.Clock_Init(); //sets up 48MHz clock in closed loop mode as the default system clock

	clocks.Change_Clock(CLOCK_8MHZ); //changes the system clock in real time, see Definitions.h for clock options
	
	
    while (1) 
    {
        LED_PB30_On();

        for (i = 0; i < 1000000; i++)
        {
            __NOP(); //delay loop
        }

        LED_PB30_Off();

        for (i = 0; i < 1000000; i++)
        {
            __NOP(); //delay loop
        }
    }
    
    return 0;
}
