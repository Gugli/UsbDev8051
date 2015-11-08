#include "compiler_defs.h"
#include "C8051F320_defs.h"
#include "Usb.h"

typedef struct {
	U8 Common;
	U8 In;
	U8 Out;
} SEvent;

#define SEventQueue_EventsMaxCount 3

typedef struct {
	SEvent	Events[SEventQueue_EventsMaxCount];
	U8		Count; 
} SEventQueue;

typedef struct {

	// 0x0X when the events are added to EventsA, 0xFX if they're added to EventsB
	// X is the current index
	U8 CurrentEvent; 

	// Double buffering
	SEventQueue EventsA;
	SEventQueue EventsB;

} SNearContext;







/////////////////////////////////
/////////////////////////////////
// Globals

SEG_NEAR SNearContext NC;


/////////////////////////////////
/////////////////////////////////
// INTERRUPTS

INTERRUPT(Usb_ISR, INTERRUPT_USB0)        // Top-level USB ISR
{
	SEventQueue* EventsQueue = (NC.CurrentEvent & 0xF0)? &NC.EventsB: &NC.EventsA;
	SEvent* Event = &(EventsQueue->Events[NC.CurrentEvent & 0x0F]);

	while (USB0ADR & USB0ADR_BUSY);
	USB0ADR = (USB0ADR_BUSY | USB0ADR_CMINT);
    while (USB0ADR & USB0ADR_BUSY); 
	 Event->Common = USB0DAT;
	while (USB0ADR & USB0ADR_BUSY);
	USB0ADR = (USB0ADR_BUSY | USB0ADR_IN1INT);
    while (USB0ADR & USB0ADR_BUSY); 
	Event->In = USB0DAT;
	while(USB0ADR & USB0ADR_BUSY);
	USB0ADR = (USB0ADR_BUSY | USB0ADR_OUT1INT);
    while (USB0ADR & USB0ADR_BUSY); 
	Event->Out = USB0DAT;

	NC.CurrentEvent++;
	EventsQueue->Count = NC.CurrentEvent & 0x0F;
}

/////////////////////////////////
/////////////////////////////////
// MAIN

void main (void)
{
	U16 I;
	SEventQueue* EventsQueue;

    PCA0MD &= ~0x40;                    			// Disable Watchdog timer 
 	OSCICN = OSCICN_IOSCEN | OSCICN_IFCN1 | OSCICN_IFCN0;          // Configure internal oscillator

  	CLKMUL = CLKMUL_MULSEL_INTERNAL;   
   	CLKMUL = CLKMUL_MULSEL_INTERNAL | CLKMUL_MULEN;                       // Enable clock multiplier
   	CLKMUL = CLKMUL_MULSEL_INTERNAL | CLKMUL_MULEN | CLKMUL_MULINIT;      // Initialize the clock multiplier
   	
	for(I = 0;I < 500;I++);             // Delay for clock multiplier to begin
   	while(!(CLKMUL & CLKMUL_MULRDY));   // Wait for multiplier to lock

  	CLKSEL = CLKSEL_CLKSL_INTERNAL | CLKSEL_USBCLK_4XMUL;            // Select system clock and USB clock

	while (USB0ADR & USB0ADR_BUSY);
	USB0ADR = USB0ADR_POWER;
	USB0DAT = USB0DAT_POWER_USBRST; // Request Reset
	while (USB0ADR & USB0ADR_BUSY);
	USB0ADR = USB0ADR_IN1IE;
	USB0DAT = USB0DAT_IN1IE_EP0 | USB0DAT_IN1IE_EP1 | USB0DAT_IN1IE_EP2; // Enable Endpoint 0-2 in interrupts
	while (USB0ADR & USB0ADR_BUSY);
	USB0ADR = USB0ADR_OUT1IE;
	USB0DAT = USB0DAT_OUT1IE_EP1 | USB0DAT_OUT1IE_EP2; // Enable Endpoint 1-2 out interrupts
	while (USB0ADR & USB0ADR_BUSY);
	USB0ADR = USB0ADR_CMIE;
	USB0DAT = USB0DAT_CMIE_RSTINTE | USB0DAT_CMIE_RSUINTE | USB0DAT_CMIE_SUSINTE; // Enable Reset, Resume, and Suspend interrupts

	USB0XCN = USB0XCN_PREN | USB0XCN_PHYEN | USB0XCN_SPEED; // Activate Usb transciever at Full speed

	while (USB0ADR & USB0ADR_BUSY);
	USB0ADR = USB0ADR_CLKREC;
	USB0DAT = USB0DAT_CLKREC_CRE; // Enable clock recovery

	EIE1 |= 0x02;// Enable USB0 Interrupts

	while (USB0ADR & USB0ADR_BUSY);
	USB0ADR = USB0ADR_POWER;
	USB0DAT = (USB0DAT & ~USB0DAT_POWER_USBINH) | USB0DAT_POWER_SUSEN; // Activate transciever and enable suspend detection

	//RSTSRC |= RSTSRC_USBRSF; // Enables Usb as a reset source. Will reset immediately if USB is not connected
   	EA = 1; // global interrupts enable 

	while (1)
	{
   		// Swap EventsQueues and reset index : atomic operation (set can't be interrupted)
		NC.CurrentEvent = (NC.CurrentEvent & 0xF0) ? 0x00 : 0xF0; 
   		
		// Read events and answer
		EventsQueue = (NC.CurrentEvent & 0xF0)? &NC.EventsA: &NC.EventsB;
		for(I = 0; I<EventsQueue->Count; I++ )
		{

		}
	}
}
