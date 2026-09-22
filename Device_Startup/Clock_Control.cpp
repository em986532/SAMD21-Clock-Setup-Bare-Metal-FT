/*
 * Clock_Control.cpp
 *
 * Created: 12/5/2020
 * Author: ForceTronics
 */

#include "definitions.h"
#include "Clock_Control.h"

// class constructor
Clock_Control::Clock_Control() { }

// setup initial 48MHz system clock in closed loop control
void Clock_Control::Clock_Init(void)
{
    //=========================================================
    // NVM WAIT STATES
    //=========================================================

    // 1 wait state required @ 48MHz / 3.3V
    NVMCTRL_REGS->NVMCTRL_CTRLB |= NVMCTRL_CTRLB_RWS(1);

    //=========================================================
    // EXTERNAL 32.768kHz CRYSTAL
    //=========================================================

    SYSCTRL_REGS->SYSCTRL_XOSC32K =
          SYSCTRL_XOSC32K_STARTUP(0x2)
        | SYSCTRL_XOSC32K_EN32K_Msk
        | SYSCTRL_XOSC32K_XTALEN_Msk;

    // Enable oscillator
    SYSCTRL_REGS->SYSCTRL_XOSC32K |= SYSCTRL_XOSC32K_ENABLE_Msk;

    // Wait for oscillator ready
    while (!(SYSCTRL_REGS->SYSCTRL_PCLKSR &
             SYSCTRL_PCLKSR_XOSC32KRDY_Msk))
    {
    }

    //=========================================================
    // GENERIC CLOCK GENERATOR 1
    //=========================================================

    // Divide factor for GCLK1
    GCLK_REGS->GCLK_GENDIV =
          GCLK_GENDIV_DIV(1)
        | GCLK_GENDIV_ID(1);

    // Configure GCLK1 source = XOSC32K
    GCLK_REGS->GCLK_GENCTRL =
          GCLK_GENCTRL_ID(1)
        | GCLK_GENCTRL_SRC_XOSC32K
        | GCLK_GENCTRL_IDC_Msk
        | GCLK_GENCTRL_GENEN_Msk;

    // Wait for sync
    while (GCLK_REGS->GCLK_STATUS & GCLK_STATUS_SYNCBUSY_Msk)
    {
    }

    //=========================================================
    // CONNECT GCLK1 TO DFLL48M REFERENCE
    //=========================================================

    GCLK_REGS->GCLK_CLKCTRL =
          GCLK_CLKCTRL_CLKEN_Msk
        | GCLK_CLKCTRL_GEN_GCLK1
        | GCLK_CLKCTRL_ID_DFLL48;

    while (GCLK_REGS->GCLK_STATUS & GCLK_STATUS_SYNCBUSY_Msk)
    {
    }

    //=========================================================
    // DFLL48M CONFIGURATION
    //=========================================================

    // Wait for DFLL ready
    while (!(SYSCTRL_REGS->SYSCTRL_PCLKSR &
             SYSCTRL_PCLKSR_DFLLRDY_Msk))
    {
    }

    // Enable DFLL in open loop mode first
    SYSCTRL_REGS->SYSCTRL_DFLLCTRL =
        SYSCTRL_DFLLCTRL_ENABLE_Msk;

    while (!(SYSCTRL_REGS->SYSCTRL_PCLKSR &
             SYSCTRL_PCLKSR_DFLLRDY_Msk))
    {
    }

    // Configure multiplier
    SYSCTRL_REGS->SYSCTRL_DFLLMUL =
          SYSCTRL_DFLLMUL_CSTEP(31)
        | SYSCTRL_DFLLMUL_FSTEP(511)
        | SYSCTRL_DFLLMUL_MUL(1465);

    while (!(SYSCTRL_REGS->SYSCTRL_PCLKSR &
             SYSCTRL_PCLKSR_DFLLRDY_Msk))
    {
    }

    // Closed loop mode + wait lock
    SYSCTRL_REGS->SYSCTRL_DFLLCTRL |=
          SYSCTRL_DFLLCTRL_MODE_Msk
        | SYSCTRL_DFLLCTRL_WAITLOCK_Msk;

    while (!(SYSCTRL_REGS->SYSCTRL_PCLKSR &
             SYSCTRL_PCLKSR_DFLLRDY_Msk))
    {
    }

    //=========================================================
    // GENERIC CLOCK GENERATOR 0 (MAIN SYSTEM CLOCK)
    //=========================================================

    GCLK_REGS->GCLK_GENCTRL =
          GCLK_GENCTRL_ID(0)
        | GCLK_GENCTRL_SRC_DFLL48M
        | GCLK_GENCTRL_IDC_Msk
        | GCLK_GENCTRL_GENEN_Msk;

    while (GCLK_REGS->GCLK_STATUS &
           GCLK_STATUS_SYNCBUSY_Msk)
    {
    }

    //=========================================================
    // INTERNAL 8MHz OSCILLATOR
    //=========================================================

    SYSCTRL_REGS->SYSCTRL_OSC8M &= ~SYSCTRL_OSC8M_PRESC_Msk;
    SYSCTRL_REGS->SYSCTRL_OSC8M &= ~SYSCTRL_OSC8M_ONDEMAND_Msk;

    // Setup power management clocks
    PM_Clock_Bus_Setup();
}

//=========================================================
// POWER MANAGEMENT SETUP
//=========================================================

void Clock_Control::PM_Clock_Bus_Setup(void)
{
    PM_REGS->PM_CPUSEL = PM_CPUSEL_CPUDIV_DIV1;
    PM_REGS->PM_APBASEL = PM_APBASEL_APBADIV_DIV1;
    PM_REGS->PM_APBBSEL = PM_APBBSEL_APBBDIV_DIV1;
    PM_REGS->PM_APBCSEL = PM_APBCSEL_APBCDIV_DIV1;
}

//=========================================================
// CHANGE SYSTEM CLOCK SOURCE
//=========================================================

void Clock_Control::Change_Clock(uint32_t clk)
{
    GCLK_REGS->GCLK_GENCTRL =
          GCLK_GENCTRL_ID(0)
        | GCLK_GENCTRL_SRC(clk)
        | GCLK_GENCTRL_IDC_Msk
        | GCLK_GENCTRL_GENEN_Msk;

    while (GCLK_REGS->GCLK_STATUS &
           GCLK_STATUS_SYNCBUSY_Msk)
    {
    }
}