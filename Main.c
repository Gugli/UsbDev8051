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
// Config

typedef enum 
{
	EMouseButtonsMask_LeftClick			= 0x01,
	EMouseButtonsMask_RightClick		= 0x02,
	EMouseButtonsMask_MiddleClick		= 0x04,
} EMouseButtonsMask;
typedef enum 
{
	EKeboardModifiersMask_LeftCtrl			= 0x01,
	EKeboardModifiersMask_RightCtrl			= 0x02,
	EKeboardModifiersMask_Shift				= 0x04,
} EKeboardModifiersMask;


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

#define MUsb_Endpoint1_PacketSize 10
#define MUsb_Endpoint2_PacketSize 10

typedef enum 
{
	EStringDesc_None              = 0x00,
	EStringDesc_Manufacturer      = 0x01,
	EStringDesc_Product		      = 0x02,
	EStringDesc_SerialNb	      = 0x03,
	EStringDesc_Config		      = 0x04,
	EStringDesc_Interface         = 0x05
} EStringDesc;

/*
SEG_CODE const unsigned char UsbDescriptor_String_Manufacturer            [] = USB_STRING('G','u','g','l','i');
SEG_CODE const unsigned char UsbDescriptor_String_Product                 [] = USB_STRING('F','o','o','t','s','w','i','t','c','h');
SEG_CODE const unsigned char UsbDescriptor_String_SerialNb                [] = USB_STRING('S','N','-','4','4');
SEG_CODE const unsigned char UsbDescriptor_String_Config                  [] = USB_STRING('C','o','n','f','1');
SEG_CODE const unsigned char UsbDescriptor_String_Interface               [] = USB_STRING('I','n','t','e','r','f','a','c','e');
//*/
SEG_CODE const unsigned char UsbDescriptor_String_AvailableLanguages      [] = { 4, 0x03, 0x09, 0x04};
SEG_CODE const unsigned char UsbDescriptor_String_Manufacturer            [] = {12, 0x03, 'G', 0, 'u', 0, 'g', 0, 'l', 0, 'i', 0};
SEG_CODE const unsigned char UsbDescriptor_String_Product                 [] = {22, 0x03, 'F', 0, 'o', 0, 'o', 0, 't', 0, 's', 0, 'w', 0, 'i', 0, 't', 0, 'c', 0, 'h', 0};
SEG_CODE const unsigned char UsbDescriptor_String_SerialNb                [] = {12, 0x03, 'S', 0, 'N', 0, '-', 0, '4', 0, '4', 0};
SEG_CODE const unsigned char UsbDescriptor_String_Config                  [] = {12, 0x03, 'C', 0, 'o', 0, 'n', 0, 'f', 0, '1', 0};
SEG_CODE const unsigned char UsbDescriptor_String_Interface               [] = {20, 0x03, 'I', 0, 'n', 0, 't', 0, 'e', 0, 'r', 0, 'f', 0, 'a', 0, 'c', 0, 'e', 0};
//*/

SEG_CODE const unsigned char UsbDescriptor_Report_Mouse [50] = 
{
    0x05, 0x01,                    // USAGE_PAGE (Generic Desktop)
    0x09, 0x02,                    // USAGE (Mouse)
    0xa1, 0x01,                    // COLLECTION (Application)
    0x09, 0x01,                    //   USAGE (Pointer)
    0xa1, 0x00,                    //   COLLECTION (Physical)
    0x05, 0x09,                    //     USAGE_PAGE (Button)
    0x19, 0x01,                    //     USAGE_MINIMUM (Button 1)
    0x29, 0x03,                    //     USAGE_MAXIMUM (Button 3)
    0x15, 0x00,                    //     LOGICAL_MINIMUM (0)
    0x25, 0x01,                    //     LOGICAL_MAXIMUM (1)
    0x95, 0x03,                    //     REPORT_COUNT (3)
    0x75, 0x01,                    //     REPORT_SIZE (1)
    0x81, 0x02,                    //     INPUT (Data,Var,Abs)
    0x95, 0x01,                    //     REPORT_COUNT (1)
    0x75, 0x05,                    //     REPORT_SIZE (5)
    0x81, 0x03,                    //     INPUT (Cnst,Var,Abs)
    0x05, 0x01,                    //     USAGE_PAGE (Generic Desktop)
    0x09, 0x30,                    //     USAGE (X)
    0x09, 0x31,                    //     USAGE (Y)
    0x15, 0x81,                    //     LOGICAL_MINIMUM (-127)
    0x25, 0x7f,                    //     LOGICAL_MAXIMUM (127)
    0x75, 0x08,                    //     REPORT_SIZE (8)
    0x95, 0x02,                    //     REPORT_COUNT (2)
    0x81, 0x06,                    //     INPUT (Data,Var,Rel)
    0xc0,                          //   END_COLLECTION
    0xc0                           // END_COLLECTION
};

