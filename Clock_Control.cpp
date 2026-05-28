/*
 * Clock_Control.cpp
 *
 * Modernized for MPLAB X + CMSIS headers
 * ATSAMD21J18A
 */

#include "samd21j18a.h"
#include "Clock_Control.h"
#include "Definitions.h"

static uint32_t ClockSourceToFrequency(uint32_t clk)
{
    switch (clk)
    {
        case CLOCK_XOSC32K:
            return 32768UL;
        case CLOCK_DFLL48:
            return 48000000UL;
        case CLOCK_8MHZ:
            return 8000000UL;
        default:
            return SystemCoreClock;
    }
}

// class constructor
Clock_Control::Clock_Control()
{
}

// setup initial 48MHz system clock in closed loop control
void Clock_Control::Clock_Init(void)
{
    /************************************************************************/
    /* NVM WAIT STATES                                                      */
    /************************************************************************/

    NVMCTRL_REGS->NVMCTRL_CTRLB =
        (NVMCTRL_REGS->NVMCTRL_CTRLB & ~NVMCTRL_CTRLB_RWS_Msk) |
        NVMCTRL_CTRLB_RWS(1);

    /************************************************************************/
    /* EXTERNAL 32.768kHz OSCILLATOR (XOSC32K)                              */
    /************************************************************************/

    // Disable oscillator before configuration
    SYSCTRL_REGS->SYSCTRL_XOSC32K = 0;

    SYSCTRL_REGS->SYSCTRL_XOSC32K =
        SYSCTRL_XOSC32K_STARTUP(0x2) |
        SYSCTRL_XOSC32K_EN32K(1) |
        SYSCTRL_XOSC32K_XTALEN(1);

    // Enable oscillator (after config per SAMD21 requirement)
    SYSCTRL_REGS->SYSCTRL_XOSC32K |= SYSCTRL_XOSC32K_ENABLE(1);

    // Wait for ready
    while (!(SYSCTRL_REGS->SYSCTRL_PCLKSR & SYSCTRL_PCLKSR_XOSC32KRDY_Msk))
    {
    }

    /************************************************************************/
    /* GENERIC CLOCK GENERATOR 1                                            */
    /************************************************************************/

    // Configure divider
    GCLK_REGS->GCLK_GENDIV =
        GCLK_GENDIV_ID(1) |
        GCLK_GENDIV_DIV(1);

    while (GCLK_REGS->GCLK_STATUS & GCLK_STATUS_SYNCBUSY_Msk)
    {
    }

    // Configure GCLK1 source = XOSC32K
    GCLK_REGS->GCLK_GENCTRL =
        GCLK_GENCTRL_ID(1) |
        GCLK_GENCTRL_SRC_XOSC32K |
        GCLK_GENCTRL_IDC(1) |
        GCLK_GENCTRL_GENEN(1);

    while (GCLK_REGS->GCLK_STATUS & GCLK_STATUS_SYNCBUSY_Msk)
    {
    }

    /************************************************************************/
    /* CONNECT GCLK1 TO DFLL48M                                             */
    /************************************************************************/

    GCLK_REGS->GCLK_CLKCTRL =
        GCLK_CLKCTRL_ID_DFLL48 |
        GCLK_CLKCTRL_GEN_GCLK1 |
        GCLK_CLKCTRL_CLKEN(1);

    while (GCLK_REGS->GCLK_STATUS & GCLK_STATUS_SYNCBUSY_Msk)
    {
    }

    /************************************************************************/
    /* DFLL48M CONFIGURATION                                                */
    /************************************************************************/

    // Disable DFLL before config
    SYSCTRL_REGS->SYSCTRL_DFLLCTRL = 0;

    while (!(SYSCTRL_REGS->SYSCTRL_PCLKSR & SYSCTRL_PCLKSR_DFLLRDY_Msk))
    {
    }

    // Enable DFLL in open loop first
    SYSCTRL_REGS->SYSCTRL_DFLLCTRL = SYSCTRL_DFLLCTRL_ENABLE(1);

    while (!(SYSCTRL_REGS->SYSCTRL_PCLKSR & SYSCTRL_PCLKSR_DFLLRDY_Msk))
    {
    }

    // Set multiplier
    SYSCTRL_REGS->SYSCTRL_DFLLMUL =
        SYSCTRL_DFLLMUL_CSTEP(31) |
        SYSCTRL_DFLLMUL_FSTEP(511) |
        SYSCTRL_DFLLMUL_MUL(1465);

    while (!(SYSCTRL_REGS->SYSCTRL_PCLKSR & SYSCTRL_PCLKSR_DFLLRDY_Msk))
    {
    }

    // Enable closed-loop mode
    SYSCTRL_REGS->SYSCTRL_DFLLCTRL |=
        SYSCTRL_DFLLCTRL_MODE(1) |
        SYSCTRL_DFLLCTRL_WAITLOCK(1);

    while (!(SYSCTRL_REGS->SYSCTRL_PCLKSR & SYSCTRL_PCLKSR_DFLLRDY_Msk))
    {
    }

    /************************************************************************/
    /* SWITCH MAIN CLOCK TO DFLL48M                                         */
    /************************************************************************/

    GCLK_REGS->GCLK_GENCTRL =
        GCLK_GENCTRL_ID(0) |
        GCLK_GENCTRL_SRC_DFLL48M |
        GCLK_GENCTRL_IDC(1) |
        GCLK_GENCTRL_GENEN(1);

    while (GCLK_REGS->GCLK_STATUS & GCLK_STATUS_SYNCBUSY_Msk)
    {
    }

    /************************************************************************/
    /* INTERNAL 8MHz OSCILLATOR                                             */
    /************************************************************************/

    SYSCTRL_REGS->SYSCTRL_OSC8M =
        SYSCTRL_OSC8M_PRESC(0) |
        SYSCTRL_OSC8M_ONDEMAND(1);

    /************************************************************************/
    /* POWER MANAGER                                                        */
    /************************************************************************/

    PM_Clock_Bus_Setup();
    SystemCoreClock = 48000000UL;
}

/************************************************************************/
/* POWER MANAGER CLOCK SETUP                                            */
/************************************************************************/

void Clock_Control::PM_Clock_Bus_Setup(void)
{
    PM_REGS->PM_CPUSEL  = PM_CPUSEL_CPUDIV_DIV1;
    PM_REGS->PM_APBASEL = PM_APBASEL_APBADIV_DIV1;
    PM_REGS->PM_APBBSEL = PM_APBBSEL_APBBDIV_DIV1;
    PM_REGS->PM_APBCSEL = PM_APBCSEL_APBCDIV_DIV1;
}

/************************************************************************/
/* CHANGE SYSTEM CLOCK SOURCE                                           */
/************************************************************************/

void Clock_Control::Change_Clock(uint32_t clk)
{
    GCLK_REGS->GCLK_GENCTRL =
        GCLK_GENCTRL_ID(0) |
        GCLK_GENCTRL_SRC(clk) |
        GCLK_GENCTRL_IDC(1) |
        GCLK_GENCTRL_GENEN(1);

    while (GCLK_REGS->GCLK_STATUS & GCLK_STATUS_SYNCBUSY_Msk)
    {
    }

    SystemCoreClock = ClockSourceToFrequency(clk);
}