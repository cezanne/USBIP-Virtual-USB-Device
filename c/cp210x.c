#include "vstub.h"

//CP210x Configuration
typedef struct __attribute__ ((__packed__)) _CONFIG_CP210x
{
	USB_CONFIGURATION_DESCRIPTOR	dev_conf;
	USB_INTERFACE_DESCRIPTOR	dev_int;
	USB_ENDPOINT_DESCRIPTOR		dev_ep1, dev_ep2, dev_ep3;
} CONFIG_BTH;

/* Device Descriptor */
const USB_DEVICE_DESCRIPTOR dev_dsc = {
	0x12,                   // Size of this descriptor in bytes
	0x01,                   // DEVICE descriptor type
	0x0110,                 // USB Spec Release Number in BCD format
	0x00,                   // Class Code
	0x00,                   // Subclass code
	0x00,                   // Protocol code
	0x40,                   // Max packet size for EP0, see usb_config.h
	0x10c4,                 // Vendor ID
	0xea60,                 // Product ID: Mouse in a circle fw demo
	0x0100,                 // Device release number in BCD format
	0x01,                   // Manufacturer string index
	0x02,                   // Product string index
	0x03,                   // Device serial number string index
	0x01                    // Number of possible configurations
};

/* Configuration 1 Descriptor */
const char	configuration_cp210x[] = {
	0x09, 0x02, 0x20, 0x00, 0x01, 0x01, 0x00, 0x80,
	0x32, 0x09, 0x04, 0x00, 0x00, 0x02, 0xff, 0x00, 
	0x00, 0x02, 0x07, 0x05, 0x81, 0x02, 0x40, 0x00, 
	0x00, 0x07, 0x05, 0x01, 0x02, 0x40, 0x00, 0x00
};

const unsigned char string_2[] = {
	0x4a,
	USB_DESCRIPTOR_STRING,
	0x43, 0x00, 0x50, 0x00, 0x32, 0x00, 0x31, 0x00, 0x30, 0x00, 0x32, 0x00,
	0x20, 0x00, 0x55, 0x00, 0x53, 0x00, 0x42, 0x00, 0x20, 0x00, 0x74, 0x00,
	0x6f, 0x00, 0x20, 0x00, 0x55, 0x00, 0x41, 0x00, 0x52, 0x00, 0x54, 0x00,
	0x20, 0x00, 0x42, 0x00, 0x72, 0x00, 0x69, 0x00, 0x64, 0x00, 0x67, 0x00,
	0x65, 0x00, 0x20, 0x00, 0x43, 0x00, 0x6f, 0x00, 0x6e, 0x00, 0x74, 0x00,
	0x72, 0x00, 0x6f, 0x00, 0x6c, 0x00, 0x6c, 0x00, 0x65, 0x00, 0x72, 0x00
};

const unsigned char string_3[] = {
	0x0a,
	USB_DESCRIPTOR_STRING,
	0x30, 0x00, 0x30, 0x00, 0x30, 0x00, 0x31, 0x00
};

const char	*configuration = (char *)configuration_cp210x;
const USB_INTERFACE_DESCRIPTOR	*interfaces[] = { };
const unsigned char	*strings[] = { NULL, NULL, string_2, string_3 };
const USB_DEVICE_QUALIFIER_DESCRIPTOR	dev_qua = {};

static char	control_reply[] = {
	0x42, 0x00, 0x00, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00,
	0x00, 0x02, 0x00, 0x00, 0xc0, 0xc6, 0x2d, 0x00, 0x01, 0x00, 0x00, 0x00, 0x3f, 0x01, 0x00, 0x00,
	0x7f, 0x00, 0x00, 0x00, 0xf0, 0xff, 0x07, 0x10, 0x0f, 0x00, 0x07, 0x1f, 0x00, 0x02, 0x00, 0x00,
	0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x33, 0x00, 0x2e, 0x00,
	0x30, 0x00
};

void
handle_non_control_transfer(vstub_t *vstub, USBIP_CMD_SUBMIT *cmd_submit)
{
	if (cmd_submit->ep == 1 && cmd_submit->direction) {
		reply_cmd_submit(vstub, cmd_submit, NULL, 0);
		/* bulk transfers are requested indefinitely */
		usleep(300000);
	}
	else {
		error("unhandled non-control\n");
	}
}

void
handle_control_transfer(vstub_t *vstub, USBIP_CMD_SUBMIT *cmd_submit)
{
	setup_pkt_t	*setup_pkt = (setup_pkt_t *)cmd_submit->setup;

        if (setup_pkt->bmRequestType == 0xc0 &&
	    setup_pkt->bRequest == 0xff &&
	    setup_pkt->wValue.W == 0x370b) {
		char	reply = 0x02;

		reply_cmd_submit(vstub, cmd_submit, &reply, 1);
	}
	else if (setup_pkt->bmRequestType == 0xc1 &&
		 setup_pkt->bRequest == 0x0f &&
		 setup_pkt->wValue.W == 0x0000) {
		reply_cmd_submit(vstub, cmd_submit, control_reply, 1);
	}
	else if (setup_pkt->bmRequestType == 0x41 &&
		 setup_pkt->bRequest == 0x03 &&
		 setup_pkt->wValue.W == 0x0800) {
		reply_cmd_submit(vstub, cmd_submit, NULL, 0);
	}
	else if (setup_pkt->bmRequestType == 0x41 &&
		 (setup_pkt->bRequest == 0x13 || setup_pkt->bRequest == 0x19 ||
		  setup_pkt->bRequest == 0x1e) &&
		 setup_pkt->wValue.W == 0x0000) {
		char	data[256];
		printf("  recv %d\n", cmd_submit->transfer_buffer_length);
		if (setup_pkt->bRequest == 0x1e) {
			/* in this case, some pause makes a client to proceed */
			usleep(400000);
		}
		if (!recv_data(vstub, data, cmd_submit->transfer_buffer_length)) {
			error("failed to recv payload data\n");
			return;
		}
		reply_cmd_submit(vstub, cmd_submit, NULL, 0);
	}
	else {
		error("unhandled control transfer\n");
	}
}

int
main(void)
{
	printf("cp210x started....\n");
	usbip_run(&dev_dsc);
}
