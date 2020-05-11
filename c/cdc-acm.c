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
static const USB_DEVICE_DESCRIPTOR dev_dsc=
{
	0x12,                   // Size of this descriptor in bytes
	0x01,                   // DEVICE descriptor type
	0x0002,                 // USB Spec Release Number in BCD format
	0x00,                   // Class Code
	0x00,                   // Subclass code
	0x00,                   // Protocol code
	0x40,                   // Max packet size for EP0, see usb_config.h
	0x8304,                 // Vendor ID
	0x4057,                 // Product ID
	0x0002,                 // Device release number in BCD format
	0x01,                   // Manufacturer string index
	0x02,                   // Product string index
	0x03,                   // Device serial number string index
	0x01                    // Number of possible configurations
};

static const USB_DEVICE_QUALIFIER_DESCRIPTOR dev_qua = {
	0x0A,       // bLength
	0x06,       // bDescriptorType
	0x0002,     // bcdUSB
	0x00,       // bDeviceClass
	0x00,       // bDeviceSubClass
	0x00,       // bDeviceProtocol
	0x40,       // bMaxPacketSize
	0x01,       // iSerialNumber
	0x00        //bNumConfigurations*/
};

/* Configuration 1 Descriptor */
static const CONFIG_CDC  configuration_cdc = {
	{
		/* Configuration Descriptor */
		0x09,//sizeof(USB_CFG_DSC),    // Size of this descriptor in bytes
		USB_DESCRIPTOR_CONFIGURATION,                // CONFIGURATION descriptor type
		sizeof(CONFIG_CDC),                 // Total length of data for this cfg
		2,                      // Number of interfaces in this cfg
		1,                      // Index value of this configuration
		0,                      // Configuration string index
		0xC0,
		50,                     // Max power consumption (2X mA)
	},
	{
		/* Interface Descriptor */
		0x09,//sizeof(USB_INTF_DSC),   // Size of this descriptor in bytes
		USB_DESCRIPTOR_INTERFACE,               // INTERFACE descriptor type
		0,                      // Interface Number
		0,                      // Alternate Setting Number
		1,                      // Number of endpoints in this intf
		0x02,                   // Class code
		0x02,                   // Subclass code
		0x01,                   // Protocol code
		0                       // Interface string index
	},
	{
		/* CDC Class-Specific Descriptors */
		/* Header Functional Descriptor */
		0x05,                   // bFNLength
		0x24,                   // bDscType
		0x00,                   // bDscSubType
		0x1001                  // bcdCDC
	},
	{
		/* Call Management Functional Descriptor */
		0x05,                   // bFNLength
		0x24,                   // bDscType
		0x01,                   // bDscSubType
		0x00,                   // bmCapabilities
		0x01                    // bDataInterface
	},
	{
		/* Abstract Control Management Functional Descriptor */
		0x04,                   // bFNLength
		0x24,                   // bDscType
		0x02,                   // bDscSubType
		0x02                    // bmCapabilities
	},
	{
		/* Union Functional Descriptor */
		0x05,                   // bFNLength
		0x24,                   // bDscType
		0x06,                   // bDscSubType
		0x00,                   // bMasterIntf
		0x01                    // bSaveIntf0
	},
	{
		/* Endpoint Descriptor */
		0x07,/*sizeof(USB_EP_DSC)*/
		USB_DESCRIPTOR_ENDPOINT,    //Endpoint Descriptor
		0x82,                       //EndpointAddress
		0x03,                       //Attributes
		0x0800,                     //size
		0xFF                        //Interval
	},
	{
		/* Interface Descriptor */
		0x09,//sizeof(USB_INTF_DSC),   // Size of this descriptor in bytes
		USB_DESCRIPTOR_INTERFACE,               // INTERFACE descriptor type
		1,                      // Interface Number
		0,                      // Alternate Setting Number
		2,                      // Number of endpoints in this intf
		0x0A,                   // Class code
		0x00,                   // Subclass code
		0x00,                   // Protocol code
		0x00,                   // Interface string index
	},
	{
		/* Endpoint OUT Descriptor */
		0x07,/*sizeof(USB_EP_DSC)*/
		USB_DESCRIPTOR_ENDPOINT,    //Endpoint Descriptor
		0x01,                       //EndpointAddress
		0x02,                       //Attributes
		0x0400,                     //size
		0x00                        //Interval
	},
	{
		/* Endpoint IN Descriptor */
		0x07,/*sizeof(USB_EP_DSC)*/
		USB_DESCRIPTOR_ENDPOINT,    //Endpoint Descriptor
		0x81,                       //EndpointAddress
		0x02,                       //Attributes
		0x0400,                     //size
		0x00                        //Interval
	}
};

static const unsigned char string_0[] = { // available languages  descriptor
	0x04,
	USB_DESCRIPTOR_STRING,
	0x09,
	0x04
};

static const unsigned char string_1[] = { //
	0x0A,
	USB_DESCRIPTOR_STRING, // bLength, bDscType
	'T', 0x00, //
	'e', 0x00, //
	's', 0x00, //
	't', 0x00, //
};

