/* ########################################################################

   USBIP hardware emulation 

   ########################################################################

   Copyright (c) : 2016  Luis Claudio Gambôa Lopes

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

   For e-mail suggestions :  lcgamboa@yahoo.com
   ######################################################################## */

#include "vstub.h"

/* Device Descriptor */
const USB_DEVICE_DESCRIPTOR dev_dsc=
{
    0x12,                   // Size of this descriptor in bytes
    0x01,                   // DEVICE descriptor type
    0x0110,                 // USB Spec Release Number in BCD format
    0x00,                   // Class Code
    0x00,                   // Subclass code
    0x00,                   // Protocol code
    0x08,                   // Max packet size for EP0, see usb_config.h
    0x2706,                 // Vendor ID
    0x0100,                 // Product ID: Mouse in a circle fw demo
    0x0000,                 // Device release number in BCD format
    0x00,                   // Manufacturer string index
    0x00,                   // Product string index
    0x00,                   // Device serial number string index
    0x01                    // Number of possible configurations
};

/* Configuration 1 Descriptor */
const CONFIG_HID  configuration_hid={{
    /* Configuration Descriptor */
    0x09,//sizeof(USB_CFG_DSC),    // Size of this descriptor in bytes
    USB_DESCRIPTOR_CONFIGURATION,                // CONFIGURATION descriptor type
    0x0022,                 // Total length of data for this cfg
    1,                      // Number of interfaces in this cfg
    1,                      // Index value of this configuration
    0,                      // Configuration string index
    0x80,      
    50,                     // Max power consumption (2X mA)
    },{ 
    /* Interface Descriptor */
    0x09,//sizeof(USB_INTF_DSC),   // Size of this descriptor in bytes
    USB_DESCRIPTOR_INTERFACE,               // INTERFACE descriptor type
    0,                      // Interface Number
    0,                      // Alternate Setting Number
    1,                      // Number of endpoints in this intf
    0x03,                   // Class code
    0x01,                   // Subclass code
    0x01,                   // Protocol code
    0,                      // Interface string index
    },{
    /* HID Class-Specific Descriptor */
    0x09,               // Size of this descriptor in bytes RRoj hack
    0x21,                // HID descriptor type
    0x0001,                 // HID Spec Release Number in BCD format (1.11)
    0x00,                   // Country Code (0x00 for Not supported)
    0x01,         // Number of class descriptors, see usbcfg.h
    0x22,                // Report descriptor type
    0x003F,           // Size of the report descriptor
    },{
    /* Endpoint Descriptor */
    0x07,/*sizeof(USB_EP_DSC)*/
    USB_DESCRIPTOR_ENDPOINT,    //Endpoint Descriptor
    0x81,            //EndpointAddress
    0x03,                       //Attributes
    0x0008,                  //size
    0xFF                        //Interval
}};

const char *configuration = (const char *)&configuration_hid; 
const USB_INTERFACE_DESCRIPTOR *interfaces[]={ &configuration_hid.dev_int };
const unsigned char *strings[]={};
const USB_DEVICE_QUALIFIER_DESCRIPTOR  dev_qua={};


//Class specific descriptor - HID keyboard
const byte keyboard_report[0x3F]={
               0x05, 0x01, 
	       0x09, 0x06,		//Usage Page (Generic Desktop),
	       0xA1, 0x01,		//Usage (Keyboard),
	       0x05, 0x07, 		//Collection (Application),
	       0x19, 0xE0, 		//Usage Page (Key Codes);
	       0x29, 0xE7, 		//Usage Minimum (224),
	       0x15, 0x00, 		//Usage Maximum (231),
	       0x25, 0x01, 		//Logical Minimum (0),
	       0x75, 0x01, 		//Logical Maximum (1),
	       0x95, 0x08, 		//Report Size (1),
	       0x81, 0x02, 		//Report Count (8),
	       0x95, 0x01, 		//Input (Data, Variable, Absolute),
	       0x75, 0x08, 		//Report Count (1),
	       0x81, 0x01, 		//Report Size (8),
	       0x95, 0x05, 		//Input (Constant),
	       0x75, 0x01, 		//Report Count (5),
	       0x05, 0x08, 		//Report Size (1),
	       0x19, 0x01, 		//Usage Page (Page# for LEDs),
	       0x29, 0x05, 		//Usage Minimum (1),
	       0x91, 0x02, 		//Usage Maximum (5),
	       0x95, 0x01, 		//Output (Data, Variable, Absolute),
	       0x75, 0x03, 		//Report Count (1),
	       0x91, 0x01, 		//Report Size (3),
	       0x95, 0x06, 		//Output (Constant),
	       0x75, 0x08, 		//Report Count (6),
	       0x15, 0x00, 		//Report Size (8),
	       0x25, 0x65, 		//Logical Minimum (0),
	       0x05, 0x07, 		//Logical Maximum(101),
	       0x19, 0x00, 		//Usage Page (Key Codes),
	       0x29, 0x65, 		//Usage Minimum (0),
	       0x81, 0x00, 		//Usage Maximum (101), #Input (Data, Array),
	       0xC0};  			//End Collection 


void
handle_data(vstub_t *vstub, USBIP_RET_SUBMIT *ret_submit)
{
        // Sending random keyboard data
        // Send data only for 5 seconds
         static int  count=0;
         char return_val[8];

         printf("data\n");
         memset(return_val, 0, 8);
         if (count < 20) {
		 if((count % 2 ) == 0)
			 return_val[2] = (char)((((25l*rand())/RAND_MAX))+4);
		 send_ret_submit(vstub, ret_submit, return_val, 4, 0);
         } 
         usleep(250000);
         count=count+1;
};

void
handle_unknown_control(vstub_t *vstub, setup_pkt_t *setup_pkt, USBIP_RET_SUBMIT *ret_submit)
{
        if(setup_pkt->bmRequestType == 0x81) { 
		if (setup_pkt->bRequest == 0x6) {
			if (setup_pkt->wValue1 == 0x22) { // send initial report
				printf("send initial report\n");
				send_ret_submit(vstub, ret_submit, (char *)keyboard_report, 0x3F, 0);
			}
		} 
        } 
        if (setup_pkt->bmRequestType == 0x21) { // Host Request
		if (setup_pkt->bRequest == 0x0a) { // set idle
			printf("Idle\n");
			// Idle
			send_ret_submit(vstub, ret_submit, "", 0, 0);
		}
		if (setup_pkt->bRequest == 0x09) { // set report
			printf("set report\n");
			char data[20];
			if (!(recv_data(vstub, data, setup_pkt->wLength))) {
				printf ("receive error : %s \n", strerror (errno));
				return;
			}
			send_ret_submit(vstub, ret_submit,"", 0, 0);
		}
        }
}

int
main(void)
{
	printf("hid keyboard started....\n");
	usbip_run(&dev_dsc);
}

