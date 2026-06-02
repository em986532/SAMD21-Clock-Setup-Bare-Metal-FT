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
    PORT_REGS->GROUP[1].PORT_PMUX[15] = 0x0U;          // Clear MUX for GPIO mode
    PORT_REGS->GROUP[1].PORT_PINCFG[30] &= ~0x01U;     // Disable PMUXEN (peripheral function)
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

static inline void delay_ms_systick(int milliseconds)
{
    const uint32_t ticks_per_ms = 48000UL; // 48 MHz / 1000

    SysTick->CTRL = 0;                 // Disable SysTick
    SysTick->VAL  = 0;                 // Clear current value
    SysTick->LOAD = ticks_per_ms - 1;  // 1 ms interval
    SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_ENABLE_Msk;

    while (milliseconds > 0)
    {
        while (!(SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk))
        {
        }

        --milliseconds;
    }

    SysTick->CTRL = 0; // Disable SysTick again
    SysTick->VAL  = 0;
}

static inline void delay_ms(int milliseconds)
{
    #define CLOCK_FREQUENCY_HZ 48000000UL
    const uint32_t loop_count = CLOCK_FREQUENCY_HZ/25000; // 48 MHz / 1000
    //1ms ~ 4800 loops of 10 NOPs, so we divide by 10 to get the number of iterations for the loop at 48MHz  
    //1ms ~ 800 loops of 10 NOPs, so we divide by 10 to get the number of iterations for the loop at 8MHz
    
    // SysTick->CTRL = 0;                 // Disable SysTick
    // SysTick->VAL  = 0;                 // Clear current value
    // SysTick->LOAD = ticks_per_ms - 1;  // 1 ms interval
    // SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_ENABLE_Msk;

    while (milliseconds > 0)
    {

        //10cycle delay block
        for (volatile uint32_t i = 0; i < loop_count; i++)
        {
            //for loop assuemd to be 2 cycles per iteration, so 48,000 iterations should give us 1ms at 48MHz
            __NOP();
            __NOP();
            __NOP();
            __NOP();
            __NOP();
            __NOP();
            __NOP();
            __NOP();
        }

        --milliseconds;
    }

    //SysTick->CTRL = 0; // Disable SysTick again
    //SysTick->VAL  = 0;
}

extern "C" int main(void)
{
    int i = 0;

	clocks.Clock_Init(); //sets up 48MHz clock in closed loop mode as the default system clock
	LED_PB30_Init();     // CRITICAL: Reclaim PB30 from peripheral function after Clock_Init
	//clocks.Change_Clock(CLOCK_8MHZ); //changes the system clock in real time, see Definitions.h for clock options
    LED_PB30_On();      // Now LED control should work
	
    // 115,200
    // 230,400


    while (1) 
    {
        LED_PB30_On();
        delay_ms(50); // Delay for 1000 ms (1 second)

        // for (i = 0; i < 100000; i++)
        // {
        //     __NOP(); //delay loop
        // }

        LED_PB30_Off();
        delay_ms(950); // Delay for 1000 ms (1 second)

        // for (i = 0; i < 100000; i++)
        // {
        //     __NOP(); //delay loop
        // }
    }
    
    return 0;
}
