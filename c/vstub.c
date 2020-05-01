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

handler_t	vstub_get_status;

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

static void
handle_device_list(const USB_DEVICE_DESCRIPTOR *dev_dsc, OP_REP_DEVLIST *list)
{
	CONFIG_GEN * conf= (CONFIG_GEN *)configuration;
	int i;

	list->header.version = htons(273);
	list->header.command = htons(5);
	list->header.status=0;
	list->header.nExportedDevice=htonl(1);
	memset(list->device.usbPath,0,256);
	strcpy(list->device.usbPath,"/sys/devices/pci0000:00/0000:00:01.2/usb1/1-1");
	memset(list->device.busID,0,32);
	strcpy(list->device.busID,"1-1");
	list->device.busnum = htonl(1);
	list->device.devnum = htonl(2);
	list->device.speed = htonl(2);
	list->device.idVendor=htons(dev_dsc->idVendor);
	list->device.idProduct=htons(dev_dsc->idProduct);
	list->device.bcdDevice=htons(dev_dsc->bcdDevice);
	list->device.bDeviceClass=dev_dsc->bDeviceClass;
	list->device.bDeviceSubClass=dev_dsc->bDeviceSubClass;
	list->device.bDeviceProtocol=dev_dsc->bDeviceProtocol;
	list->device.bConfigurationValue=conf->dev_conf.bConfigurationValue;
	list->device.bNumConfigurations=dev_dsc->bNumConfigurations; 
	list->device.bNumInterfaces=conf->dev_conf.bNumInterfaces;
	list->interfaces = malloc(list->device.bNumInterfaces*sizeof(OP_REP_DEVLIST_INTERFACE));
	for(i = 0; i < list->device.bNumInterfaces; i++) {
		list->interfaces[i].bInterfaceClass=interfaces[i]->bInterfaceClass;
		list->interfaces[i].bInterfaceSubClass=interfaces[i]->bInterfaceSubClass;
		list->interfaces[i].bInterfaceProtocol=interfaces[i]->bInterfaceProtocol;
		list->interfaces[i].padding=0;
	}
}

static void
handle_attach(const USB_DEVICE_DESCRIPTOR *dev_dsc, OP_REP_IMPORT *rep)
{
	CONFIG_GEN	*conf= (CONFIG_GEN *)configuration;
    
	rep->version = htons(273);
	rep->command = htons(3);
	rep->status = 0;
	memset(rep->usbPath, 0, 256);
	strcpy(rep->usbPath, "/sys/devices/pci0000:00/0000:00:01.2/usb1/1-1");
	memset(rep->busID, 0, 32);
	strcpy(rep->busID, "1-1");
	rep->busnum = htonl(1);
	rep->devnum = htonl(2);
	rep->speed = htonl(2);
	rep->idVendor = dev_dsc->idVendor;
	rep->idProduct = dev_dsc->idProduct;
	rep->bcdDevice = dev_dsc->bcdDevice;
	rep->bDeviceClass = dev_dsc->bDeviceClass;
	rep->bDeviceSubClass = dev_dsc->bDeviceSubClass;
	rep->bDeviceProtocol = dev_dsc->bDeviceProtocol;
	rep->bNumConfigurations = dev_dsc->bNumConfigurations;
	rep->bConfigurationValue = conf->dev_conf.bConfigurationValue;
	rep->bNumInterfaces = conf->dev_conf.bNumInterfaces;
}

static void
handle_get_descriptor_string(vstub_t *vstub, USBIP_CMD_SUBMIT *cmd_submit)
{
	setup_pkt_t	*setup_pkt = (setup_pkt_t *)cmd_submit->setup;
	char	str[255];
	int	i, len;

	memset(str, 0, 255);

	len = cmd_submit->transfer_buffer_length;
	if (len > *strings[setup_pkt->wValue.lowByte])
		len = *strings[setup_pkt->wValue.lowByte];
	for (i = 0; i < len / 2 - 1; i++)
		str[i] = strings[setup_pkt->wValue.lowByte][i * 2 + 2];

	printf(" get_descriptor_string: %s\n", str);
	reply_cmd_submit(vstub, cmd_submit, (char *)strings[setup_pkt->wValue.lowByte], len);
}

