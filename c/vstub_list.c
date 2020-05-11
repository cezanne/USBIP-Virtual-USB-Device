#include "vstub.h"

static OP_REP_DEVLIST	*devlist;
static unsigned		devlist_len;

static void
add_entry(OP_REP_DEVLIST_DEVICE *dev, vstubmod_t *mod, unsigned devno)
{
	snprintf(dev->usbPath, 256, "/sys/devices/pci0000:00/0000:00:01.2/usb1/1-%u", devno);
	snprintf(dev->busID, 32, "1-%u", devno);
	dev->busnum = htonl(1);
	dev->devnum = htonl(2);
	dev->speed = htonl(2);
	dev->idVendor = htons(mod->dev_dsc->idVendor);
	dev->idProduct = htons(mod->dev_dsc->idProduct);
	dev->bcdDevice = htons(mod->dev_dsc->bcdDevice);
	dev->bDeviceClass = mod->dev_dsc->bDeviceClass;
	dev->bDeviceSubClass = mod->dev_dsc->bDeviceSubClass;
	dev->bDeviceProtocol = mod->dev_dsc->bDeviceProtocol;
	dev->bConfigurationValue = mod->conf->dev_conf.bConfigurationValue;
	dev->bNumConfigurations = mod->dev_dsc->bNumConfigurations;
	dev->bNumInterfaces = 0;
}

static void
setup_device_list(void)
{
	OP_REP_DEVLIST_DEVICE	*dev_dsc;
	int	i;

	devlist_len = sizeof(OP_REP_DEVLIST_HEADER) + sizeof(OP_REP_DEVLIST_DEVICE) * n_mods_bound;
	devlist = malloc(devlist_len);
	
	devlist->header.version = htons(273);
	devlist->header.command = htons(5);
	devlist->header.status = 0;
	devlist->header.nExportedDevice = htonl(n_mods_bound);

	dev_dsc = &devlist->device;
	for (i = 0; i  < n_mods_bound; i++) {
		add_entry(dev_dsc, mods_bound[i], i + 1);
		dev_dsc++;
	}
}

BOOL
handle_unattached_devlist(vstub_t *vstub)
{
	printf("list devices\n");

	if (devlist == NULL)
		setup_device_list();

	if (!send_data(vstub, (char *)devlist, devlist_len))
		return FALSE;

	return TRUE;
}