SEG_CODE const unsigned char UsbDescriptor_Report_Keyboard [45] = {
    0x05, 0x01,                    // USAGE_PAGE (Generic Desktop)
    0x09, 0x06,                    // USAGE (Keyboard)
    0xa1, 0x01,                    // COLLECTION (Application)
    0x05, 0x07,                    //   USAGE_PAGE (Keyboard)
    0x19, 0xe0,                    //   USAGE_MINIMUM (Keyboard LeftControl)
    0x29, 0xe7,                    //   USAGE_MAXIMUM (Keyboard Right GUI)
    0x15, 0x00,                    //   LOGICAL_MINIMUM (0)
    0x25, 0x01,                    //   LOGICAL_MAXIMUM (1)
    0x75, 0x01,                    //   REPORT_SIZE (1)
    0x95, 0x08,                    //   REPORT_COUNT (8)
    0x81, 0x02,                    //   INPUT (Data,Var,Abs)
    0x95, 0x01,                    //   REPORT_COUNT (1)
    0x75, 0x08,                    //   REPORT_SIZE (8)
    0x81, 0x03,                    //   INPUT (Cnst,Var,Abs)
    0x95, 0x06,                    //   REPORT_COUNT (6)
    0x75, 0x08,                    //   REPORT_SIZE (8)
    0x15, 0x00,                    //   LOGICAL_MINIMUM (0)
    0x25, 0x65,                    //   LOGICAL_MAXIMUM (101)
    0x05, 0x07,                    //   USAGE_PAGE (Keyboard)
    0x19, 0x00,                    //   USAGE_MINIMUM (Reserved (no event indicated))
    0x29, 0x65,                    //   USAGE_MAXIMUM (Keyboard Application)
    0x81, 0x00,                    //   INPUT (Data,Ary,Abs)
    0xc0                           // END_COLLECTION
};


SEG_CODE const SUsbDescriptor_Device UsbDescriptor_Device =
{
	{
		sizeof(SUsbDescriptor_Device),		// Size of this Descriptor in Bytes
		EUsbDescriptorType_Device			// Descriptor Type (=1)
	},			  
	MUsbWord_Word(EUsbSpecVer_1d1),						   // USB Spec Release Number in BCD
	0x00,			 // Device Class Code
	0x00,			// Device Subclass Code
	0x00,			 // Device Protocol Code
	0x40,			  // Maximum Packet Size for EP0
	MUsbWord_Word(EUsbVendorId_SciLabs),	 // Vendor ID
	MUsbWord_Bytes(81,B9),					 // Product ID
	MUsbWord_Bytes(00,00),					   // Device Release Number
	EStringDesc_Manufacturer,
	EStringDesc_Product,		
	EStringDesc_SerialNb,		
	0x01		   // Number of possible Configurations
};

typedef struct 
{
   SUsbDescriptor_Configuration  Common;

   SUsbDescriptor_Interface      Mouse_Interface;
   SUsbDescriptor_HIDClass       Mouse_HIDClass;
   SUsbDescriptor_Endpoint       Mouse_Endpoint1_In;
 
   SUsbDescriptor_Interface      Keyboard_Interface;
   SUsbDescriptor_HIDClass       Keyboard_HIDClass;
   SUsbDescriptor_Endpoint       Keyboard_Endpoint2_In;

} SUsbDescriptor_Configuration1;

SEG_CODE SUsbDescriptor_Configuration1 UsbDescriptor_Configuration =
{
	// Common
	{
		{
			sizeof(SUsbDescriptor_Configuration),		
			EUsbDescriptorType_Configuration			
		},			  
		MUsbWord_Word(sizeof(SUsbDescriptor_Configuration1)),
		0x02,	// InterfacesCount
		0x01,	// Designator Value for *this* Configuration
		EStringDesc_Config,
		EUsbConfiguration_Attributes_MustWrite | 
		EUsbConfiguration_Attributes_HostPowered | 
		EUsbConfiguration_Attributes_RemoteWakeupOff,
		0x20 	// (MaxPower*2)mA is the power allowed
	},

	// Mouse Interface
	{
		{
			sizeof(SUsbDescriptor_Interface),		
			EUsbDescriptorType_Interface			
		},	
		0x00,  // bInterfaceNumber
		0x00,  // bAlternateSetting
		0x01,  // bNumEndpoints
		EUsbInterface_Class_HID, 
		EUsbInterface_SubClass_HID_NoBootInterface,		
		//EUsbInterface_SubClass_HID_BootInterface,
		EUsbInterface_Protocol_HID_NoBootInterface,	
		//EUsbInterface_Protocol_HID_BootInterface_Mouse, 
		EStringDesc_Interface,
	},

	// HIDClass
	{
		{
			sizeof(SUsbDescriptor_HIDClass),		
			EUsbDescriptorType_HIDClass,			
		},	
		MUsbWord_Word( EUsbHIDSpecVer_1d01 ),   
		EUsbDescriptor_HIDClass_CountryCode_Default, 
		0x01,                           // SubordinateDescriptorsCount;
		EUsbDescriptorType_HIDReport,   // DescriptorType
		MUsbWord_Word( sizeof(UsbDescriptor_Report_Mouse) ),    
	},

	// Endpoints
	{
		{
			sizeof(SUsbDescriptor_Endpoint),		
			EUsbDescriptorType_Endpoint,			
		},	
		EUsbEndpoint_Address_In | EUsbEndpoint_Address_EP1, 
		EUsbEndpoint_Attributes_Type_Interrupt, // bmAttributes
		MUsbWord_Word( MUsb_Endpoint1_PacketSize ), // MaxPacketSize
		10    // bInterval in ms
	},
	// Keyboard Interface
	{
		{
			sizeof(SUsbDescriptor_Interface),		
			EUsbDescriptorType_Interface			
		},	
		0x01,  // bInterfaceNumber
		0x00,  // bAlternateSetting
		0x01,  // bNumEndpoints
		EUsbInterface_Class_HID, 
		EUsbInterface_SubClass_HID_NoBootInterface,		
		//EUsbInterface_SubClass_HID_BootInterface,
		EUsbInterface_Protocol_HID_NoBootInterface,	
		//EUsbInterface_Protocol_HID_BootInterface_Mouse, 
		EStringDesc_Interface,
	},

	// HIDClass
	{
		{
			sizeof(SUsbDescriptor_HIDClass),		
			EUsbDescriptorType_HIDClass,			
		},	
		MUsbWord_Word( EUsbHIDSpecVer_1d01 ),   
		EUsbDescriptor_HIDClass_CountryCode_Default, 
		0x01,                           // SubordinateDescriptorsCount;
		EUsbDescriptorType_HIDReport,   // DescriptorType
		MUsbWord_Word( sizeof(UsbDescriptor_Report_Keyboard) ),    
	},

	// Endpoints
	{
		{
			sizeof(SUsbDescriptor_Endpoint),		
			EUsbDescriptorType_Endpoint,			
		},	
		EUsbEndpoint_Address_In | EUsbEndpoint_Address_EP2, 
		EUsbEndpoint_Attributes_Type_Interrupt, // bmAttributes
		MUsbWord_Word( MUsb_Endpoint2_PacketSize ), // MaxPacketSize
		10    // bInterval in ms
	}
};           

