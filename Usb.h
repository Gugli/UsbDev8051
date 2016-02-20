#ifndef USB_H
#define USB_H

#include "compiler_defs.h"

typedef struct
{
   U8				LeastSignificantByte;
   U8				MostSignificantByte;
} TUsbWord;


#define MUsbWord_Bytes(_MSB, _LSB) {0x##_LSB, 0x##_MSB}
#define MUsbWord_Word(_WORD) {_WORD & 0xFF, (_WORD > 8) & 0xFF }

typedef U8 TUsbByte;


typedef struct
{
   TUsbByte 			Size;              // Size of this Descriptor in Bytes
   TUsbByte				DescriptorType;      
} SUsbDescriptorHeader;

typedef enum 
{
	EUsbDescriptorType_Device = 0x01,
	EUsbDescriptorType_Configuration = 0x02,
	EUsbDescriptorType_Interface = 0x04,
	EUsbDescriptorType_Endpoint = 0x05
} EUsbDescriptorType;
// Merge this 2 enums ?
typedef enum
{
	EUsbClass_ClassDescriptorType_HID = 0x21,
	EUsbClass_ReportDescriptorType_HID = 0x22 
} EUsbClass_DescriptorType;

typedef enum 
{
	EUsbSpecVer_1d1 = 0x1001,	// USB 1.1
	EUsbSpecVer_2 = 0x0002,		// USB 2
	EUsbSpecVer_3 = 0x0003		// USB 3
} EUsbSpecVer;

typedef enum 
{
	EUsbVendorId_Prototype = 0x6666
} EUsbVendorId;

typedef struct
{
   SUsbDescriptorHeader		Header; 
   TUsbWord					UsbSpecReleaseNumber;
   TUsbByte					DeviceClass;         
   TUsbByte					DeviceSubClass;      
   TUsbByte					DeviceProtocol;      
   TUsbByte					MaxPacketSizeEP0;    
   TUsbWord					VendorId;                      
   TUsbWord					ProductId;                    
   TUsbWord					DeviceReleaseNumber;            
   TUsbByte					ManufacturerStringIndex;        
   TUsbByte					ProductStringIndex;             
   TUsbByte					SerialNumberStringIndex;       
   TUsbByte					ConfigurationsCount;	

} SUsbDescriptor_Device;

typedef struct
{
   SUsbDescriptorHeader		Header; 
   TUsbWord					TotalLength;					// Total Length of Data for this Conf
   TUsbByte					InterfacesCount;
   TUsbByte					ConfigurationValue;				// Designator Value for *this* Configuration
   TUsbByte					ConfigurationStringIndex;
   TUsbByte					Attributes;						// Configuration Characteristics (see below)
   TUsbByte					MaxPower;						// (MaxPower*2)mA is the power allowed
                                      
} SUsbDescriptor_Configuration;       

typedef enum
{
	EUsbInterface_Class_HID = 0x03
} EUsbInterface_Class;

typedef enum
{
	EUsbInterface_SubClass_None = 0x00
} EUsbInterface_SubClass;

typedef enum
{
	EUsbInterface_Protocol_None = 0x00
} EUsbInterface_Protocol;

typedef struct
{
   SUsbDescriptorHeader		Header; 
   TUsbByte					InterfaceNumber;     
   TUsbByte					AlternateSetting;
   TUsbByte					EndpointsCount; 
   TUsbByte					InterfaceClass;					// Interface Class Code
   TUsbByte					InterfaceSubClass;				// Interface Subclass Code
   TUsbByte					InterfaceProtocol;				// Interface Protocol Code
   TUsbByte					InterfaceStringIndex;
} SUsbDescriptor_Interface;

typedef struct
{
   SUsbDescriptorHeader				Header; 
   TUsbWord							ClassSpecReleaseNumber;
   TUsbByte							CountryCode; 
   TUsbByte							DescriptorsCount;
   TUsbByte							ReportDescriptorType; 
   TUsbWord							ItemLength;
} SUsbDescriptor_Class;              

typedef struct
{
   SUsbDescriptorHeader				Header; 
   TUsbByte EndpointAddress;		// Endpoint Address (Number + Direction)
   TUsbByte Attributes;				// Endpoint Attributes (Transfer Type)
   TUsbWord MaxPacketSize;          // Max. Endpoint Packet Size
   TUsbByte PollingInterval;        // In ms
} SUsbDescriptor_Endpoint;              

typedef unsigned char const * const * const SUsbDescriptor_StringTable;


typedef struct 
{
   TUsbByte RequestType;        // Request recipient, type, and dir.
   TUsbByte Request;            // EUsbPacket_Setup_Request
   TUsbWord Value;              // varies according to request
   TUsbWord Index;              // varies according to request
   TUsbWord Length;             // Number of bytes to transfer
} SUsbPacket_Setup; 