static BOOL
handle_get_descriptor(vstub_t *vstub, USBIP_CMD_SUBMIT *cmd_submit)
{
	setup_pkt_t	*setup_pkt = (setup_pkt_t *)cmd_submit->setup;

	switch (setup_pkt->wValue.hiByte) {
	case 0x1:
		// Device
		printf(" get_descriptor: Device\n");
		reply_cmd_submit(vstub, cmd_submit, (char *)&dev_dsc, sizeof(USB_DEVICE_DESCRIPTOR));
		break;
	case 0x2:
		// configuration
		printf(" get_descriptor: Configuration\n");
		reply_cmd_submit(vstub, cmd_submit, (char *)configuration, setup_pkt->wLength);
		break;
	case 0x3:
		handle_get_descriptor_string(vstub, cmd_submit);
		break;
	case 0x6:
		// qualifier
		printf(" get_descriptor: Qualifier\n");
		reply_cmd_submit(vstub, cmd_submit, (char *)&dev_qua, setup_pkt->wLength);
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
			if (vstub_get_status)
				vstub_get_status(vstub, cmd_submit);
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
		handle_control_transfer(vstub, cmd_submit);
	}
	else {
		handle_non_control_transfer(vstub, cmd_submit);
	}
}

static BOOL
handle_unattached_devlist(vstub_t *vstub, const USB_DEVICE_DESCRIPTOR *dev_dsc)
{
	OP_REP_DEVLIST	list;

	printf("list devices\n");

	handle_device_list(dev_dsc, &list);

	if (!send_data(vstub, (char *)&list.header, sizeof(OP_REP_DEVLIST_HEADER)))
		return FALSE;

	if (!send_data(vstub, (char *)&list.device, sizeof(OP_REP_DEVLIST_DEVICE)))
		return FALSE;

	if (!send_data(vstub, (char *)list.interfaces, sizeof(OP_REP_DEVLIST_INTERFACE) * list.device.bNumInterfaces))
		return FALSE;

	free(list.interfaces);

	return TRUE;
}

static BOOL
handle_unattached_import(vstub_t *vstub, const USB_DEVICE_DESCRIPTOR *dev_dsc)
{
	char	busid[32];
	OP_REP_IMPORT	rep;
	
	printf("attach device\n");

	if (!recv_data(vstub, busid, 32)) {
		return FALSE;
	}
	handle_attach(dev_dsc, &rep);
	if (!send_data(vstub, (char *)&rep, sizeof(OP_REP_IMPORT))) {
		return FALSE;
	}
	vstub->attached = TRUE;
	return TRUE;
}

static BOOL
handle_unattached(vstub_t *vstub, const USB_DEVICE_DESCRIPTOR *dev_dsc)
{
	OP_REQ_DEVLIST	req;

	if (!recv_data(vstub, (char *)&req, sizeof(OP_REQ_DEVLIST)))
		return FALSE;

	req.command = ntohs(req.command);

	switch (req.command) {
	case 0x8005:
		return handle_unattached_devlist(vstub, dev_dsc);
	case 0x8003:
		return handle_unattached_import(vstub, dev_dsc);
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
		error("Unknown USBIP cmd: %d\n", cmd_submit->command);
		ret = FALSE;
	}

	free(cmd_submit);
	return ret;
}

void
usbip_run(const USB_DEVICE_DESCRIPTOR *dev_dsc)
{
	init_vstub_net();

	for (;;) {
		vstub_t	vStub;

		if (!accept_vstub(&vStub))
			break;

		while (1) {
			if (!vStub.attached)
				handle_unattached(&vStub, dev_dsc);
			else {
				if (!handle_attached(&vStub))
					break;
			}
		}

		close_vstub(&vStub);
	}

	fini_vstub_net();
}
