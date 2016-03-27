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

typedef enum 
{
	EStringDesc_Manufacturer = 0x00,
	EStringDesc_Product		 = 0x01,
	EStringDesc_SerialNb	 = 0x02,
	EStringDesc_Config		 = 0x03
} EStringDesc;

/*
	const unsigned char StringDesc_Manufacturer [] = USB_STRING('G', 'u', 'g', 'l', 'i');
	const unsigned char StringDesc_Product      [] = USB_STRING('F', 'o', 'o', 't', 's', 'w', 'i', 't', 'c', 'h');
	const unsigned char StringDesc_SerialNb     [] = USB_STRING('S', 'N', '-', '4', '2');
	const unsigned char StringDesc_Config       [] = USB_STRING('C', 'o', 'n', 'f', '1');
//*/
const unsigned char StringDesc_Manufacturer [] = {12, 0x03, 'G', 0, 'u', 0, 'g', 0, 'l', 0, 'i', 0};
const unsigned char StringDesc_Product      [] = {22, 0x03, 'F', 0, 'o', 0, 'o', 0, 't', 0, 's', 0, 'w', 0, 'i', 0, 't', 0, 'c', 0, 'h', 0};
const unsigned char StringDesc_SerialNb     [] = {12, 0x03, 'S', 0, 'N', 0, '-', 0, '4', 0, '2', 0};
const unsigned char StringDesc_Config       [] = {12, 0x03, 'C', 0, 'o', 0, 'n', 0, 'f', 0, '1', 0};
//*/

SEG_CODE SUsbDescriptor_Device UsbDescriptor_Device =
{
	{
		sizeof(SUsbDescriptor_Device),		// Size of this Descriptor in Bytes
		EUsbDescriptorType_Device			// Descriptor Type (=1)
	},			  
	MUsbWord_Word(EUsbSpecVer_1d1),						   // USB Spec Release Number in BCD
	0x00,			 // Device Class Code
	0x00,			// Device Subclass Code
	0x00,			 // Device Protocol Code
	10,			  // Maximum Packet Size for EP0
	MUsbWord_Word(EUsbVendorId_Prototype),	 // Vendor ID
	MUsbWord_Bytes(00,01),					 // Product ID
	MUsbWord_Bytes(00,01),					   // Device Release Number
	EStringDesc_Manufacturer,
	EStringDesc_Product,		
	EStringDesc_SerialNb,		
	0x01		   // Number of possible Configurations
};

SEG_CODE SUsbDescriptor_Configuration UsbDescriptor_Configuration =
{
	{
		sizeof(SUsbDescriptor_Configuration),		
		EUsbDescriptorType_Configuration			
	},			  
   MUsbWord_Bytes(00,01), // Total Length of Data for this Conf
   0x02,	// InterfacesCount
   0x01,	// Designator Value for *this* Configuration
   EStringDesc_Config,	// ConfigurationStringIndex;
   0x00,	// Configuration Characteristics (see below)
   0x32 	// (MaxPower*2)mA is the power allowed
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
	U16 DataToSend_Size;			// Size that the software wants to send
	U16 DataToSend_Size_Host;		// Size that the host wants. If this is lower than DataToSend_Size, we will have to wait another setup packet and split
	U16 DataToSend_CurrentOffset;	 
	
	U8* DataToRecieve_Ptr;	 
	U16 DataToRecieve_Size;			// Size that the software expect to recieve
	U16 DataToRecieve_Size_Host;	// Size that the host is sending in this transaction.
	U16 DataToRecieve_CurrentOffset;			  
	
} SMainContext;

void MC_SetDefault(SMainContext* _MC) {
	_MC->EndpointStates[0] = EUsbEndpointState_Idle;
	_MC->EndpointStates[1] = EUsbEndpointState_Halt;
	_MC->EndpointStates[2] = EUsbEndpointState_Halt;
	
	_MC->Usb_FADDR = 0x00;
	
	_MC->DataToSend_Ptr = 0x00;
	_MC->DataToSend_Size = 0x00;
	_MC->DataToSend_Size_Host = 0x00;
	_MC->DataToSend_CurrentOffset = 0x00;
	
	_MC->DataToRecieve_Ptr = 0x00;
	_MC->DataToRecieve_Size = 0x00;
	_MC->DataToRecieve_Size_Host = 0x00;
	_MC->DataToRecieve_CurrentOffset = 0x00;
}

