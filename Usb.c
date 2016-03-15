#include "Usb.h"



void USB_ReadEndpointFifo (U8 _UsbAddress, U8* _Data, U16 _Length)
{
	U16 I;

	if (!_Length)
		return;
      
	while (USB0ADR & USB0ADR_BUSY);
	USB0ADR = _UsbAddress | USB0ADR_BUSY | USB0ADR_AUTORD;       
	for(I=0;I< (_Length);I++)
	{
		while (USB0ADR & USB0ADR_BUSY); 
		_Data[I] = USB0DAT;         
	} 
	// clear auto read
	USB0ADR = 0;          
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