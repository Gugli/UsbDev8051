#ifndef USB_H
#define USB_H

#include "compiler_defs.h"
#include "C8051F320_defs.h"

typedef struct
{
   U8				LeastSignificantByte;
   U8				MostSignificantByte;
} TUsbWord;


#define MUsbWord_Bytes(_MSB, _LSB) {0x##_LSB, 0x##_MSB}
#define MUsbWord_Word(_WORD) {(_WORD & 0xFF), ((_WORD >> 8) & 0xFF)}

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
	EUsbDescriptorType_String = 0x03,
	EUsbDescriptorType_Interface = 0x04,
	EUsbDescriptorType_Endpoint = 0x05,
	EUsbDescriptorType_HIDClass = 0x21,
	EUsbDescriptorType_HIDReport = 0x22 
} EUsbDescriptorType;

typedef enum 
{
	EUsbSpecVer_1d1 = 0x0110,	// USB 1.1
	EUsbSpecVer_2 = 0x0200,		// USB 2
	EUsbSpecVer_3 = 0x0300		// USB 3
} EUsbSpecVer;

typedef enum 
{
	EUsbHIDSpecVer_1d01 = 0x0101,	// USB 1.01
} EUsbHIDSpecVer;

typedef enum 
{
	EUsbVendorId_Prototype = 0x6666,
	EUsbVendorId_SciLabs = 0x10C4
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
	EUsbInterface_SubClass_HID_NoBootInterface = 0x00,
	EUsbInterface_SubClass_HID_BootInterface = 0x01,
} EUsbInterface_SubClass;

typedef enum
{
	EUsbInterface_Protocol_HID_NoBootInterface = 0x00,
	EUsbInterface_Protocol_HID_BootInterface_Keyboard = 0x01,
	EUsbInterface_Protocol_HID_BootInterface_Mouse = 0x02,
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
   TUsbByte							SubordinateDescriptorsCount;
   TUsbByte							DescriptorType; 
   TUsbWord							DescriptorLength;
} SUsbDescriptor_HIDClass;              

typedef struct
{
   SUsbDescriptorHeader				Header; 
   TUsbByte EndpointAddress;		// Endpoint Address (Number + Direction)
   TUsbByte Attributes;				// Endpoint Attributes (Transfer Type)
   TUsbWord MaxPacketSize;          // Max. Endpoint Packet Size
   TUsbByte PollingInterval;        // In ms
} SUsbDescriptor_Endpoint;              

typedef enum
{
	EUsbDescriptor_HIDClass_CountryCode_Default = 0x00,
} EUsbDescriptor_HIDClass_CountryCode;

typedef unsigned char const * const SUsbDescriptor_Report;

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
	// Masks
	EUsbPacket_Setup_RequestType_DirMask         = 0x80,
	EUsbPacket_Setup_RequestType_TypeMask        = 0x60,
	EUsbPacket_Setup_RequestType_RecipientMask   = 0x1F,

	EUsbPacket_Setup_RequestType_Dir_Out		 = 0x00, // Host to device
	EUsbPacket_Setup_RequestType_Dir_In  		 = 0x80, // Device to host

	EUsbPacket_Setup_RequestType_Type_Standard   = 0x00,
	EUsbPacket_Setup_RequestType_Type_Class 	 = 0x20,
	EUsbPacket_Setup_RequestType_Type_Product 	 = 0x40,

	EUsbPacket_Setup_RequestType_Recipient_Device    = 0x00,
	EUsbPacket_Setup_RequestType_Recipient_Interface = 0x01,
	EUsbPacket_Setup_RequestType_Recipient_Endpoint	 = 0x02,
	EUsbPacket_Setup_RequestType_Recipient_Element 	 = 0x03,

} EUsbPacket_Setup_RequestType;

typedef enum
{
	// Standard requests
	EUsbPacket_Setup_Request_GetStatus     = 0x00,
	EUsbPacket_Setup_Request_ClearFeature  = 0x01,
	EUsbPacket_Setup_Request_SetFeature    = 0x03, // or 0x02 ???
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


typedef enum
{
	EUsbConfiguration_Attributes_SelfPowered = 0x40,
	EUsbConfiguration_Attributes_HostPowered = 0x00,

	EUsbConfiguration_Attributes_RemoteWakeupOn = 0x20,
	EUsbConfiguration_Attributes_RemoteWakeupOff = 0x00,

	EUsbConfiguration_Attributes_MustWrite = 0x80,

} EUsbConfiguration_Attributes;

typedef enum
{
	EUsbEndpoint_Attributes_Control = 0x00,
	EUsbEndpoint_Attributes_Isochronous = 0x01,
	EUsbEndpoint_Attributes_Bulk = 0x10,
	EUsbEndpoint_Attributes_Interrupt = 0x11,

} EUsbEndpoint_Attributes;

typedef enum
{
	EUsbEndpoint_Address_EP1 = 0x01,
	EUsbEndpoint_Address_EP2 = 0x02,
	EUsbEndpoint_Address_EP3 = 0x03,

	EUsbEndpoint_Address_In  = 0x80,
	EUsbEndpoint_Address_Out = 0x00,

} EUsbEndpoint_Address;


//////////////////////////////////////////////////
//////////////////////////////////////////////////
// Functions & macros

#define USB_Word_GetValue(_UsbWord) ( _UsbWord.LeastSignificantByte + 0x0100 * _UsbWord.MostSignificantByte)
#define USB_Word_IsEqual(_UsbWord, _MSB, _LSB) (( _UsbWord.LeastSignificantByte == _LSB) && ( _UsbWord.MostSignificantByte == _MSB))
#define USB_Word_IsNotEqual(_UsbWord, _MSB, _LSB) (( _UsbWord.LeastSignificantByte != _LSB) || ( _UsbWord.MostSignificantByte != _MSB))


#define USB_ReadRegister( ___Result, __UsbAddress) do { \
	while (USB0ADR & USB0ADR_BUSY);                     \
	USB0ADR = (USB0ADR_BUSY | __UsbAddress);            \
    while (USB0ADR & USB0ADR_BUSY);                     \
	___Result = USB0DAT;                                \
} while(0)                     	 

#define USB_WriteRegister( __UsbAddress, __Value) do { \
	while (USB0ADR & USB0ADR_BUSY);                    \
	USB0ADR = (__UsbAddress);          				   \
    USB0DAT = (__Value);                               \
} while(0)                 

void USB_ReadEndpointFifo (U8 _UsbAddress, U8* _Data, U8 _Length);
void USB_WriteEndpointFifo (U8 _UsbAddress, U8* _Data, U8 _Length);

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