SEG_NEAR volatile SMainContext MC;

/////////////////////////////////
/////////////////////////////////
// INTERRUPTS

typedef enum 
{
	EUsbEventType_None,
	EUsbEventType_Resume,
	EUsbEventType_Suspend,
	EUsbEventType_SetupEnd,
	EUsbEventType_Reset,
	EUsbEventType_Stall,
	EUsbEventType_EP0_In_Before,
	EUsbEventType_EP0_In_SetupEnd,
	EUsbEventType_EP0_In_OutgoingPacketTransmitted,
	EUsbEventType_EP0_In_IncomingPacketRecieved,
	EUsbEventType_EP0_In_After,
	
	EUsbEventType_EP1_In,
	EUsbEventType_EP1_Out,
} EUsbEventType;

typedef struct
{
	EUsbEventType Type;
	
} SUsbEvent;

void Usb_ISR_HandleEvent(SUsbEvent* _Event)
{	
	U8 ControlRegister, NewControlRegister;
	SUsbPacket_Setup SetupPacket;
	U8 SendOrRecieveIsFinished;
	U8 Setup_RequestType_Type, Setup_RequestType_Direction, Setup_RequestType_Recipient;
	U8 bob;
	
	switch( _Event->Type )
	{
		case EUsbEventType_Resume:		break;
		case EUsbEventType_Suspend:		break;
		
		case EUsbEventType_SetupEnd:	
		USB_WriteRegister (USB0ADR_E0CSR, USB0ADR_E0CSR_DATAEND);
		USB_WriteRegister (USB0ADR_E0CSR, USB0ADR_E0CSR_SSUEND);
		MC.EndpointStates[0] = EUsbEndpointState_Idle;
		break;
		
		case EUsbEventType_Reset:
		USB_WriteRegister( USB0ADR_POWER, USB0DAT_POWER_SUSEN); // Activate transciever and enable suspend detection
		MC_SetDefault(&MC);
		break;
		
		case EUsbEventType_EP0_In_Before:
		if (MC.EndpointStates[0] == EUsbEndpointState_Address)
		{
			USB_WriteRegister (USB0ADR_FADDR, MC.Usb_FADDR | USB0ADR_FADDR_BUSY );
			MC.EndpointStates[0] = EUsbEndpointState_Idle;
		} 
		else if (MC.EndpointStates[0] == EUsbEndpointState_Stall )
		{
			USB_WriteRegister (USB0ADR_E0CSR, 0);
			MC.EndpointStates[0] = EUsbEndpointState_Idle;
		}
		break;
		
		case EUsbEventType_EP0_In_IncomingPacketRecieved:
		if (MC.EndpointStates[0] == EUsbEndpointState_Recieve)	 
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
				MC.DataToRecieve_CurrentOffset = MC.DataToRecieve_Size;
				SendOrRecieveIsFinished = True;
			}
			if (MC.DataToRecieve_CurrentOffset == MC.DataToRecieve_Size_Host )
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
		else if (MC.EndpointStates[0] == EUsbEndpointState_Idle)
		{
			USB_ReadEndpointFifo(USB0ADR_FIFO_EP0, (U8*)&SetupPacket, sizeof(SetupPacket) );
			
			Setup_RequestType_Type		= (SetupPacket.RequestType & EUsbPacket_Setup_RequestType_TypeMask);
			Setup_RequestType_Direction = (SetupPacket.RequestType & EUsbPacket_Setup_RequestType_DirMask);
			Setup_RequestType_Recipient = (SetupPacket.RequestType & EUsbPacket_Setup_RequestType_RecipientMask);
			
			if (Setup_RequestType_Type == EUsbPacket_Setup_RequestType_Type_Standard) {
				// standard requests
				switch (SetupPacket.Request)	  
				{	
					case EUsbPacket_Setup_Request_GetStatus:			  
					{
						if ( USB_Word_IsNotEqual(SetupPacket.Value, 0x00, 0x00 ) ||
						USB_Word_IsNotEqual(SetupPacket.Value, 0x00, 0x02 ) ||
						Setup_RequestType_Direction == EUsbPacket_Setup_RequestType_Dir_In ) 
						{
							USB_WriteRegister(USB0ADR_E0CSR, USB0ADR_E0CSR_SDSTL);
							MC.EndpointStates[0] = EUsbEndpointState_Stall;
						}						
						else 
						{
							switch(Setup_RequestType_Recipient)	   
							{
								case EUsbPacket_Setup_RequestType_Recipient_Device:
								case EUsbPacket_Setup_RequestType_Recipient_Interface:
								case EUsbPacket_Setup_RequestType_Recipient_Endpoint:
								break;
								default:
								USB_WriteRegister(USB0ADR_E0CSR, USB0ADR_E0CSR_SDSTL);
								MC.EndpointStates[0] = EUsbEndpointState_Stall;
								break;
							}
						}
						break;
					}
					break;
					case EUsbPacket_Setup_Request_ClearFeature:			  bob++; break;
					case EUsbPacket_Setup_Request_SetFeature:			  bob++; break;
					case EUsbPacket_Setup_Request_SetAddress:			  bob++; break;
					case EUsbPacket_Setup_Request_GetDescriptor:		  
					{
						switch(SetupPacket.Value.MostSignificantByte) // MSB =Descriptor type
						{
							case EUsbDescriptorType_Device:
							if(SetupPacket.Value.LeastSignificantByte > 0)
							{
								USB_WriteRegister(USB0ADR_E0CSR, USB0ADR_E0CSR_SDSTL);
								MC.EndpointStates[0] = EUsbEndpointState_Stall;
							}
							else
							{
								MC.DataToSend_Ptr = (U8*)&UsbDescriptor_Device;
								MC.DataToSend_Size = sizeof(UsbDescriptor_Device);
								MC.DataToSend_Size_Host = USB_Word_GetValue(SetupPacket.Length);
								MC.DataToSend_CurrentOffset = 0;
								MC.EndpointStates[0] = EUsbEndpointState_Transmit;	
								USB_WriteRegister(USB0ADR_E0CSR, USB0ADR_E0CSR_SOPRDY);
							}
							break;

							case EUsbDescriptorType_Configuration:
							if(SetupPacket.Value.LeastSignificantByte > 0)
							{
								USB_WriteRegister(USB0ADR_E0CSR, USB0ADR_E0CSR_SDSTL);
								MC.EndpointStates[0] = EUsbEndpointState_Stall;
							}
							else
							{
								MC.DataToSend_Ptr = (U8*)&UsbDescriptor_Configuration;
								MC.DataToSend_Size = sizeof(UsbDescriptor_Configuration);
								MC.DataToSend_Size_Host = USB_Word_GetValue(SetupPacket.Length);
								MC.DataToSend_CurrentOffset = 0;
								MC.EndpointStates[0] = EUsbEndpointState_Transmit;	
								USB_WriteRegister(USB0ADR_E0CSR, USB0ADR_E0CSR_SOPRDY);
							}
							break;

							case EUsbDescriptorType_String:
							switch (SetupPacket.Value.LeastSignificantByte)
							{
								case EStringDesc_Manufacturer:    MC.DataToSend_Ptr = (U8*)&StringDesc_Manufacturer; break;
								case EStringDesc_Product:         MC.DataToSend_Ptr = (U8*)&StringDesc_Product; break;
								case EStringDesc_SerialNb:        MC.DataToSend_Ptr = (U8*)&StringDesc_SerialNb; break;
								case EStringDesc_Config:          MC.DataToSend_Ptr = (U8*)&StringDesc_Config; break;
								default:
								USB_WriteRegister(USB0ADR_E0CSR, USB0ADR_E0CSR_SDSTL);
								MC.EndpointStates[0] = EUsbEndpointState_Stall;
								break;
							}
							if( MC.EndpointStates[0] != EUsbEndpointState_Stall )
							{
								MC.DataToSend_Size = *MC.DataToSend_Ptr;
								MC.DataToSend_Size_Host = USB_Word_GetValue(SetupPacket.Length);
								MC.DataToSend_CurrentOffset = 0;
								MC.EndpointStates[0] = EUsbEndpointState_Transmit;	
								USB_WriteRegister(USB0ADR_E0CSR, USB0ADR_E0CSR_SOPRDY);
							}
							break;

							case EUsbDescriptorType_Interface:
							case EUsbDescriptorType_Endpoint:
							default:
							USB_WriteRegister(USB0ADR_E0CSR, USB0ADR_E0CSR_SDSTL);
							MC.EndpointStates[0] = EUsbEndpointState_Stall;
							break;
						}
						break;
					}
					case EUsbPacket_Setup_Request_GetConfig:			  bob++; break;
					case EUsbPacket_Setup_Request_SetConfig:			  bob++; break;
					case EUsbPacket_Setup_Request_GetInterface:			  bob++; break;
					case EUsbPacket_Setup_Request_SetInterface:	 
					
					break;
					default:
					USB_WriteRegister(USB0ADR_E0CSR, USB0ADR_E0CSR_SDSTL);
					MC.EndpointStates[0] = EUsbEndpointState_Stall;				
					break;
				}
			} 
			else if( Setup_RequestType_Type == EUsbPacket_Setup_RequestType_Type_Class ) 
			{
				// HID class-specific requests
				switch (SetupPacket.Request) 
				{
					case EUsbPacket_Setup_Request_GetReport:			  bob++; break;
					case EUsbPacket_Setup_Request_SetReport:			  bob++; break;
					case EUsbPacket_Setup_Request_GetIdle:				  bob++; break;
					case EUsbPacket_Setup_Request_SetIdle:				  bob++; break;
					case EUsbPacket_Setup_Request_GetProtocol:			  bob++; break;
					case EUsbPacket_Setup_Request_SetProtocol:			  
					
					break;
					
					default:
					USB_WriteRegister(USB0ADR_E0CSR, USB0ADR_E0CSR_SDSTL);
					MC.EndpointStates[0] = EUsbEndpointState_Stall;							
					break;				  
					
				} 
			}  
		}
		break;
		
		case EUsbEventType_EP0_In_After:
		// Current event has been handled,
		// check if we can start transmitting
		case EUsbEventType_EP0_In_OutgoingPacketTransmitted:
		USB_WriteRegister( USB0ADR_INDEX, 0);  // Select Endpoint Zero
		USB_ReadRegister(ControlRegister, USB0ADR_E0CSR);  // Read the control register 
		
		if (MC.EndpointStates[0] == EUsbEndpointState_Transmit &&
		(ControlRegister & USB0ADR_E0CSR_SUEND) == 0 &&
		(ControlRegister & USB0ADR_E0CSR_INPRDY) == 0 &&
		(ControlRegister & USB0ADR_E0CSR_OPRDY) == 0 ) // We're not doing anything else
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
				MC.DataToSend_CurrentOffset = MC.DataToSend_Size;	  
				SendOrRecieveIsFinished = True;
			}
			if (MC.DataToSend_CurrentOffset == MC.DataToSend_Size_Host)
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
		break;
		default:
		USB_WriteRegister(USB0ADR_E0CSR, USB0ADR_E0CSR_SDSTL);
		MC.EndpointStates[0] = EUsbEndpointState_Stall;
		break;
	}
}

