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
const USB_DEVICE_DESCRIPTOR dev_dsc = {
	0x12,                   // Size of this descriptor in bytes
	0x01,                   // DEVICE descriptor type
	0x0110,                 // USB Spec Release Number in BCD format
	0x00,                   // Class Code
	0x00,                   // Subclass code
	0x00,                   // Protocol code
	0x08,                   // Max packet size for EP0, see usb_config.h
	0x2706,                 // Vendor ID
	0x0000,                 // Product ID: Mouse in a circle fw demo
	0x0000,                 // Device release number in BCD format
	0x00,                   // Manufacturer string index
	0x00,                   // Product string index
	0x00,                   // Device serial number string index
	0x01                    // Number of possible configurations
};

/* Configuration 1 Descriptor */
static const CONFIG_HID configuration_hid = {
	{
		/* Configuration Descriptor */
		0x09,//sizeof(USB_CFG_DSC),    // Size of this descriptor in bytes
		USB_DESCRIPTOR_CONFIGURATION,                // CONFIGURATION descriptor type
		0x0022,                 // Total length of data for this cfg
		1,                      // Number of interfaces in this cfg
		1,                      // Index value of this configuration
		0,                      // Configuration string index
		0x80,                   //Configuration characteristics
		50                      // Max power consumption (2X mA)
	},
	{
		/* Interface Descriptor */
		0x09,//sizeof(USB_INTF_DSC),   // Size of this descriptor in bytes
		USB_DESCRIPTOR_INTERFACE,               // INTERFACE descriptor type
		0,                      // Interface Number
		0,                      // Alternate Setting Number
		1,                      // Number of endpoints in this intf
		0x03,                   // Class code
		0x01,                   // Subclass code
		0x02,                   // Protocol code
		0x00                    // Interface string index
	},
	{
		/* HID Class-Specific Descriptor */
		0x09,                   // Size of this descriptor in bytes RRoj hack
		0x21,                   // HID descriptor type
		0x0001,                 // HID Spec Release Number in BCD format (1.11)
		0x00,                   // Country Code (0x00 for Not supported)
		0x01,                   // Number of class descriptors, see usbcfg.h
		0x22,                   // Report descriptor type
		0x0034                  // Size of the report descriptor
	},
	{
		/* Endpoint Descriptor */
		0x07,/*sizeof(USB_EP_DSC)*/
		USB_DESCRIPTOR_ENDPOINT,    //Endpoint Descriptor
		0x81,                   //EndpointAddress
		0x03,                   //Attributes
		0x0008,                 //size
		0xFF                    //Interval
	}
};

////DEL static const USB_INTERFACE_DESCRIPTOR	*interfaces[] = { &configuration_hid.dev_int };
static const unsigned char	*strings[] = {};
static const USB_DEVICE_QUALIFIER_DESCRIPTOR	dev_qua = {};

//Class specific descriptor - HID mouse
const byte mouse_report[0x34] = {
	0x05, 0x01, /* Usage Page (Generic Desktop)             */
	0x09, 0x02, /* Usage (Mouse)                            */
	0xA1, 0x01, /* Collection (Application)                 */
	0x09, 0x01, /*  Usage (Pointer)                         */
	0xA1, 0x00, /*  Collection (Physical)                   */
	0x05, 0x09, /*      Usage Page (Buttons)                */
	0x19, 0x01, /*      Usage Minimum (01)                  */
	0x29, 0x03, /*      Usage Maximum (03)                  */
	0x15, 0x00, /*      Logical Minimum (0)                 */
	0x25, 0x01, /*      Logical Maximum (1)                 */
	0x95, 0x03, /*      Report Count (3)                    */
	0x75, 0x01, /*      Report Size (1)                     */
	0x81, 0x02, /*      Input (Data, Variable, Absolute)    */
	0x95, 0x01, /*      Report Count (1)                    */
	0x75, 0x05, /*      Report Size (5)                     */
	0x81, 0x01, /*      Input (Constant)    ;5 bit padding  */
	0x05, 0x01, /*      Usage Page (Generic Desktop)        */
	0x09, 0x30, /*      Usage (X)                           */
	0x09, 0x31, /*      Usage (Y)                           */
	0x09, 0x38, /*      Logical Minimum (-127)              */
	0x15, 0x81, /*      Logical Maximum (127)               */
	0x25, 0x7F, /*      Report Size (8)                     */
	0x75, 0x08, /*      Report Count (3)                    */
	0x95, 0x03, /*      Input (Data, Variable, Relative)    */
	0x81, 0x06,
	0xC0, 0xC0
};

static BOOL
handle_non_control_transfer(vstub_t *vstub, USBIP_CMD_SUBMIT *cmd_submit)
{
        // Sending random mouse data
        // Send data only for 5 seconds
	static int  count = 0;
	char	return_val[4];

	printf("data\n");
	if (count < 20) {
		return_val[0] = 0;
		return_val[1] = (char)((((10l * rand()) / RAND_MAX)) - 5);
		return_val[2] = (char)((((10l * rand()) / RAND_MAX)) - 5);
		return_val[3] = 0;
		reply_cmd_submit(vstub, cmd_submit, return_val, 4);
		usleep(250000);
	}
	count++;
	return TRUE;
}

static BOOL
handle_control_transfer(vstub_t *vstub, USBIP_CMD_SUBMIT *cmd_submit)
{
	setup_pkt_t	*setup_pkt = (setup_pkt_t *)cmd_submit->setup;

        if (setup_pkt->bmRequestType == 0x81 &&
	    setup_pkt->bRequest == 0x6 &&
	    setup_pkt->wValue.hiByte == 0x22) {
		// Get Descriptor
		// send initial report
		printf("send initial report\n");
		reply_cmd_submit(vstub, cmd_submit, (char *)mouse_report, 0x34);
		return TRUE;
	}
        if (setup_pkt->bmRequestType == 0x21 &&
	    setup_pkt->bRequest == 0x0a) {
		// Host Request
		// set idle
		printf("Idle\n");
		// Idle
		reply_cmd_submit(vstub, cmd_submit, NULL, 0);
		return TRUE;
	}
	return FALSE;
}

vstubmod_t	vstubmod_hid_mouse = {
	"hid-mouse",
	"USB mouse",
	&dev_dsc,
	&dev_qua,
	(CONFIG_GEN *)&configuration_hid,
	strings,
	NULL,
	handle_control_transfer,
	handle_non_control_transfer
};
