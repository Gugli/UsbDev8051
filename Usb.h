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
	EUsbDescriptorType_Endpoint = 0x05,
	EUsbDescriptorType_Class_HID = 0x21
} EUsbDescriptorType;

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

typedef enum
{
	EUsbClass_ReportDescriptorType_HID = 0x22 
} EUsbClass_ReportDescriptorType;

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

typedef unsigned char const * const * const SUsbDescriptor_StringTable;

#endif