INTERRUPT(Usb_ISR, INTERRUPT_USB0)	
{
	SUsbEvent	EventQueue[10];
	U8			EventQueue_Size;
	U8			I;
	U8			IntFlags_Common, IntFlags_In, IntFlags_Out;
	U8			ControlRegister;
	
	USB_ReadRegister(IntFlags_Common, USB0ADR_CMINT);
	USB_ReadRegister(IntFlags_In,	  USB0ADR_IN1INT);
	USB_ReadRegister(IntFlags_Out,	  USB0ADR_OUT1INT);
	
	EventQueue_Size = 0;
	
	if (IntFlags_Common & USB0ADR_CMINT_RSUINT)			 
	{
		EventQueue[EventQueue_Size].Type = EUsbEventType_Resume;
		EventQueue_Size++;
	}
	if (IntFlags_Common & USB0ADR_CMINT_SUSINT)			 
	{
		EventQueue[EventQueue_Size].Type = EUsbEventType_Suspend;
		EventQueue_Size++;
	}
	if (IntFlags_Common & USB0ADR_CMINT_RSTINT)			 
	{
		EventQueue[EventQueue_Size].Type = EUsbEventType_Reset;
		EventQueue_Size++;
	}
	if (IntFlags_In & USB0DAT_IN1IE_EP0)				 
	{					   
		// Handle packet received
		USB_WriteRegister( USB0ADR_INDEX, 0);  // Select Endpoint Zero
		USB_ReadRegister(ControlRegister, USB0ADR_E0CSR);  // Read the control register		
		
		EventQueue[EventQueue_Size].Type = EUsbEventType_EP0_In_Before;
		EventQueue_Size++;
		
		if (ControlRegister & USB0ADR_E0CSR_SUEND)
		{
			// Handle setup end
			EventQueue[EventQueue_Size].Type = EUsbEventType_EP0_In_SetupEnd;
			EventQueue_Size++;
		}
		if (ControlRegister & USB0ADR_E0CSR_INPRDY)
		{
			// the device has send a Packet to the host 
			EventQueue[EventQueue_Size].Type = EUsbEventType_EP0_In_OutgoingPacketTransmitted;
			EventQueue_Size++;
		} 
		if (ControlRegister & USB0ADR_E0CSR_OPRDY)		  
		{
			// the host has send a Packet to the device			
			EventQueue[EventQueue_Size].Type = EUsbEventType_EP0_In_IncomingPacketRecieved;
			EventQueue_Size++;
		}
		
		EventQueue[EventQueue_Size].Type = EUsbEventType_EP0_In_After;
		EventQueue_Size++;		
	}
	if (IntFlags_In & USB0DAT_IN1IE_EP1)				 
	{	  
		EventQueue[EventQueue_Size].Type = EUsbEventType_EP1_In;
		EventQueue_Size++;
	}
	if (IntFlags_Out & USB0DAT_OUT1IE_EP1)				 
	{								  
		EventQueue[EventQueue_Size].Type = EUsbEventType_EP1_Out;
		EventQueue_Size++;
	}
	
	for( I=0; I<EventQueue_Size; I++) {
		Usb_ISR_HandleEvent(&EventQueue[I]);
		//if (stalled)
		// then break
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
		OSCICN = OSCICN_IOSCEN | OSCICN_DIVBY_1;		  // Configure internal oscillator
		for(I = 0;I < 500;I++);							  // Delay for Oscillator to begin
		while(!(OSCICN & OSCICN_IFRDY));				  // Wait for Oscillator to be ready
		
		CLKMUL = CLKMUL_MULSEL_INTERNAL | CLKMUL_MULEN;		   // Enable clock multiplier 
		
		CLKMUL |= CLKMUL_MULINIT;								// Start the multiplier initialization this can not be merged with previous step
		for(I = 0;I < 500;I++);												 // Delay for clock multiplier to begin
		while(!(CLKMUL & CLKMUL_MULRDY));									// Wait for multiplier to lock
		
		CLKSEL = CLKSEL_CLKSL_INTERNAL |CLKSEL_USBCLK_4XMUL;			// Select system clock and USB clock
		
	#else
		OSCICN = OSCICN_IOSCEN | OSCICN_DIVBY_1;
		CLKSEL	= CLKSEL_CLKSL_INTERNAL | CLKSEL_USBCLK_INTERNALDIV2;  
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
	
	
	
	XBR0 = 0x00;						// No digital peripherals selected
	XBR1 = 0x40;						// Enable crossbar and weak pull-ups
	P2MDOUT |= 0x04;					// Enable LED as a push-pull output
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
		