typedef enum
{
	// Standard requests
	EUsbPacket_Setup_Request_GetStatus     = 0x00,
	EUsbPacket_Setup_Request_CleatFeature  = 0x01,
	EUsbPacket_Setup_Request_SetFeature    = 0x02,
	EUsbPacket_Setup_Request_SetAddress    = 0x05,
	EUsbPacket_Setup_Request_GetDescriptor = 0x06,
	EUsbPacket_Setup_Request_SetDescriptor = 0x07,
	EUsbPacket_Setup_Request_GetConfig     = 0x08,
	EUsbPacket_Setup_Request_SetConfig     = 0x09,
	EUsbPacket_Setup_Request_GetInterface  = 0x0A,
	EUsbPacket_Setup_Request_SetInterface  = 0x0B,
	EUsbPacket_Setup_Request_SyncFrame     = 0x0C,
	
	// HID requests
	EUsbPacket_Setup_Request_GetReport     = 0x01,
	EUsbPacket_Setup_Request_GetIdle       = 0x02,
	EUsbPacket_Setup_Request_GetProtocol   = 0x03,
	EUsbPacket_Setup_Request_SetReport     = 0x09,
	EUsbPacket_Setup_Request_SetIdle       = 0x0A,
	EUsbPacket_Setup_Request_SetProtocol   = 0x0B

} EUsbPacket_Setup_Request;
/*

// Define device states
#define  DEV_ATTACHED            0x00  // Device is in Attached State
#define  DEV_POWERED             0x01  // Device is in Powered State
#define  DEV_DEFAULT             0x02  // Device is in Default State
#define  DEV_ADDRESS             0x03  // Device is in Addressed State
#define  DEV_CONFIGURED          0x04  // Device is in Configured State
#define  DEV_SUSPENDED           0x05  // Device is in Suspended State

// Define bmRequestType bitmaps
#define  IN_DEVICE               0x00  // Request made to device,
                                       // direction is IN
#define  OUT_DEVICE              0x80  // Request made to device,
                                       // direction is OUT
#define  IN_INTERFACE            0x01  // Request made to interface,
                                       // direction is IN
#define  OUT_INTERFACE           0x81  // Request made to interface,
                                       // direction is OUT
#define  IN_ENDPOINT             0x02  // Request made to endpoint,
                                       // direction is IN
#define  OUT_ENDPOINT            0x82  // Request made to endpoint,
                                       // direction is OUT

// Define wIndex bitmaps
#define  IN_EP1                  0x81  // Index values used by Set and Clear
                                       // feature
#define  OUT_EP1                 0x01  // commands for Endpoint_Halt
#define  IN_EP2                  0x82
#define  OUT_EP2                 0x02

// Define wValue bitmaps for Standard Feature Selectors
#define  DEVICE_REMOTE_WAKEUP    0x01  // Remote wakeup feature(not used)
#define  ENDPOINT_HALT           0x00  // Endpoint_Halt feature selector

// Define Endpoint States
#define  EP_IDLE                 0x00  // This signifies Endpoint Idle State
#define  EP_TX                   0x01  // Endpoint Transmit State
#define  EP_RX                   0x02  // Endpoint Receive State
#define  EP_HALT                 0x03  // Endpoint Halt State (return stalls)
#define  EP_STALL                0x04  // Endpoint Stall (send procedural stall
                                       // next status phase)
#define  EP_ADDRESS              0x05  // Endpoint Address (change FADDR during
                                       // next status phase)
#define  EP_GetReport            0x06  // Special Control Endpoint State for
                                       // GetReport HID Request
#define  EP_SetReport            0x07  // Special Control Endpoint State for
                                       // SetReport HID Request
*/            

//////////////////////////////////////////////////
//////////////////////////////////////////////////
// Functions & macros

#define USB_GetWordValue(_UsbWord) ( _UsbWord.LeastSignificantByte + 0xFF * _UsbWord.MostSignificantByte)

U8 USB_ReadRegister(U8 _UsbAddress) {
	while (USB0ADR & USB0ADR_BUSY);
	USB0ADR = (USB0ADR_BUSY | _UsbAddress);
    while (USB0ADR & USB0ADR_BUSY); 
	return USB0DAT;
}

void USB_WriteRegister(U8 _UsbAddress, U8 _Value) {
	while (USB0ADR & USB0ADR_BUSY);
	USB0ADR = (USB0ADR_BUSY | _UsbAddress);
    USB0DAT = _Value;
}

void USB_ReadEndpointFifo (U8 _UsbAddress, U8* _Data, U16 _Length)
{
	U16 I;

	if (!_Length)
		return;
      
	while (USB0ADR & USB0ADR_BUSY);
	USB0ADR = _UsbAddress | USB0ADR_AUTORD | USB0ADR_BUSY;       
	for(I=0;I< (_Length);I++)
	{
		while (USB0ADR & USB0ADR_BUSY); 
		_Data[I] = USB0DAT;           	
	}        
}

