#include "vstub.h"

//AvrMKII Configuration
typedef struct __attribute__ ((__packed__))
{
	USB_CONFIGURATION_DESCRIPTOR	dev_conf;
	USB_INTERFACE_DESCRIPTOR	dev_int;
	USB_ENDPOINT_DESCRIPTOR		dev_ep1, dev_ep2, dev_ep3;
} CONFIG_AVRMKII;

/* Device Descriptor */
static const USB_DEVICE_DESCRIPTOR	dev_dsc = {
	0x12, 0x01,
	0x1001,
	0xff, 0x00, 0x00, 0x10,
	0xeb03,
	0x0421,
	0x0002,
	0x01, 0x02, 0x03, 0x01
};

/* Configuration 1 Descriptor */
static const char	configuration_avrmkii[] = {
	0x09, 0x02, 0x20, 0x00, 0x01, 0x01, 0x00, 0xc0, 0x32, 0x09, 0x04, 0x00, 0x00, 0x02, 0xff, 0x00,
	0x00, 0x00, 0x07, 0x05, 0x82, 0x02, 0x40, 0x00, 0x0a, 0x07, 0x05, 0x02, 0x02, 0x40, 0x00, 0x0a
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

static BOOL
handle_non_control_transfer(vstub_t *vstub, USBIP_CMD_SUBMIT *cmd_submit)
{
	if (cmd_submit->ep == 2 && !cmd_submit->direction) {
		char	data[1024];
		if (!recv_data(vstub, data, cmd_submit->transfer_buffer_length)) {
			error("failed to recv bulk out data");
			return FALSE;
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
		return FALSE;
	}
	return TRUE;
}

static BOOL
handle_control_transfer(vstub_t *vstub, USBIP_CMD_SUBMIT *cmd_submit)
{
	return FALSE;
}

vstubmod_t	vstubmod_avrmkii = {
	"avrmkii",
	"AVRISPMKII",
	&dev_dsc,
	&dev_qua,
	(CONFIG_GEN *)&configuration_avrmkii,
	strings,
	NULL,
	handle_control_transfer,
	handle_non_control_transfer
};
