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

#define MUsb_HighSpeed MEnabled
//#define MUsb_HighSpeed MDisabled

#if MIsEnabled(MUsb_HighSpeed) 
#define MUsb_Endpoint0_PacketSize 0x40
#else
#define MUsb_Endpoint0_PacketSize 0x08
#endif

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
/*
const unsigned char String1Desc [] = USB_STRING('G', 'u', 'g', 'l', 'i');
const unsigned char String2Desc [] = USB_STRING('F', 'o', 'o', 't', 's', 'w', 'i', 't', 'c', 'h');
const unsigned char String3Desc [] = USB_STRING('S', 'N', '-', '4', '2');
//*/
const unsigned char String1Desc [] = {12, 0x03, 'G', 0, 'u', 0, 'g', 0, 'l', 0, 'i', 0};
const unsigned char String2Desc [] = {22, 0x03, 'F', 0, 'o', 0, 'o', 0, 't', 0, 's', 0, 'w', 0, 'i', 0, 't', 0, 'c', 0, 'h', 0};
const unsigned char String3Desc [] = {12, 0x03, 'S', 0, 'N', 0, '-', 0, '4', 0, '2', 0};
//*/
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
	EUsbEndpointState_Address		, // change FADDR during next status phase
	EUsbEndpointState_HidGetReport	,
	EUsbEndpointState_HidSetReport		
} EUsbEndpointState;

typedef struct {
	EUsbEndpointState EndpointStates[3];

	U8 Usb_FADDR;
               
	U8* DataToSend_Ptr;   
	U16 DataToSend_Size; 
	U16 DataToSend_CurrentOffset;    

	U8* DataToRecieve_Ptr;   
	U16 DataToRecieve_Size; 
	U16 DataToRecieve_CurrentOffset;              

} SMainContext;

void MC_SetDefault(SMainContext* _MC) {
	_MC->EndpointStates[0] = EUsbEndpointState_Idle;
	_MC->EndpointStates[1] = EUsbEndpointState_Idle;
	_MC->EndpointStates[2] = EUsbEndpointState_Idle;

	_MC->Usb_FADDR = 0x00;

	_MC->DataToSend_Ptr = 0x00;
	_MC->DataToSend_Size = 0x00;
	_MC->DataToSend_CurrentOffset = 0x00;

	_MC->DataToRecieve_Ptr = 0x00;
	_MC->DataToRecieve_Size = 0x00;
	_MC->DataToRecieve_CurrentOffset = 0x00;
}

SEG_NEAR volatile SMainContext MC;

/////////////////////////////////
/////////////////////////////////
// INTERRUPTS

typedef struct {
	U8 Common;
	U8 In;
	U8 Out;
} SEvent;