void USB_WriteEndpointFifo (U8 _UsbAddress, U8* _Data, U16 _Length)
{
	U16 i;

	if (!_Length)
		return;
       
	while (USB0ADR & USB0ADR_BUSY);
	USB0ADR = _UsbAddress | USB0ADR_AUTORD | USB0ADR_BUSY;
	for (i=0; i<_Length; i++)
	{
		while (USB0ADR & USB0ADR_BUSY);  
		USB0DAT = _Data[i];
	}
}
/*
#define USB_STRING_GET_17TH(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16, N, ...) N
#define USB_STRING_NARGS(...) USB_STRING_GET_17TH(__VA_ARGS__, 16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1)

#define USB_STRING_ADD_ZERO_01(_X) _X, 0 
#define USB_STRING_ADD_ZERO_02(_X, ...) USB_STRING_ADD_ZERO_01(_X), USB_STRING_ADD_ZERO_01(__VA_ARGS__) 
#define USB_STRING_ADD_ZERO_03(_X, ...) USB_STRING_ADD_ZERO_01(_X), USB_STRING_ADD_ZERO_02(__VA_ARGS__) 
#define USB_STRING_ADD_ZERO_04(_X, ...) USB_STRING_ADD_ZERO_01(_X), USB_STRING_ADD_ZERO_03(__VA_ARGS__) 
#define USB_STRING_ADD_ZERO_05(_X, ...) USB_STRING_ADD_ZERO_01(_X), USB_STRING_ADD_ZERO_04(__VA_ARGS__) 
#define USB_STRING_ADD_ZERO_06(_X, ...) USB_STRING_ADD_ZERO_01(_X), USB_STRING_ADD_ZERO_05(__VA_ARGS__) 
#define USB_STRING_ADD_ZERO_07(_X, ...) USB_STRING_ADD_ZERO_01(_X), USB_STRING_ADD_ZERO_06(__VA_ARGS__) 
#define USB_STRING_ADD_ZERO_08(_X, ...) USB_STRING_ADD_ZERO_01(_X), USB_STRING_ADD_ZERO_07(__VA_ARGS__) 
#define USB_STRING_ADD_ZERO_09(_X, ...) USB_STRING_ADD_ZERO_01(_X), USB_STRING_ADD_ZERO_08(__VA_ARGS__) 
#define USB_STRING_ADD_ZERO_10(_X, ...) USB_STRING_ADD_ZERO_01(_X), USB_STRING_ADD_ZERO_09(__VA_ARGS__) 
#define USB_STRING_ADD_ZERO_11(_X, ...) USB_STRING_ADD_ZERO_01(_X), USB_STRING_ADD_ZERO_10(__VA_ARGS__) 
#define USB_STRING_ADD_ZERO_12(_X, ...) USB_STRING_ADD_ZERO_01(_X), USB_STRING_ADD_ZERO_11(__VA_ARGS__) 
#define USB_STRING_ADD_ZERO_13(_X, ...) USB_STRING_ADD_ZERO_01(_X), USB_STRING_ADD_ZERO_12(__VA_ARGS__) 
#define USB_STRING_ADD_ZERO_14(_X, ...) USB_STRING_ADD_ZERO_01(_X), USB_STRING_ADD_ZERO_13(__VA_ARGS__) 
#define USB_STRING_ADD_ZERO_15(_X, ...) USB_STRING_ADD_ZERO_01(_X), USB_STRING_ADD_ZERO_14(__VA_ARGS__) 
#define USB_STRING_ADD_ZERO_16(_X, ...) USB_STRING_ADD_ZERO_01(_X), USB_STRING_ADD_ZERO_15(__VA_ARGS__) 

#define USB_STRING_ADD_ZEROES(...) USB_STRING_GET_17TH(__VA_ARGS__,USB_STRING_ADD_ZERO_16,USB_STRING_ADD_ZERO_15,USB_STRING_ADD_ZERO_14,USB_STRING_ADD_ZERO_13,USB_STRING_ADD_ZERO_12,USB_STRING_ADD_ZERO_11,USB_STRING_ADD_ZERO_10,USB_STRING_ADD_ZERO_09,USB_STRING_ADD_ZERO_08,USB_STRING_ADD_ZERO_07,USB_STRING_ADD_ZERO_06,USB_STRING_ADD_ZERO_05,USB_STRING_ADD_ZERO_04,USB_STRING_ADD_ZERO_03,USB_STRING_ADD_ZERO_02,USB_STRING_ADD_ZERO_01)(__VA_ARGS__)

#define USB_STRING(...) { (USB_STRING_NARGS(__VA_ARGS__) + 1)*2, 0x03, USB_STRING_ADD_ZEROES(__VA_ARGS__)}
*/
#endif
