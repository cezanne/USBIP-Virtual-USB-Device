#include "vstub.h"

/* Device Descriptor */
static const USB_DEVICE_DESCRIPTOR dev_dsc = {
	0x12,                   // Size of this descriptor in bytes
	0x01,                   // DEVICE descriptor type
	0x1001,                 // USB Spec Release Number in BCD format
	0x00,                   // Class Code
	0x00,                   // Subclass code
	0x00,                   // Protocol code
	0x40,                   // Max packet size for EP0, see usb_config.h
	0xc410,                 // Vendor ID
	0x60ea,                 // Product ID: Mouse in a circle fw demo
	0x0001,                 // Device release number in BCD format
	0x01,                   // Manufacturer string index
	0x02,                   // Product string index
	0x03,                   // Device serial number string index
	0x01                    // Number of possible configurations
};

/* Configuration 1 Descriptor */
static const char	configuration_cp210x[] = {
	0x09, 0x02, 0x20, 0x00, 0x01, 0x01, 0x00, 0x80,
	0x32, 0x09, 0x04, 0x00, 0x00, 0x02, 0xff, 0x00, 
	0x00, 0x02, 0x07, 0x05, 0x81, 0x02, 0x40, 0x00, 
	0x00, 0x07, 0x05, 0x01, 0x02, 0x40, 0x00, 0x00
};

static const char	*strings[] = { NULL, NULL, "CP2102 USB to UART Bridge Controller", "0001" };
static const USB_DEVICE_QUALIFIER_DESCRIPTOR	dev_qua = {};

static char	control_reply[] = {
	0x42, 0x00, 0x00, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x00,
	0x00, 0x02, 0x00, 0x00, 0xc0, 0xc6, 0x2d, 0x00, 0x01, 0x00, 0x00, 0x00, 0x3f, 0x01, 0x00, 0x00,
	0x7f, 0x00, 0x00, 0x00, 0xf0, 0xff, 0x07, 0x10, 0x0f, 0x00, 0x07, 0x1f, 0x00, 0x02, 0x00, 0x00,
	0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x33, 0x00, 0x2e, 0x00,
	0x30, 0x00
};

static BOOL
handle_non_control_transfer(vstub_t *vstub, USBIP_CMD_SUBMIT *cmd_submit)
{
	if (cmd_submit->ep == 1 && cmd_submit->direction) {
		reply_cmd_submit(vstub, cmd_submit, NULL, 0);
		/* bulk transfers are requested indefinitely */
		usleep(300000);
		return TRUE;
	}
	else {
		return FALSE;
	}
}

static BOOL
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
			error("failed to recv payload data");
			return FALSE;
		}
		reply_cmd_submit(vstub, cmd_submit, NULL, 0);
	}
	else
		return FALSE;
	return TRUE;
}

vstubmod_t	vstubmod_cp210x = {
	"cp210x",
	"CP210X",
	&dev_dsc,
	&dev_qua,
	(CONFIG_GEN *)&configuration_cp210x,
	4,
	strings,
	NULL,
	handle_control_transfer,
	handle_non_control_transfer
};
