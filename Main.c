#include "compiler_defs.h"
#include "C8051F320_defs.h"
#include "Usb.h"

#define MEnabled 100
#define MDisabled 200
#define MIsEnabled(__Value) (2/(MEnabled/100) == 2/(__Value/100) )
#define MIsDisabled(__Value) (2/(MDisabled/100) == 2/(__Value/100) )

typedef U8 Bool;

typedef enum 
{
	False = 0x00,
	True = 0xFF
} EBoolValues;

/////////////////////////////////
/////////////////////////////////
// Globals


SEG_CODE SUsbDescriptor_Device UsbDescriptor =
{
	{
		sizeof(SUsbDescriptor_Device),		// Size of this Descriptor in Bytes
		EUsbDescriptorType_Device			// Descriptor Type (=1)
	},            
	MUsbWord_Word(EUsbSpecVer_1d1),                        // USB Spec Release Number in BCD
	0x00,            // Device Class Code
	0x00,           // Device Subclass Code
	0x00,            // Device Protocol Code
	10,           // Maximum Packet Size for EP0
	MUsbWord_Word(EUsbVendorId_Prototype),   // Vendor ID
	MUsbWord_Bytes(00,01),                   // Product ID
	MUsbWord_Bytes(00,01),                     // Device Release Number in BCD
	0x01,            // Index of String Desc for Manufacturer
	0x02,           // Index of String Desc for Product
	0x00,           // Index of String Desc for SerNo
	0x01           // Number of possible Configurations
};

const unsigned char String1Desc [] = USB_STRING('G', 'u', 'g', 'l', 'i');
const unsigned char String2Desc [] = USB_STRING('F', 'o', 'o', 't', 's', 'w', 'i', 't', 'c', 'h');
const unsigned char String3Desc [] = USB_STRING('S', 'N', '-', '4', '2');

unsigned char const * const StringTable[3] =
{
   String1Desc,
   String2Desc,
   String3Desc
};

/////////////////////////////////
/////////////////////////////////
// Structs

typedef enum 
{
	EUsbEndpointState_Idle			,
	EUsbEndpointState_Transmit		,
	EUsbEndpointState_Recieve		,
	EUsbEndpointState_Halt			, // return stalls
	EUsbEndpointState_Stall			, // send procedural stall next status phase
	EUsbEndpointState_Adress		, // change FADDR during next status phase
	EUsbEndpointState_HidGetReport	,
	EUsbEndpointState_HidSetReport		
} EUsbEndpointState;

typedef struct {
	EUsbEndpointState EndpointStates[3];
} SMainContext;

SEG_NEAR SMainContext MC;

/////////////////////////////////
/////////////////////////////////
// INTERRUPTS

typedef struct {
	U8 Common;
	U8 In;
	U8 Out;
} SEvent;




Bool Usb_ReadEndpointFifo (U8 _UsbAddress, U8* _Data, U16 _Length)
{
	U16 I;

	if (!_Length)
		return True;
      
	USB0ADR = _UsbAddress | USB0ADR_AUTORD | USB0ADR_BUSY;       
	for(I=0;I< (_Length);I++)
	{
		while (USB0ADR & USB0ADR_BUSY); 
		_Data[I] = USB0DAT;           	
	}         
	return True;
}

INTERRUPT(Usb_ISR, INTERRUPT_USB0)  
{
	SEvent CurrentEvent;

	while (USB0ADR & USB0ADR_BUSY);
	USB0ADR = (USB0ADR_BUSY | USB0ADR_CMINT);
    while (USB0ADR & USB0ADR_BUSY); 
	CurrentEvent.Common = USB0DAT;
	while (USB0ADR & USB0ADR_BUSY);
	USB0ADR = (USB0ADR_BUSY | USB0ADR_IN1INT);
    while (USB0ADR & USB0ADR_BUSY); 
	CurrentEvent.In = USB0DAT;
	while(USB0ADR & USB0ADR_BUSY);
	USB0ADR = (USB0ADR_BUSY | USB0ADR_OUT1INT);
    while (USB0ADR & USB0ADR_BUSY); 
	CurrentEvent.Out = USB0DAT;

	if (CurrentEvent.Common & USB0ADR_CMINT_RSUINT)          
    {
    	// Handle Resume interrupt		
    }
	if (CurrentEvent.Common & USB0ADR_CMINT_SUSINT)          
	{
		// Handle Suspend interrupt
	}
	if (CurrentEvent.Common & USB0ADR_CMINT_RSTINT)          
	{
		// Handle Reset interrupt	
		
		while (USB0ADR & USB0ADR_BUSY);
		USB0ADR = USB0ADR_POWER;
		USB0DAT = USB0DAT_POWER_SUSEN; // enable USB suspend detection

		// Set default Endpoint Status
		MC.EndpointStates[0] = EUsbEndpointState_Idle; 
		MC.EndpointStates[1] = EUsbEndpointState_Halt;
		MC.EndpointStates[2] = EUsbEndpointState_Halt;
	}
	if (CurrentEvent.In & USB0DAT_IN1IE_EP0)                 
	{                      
		// Handle SETUP packet received or packet transmitted if Endpoint 0 is in transmit mode
	}
	if (CurrentEvent.In & USB0DAT_IN1IE_EP1)                 
	{                                
		// Handle In Packet sent, endpoint 1 fifo ready to get more
		MC.EndpointStates[1] = EUsbEndpointState_Idle;
	}
	if (CurrentEvent.Out & USB0DAT_OUT1IE_EP1)               
	{                                 
		// Handle Out packet received, take data off endpoint 2 fifo
	}
}

/////////////////////////////////
/////////////////////////////////
// MAIN

void main (void)
{
	U16 I;

    PCA0MD &= ~0x40;                    			// Disable Watchdog timer 
 	OSCICN = OSCICN_IOSCEN | OSCICN_IFCN1 | OSCICN_IFCN0;          // Configure internal oscillator

  	CLKMUL = CLKMUL_MULSEL_INTERNAL;   
   	CLKMUL = CLKMUL_MULSEL_INTERNAL | CLKMUL_MULEN;                       // Enable clock multiplier
	for(I = 0;I < 500;I++);            									 // Delay for clock multiplier to begin
   	CLKMUL = CLKMUL_MULSEL_INTERNAL | CLKMUL_MULEN | CLKMUL_MULINIT;      // Initialize the clock multiplier
   	while(!(CLKMUL & CLKMUL_MULRDY));   								// Wait for multiplier to lock

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
	USB0DAT = USB0DAT_CLKREC_CRE | USB0DAT_CLKREC_MUSTWRITE; // Enable clock recovery

	EIE1 |= 0x02;// Enable USB0 Interrupts

	while (USB0ADR & USB0ADR_BUSY);
	USB0ADR = USB0ADR_POWER;
	USB0DAT = (USB0DAT & ~USB0DAT_POWER_USBINH) | USB0DAT_POWER_SUSEN; // Activate transciever and enable suspend detection

   	EA = 1; // global interrupts enable 

	while (1)
	{
		// if (properly set up)
		//RSTSRC |= RSTSRC_USBRSF; // Enables Usb as a reset source. Will reset immediately if USB is not connected
	}
}