INTERRUPT(Usb_ISR, INTERRUPT_USB0)  
{
	SEvent CurrentEvent;
	U8 ControlRegister, NewControlRegister;
	U8 SendStall;
	SUsbPacket_Setup SetupPacket;
	U16 RequestedLength;
	U8 SendOrRecieveIsFinished;
	U8 Setup_RequestType_Type, Setup_RequestType_Direction, Setup_RequestType_Recipient;

	volatile U8 bob = 0;
	
	USB_ReadRegister(CurrentEvent.Common, USB0ADR_CMINT);
	USB_ReadRegister(CurrentEvent.In,     USB0ADR_IN1INT);
	USB_ReadRegister(CurrentEvent.Out,    USB0ADR_OUT1INT);
	RequestedLength      = 0;
	SendStall			 = False;

	if (CurrentEvent.Common & USB0ADR_CMINT_RSUINT)          
    {
    	// Handle Resume interrupt	
		bob++;	
    }
	if (CurrentEvent.Common & USB0ADR_CMINT_SUSINT)          
	{
		// Handle Suspend interrupt
		bob++;
	}
	if (CurrentEvent.Common & USB0ADR_CMINT_RSTINT)          
	{
		// Handle Reset interrupt	
		USB_WriteRegister( USB0ADR_POWER, USB0DAT_POWER_SUSEN); // enable USB suspend detection

		// Set default Endpoint Status
		MC.EndpointStates[0] = EUsbEndpointState_Idle; 
		MC.EndpointStates[1] = EUsbEndpointState_Halt;
		MC.EndpointStates[2] = EUsbEndpointState_Halt;
	}
	if (CurrentEvent.In & USB0DAT_IN1IE_EP0)                 
	{                      
		// Handle SETUP packet received or packet transmitted if Endpoint 0 is in transmit mode
		
		USB_WriteRegister( USB0ADR_INDEX, 0);  // Select Endpoint Zero
		USB_ReadRegister(ControlRegister, USB0ADR_E0CSR);  // Read the control register

	   if (MC.EndpointStates[0] == EUsbEndpointState_Address )
	   {
	      USB_WriteRegister (USB0ADR_FADDR, MC.Usb_FADDR );
		  if (ControlRegister & USB0ADR_E0CSR_STSTL) // should always be the case, or never ?
		  {
	     	 USB_WriteRegister (USB0ADR_E0CSR, 0);
		  }
	      MC.EndpointStates[0] = EUsbEndpointState_Idle;
	   } 
	   else 
	   {
	       // Common business
		   // Handle stalls
		   if (ControlRegister & USB0ADR_E0CSR_STSTL)   
		   {                                   
		      USB_WriteRegister (USB0ADR_E0CSR, 0);
	    	  MC.EndpointStates[0] = EUsbEndpointState_Idle;
     		  return;		      
		   }

		   // Handle suspend end
		   if (ControlRegister & USB0ADR_E0CSR_SUEND)
		   {
		      USB_WriteRegister (USB0ADR_E0CSR, USB0ADR_E0CSR_DATAEND);
	    	  USB_WriteRegister (USB0ADR_E0CSR, USB0ADR_E0CSR_SSUEND);
	    	  MC.EndpointStates[0] = EUsbEndpointState_Idle;
		   }

		   // State machine
		   if (MC.EndpointStates[0] == EUsbEndpointState_Idle)        // If Endpoint 0 is in idle mode
		   {
		      if (ControlRegister & USB0ADR_E0CSR_OPRDY)        // Make sure that host has an Out Packet
		      {     
			 	  USB_ReadEndpointFifo(USB0ADR_FIFO_EP0, (U8*)&SetupPacket, sizeof(SetupPacket) );
					
				  RequestedLength  = USB_GetWordValue(SetupPacket.Length);


				  Setup_RequestType_Type      = (SetupPacket.RequestType & EUsbPacket_Setup_RequestType_TypeMask);
				  Setup_RequestType_Direction = (SetupPacket.RequestType & EUsbPacket_Setup_RequestType_DirMask);
				  Setup_RequestType_Recipient = (SetupPacket.RequestType & EUsbPacket_Setup_RequestType_RecipientMask);

			      if (Setup_RequestType_Type == EUsbPacket_Setup_RequestType_Type_Standard) {
					 // standard requests
			         switch (SetupPacket.Request)      
			         {   
						case EUsbPacket_Setup_Request_GetStatus:              
						{
							if ( USB_GetWordValue(SetupPacket.Value) != 0 ) {
								SendStall = True;
							}
							else if ( USB_GetWordValue(SetupPacket.Length) != 2 ) {
								SendStall = True;
							}
							else if ( Setup_RequestType_Type == EUsbPacket_Setup_RequestType_Dir_In ) {
								SendStall = True;
							}
							else {
								switch(Setup_RequestType_Recipient)    
	   							{
								case EUsbPacket_Setup_RequestType_Recipient_Device:
								case EUsbPacket_Setup_RequestType_Recipient_Interface:
								case EUsbPacket_Setup_RequestType_Recipient_Endpoint:
									break;
								default:
									SendStall = True;
									break;
								}
							}
							break;
						}
						break;
						case EUsbPacket_Setup_Request_ClearFeature:           // bob++; break;
						case EUsbPacket_Setup_Request_SetFeature:			  // bob++; break;
						case EUsbPacket_Setup_Request_SetAddress:             // bob++; break;
						case EUsbPacket_Setup_Request_GetDescriptor:          bob = UsbDescriptor.DeviceClass;// bob++; break;
						case EUsbPacket_Setup_Request_GetConfig:              // bob++; break;
						case EUsbPacket_Setup_Request_SetConfig:              // bob++; break;
						case EUsbPacket_Setup_Request_GetInterface:           // bob++; break;
						case EUsbPacket_Setup_Request_SetInterface:  
						         bob++; 
								 break;
						default:	
							SendStall = True;					
						break;
					      
			         }
				 } 
				 else if( Setup_RequestType_Type == EUsbPacket_Setup_RequestType_Type_Class ) {
					 // HID class-specific requests
			         switch (SetupPacket.Request) {
						case EUsbPacket_Setup_Request_GetReport:              // bob++; break;
						case EUsbPacket_Setup_Request_SetReport:              // bob++; break;
						case EUsbPacket_Setup_Request_GetIdle:			      // bob++; break;
						case EUsbPacket_Setup_Request_SetIdle:                // bob++; break;
						case EUsbPacket_Setup_Request_GetProtocol:            // bob++; break;
						case EUsbPacket_Setup_Request_SetProtocol:            
						    bob++; 
							break;
						default:
							SendStall = True;								
						break;                
			         }
			      } 
				  
				  if(SendStall) {									
						USB_WriteRegister(USB0ADR_E0CSR, USB0ADR_E0CSR_SDSTL);
						MC.EndpointStates[0] = EUsbEndpointState_Stall;
				  } else {

	    	 		 USB_WriteRegister (USB0ADR_E0CSR, USB0ADR_E0CSR_SOPRDY);
				  }
		      }
		   }

		   if (MC.EndpointStates[0] == EUsbEndpointState_Transmit) 
		   {
		      if (!(ControlRegister & USB0ADR_E0CSR_INPRDY) )   // Don't overwrite last packet
		      {
			  		// Read the control register again ?			  
					// ControlRegister2 = USB_ReadRegister(USB0ADR_E0CSR);  

		         // Check to see if SETUP End or Out Packet received, if so do not put
		         // any new data on FIFO
		         if ((!(ControlRegister & USB0ADR_E0CSR_SUEND)) || (!(ControlRegister & USB0ADR_E0CSR_OPRDY)))
		         {
					 SendOrRecieveIsFinished = False;

		            NewControlRegister = USB0ADR_E0CSR_INPRDY;
		            if ( MUsb_Endpoint0_PacketSize <= MC.DataToSend_Size - MC.DataToSend_CurrentOffset )
		            {
		               // Break Data into multiple packets if larger than Max Packet
		               USB_WriteEndpointFifo(USB0ADR_FIFO_EP0, MC.DataToSend_Ptr + MC.DataToSend_CurrentOffset, MUsb_Endpoint0_PacketSize ); 
						MC.DataToSend_CurrentOffset += MUsb_Endpoint0_PacketSize;              
		            }
		            else
		            {
		               USB_WriteEndpointFifo(USB0ADR_FIFO_EP0, MC.DataToSend_Ptr + MC.DataToSend_CurrentOffset, MC.DataToSend_Size - MC.DataToSend_CurrentOffset ); 
					   SendOrRecieveIsFinished = True;
		            }
		            if (MC.DataToSend_CurrentOffset == RequestedLength)
		            {
		               // This case exists when the host requests an even multiple of
		               // your endpoint zero max packet size, and you need to exit
		               // transmit mode without sending a zero length packet
						 SendOrRecieveIsFinished = True;
		            }
					if( SendOrRecieveIsFinished ) {
		               NewControlRegister |= USB0ADR_E0CSR_DATAEND; // Add Data End bit to mask
		               MC.EndpointStates[0] = EUsbEndpointState_Idle; // Return EP 0 to idle state

					}
		            USB_WriteRegister(USB0ADR_E0CSR, NewControlRegister);
		         }
		      }
		   }

		   if (MC.EndpointStates[0] == EUsbEndpointState_Recieve)   
		   {
	  	  	  // Read the control register again ?			  
		  	  // ControlRegister2 = USB_ReadRegister(USB0ADR_E0CSR);  
		      if (ControlRegister & USB0ADR_E0CSR_OPRDY)        // packet was received
		      {
				 SendOrRecieveIsFinished = False;
		         NewControlRegister = USB0ADR_E0CSR_SOPRDY;
		         if (MUsb_Endpoint0_PacketSize <= MC.DataToRecieve_Size - MC.DataToRecieve_CurrentOffset )
		         {
		            USB_ReadEndpointFifo(USB0ADR_FIFO_EP0, MC.DataToRecieve_Ptr + MC.DataToRecieve_CurrentOffset, MUsb_Endpoint0_PacketSize );
					MC.DataToRecieve_CurrentOffset += MUsb_Endpoint0_PacketSize;
		         }
		         else
		         {
		            USB_ReadEndpointFifo(USB0ADR_FIFO_EP0, MC.DataToRecieve_Ptr + MC.DataToRecieve_CurrentOffset, MC.DataToRecieve_Size - MC.DataToRecieve_CurrentOffset );
		            SendOrRecieveIsFinished = True;
		         }
		         if (MC.DataToRecieve_CurrentOffset == RequestedLength )
		         {
					 SendOrRecieveIsFinished = True;
		         }
				 if( SendOrRecieveIsFinished )
				 {
		             NewControlRegister |= USB0ADR_E0CSR_DATAEND; // Add Data End bit to mask
		             MC.EndpointStates[0] = EUsbEndpointState_Idle; // Return EP 0 to idle state
				 }
		         USB_WriteRegister(USB0ADR_E0CSR, NewControlRegister);				
		      }
		   }
	   }
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
	volatile U16 I;
	U16 FrameCyclicCounterHigh, FrameCyclicCounterLow;

    // Disable Watchdog timer
    PCA0MD &= ~0x40;                    			 

    // Setup clock
	#if MIsEnabled(MUsb_HighSpeed) 
	 	OSCICN = OSCICN_IOSCEN | OSCICN_DIVBY_1;          // Configure internal oscillator
		for(I = 0;I < 500;I++);            				  // Delay for Oscillator to begin
		while(!(OSCICN & OSCICN_IFRDY));                  // Wait for Oscillator to be ready

	   	CLKMUL = CLKMUL_MULSEL_INTERNAL | CLKMUL_MULEN;        // Enable clock multiplier 

		CLKMUL |= CLKMUL_MULINIT;								// Start the multiplier initialization this can not be merged with previous step
		for(I = 0;I < 500;I++);            									 // Delay for clock multiplier to begin
	   	while(!(CLKMUL & CLKMUL_MULRDY));   								// Wait for multiplier to lock

	  	CLKSEL = CLKSEL_CLKSL_INTERNAL |CLKSEL_USBCLK_4XMUL;            // Select system clock and USB clock
 
    #else
	   OSCICN = OSCICN_IOSCEN | OSCICN_DIVBY_1;
	   CLKSEL  = CLKSEL_CLKSL_INTERNAL | CLKSEL_USBCLK_INTERNALDIV2;  
	#endif

    // Setup USB
	USB_WriteRegister(USB0ADR_POWER, USB0DAT_POWER_USBRST); // Request Reset
    USB_WriteRegister(USB0ADR_IN1IE, USB0DAT_IN1IE_EP0 | USB0DAT_IN1IE_EP1 | USB0DAT_IN1IE_EP2); // Enable Endpoint 0-2 in interrupts
    USB_WriteRegister(USB0ADR_OUT1IE, USB0DAT_OUT1IE_EP1 | USB0DAT_OUT1IE_EP2); // Enable Endpoint 1-2 out interrupts
	USB_WriteRegister(USB0ADR_CMIE, USB0DAT_CMIE_RSTINTE | USB0DAT_CMIE_RSUINTE | USB0DAT_CMIE_SUSINTE);  // Enable Reset, Resume, and Suspend interrupts

	#if MIsEnabled(MUsb_HighSpeed) 
		USB0XCN = USB0XCN_PREN | USB0XCN_PHYEN | USB0XCN_SPEED; // Activate Usb transciever at Full speed
    #else
		USB0XCN = USB0XCN_PREN | USB0XCN_PHYEN; // Activate Usb transciever at Low speed
	#endif
	USB_WriteRegister(USB0ADR_CLKREC, USB0DAT_CLKREC_CRE | USB0DAT_CLKREC_MUSTWRITE); // Enable clock recovery


	
	XBR0 = 0x00;                        // No digital peripherals selected
	XBR1 = 0x40;                        // Enable crossbar and weak pull-ups
	P2MDOUT |= 0x04;                    // Enable LED as a push-pull output
	RSTSRC = 0;							// Disable all reset sources

	MC_SetDefault(&MC);
	FrameCyclicCounterLow = 0;
	FrameCyclicCounterHigh = 0;

	EIE1 |= 0x02; // Enable USB0 Interrupts
	USB_WriteRegister(USB0ADR_POWER, USB0DAT_POWER_SUSEN); // Activate transciever and enable suspend detection
   	IE = (IE | 0x80); // global interrupts enable 
	while (1)
	{
   		// IE = (IE & (~0x80)) ; // Disable interrupts
		// if (properly set up)
		//RSTSRC |= RSTSRC_USBRSF; // Enables Usb as a reset source. Will reset immediately if USB is not connected
		
   		// IE |= 0x80 ; // global interrupts enable 

		// blink leds, to show when running
		P2 = (FrameCyclicCounterHigh % 0x0008 < 0x0004) ? 0x00 : 0x04;
		FrameCyclicCounterLow++;
		if(!FrameCyclicCounterLow) FrameCyclicCounterHigh++;
	}
}