/////////////////////////////////
/////////////////////////////////
// Structs


typedef enum 
{
	EUsbDeviceState_JustAfterReset	, 
	EUsbDeviceState_HasAdress		,
	EUsbDeviceState_FullyConfigured		
} EUsbDeviceState;

typedef enum 
{
	EUsbEndpointState_Idle			= 0x00,
	EUsbEndpointState_Transmit		= 0x01,
	EUsbEndpointState_Halt			= 0x02, // return stalls
	EUsbEndpointState_Stall			= 0x03, // send procedural stall next status phase

} EUsbEndpointState;

typedef struct {	
	U8* Ptr;	  
	U16 Size;			// Size that the software wants to send
	U16 Size_Host;		// Size that the host wants. If this is lower than EP0_SendQueue.Size, we will have to wait another setup packet and split
	U16 CurrentOffset;

} SSendQueue_EP0;

typedef struct {

	U8  TempBuffer[8];   
	U8  TempBuffer_Size;
} SSendQueue_EPX;

typedef struct 
{
	U8 Mouse_Buttons;
	U8 Keboard_Modifiers;
	U8 Keboard_Keys[6];
} SOutput;

typedef struct {
	EUsbDeviceState   DeviceState;
	EUsbEndpointState EndpointStates[3];
	
	U8 Usb_FADDR;
	
	SSendQueue_EP0 EP0_SendQueue; 
	SSendQueue_EPX EPX_SendQueue; 
	
	SOutput  Output;
	Bool     OutputHasChangedSinceLatestSent_EP1;			  
	Bool     OutputHasChangedSinceLatestSent_EP2;			  
	
} SMainContext;

void MC_SetDefault(SMainContext* _MC) {

	_MC->DeviceState       = EUsbDeviceState_JustAfterReset;

	_MC->EndpointStates[0] = EUsbEndpointState_Idle;
	_MC->EndpointStates[1] = EUsbEndpointState_Halt;
	_MC->EndpointStates[2] = EUsbEndpointState_Halt;
	
	_MC->Usb_FADDR = 0x00;
	
	
	_MC->EP0_SendQueue.Ptr = 0x00;
	_MC->EP0_SendQueue.Size = 0x00;
	_MC->EP0_SendQueue.Size_Host = 0x00;
	_MC->EP0_SendQueue.CurrentOffset = 0x00;	
	
	_MC->Output.Mouse_Buttons = 0x00;
	_MC->Output.Keboard_Modifiers = 0x00;
	_MC->Output.Keboard_Keys[0] = 0x00;
	_MC->Output.Keboard_Keys[1] = 0x00;
	_MC->Output.Keboard_Keys[2] = 0x00;
	_MC->Output.Keboard_Keys[3] = 0x00;
	_MC->Output.Keboard_Keys[4] = 0x00;
	_MC->Output.Keboard_Keys[5] = 0x00;
	
	_MC->OutputHasChangedSinceLatestSent_EP1 = False;
	_MC->OutputHasChangedSinceLatestSent_EP2 = False;
}

SEG_NEAR volatile SMainContext MC;

/////////////////////////////////
/////////////////////////////////
// INTERRUPTS

typedef enum 
{
	EUsbEventType_None                              = 0x00, 
	EUsbEventType_Resume                            = 0x01, 
	EUsbEventType_Suspend                           = 0x02, 
	EUsbEventType_Reset                             = 0x04, 
	EUsbEventType_Stall                             = 0x05, 
	EUsbEventType_EP0_In_Before                     = 0x06, 
	EUsbEventType_EP0_In_SetupEnd                   = 0x07,
	EUsbEventType_EP0_In_StallHasBeenSent			= 0x08,  
	EUsbEventType_EP0_In_IncomingPacketRecieved     = 0x09, 
	EUsbEventType_EP0_In_After                      = 0x0a,
	
	EUsbEventType_EP1_In_OutgoingPacketTransmitted  = 0x0b, 
	EUsbEventType_EP2_In_OutgoingPacketTransmitted  = 0x0c, 
} EUsbEvent;


