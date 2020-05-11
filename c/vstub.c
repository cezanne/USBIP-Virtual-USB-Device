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

#include <signal.h>
#include <pthread.h>

void
error(const char *fmt, ...)
{
	va_list	ap;
	char	buf[1024];

	va_start(ap, fmt);
	vsnprintf(buf, 1024, fmt, ap);
	va_end(ap);

	printf("error: %s\n", buf);
}

static BOOL
handle_attach(vstub_t *vstub, unsigned devno, OP_REP_IMPORT *rep)
{
	vstubmod_t	*mod;

	mod = find_vstubmod(devno);
	if (mod == NULL) {
		error("not exist devno: devno: %u", devno);
		return FALSE;
	}

	rep->version = htons(273);
	rep->command = htons(3);
	rep->status = 0;
	snprintf(rep->usbPath, 256, "/sys/devices/pci0000:00/0000:00:01.2/usb1/1-%u", devno);
	snprintf(rep->busID, 32, "1-%u", devno);
	rep->busnum = htonl(1);
	rep->devnum = htonl(2);
	rep->speed = htonl(2);
	rep->idVendor = mod->dev_dsc->idVendor;
	rep->idProduct = mod->dev_dsc->idProduct;
	rep->bcdDevice = mod->dev_dsc->bcdDevice;
	rep->bDeviceClass = mod->dev_dsc->bDeviceClass;
	rep->bDeviceSubClass = mod->dev_dsc->bDeviceSubClass;
	rep->bDeviceProtocol = mod->dev_dsc->bDeviceProtocol;
	rep->bNumConfigurations = mod->dev_dsc->bNumConfigurations;
	rep->bConfigurationValue = mod->conf->dev_conf.bConfigurationValue;
	rep->bNumInterfaces = mod->conf->dev_conf.bNumInterfaces;

	vstub->mod = mod;

	printf("device attached: %s\n", mod->desc);

	return TRUE;
}

static void
handle_get_descriptor_string(vstub_t *vstub, USBIP_CMD_SUBMIT *cmd_submit)
{
	setup_pkt_t	*setup_pkt = (setup_pkt_t *)cmd_submit->setup;
	char	str[255];
	int	i, len;

	memset(str, 0, 255);

	len = cmd_submit->transfer_buffer_length;
	if (len > *vstub->mod->strings[setup_pkt->wValue.lowByte])
		len = *vstub->mod->strings[setup_pkt->wValue.lowByte];
	for (i = 0; i < len / 2 - 1; i++)
		str[i] = vstub->mod->strings[setup_pkt->wValue.lowByte][i * 2 + 2];

	printf(" get_descriptor_string: %s\n", str);
	reply_cmd_submit(vstub, cmd_submit, (char *)vstub->mod->strings[setup_pkt->wValue.lowByte], len);
}

static BOOL
handle_get_descriptor(vstub_t *vstub, USBIP_CMD_SUBMIT *cmd_submit)
{
	setup_pkt_t	*setup_pkt = (setup_pkt_t *)cmd_submit->setup;

	switch (setup_pkt->wValue.hiByte) {
	case 0x1:
		// Device
		printf(" get_descriptor: Device\n");
		reply_cmd_submit(vstub, cmd_submit, (char *)vstub->mod->dev_dsc, sizeof(USB_DEVICE_DESCRIPTOR));
		break;
	case 0x2:
		// configuration
		printf(" get_descriptor: Configuration\n");
		reply_cmd_submit(vstub, cmd_submit, (char *)vstub->mod->conf, setup_pkt->wLength);
		break;
	case 0x3:
		handle_get_descriptor_string(vstub, cmd_submit);
		break;
	case 0x6:
		// qualifier
		printf(" get_descriptor: Qualifier\n");
		reply_cmd_submit(vstub, cmd_submit, (char *)vstub->mod->dev_qua, setup_pkt->wLength);
	default:
		return FALSE;
	}
	return TRUE;
}

static void
handle_get_status(vstub_t *vstub, USBIP_CMD_SUBMIT *cmd_submit)
{
	char	data[2];

	data[0] = 0x00;
	data[1] = 0x00;

	reply_cmd_submit(vstub, cmd_submit, data, 2);
	printf("GET_STATUS\n");
}

static void
handle_set_configuration(vstub_t *vstub, USBIP_CMD_SUBMIT *cmd_submit)
{
	setup_pkt_t	*setup_pkt = (setup_pkt_t *)cmd_submit->setup;

	printf(" handle_set_configuration: %hu\n", setup_pkt->wValue.W);
	reply_cmd_submit(vstub, cmd_submit, NULL, 0);
}

