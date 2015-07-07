#include "compiler_defs.h"
#include "C8051F320_defs.h"


#define SYSCLK       24500000 / 8      // SYSCLK frequency in Hz

SBIT(LED1, SFR_P2, 2);                 // LED='1' means ON
SBIT(LED2, SFR_P2, 3);                 // LED='1' means ON
SBIT(SW2, SFR_P2, 0);                  // SW2='0' means switch pressed


INTERRUPT(Timer2_ISR, INTERRUPT_TIMER2)
{
   TF2H = 0;                           // Clear Timer2 interrupt flag
   LED1 = !LED1;                         // Change state of LED
   LED2 = LED1;
}

void main (void)
{
   PCA0MD &= ~0x40;                    // Disable watchdog timer
   /////////////////////////////////
   // Initialize system clock to 24.5MHz
   OSCICN = 0x80;                      // Configure internal oscillator for
                                       // its lowest frequency
   RSTSRC = 0x04;                      // Enable missing clock detector
   /////////////////////////////////
   // Initialize crossbar and GPIO
   P2MDOUT |= 0x0C;                    // Enable LED as a push-pull output

   XBR0 = 0x00;                        // No digital peripherals selected
   XBR1 = 0x40;                        // Enable crossbar and weak pull-ups

   /////////////////////////////////
   // Init Timer2 to generate interrupts at a 10Hz rate.
   TMR2CN = 0x00;                      // Stop Timer2; Clear TF2;
                                       // use SYSCLK/12 as timebase
   CKCON &= ~0x30;                     // Timer2 clocked based on T2XCLK;

   TMR2RL = -(SYSCLK / 12 / 10);       // Init reload values
   TMR2 = 0xffff;                      // Set to reload immediately
   ET2 = 1;                            // Enable Timer2 interrupts
   TR2 = 1;                            // Start Timer2
   /////////////////////////////////

   EA = 1;                             // Enable global interrupts

   while (1);                          // Spin forever
}