void Usb_ISR_HandleEvent(EUsbEvent _Event)
{	
	U8 ControlRegister, NewControlRegister;
	U8 DataCount;
	U8 Index;
	SUsbPacket_Setup SetupPacket;
	U8 SendOrRecieveIsFinished;
	U8 Setup_RequestType_Type, Setup_RequestType_Direction, Setup_RequestType_Recipient;
	U8 bob;
	U16 DataToSend_ActualSize;

	switch( _Event )
	{
		case EUsbEventType_Resume:				
		break;
		
		case EUsbEventType_Suspend:		
		// Disable oscillator until resume
		// OSCICN |= OSCICN_SUSPEND;
		break;
		
		case EUsbEventType_Reset:	
		USB_WriteRegister( USB0ADR_POWER, USB0DAT_POWER_SUSEN); // Activate transciever and enable suspend detection
		MC_SetDefault(&MC);
		break;
		
		case EUsbEventType_EP0_In_SetupEnd:	
		USB_WriteRegister( USB0ADR_INDEX, 0);  // Select Endpoint Zero
		//USB_WriteRegister (USB0ADR_E0CSR, USB0ADR_E0CSR_DATAEND);
		USB_WriteRegister (USB0ADR_E0CSR, USB0ADR_E0CSR_SSUEND);
		MC.EndpointStates[0] = EUsbEndpointState_Idle;
		break;
		
		case EUsbEventType_EP0_In_Before:
		if ( MC.DeviceState == EUsbDeviceState_HasAdress )
		{
			USB_WriteRegister (USB0ADR_FADDR, MC.Usb_FADDR | USB0ADR_FADDR_BUSY );
			MC.EndpointStates[0] = EUsbEndpointState_Idle;
		} 
		break;

		case EUsbEventType_EP0_In_StallHasBeenSent:
		USB_WriteRegister( USB0ADR_INDEX, 0);  // Select Endpoint Zero
		USB_WriteRegister( USB0ADR_E0CSR, 0);  // Cleat USB0ADR_E0CSR_STSTL bit
		MC.EndpointStates[0] = EUsbEndpointState_Idle;
		break;
		
		case EUsbEventType_EP0_In_IncomingPacketRecieved:
		if (MC.EndpointStates[0] == EUsbEndpointState_Idle)
		{
			USB_WriteRegister( USB0ADR_INDEX, 0);  // Select Endpoint Zero

			USB_ReadRegister(DataCount, USB0ADR_E0CNT);  // Read the control register 
			USB_ReadEndpointFifo(USB0ADR_FIFO_EP0, (U8*)&SetupPacket, sizeof(SetupPacket) );
			
			Setup_RequestType_Type		= (SetupPacket.RequestType & EUsbPacket_Setup_RequestType_TypeMask);
			Setup_RequestType_Direction = (SetupPacket.RequestType & EUsbPacket_Setup_RequestType_DirMask);
			Setup_RequestType_Recipient = (SetupPacket.RequestType & EUsbPacket_Setup_RequestType_RecipientMask);

			if (Setup_RequestType_Type == EUsbPacket_Setup_RequestType_Type_Standard) {
				// standard requests
				switch (SetupPacket.Request)	  
				{	
					case EUsbPacket_Setup_Request_SetAddress:			  
					{	
						if ( USB_Word_IsNotEqual(SetupPacket.Index, 0x00, 0x00 ) ||
						     USB_Word_IsNotEqual(SetupPacket.Length, 0x00, 0x00 ) ||
						     (Setup_RequestType_Direction != EUsbPacket_Setup_RequestType_Dir_Out) ||
							 (Setup_RequestType_Recipient != EUsbPacket_Setup_RequestType_Recipient_Device) ||
							 (SetupPacket.Value.MostSignificantByte != 0x00) ||
							 ((SetupPacket.Value.LeastSignificantByte & 0x80) != 0x00) ) 
						{
							USB_WriteRegister(USB0ADR_E0CSR, USB0ADR_E0CSR_SDSTL);
							MC.EndpointStates[0] = EUsbEndpointState_Stall;
						}						
						else 
						{
							MC.DeviceState = EUsbDeviceState_HasAdress;
							MC.EndpointStates[0] = EUsbEndpointState_Idle;
							MC.Usb_FADDR = SetupPacket.Value.LeastSignificantByte;
							USB_WriteRegister(USB0ADR_E0CSR, USB0ADR_E0CSR_SOPRDY | USB0ADR_E0CSR_DATAEND);
						}
					}
					break;
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
								MC.EP0_SendQueue.Ptr = (U8*)&UsbDescriptor_Device;
								MC.EP0_SendQueue.Size = sizeof(UsbDescriptor_Device);
								MC.EP0_SendQueue.Size_Host = USB_Word_GetValue(SetupPacket.Length);
								MC.EP0_SendQueue.CurrentOffset = 0;
								MC.EndpointStates[0] = EUsbEndpointState_Transmit;	
								USB_WriteRegister(USB0ADR_E0CSR, USB0ADR_E0CSR_SOPRDY );
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
								MC.EP0_SendQueue.Ptr = (U8*)&UsbDescriptor_Configuration;
								MC.EP0_SendQueue.Size = sizeof(UsbDescriptor_Configuration);
								MC.EP0_SendQueue.Size_Host = USB_Word_GetValue(SetupPacket.Length);
								MC.EP0_SendQueue.CurrentOffset = 0;
								MC.EndpointStates[0] = EUsbEndpointState_Transmit;	
								USB_WriteRegister(USB0ADR_E0CSR, USB0ADR_E0CSR_SOPRDY );
							}
							break;

							case EUsbDescriptorType_String:
							switch (SetupPacket.Value.LeastSignificantByte)
							{
								case EStringDesc_None:            MC.EP0_SendQueue.Ptr = (U8*)&UsbDescriptor_String_AvailableLanguages; break;
								case EStringDesc_Manufacturer:    MC.EP0_SendQueue.Ptr = (U8*)&UsbDescriptor_String_Manufacturer; break;
								case EStringDesc_Product:         MC.EP0_SendQueue.Ptr = (U8*)&UsbDescriptor_String_Product; break;
								case EStringDesc_SerialNb:        MC.EP0_SendQueue.Ptr = (U8*)&UsbDescriptor_String_SerialNb; break;
								case EStringDesc_Config:          MC.EP0_SendQueue.Ptr = (U8*)&UsbDescriptor_String_Config; break;
								case EStringDesc_Interface:       MC.EP0_SendQueue.Ptr = (U8*)&UsbDescriptor_String_Interface; break;
								default:
								USB_WriteRegister(USB0ADR_E0CSR, USB0ADR_E0CSR_SDSTL);
								MC.EndpointStates[0] = EUsbEndpointState_Stall;
								break;
							}
							if( MC.EndpointStates[0] != EUsbEndpointState_Stall )
							{
								MC.EP0_SendQueue.Size = *MC.EP0_SendQueue.Ptr;
								MC.EP0_SendQueue.Size_Host = USB_Word_GetValue(SetupPacket.Length);
								MC.EP0_SendQueue.CurrentOffset = 0;
								MC.EndpointStates[0] = EUsbEndpointState_Transmit;	
								USB_WriteRegister(USB0ADR_E0CSR, USB0ADR_E0CSR_SOPRDY );
							}
							break;

							case EUsbDescriptorType_HIDReport:
							if(SetupPacket.Value.LeastSignificantByte == 0)
							{
								if(SetupPacket.Index.LeastSignificantByte == 0)
								{
									MC.EP0_SendQueue.Ptr = (U8*)&UsbDescriptor_Report_Mouse;
									MC.EP0_SendQueue.Size = sizeof(UsbDescriptor_Report_Mouse);
								} else if(SetupPacket.Index.LeastSignificantByte == 1) {
									MC.EP0_SendQueue.Ptr = (U8*)&UsbDescriptor_Report_Keyboard;
									MC.EP0_SendQueue.Size = sizeof(UsbDescriptor_Report_Keyboard);
								} else {
									USB_WriteRegister(USB0ADR_E0CSR, USB0ADR_E0CSR_SDSTL);
									MC.EndpointStates[0] = EUsbEndpointState_Stall;
								}
							}
							else
							{
								USB_WriteRegister(USB0ADR_E0CSR, USB0ADR_E0CSR_SDSTL);
								MC.EndpointStates[0] = EUsbEndpointState_Stall;
							}
							if( MC.EndpointStates[0] != EUsbEndpointState_Stall )
							{
								MC.EP0_SendQueue.Size_Host = USB_Word_GetValue(SetupPacket.Length);
								MC.EP0_SendQueue.CurrentOffset = 0;
								MC.EndpointStates[0] = EUsbEndpointState_Transmit;	
								USB_WriteRegister(USB0ADR_E0CSR, USB0ADR_E0CSR_SOPRDY );
							}
							break;

							case EUsbDescriptorType_Interface:
							case EUsbDescriptorType_Endpoint:
							case EUsbDescriptorType_HIDClass:
							default:
							USB_WriteRegister(USB0ADR_E0CSR, USB0ADR_E0CSR_SDSTL);
							MC.EndpointStates[0] = EUsbEndpointState_Stall;
							break;
						}
						break;
					}			 
					case EUsbPacket_Setup_Request_SetConfig:
					{
						if ( USB_Word_IsNotEqual(SetupPacket.Index, 0x00, 0x00 ) ||
						     USB_Word_IsNotEqual(SetupPacket.Length, 0x00, 0x00 ) ||
							 (SetupPacket.Value.MostSignificantByte != 0x00) ||
							 (SetupPacket.Value.LeastSignificantByte != 0x00 && SetupPacket.Value.LeastSignificantByte != 0x01) ||
						     Setup_RequestType_Direction != EUsbPacket_Setup_RequestType_Dir_Out ||
							 Setup_RequestType_Recipient != EUsbPacket_Setup_RequestType_Recipient_Device ) 
						{
							USB_WriteRegister(USB0ADR_E0CSR, USB0ADR_E0CSR_SDSTL);
							MC.EndpointStates[0] = EUsbEndpointState_Stall;
						}						
						else 
						{
							if( SetupPacket.Value.LeastSignificantByte == 0x00 ) {
								// Not configured
								MC.DeviceState = EUsbDeviceState_HasAdress;
								MC.EndpointStates[0] = EUsbEndpointState_Idle;
								MC.EndpointStates[1] = EUsbEndpointState_Halt;
								MC.EndpointStates[2] = EUsbEndpointState_Halt;

							} else {
								// SetupPacket.Value.LeastSignificantByte == 0x01
								MC.DeviceState = EUsbDeviceState_FullyConfigured;
								MC.EndpointStates[0] = EUsbEndpointState_Idle;
								MC.EndpointStates[1] = EUsbEndpointState_Idle;
								MC.EndpointStates[2] = EUsbEndpointState_Idle;
								
								// Configure Endpoint 1
							    USB_WriteRegister (USB0ADR_INDEX, 1); 
								USB_WriteRegister(USB0ADR_EINCSRL, 0x00); 
								USB_WriteRegister (USB0ADR_EINCSRH, USB0ADR_EINCSRH_SPLIT);
								// Configure Endpoint 2
							    USB_WriteRegister (USB0ADR_INDEX, 2);
								USB_WriteRegister(USB0ADR_EINCSRL, 0x00); 
								USB_WriteRegister (USB0ADR_EINCSRH, USB0ADR_EINCSRH_SPLIT);
								// Set index back to endpoint 0
							    USB_WriteRegister (USB0ADR_INDEX, 0);  
								
							}
							USB_WriteRegister(USB0ADR_E0CSR, USB0ADR_E0CSR_SOPRDY | USB0ADR_E0CSR_DATAEND);
						}

					}
					break;	
					case EUsbPacket_Setup_Request_GetStatus:			 
					case EUsbPacket_Setup_Request_GetConfig:
					case EUsbPacket_Setup_Request_GetInterface:	
					case EUsbPacket_Setup_Request_SetInterface:
					case EUsbPacket_Setup_Request_ClearFeature:			 
					case EUsbPacket_Setup_Request_SetFeature:			 
					bob++;

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
					case EUsbPacket_Setup_Request_SetIdle:
					{
						if ( USB_Word_IsNotEqual(SetupPacket.Length, 0x00, 0x00 ) ||
							 (SetupPacket.Index.MostSignificantByte != 0x00) ||
							 (SetupPacket.Index.LeastSignificantByte != 0x00 && SetupPacket.Index.LeastSignificantByte != 0x01) ||							 
							 (SetupPacket.Value.LeastSignificantByte != 0x00 && SetupPacket.Value.LeastSignificantByte != 0x01) ||
						     Setup_RequestType_Direction != EUsbPacket_Setup_RequestType_Dir_Out ||
							 Setup_RequestType_Recipient != EUsbPacket_Setup_RequestType_Recipient_Interface ) 
						{
							
							USB_WriteRegister(USB0ADR_E0CSR, USB0ADR_E0CSR_SDSTL);
							MC.EndpointStates[0] = EUsbEndpointState_Stall;
						}	
						
						if( MC.EndpointStates[0] != EUsbEndpointState_Stall )
						{
							// SetupPacket.Index.LeastSignificantByte; // Interface
							// SetupPacket.Value.LeastSignificantByte; // Report Id or 0x00 for all reports
							// SetupPacket.Value.MostSignificantByte; // Duration in units of 4ms

							USB_WriteRegister(USB0ADR_E0CSR, USB0ADR_E0CSR_SOPRDY | USB0ADR_E0CSR_DATAEND);
						}
					}
					break;
					case EUsbPacket_Setup_Request_GetReport:
					case EUsbPacket_Setup_Request_SetReport:			  
					case EUsbPacket_Setup_Request_GetIdle:				  
					case EUsbPacket_Setup_Request_GetProtocol:	
					case EUsbPacket_Setup_Request_SetProtocol:	
					bob++;
										
					default:
					USB_WriteRegister(USB0ADR_E0CSR, USB0ADR_E0CSR_SDSTL);
					MC.EndpointStates[0] = EUsbEndpointState_Stall;							
					break;				  
					
				} 
			} else {
				USB_WriteRegister(USB0ADR_E0CSR, USB0ADR_E0CSR_SDSTL);
				MC.EndpointStates[0] = EUsbEndpointState_Stall;	
			} 
		}
		break;
		
		case EUsbEventType_EP0_In_After:
		// Current event has been handled, or previous packet has been sent
		// check if we can start or continue transmitting
		if (MC.EndpointStates[0] == EUsbEndpointState_Transmit)
		{
			USB_WriteRegister( USB0ADR_INDEX, 0);  // Select Endpoint Zero
			USB_ReadRegister(ControlRegister, USB0ADR_E0CSR);  // Read the control register 

			if( (ControlRegister & USB0ADR_E0CSR_INPRDY) == 0 &&
			    (ControlRegister & USB0ADR_E0CSR_SUEND) == 0 &&
				(ControlRegister & USB0ADR_E0CSR_OPRDY) == 0 ) // We're not doing anything else
			{
				SendOrRecieveIsFinished = False;
			
				NewControlRegister = USB0ADR_E0CSR_INPRDY;
				DataToSend_ActualSize = (MC.EP0_SendQueue.Size < MC.EP0_SendQueue.Size_Host) ? MC.EP0_SendQueue.Size : MC.EP0_SendQueue.Size_Host; 
				if ( (DataToSend_ActualSize == 0) || (DataToSend_ActualSize == MC.EP0_SendQueue.CurrentOffset)) 
				{
					// Zero-length packet
					// or even multiple last packet
					SendOrRecieveIsFinished = True;	
				}
				else if ( MUsb_Endpoint0_PacketSize > DataToSend_ActualSize - MC.EP0_SendQueue.CurrentOffset )
				{
					// If we can send the remaining part of the packet 
					// (but not exactly so : if we have an even multiple of MUsb_Endpoint0_PacketSize, we will have to send a ZLP next)
					USB_WriteEndpointFifo(USB0ADR_FIFO_EP0, MC.EP0_SendQueue.Ptr + MC.EP0_SendQueue.CurrentOffset, DataToSend_ActualSize - MC.EP0_SendQueue.CurrentOffset );  
					MC.EP0_SendQueue.CurrentOffset = DataToSend_ActualSize;	  
					SendOrRecieveIsFinished = True;			   
				}
				else
				{
					// Break Data into multiple packets if larger than Max Packet
					USB_WriteEndpointFifo(USB0ADR_FIFO_EP0, MC.EP0_SendQueue.Ptr + MC.EP0_SendQueue.CurrentOffset, MUsb_Endpoint0_PacketSize ); 
					MC.EP0_SendQueue.CurrentOffset += MUsb_Endpoint0_PacketSize;
				}

				if( SendOrRecieveIsFinished ) {
					NewControlRegister |= USB0ADR_E0CSR_DATAEND; // Add Data End bit to mask
					MC.EndpointStates[0] = EUsbEndpointState_Idle; // Return EP 0 to idle state

				
				}
				USB_WriteRegister(USB0ADR_E0CSR, NewControlRegister);	
			}
		}
		break;

		case EUsbEventType_EP1_In_OutgoingPacketTransmitted:
		case EUsbEventType_EP2_In_OutgoingPacketTransmitted:
		{
			Index = (_Event == EUsbEventType_EP1_In_OutgoingPacketTransmitted) ? 1 : 2;
			USB_WriteRegister( USB0ADR_INDEX, Index);  // Select Endpoint
			USB_ReadRegister(ControlRegister, USB0ADR_EINCSRL);  // Read the control register

			if(MC.DeviceState == EUsbDeviceState_FullyConfigured) {
				// Clear sent stall and stop stalling 
				USB_WriteRegister(USB0ADR_EINCSRL, 0x00); 
				MC.EndpointStates[Index] = EUsbEndpointState_Idle;
			} else {
				// Clear sent stall and continue stalling
				USB_WriteRegister(USB0ADR_EINCSRL, USB0ADR_EINCSRL_SDSTL); // Stalls until device configured				
				MC.EndpointStates[Index] = EUsbEndpointState_Halt;
			}
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
	EUsbEvent	EventQueue[10];
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
		EventQueue[EventQueue_Size] = EUsbEventType_Resume;
		EventQueue_Size++;
	}
	if (IntFlags_Common & USB0ADR_CMINT_SUSINT)			 
	{
		EventQueue[EventQueue_Size] = EUsbEventType_Suspend;
		EventQueue_Size++;
	}
	if (IntFlags_Common & USB0ADR_CMINT_RSTINT)			 
	{
		EventQueue[EventQueue_Size] = EUsbEventType_Reset;
		EventQueue_Size++;
	}
	if (IntFlags_In & USB0DAT_IN1INT_EP0)				 
	{					   
		// Handle packet received
		USB_WriteRegister( USB0ADR_INDEX, 0);  // Select Endpoint Zero
		USB_ReadRegister(ControlRegister, USB0ADR_E0CSR);  // Read the control register		
		
		EventQueue[EventQueue_Size] = EUsbEventType_EP0_In_Before;
		EventQueue_Size++;
		
		if (ControlRegister & USB0ADR_E0CSR_STSTL)
		{
			// Handle stalls
			EventQueue[EventQueue_Size] = EUsbEventType_EP0_In_StallHasBeenSent;
			EventQueue_Size++;
			//Don't do anything else concerning EP0 during this call
		}
		else
		{
			if (ControlRegister & USB0ADR_E0CSR_SUEND)
			{
				// Handle setup end
				EventQueue[EventQueue_Size] = EUsbEventType_EP0_In_SetupEnd;
				EventQueue_Size++;
			}
			if (ControlRegister & USB0ADR_E0CSR_OPRDY)		  
			{
				// the host has send a Packet to the device			
				EventQueue[EventQueue_Size] = EUsbEventType_EP0_In_IncomingPacketRecieved;
				EventQueue_Size++;
			}
		
			// maybe the device has sent a Packet to the host 
			// should we send more ?
			EventQueue[EventQueue_Size] = EUsbEventType_EP0_In_After;
			EventQueue_Size++;	
		}	
	}
	if (IntFlags_In & USB0DAT_IN1INT_EP1)				 
	{	  
		EventQueue[EventQueue_Size] = EUsbEventType_EP1_In_OutgoingPacketTransmitted;
		EventQueue_Size++;
	}
	if (IntFlags_In & USB0DAT_IN1INT_EP2)				 
	{								 
		EventQueue[EventQueue_Size] = EUsbEventType_EP2_In_OutgoingPacketTransmitted;
		EventQueue_Size++;
	}
	
	for( I=0; I<EventQueue_Size; I++) {
		Usb_ISR_HandleEvent(EventQueue[I]);
	}	 
}