static const unsigned char string_2[] = { //
	0x10,
	USB_DESCRIPTOR_STRING, //
	'U', 0x00, //
	'S', 0x00, //
	'B', 0x00, //
	' ', 0x00, //
	'C', 0x00, //
	'D', 0x00, //
	'C', 0x00, //
};

static const unsigned char string_3[] = { //
	0x18,
	USB_DESCRIPTOR_STRING, //
	'V', 0x00, //
	'i', 0x00, //
	'r', 0x00, //
	't', 0x00, //
	'u', 0x00, //
	'a', 0x00, //
	'l', 0x00, //
	' ', 0x00, //
	'U', 0x00, //
	'S', 0x00, //
	'B', 0x00, //
};

///DEL static const USB_INTERFACE_DESCRIPTOR *interfaces[]={ &configuration_cdc.dev_int0, &configuration_cdc.dev_int1};
static const unsigned char *strings[] = {string_0, string_1, string_2, string_3};

#define BSIZE 64

static char	buffer[BSIZE+1];
static int	bsize = 0;

static void
handle_non_control_transfer(vstub_t *vstub, USBIP_CMD_SUBMIT *cmd_submit)
{
	if (cmd_submit->ep == 0x01) {
		printf("EP1 received \n");

		if (cmd_submit->direction == 0) {
			//input
			printf("direction=input\n");
			if (!recv_data(vstub, (char *)buffer, cmd_submit->transfer_buffer_length))
				return;
			bsize = cmd_submit->transfer_buffer_length;
			reply_cmd_submit(vstub, cmd_submit, NULL, 0);
			buffer[bsize + 1] = 0; //string terminator
			printf("received (%s)\n",buffer);
		}
		else {
			printf("direction=output\n");
		}
		//not supported
		reply_cmd_submit(vstub, cmd_submit, NULL, 0);
		usleep(500);
	}
	else if (cmd_submit->ep == 0x02) {
		printf("EP2 received \n");

		if (cmd_submit->direction == 0) {
			//input
			int i;

			printf("direction=input\n");
			if (!recv_data(vstub, (char *)buffer, cmd_submit->transfer_buffer_length))
				return;
			bsize = cmd_submit->transfer_buffer_length;
			reply_cmd_submit(vstub, cmd_submit, NULL, 0);
			buffer[bsize + 1] = 0; //string terminator
			printf("received (%s)\n",buffer);
			for (i = 0; i < bsize; i++)
				printf("%02X", (unsigned char)buffer[i]);
			printf("\n");
		}
		else {
			printf("direction=output\n");

			if (bsize != 0) {
				int i;

				for (i = 0; i < bsize; i++) //increment received char
					buffer[i]++;

				reply_cmd_submit(vstub, cmd_submit, buffer, bsize);
				printf("sending (%s)\n", buffer);

				bsize = 0;
			}
			else {
				reply_cmd_submit(vstub, cmd_submit, NULL, 0);

				usleep(500);

				printf("no data disponible\n");
			}
		}
	}
}

typedef struct _LINE_CODING
{
	word dwDTERate;  //in bits per second
	byte bCharFormat;//0-1 stop; 1-1.5 stop; 2-2 stop bits
	byte ParityType; //0 none; 1- odd; 2 -even; 3-mark; 4 -space
	byte bDataBits;  //5,6,7,8 or 16
} LINE_CODING;

LINE_CODING linec;
unsigned short linecs = 0;

static void
handle_control_transfer(vstub_t *vstub, USBIP_CMD_SUBMIT *cmd_submit)
{
	setup_pkt_t	*setup_pkt = (setup_pkt_t *)cmd_submit->setup;

        if (setup_pkt->bmRequestType != 0x21)
		return;

	//Abstract Control Model Requests
	switch (setup_pkt->bRequest) {
	case 0x20:
		//SET_LINE_CODING
		printf("SET_LINE_CODING\n");
		if (!(recv_data(vstub, (char *)&linec, setup_pkt->wLength)))
			return;
		
		reply_cmd_submit(vstub, cmd_submit, NULL, 0);
		break;
	case 0x21:
		//GET_LINE_CODING
		printf("GET_LINE_CODING\n");
		reply_cmd_submit(vstub, cmd_submit, (char *)&linec, 7);
		break;
	case 0x22:
		//SET_LINE_CONTROL_STATE
		linecs = setup_pkt->wValue.lowByte;

		printf("SET_LINE_CONTROL_STATE 0x%02X\n", linecs);
		reply_cmd_submit(vstub, cmd_submit, NULL, 0);
		break;
	case 0x23:
		//SEND_BREAK
		printf("SEND_BREAK\n");

		reply_cmd_submit(vstub, cmd_submit, NULL, 0);
		break;
	default:
		break;
	}
}

vstubmod_t	vstubmod_cdc_acm = {
	"cdc",
	"CDC ACM",
	&dev_dsc,
	&dev_qua,
	(CONFIG_GEN *)&configuration_cdc,
	strings,
	NULL,
	handle_control_transfer,
	handle_non_control_transfer
};
