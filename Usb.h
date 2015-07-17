#ifndef USB_H
#define USB_H

#include "compiler_defs.h"

typedef struct
{
   U8				LeastSignificantByte;
   U8				MostSignificantByte;
} TUsbWord;

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


#endif