/////////////////////////////////
/////////////////////////////////
// MAIN

void main (void)
{
	U16 I;
	U16 FrameCyclicCounterHigh, FrameCyclicCounterLow;
	U8 ControlRegister, NewControlRegister;
	U8 PreviousEAState;
	
	// Disable Watchdog timer
	PCA0MD &= ~0x40;								 
	
	// Disable interrupts during setup
	IE = 0;

	// Setup clock
	#if MIsEnabled(MUsb_HighSpeed) 
		OSCICN = (OSCICN_IOSCEN | OSCICN_DIVBY_1);		  // Configure internal oscillator
		for(I = 0;I < 500;I++);							  // Delay for Oscillator to begin
		while(!(OSCICN & OSCICN_IFRDY));				  // Wait for Oscillator to be ready
		
		CLKMUL = (CLKMUL_MULSEL_INTERNAL | CLKMUL_MULEN);		   // Enable clock multiplier 
		
		CLKMUL |= CLKMUL_MULINIT;								// Start the multiplier initialization this can not be merged with previous step
		for(I = 0;I < 500;I++);												 // Delay for clock multiplier to begin
		while(!(CLKMUL & CLKMUL_MULRDY));									// Wait for multiplier to lock
		
		CLKSEL = (CLKSEL_CLKSL_INTERNAL | CLKSEL_USBCLK_4XMUL);			// Select system clock and USB clock
		
	#else
		OSCICN = (OSCICN_IOSCEN | OSCICN_DIVBY_1);
		CLKSEL	= (CLKSEL_CLKSL_INTERNAL | CLKSEL_USBCLK_INTERNALDIV2);  
	#endif
	
	// Setup USB
	USB_WriteRegister(USB0ADR_POWER, USB0DAT_POWER_USBRST); // Request Reset
	USB_WriteRegister(USB0ADR_IN1IE, USB0DAT_IN1IE_EP0 | USB0DAT_IN1IE_EP1 | USB0DAT_IN1IE_EP2); // Enable Endpoint 0-2 in interrupts
	USB_WriteRegister(USB0ADR_CMIE, USB0DAT_CMIE_RSTINTE | USB0DAT_CMIE_RSUINTE | USB0DAT_CMIE_SUSINTE);  // Enable Reset, Resume, and Suspend interrupts
	
	#if MIsEnabled(MUsb_HighSpeed) 
		USB0XCN = (USB0XCN_PREN | USB0XCN_PHYEN | USB0XCN_SPEED); // Activate Usb transciever at Full speed
	#else
		USB0XCN = (USB0XCN_PREN | USB0XCN_PHYEN); // Activate Usb transciever at Low speed
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
		// blink leds, to show when running
		P2 = (FrameCyclicCounterHigh % 0x0004 < 0x0002) ? 0x00 : 0x04;
		FrameCyclicCounterLow++;
		if(!FrameCyclicCounterLow) FrameCyclicCounterHigh++;

		PreviousEAState = IE;
		IE = 0; // disable interrupts

		//*
		if(	MC.DeviceState == EUsbDeviceState_FullyConfigured &&
			(MC.OutputHasChangedSinceLatestSent_EP1 || MC.OutputHasChangedSinceLatestSent_EP2) // Else we only send NAKs
			)
		{
			if ( MC.EndpointStates[1] == EUsbEndpointState_Idle &&
			     MC.OutputHasChangedSinceLatestSent_EP1) 
			{		
				USB_WriteRegister( USB0ADR_INDEX, 1);  // Select Endpoint One
				USB_ReadRegister(ControlRegister, USB0ADR_EINCSRL);  // Read the control register
				NewControlRegister = ControlRegister;
				NewControlRegister = ( NewControlRegister & (~USB0ADR_EINCSRL_UNDRUN));	// Clear underrun

				if(((ControlRegister & USB0ADR_EINCSRL_SDSTL) == 0x00) &&
				   ((ControlRegister & USB0ADR_EINCSRL_FIFONE) == 0x00) &&
				   ((ControlRegister & USB0ADR_EINCSRL_INPRDY) == 0x00))
				{
					MC.EPX_SendQueue.TempBuffer_Size = 0;
					MC.EPX_SendQueue.TempBuffer[MC.EPX_SendQueue.TempBuffer_Size++] = MC.Output.Mouse_Buttons;
					MC.EPX_SendQueue.TempBuffer[MC.EPX_SendQueue.TempBuffer_Size++] = 0;
					MC.EPX_SendQueue.TempBuffer[MC.EPX_SendQueue.TempBuffer_Size++] = 0;   

					MC.EndpointStates[1] = EUsbEndpointState_Transmit;

					USB_WriteEndpointFifo(USB0ADR_FIFO_EP1, MC.EPX_SendQueue.TempBuffer, MC.EPX_SendQueue.TempBuffer_Size );
													 
					NewControlRegister = (NewControlRegister | USB0ADR_EINCSRL_INPRDY);  

					MC.OutputHasChangedSinceLatestSent_EP1 = False;
				}	
				USB_WriteRegister(USB0ADR_EINCSRL, NewControlRegister);
			}			
			if ( MC.EndpointStates[2] == EUsbEndpointState_Idle &&
			     MC.OutputHasChangedSinceLatestSent_EP2) 
			{		
				USB_WriteRegister( USB0ADR_INDEX, 2);  // Select Endpoint One
				USB_ReadRegister(ControlRegister, USB0ADR_EINCSRL);  // Read the control register
				NewControlRegister = ControlRegister;
				NewControlRegister = ( NewControlRegister & (~USB0ADR_EINCSRL_UNDRUN)); // Clear underrun

				if(((ControlRegister & USB0ADR_EINCSRL_SDSTL) == 0x00) &&
				   ((ControlRegister & USB0ADR_EINCSRL_FIFONE) == 0x00) &&
				   ((ControlRegister & USB0ADR_EINCSRL_INPRDY) == 0x00))
				{
					MC.EPX_SendQueue.TempBuffer_Size = 0;
					MC.EPX_SendQueue.TempBuffer[MC.EPX_SendQueue.TempBuffer_Size++] = MC.Output.Keboard_Modifiers;
					MC.EPX_SendQueue.TempBuffer[MC.EPX_SendQueue.TempBuffer_Size++] = 0xff;
					MC.EPX_SendQueue.TempBuffer[MC.EPX_SendQueue.TempBuffer_Size++] = MC.Output.Keboard_Keys[0]; 
					MC.EPX_SendQueue.TempBuffer[MC.EPX_SendQueue.TempBuffer_Size++] = MC.Output.Keboard_Keys[1]; 
					MC.EPX_SendQueue.TempBuffer[MC.EPX_SendQueue.TempBuffer_Size++] = MC.Output.Keboard_Keys[2]; 
					MC.EPX_SendQueue.TempBuffer[MC.EPX_SendQueue.TempBuffer_Size++] = MC.Output.Keboard_Keys[3]; 
					MC.EPX_SendQueue.TempBuffer[MC.EPX_SendQueue.TempBuffer_Size++] = MC.Output.Keboard_Keys[4]; 
					MC.EPX_SendQueue.TempBuffer[MC.EPX_SendQueue.TempBuffer_Size++] = MC.Output.Keboard_Keys[5];   

					MC.EndpointStates[2] = EUsbEndpointState_Transmit;

					USB_WriteEndpointFifo(USB0ADR_FIFO_EP2, MC.EPX_SendQueue.TempBuffer, MC.EPX_SendQueue.TempBuffer_Size );
													 
					NewControlRegister = (NewControlRegister | USB0ADR_EINCSRL_INPRDY);  
					MC.OutputHasChangedSinceLatestSent_EP2 = False;
				}	
				USB_WriteRegister(USB0ADR_EINCSRL, NewControlRegister);
			}
		}

		
//*/

		IE = PreviousEAState; // global interrupts enable 		
	}
}
		