/*
 * Clock_Control.cpp
 *
 * Modernized for MPLAB X + CMSIS headers
 * ATSAMD21J18A
 */

#include "samd21j18a.h"
#include "Clock_Control.h"
#include "Definitions.h"
#include "plib_port.h"

#define LED_PB30_MASK (1UL << 30)

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

    // Set default coarse and fine tuning values
    SYSCTRL_REGS->SYSCTRL_DFLLVAL =
        SYSCTRL_DFLLVAL_COARSE(0x1F) |
        SYSCTRL_DFLLVAL_FINE(512);

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
    /* WAIT FOR DFLL LOCK                                                   */
    /************************************************************************/

    // *** ADDED *** Wait for coarse lock
    while (!(SYSCTRL_REGS->SYSCTRL_PCLKSR &
            SYSCTRL_PCLKSR_DFLLLCKC_Msk))
    {
    }

    // *** ADDED *** Wait for fine lock
    //this line causes the code after it to not run, so it is commented out for now. It may be that the lock flags are not being set correctly in the SAMD21, or that the DFLL is not locking properly due to some configuration issue. Further investigation would be needed to determine the root cause.
//    while (!(SYSCTRL_REGS->SYSCTRL_PCLKSR &
//            SYSCTRL_PCLKSR_DFLLLCKF_Msk))
//    {
//    }

    /************************************************************************/
    /* SWITCH MAIN CLOCK TO DFLL48M                                         */
    /************************************************************************/

    GCLK_REGS->GCLK_GENCTRL =
        GCLK_GENCTRL_ID(0) |
        GCLK_GENCTRL_SRC_DFLL48M |
        GCLK_GENCTRL_IDC(1) |
        GCLK_GENCTRL_GENEN(1)|
        GCLK_GENCTRL_RUNSTDBY(1);

    while (GCLK_REGS->GCLK_STATUS & GCLK_STATUS_SYNCBUSY_Msk)
    {
    }
    
    /************************************************************************/
    /* GENERIC CLOCK GENERATOR 4  —  DFLL48M / 48  =  1 MHz                 */
    /************************************************************************/

    // Set divider for GCLK4 to 48
    GCLK_REGS->GCLK_GENDIV =
        GCLK_GENDIV_ID(4) |
        GCLK_GENDIV_DIV(48);

    while (GCLK_REGS->GCLK_STATUS & GCLK_STATUS_SYNCBUSY_Msk)
    {
    }

    // Configure GCLK4: source = DFLL48M, divide enable, generator enable
    GCLK_REGS->GCLK_GENCTRL =
        GCLK_GENCTRL_ID(4)          |
        GCLK_GENCTRL_SRC_DFLL48M    |
        GCLK_GENCTRL_IDC(1)         |
        GCLK_GENCTRL_DIVSEL(0)      |
        GCLK_GENCTRL_OE(1)          |   // *** CHANGED *** Enable clock output to GCLK_IO pin
        GCLK_GENCTRL_GENEN(1)|
        GCLK_GENCTRL_RUNSTDBY(1);

    while (GCLK_REGS->GCLK_STATUS & GCLK_STATUS_SYNCBUSY_Msk)
    {
    }

    /************************************************************************/
    /* GCLK4 CLOCK OUTPUT PIN DEBUG  —  PA27 (GCLK_IO[1])                 */
    /*                                                                      */
    /* Outputs GCLK4 (1 MHz) to PA27 for oscilloscope debugging            */
    /************************************************************************/
    /**/ 
    // Configure PA27 as GCLK_IO[1] - set peripheral multiplexer to H
    PORT_REGS->GROUP[0].PORT_PMUX[13] =
        (PORT_REGS->GROUP[0].PORT_PMUX[13] & 0x0FU) |  // Clear upper nibble (PA27 = pin 27, uses bits 7:4)
        (0x7U << 4);                                    // Set to peripheral H (GCLK_IO[1]) in upper nibble (H=7)

    // Enable peripheral multiplexing on PA27
    PORT_REGS->GROUP[0].PORT_PINCFG[27] |= PORT_PINCFG_PMUXEN_Msk;
    /**/

    /************************************************************************/
    /* INTERNAL 8MHz OSCILLATOR                                             */
    /************************************************************************/
    // Configure OSC8M and preserve the existing calibration value
    {
        uint32_t osc8m_calib = SYSCTRL_REGS->SYSCTRL_OSC8M & SYSCTRL_OSC8M_CALIB_Msk;

        SYSCTRL_REGS->SYSCTRL_OSC8M =
            SYSCTRL_OSC8M_ENABLE(1)        | // Keep oscillator running
            SYSCTRL_OSC8M_PRESC(0)         | // Prescaler = 1 (full 8 MHz)
            osc8m_calib                     | // Preserve the factory calibration bits
            SYSCTRL_OSC8M_ONDEMAND(1);       // Only run when requested

        while (!(SYSCTRL_REGS->SYSCTRL_PCLKSR & SYSCTRL_PCLKSR_OSC8MRDY_Msk))
        {
        }
    }

    /************************************************************************/
    /* POWER MANAGER                                                        */
    /************************************************************************/

    PM_Clock_Bus_Setup();
    SystemCoreClock = 48000000UL;

    /*
    32768×1465=48,005,120 Hz
    So the DFLL target frequency is:
    fDFLL≈48.005 MHz
    The exact 48 MHz multiplier would be:
    48,000,000/32768=1464.84375
    */

    //GCLK_IO[0] PB22,PA27,PA28,PA30,PA14,PA27,PA28,PA30
    //GCLK_IO[1] PA15,PB15,PB23,PB23
    //GCLK_IO[2] PB10,
    //GCLK_IO[3] PB10
    //GCLK_IO[4] PA20,PA10,PB10,PA20
    //GCLK_IO[5] PB10
    //GCLK_IO[6] PB10
    //GCLK_IO[7] PB10

    PORT_REGS->GROUP[1].PORT_DIRSET = (1U << 10);

    PORT_REGS->GROUP[1].PORT_PINCFG[10] |= PORT_PINCFG_PMUXEN_Msk;
    PORT_REGS->GROUP[1].PORT_PMUX[5] = (PORT_REGS->GROUP[1].PORT_PMUX[5] & 0x0FU) | (0x7U << 4); // Set PB10 to peripheral H (GCLK_IO[1])
    

    //PORT_Initialize();
    //PORT_GroupOutputEnable(PORT_GROUP_B, PORT_PIN_PB10); // Set all pins of PORT_B as output for testing
    // Disable analog function if applicable
    //ANSELAbits.ANSA0 = 0;


    // Configure RA0 as output
    //TRISAbits.TRISA0 = 0;
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
        GCLK_GENCTRL_GENEN(1)|
        GCLK_GENCTRL_RUNSTDBY(1);

    while (GCLK_REGS->GCLK_STATUS & GCLK_STATUS_SYNCBUSY_Msk)
    {
    }

    SystemCoreClock = ClockSourceToFrequency(clk);
}