static BOOL
handle_control_transfer_common(vstub_t *vstub, USBIP_CMD_SUBMIT *cmd_submit)
{
	setup_pkt_t	*setup_pkt = (setup_pkt_t *)cmd_submit->setup;
	byte	bmRequestType, bRequest;

	bmRequestType = setup_pkt->bmRequestType;
	bRequest = setup_pkt->bRequest;

	switch (bmRequestType) {
	case 0x80:
		// Host Request
		switch (bRequest) {
		case 0x06:
			return handle_get_descriptor(vstub, cmd_submit);
		case 0x00:
			if (vstub->mod->handler_get_status)
				vstub->mod->handler_get_status(vstub, cmd_submit);
			else
				handle_get_status(vstub, cmd_submit);
			return TRUE;
		}
		break;
	case 0x00:
		if (bRequest == 0x09) {
			// Set Configuration
			handle_set_configuration(vstub, cmd_submit);
			return TRUE;
		}
		break;
	case 0x01:
		if (bRequest == 0x0B) {
			//SET_INTERFACE
			printf("SET_INTERFACE\n");
			reply_cmd_submit(vstub, cmd_submit, NULL, 0);
			return TRUE;
		}
		break;
	default:
		break;
	}

	return FALSE;
}

static void
handle_cmd_submit(vstub_t *vstub, USBIP_CMD_SUBMIT *cmd_submit)
{
	if (cmd_submit->ep == 0) {
		if (handle_control_transfer_common(vstub, cmd_submit))
			return;
		vstub->mod->handler_control_transfer(vstub, cmd_submit);
	}
	else {
		vstub->mod->handler_non_control_transfer(vstub, cmd_submit);
	}
}

static BOOL
handle_unattached_import(vstub_t *vstub)
{
	OP_REP_IMPORT	rep;
	char	busid[32];
	unsigned	devno;

	if (!recv_data(vstub, busid, 32)) {
		return FALSE;
	}
	if (sscanf(busid, "%*u-%u", &devno) != 1) {
		error("invalid busid: %s", busid);
		return FALSE;
	}
	if (!handle_attach(vstub, devno, &rep))
		return FALSE;

	if (!send_data(vstub, (char *)&rep, sizeof(OP_REP_IMPORT))) {
		return FALSE;
	}
	vstub->attached = TRUE;
	return TRUE;
}

static BOOL
handle_unattached(vstub_t *vstub)
{
	OP_REQ_DEVLIST	req;

	if (!recv_data(vstub, (char *)&req, sizeof(OP_REQ_DEVLIST)))
		return FALSE;

	req.command = ntohs(req.command);

	switch (req.command) {
	case 0x8005:
		return handle_unattached_devlist(vstub);
	case 0x8003:
		return handle_unattached_import(vstub);
	default:
		return FALSE;
	}
}

static void
show_cmd_submit(USBIP_CMD_SUBMIT *cmd_submit)
{
	unsigned short	ep;
	int	i;

	ep = cmd_submit->ep;
	if (cmd_submit->direction)
		ep |= 0x80;
	printf("CMD_SUBMIT[%04d] ep:%0hx len: %d, ", cmd_submit->seqnum, ep, cmd_submit->transfer_buffer_length);
	printf("setup:");
	for (i = 0; i < 8; i++)
		printf("%02hhx", cmd_submit->setup[i]);

	printf(", devid:%x, flags:%x, np:%d, intv:%d\n", cmd_submit->devid, cmd_submit->transfer_flags, cmd_submit->number_of_packets, cmd_submit->interval);
}

static BOOL
handle_attached(vstub_t *vstub)
{
	USBIP_CMD_SUBMIT	*cmd_submit;
	BOOL	ret = TRUE;

	if ((cmd_submit = recv_cmd_submit(vstub)) == NULL)
		return FALSE;

	show_cmd_submit(cmd_submit);

	switch (cmd_submit->command) {
	case 1:
		handle_cmd_submit(vstub, cmd_submit);
		break;
	case 2:
		//unlink urb
		printf("####################### Unlink URB %u  (not working!!!)\n", cmd_submit->transfer_flags);
		break;
	default:
		error("Unknown USBIP cmd: %d", cmd_submit->command);
		ret = FALSE;
	}

	free(cmd_submit);
	return ret;
}

static void *
vstub_func(void *arg)
{
	vstub_t	*vstub = (vstub_t *)arg;

	while (TRUE) {
		if (!vstub->attached) {
			if (!handle_unattached(vstub))
				break;
		}
		else {
			if (!handle_attached(vstub))
				break;
		}
	}	

	close_vstub(vstub);
	return NULL;
}

static void
start_vstub(vstub_t *vstub)
{
	pthread_t	pthread;

	pthread_create(&pthread, NULL, vstub_func, vstub);
	pthread_detach(pthread);
}

int
main(int argc, char *argv[])
{
	signal(SIGPIPE, SIG_IGN);

	if (!setup_vstubmods(argc - 1, argv + 1))
		return 1;

	init_vstub_net();

	for (;;) {
		vstub_t	*vstub;

		if ((vstub = accept_vstub()) == NULL)
			break;

		start_vstub(vstub);
	}

	fini_vstub_net();
}
