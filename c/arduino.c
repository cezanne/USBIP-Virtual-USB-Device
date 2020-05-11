#include "vstub.h"

/* Device Descriptor */
static const USB_DEVICE_DESCRIPTOR	dev_dsc = {
	0x12, 0x01,
	0x1001,
	0x02, 0x00, 0x00, 0x08,
	0x4123,
	0x4300,
	0x0100,
	0x01, 0x02, 0xdc, 0x01
};

/* Configuration 1 Descriptor */
static const char	configuration_arduino[] = {
	0x09, 0x02, 0x3e, 0x00, 0x02, 0x01, 0x00, 0xc0, 0x32, 0x09, 0x04, 0x00, 0x00, 0x01, 0x02, 0x02,
	0x01, 0x00, 0x05, 0x24, 0x00, 0x01, 0x10, 0x04, 0x24, 0x02, 0x06, 0x05, 0x24, 0x06, 0x00, 0x01,
	0x07, 0x05, 0x82, 0x03, 0x08, 0x00, 0xff, 0x09, 0x04, 0x01, 0x00, 0x02, 0x0a, 0x00, 0x00, 0x00,
	0x07, 0x05, 0x04, 0x02, 0x40, 0x00, 0x01, 0x07, 0x05, 0x83, 0x02, 0x40, 0x00, 0x01
};

///DEL const USB_INTERFACE_DESCRIPTOR	*interfaces[] = { };
static const unsigned char	*strings[] = { NULL };
static const USB_DEVICE_QUALIFIER_DESCRIPTOR	dev_qua = {};

typedef struct {
	char	data[16];
	unsigned	datalen;
} bulk_reply_data_t;

static int	n_bulks;

static bulk_reply_data_t	bulk_reply_data[30] = {
	{ { 0x01, 0x00, 0x0a, 0x41, 0x56, 0x52, 0x49, 0x53, 0x50, 0x5f, 0x4d, 0x4b, 0x32 }, 13 },
	{ { 0x03, 0x00, 0x01 }, 3 },
	{ { 0x03, 0x00, 0x01 }, 3 },
	{ { 0x03, 0x00, 0x18 }, 3 },
	{ { 0x0a, 0x00 }, 2 },
};

static void
handle_non_control_transfer(vstub_t *vstub, USBIP_CMD_SUBMIT *cmd_submit)
{
	if (cmd_submit->ep == 0 && !cmd_submit->direction) {
		char	data[1024];
		if (!recv_data(vstub, data, cmd_submit->transfer_buffer_length)) {
			error("failed to recv bulk out data");
			return;
		}
		reply_cmd_submit(vstub, cmd_submit, NULL, 0);
	}
	if (cmd_submit->ep == 2 && cmd_submit->direction) {
		if (n_bulks < 5) {
			reply_cmd_submit(vstub, cmd_submit, bulk_reply_data[n_bulks].data, bulk_reply_data[n_bulks].datalen);
		}
		n_bulks++;
	}
	else {
		error("unhandled non-control");
	}
}

static void
handle_control_transfer(vstub_t *vstub, USBIP_CMD_SUBMIT *cmd_submit)
{
	setup_pkt_t	*setup_pkt = (setup_pkt_t *)cmd_submit->setup;

        if (setup_pkt->bmRequestType == 0xa1) {
		char	data[7] = { 0, };
		reply_cmd_submit(vstub, cmd_submit, data, 7);
	}
        else if (setup_pkt->bmRequestType == 0x21) {
		if (setup_pkt->bRequest == 0x22) {
			reply_cmd_submit(vstub, cmd_submit, NULL, 0);
		}
		else if (setup_pkt->bRequest == 0x20) {
			char	data[7] = { 0, };
			recv_data(vstub, data, 7);
			reply_cmd_submit(vstub, cmd_submit, NULL, 0);
		}
	}
	else {
		error("unhandled control transfer");
	}
}

static void
arduino_handle_get_status(vstub_t *vstub, USBIP_CMD_SUBMIT *cmd_submit)
{
	char	data[2] = { 0x64, 0x04 };

	reply_cmd_submit(vstub, cmd_submit, data, 2);
	printf("GET_STATUS\n");	
}

vstubmod_t	vstubmod_arduino = {
	"arduino",
	"Arduino",
	&dev_dsc,
	&dev_qua,
	(CONFIG_GEN *)&configuration_arduino,
	strings,
	arduino_handle_get_status,
	handle_control_transfer,
	handle_non_control_transfer
};
