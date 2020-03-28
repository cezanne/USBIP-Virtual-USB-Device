#include "vstub.h"

USBIP_RET_SUBMIT *
create_ret_submit(USBIP_CMD_SUBMIT *cmd_submit)
{
	USBIP_RET_SUBMIT	*ret_submit;

	ret_submit = (USBIP_RET_SUBMIT *)malloc(sizeof(USBIP_RET_SUBMIT));

	ret_submit->command = 0x3;
	ret_submit->seqnum = cmd_submit->seqnum;
	ret_submit->devid = cmd_submit->devid;
	ret_submit->direction = 0;
	ret_submit->ep = cmd_submit->ep;
	ret_submit->status = 0;
	ret_submit->actual_length = cmd_submit->transfer_buffer_length;
	ret_submit->start_frame = 0;
	ret_submit->number_of_packets = 0;
	ret_submit->error_count = 0;
	memset(ret_submit->setup, 0, 8);

	return ret_submit;
}

USBIP_CMD_SUBMIT *
clone_cmd_submit(USBIP_CMD_SUBMIT *cmd_submit)
{
	USBIP_CMD_SUBMIT	*cloned;

	cloned = (USBIP_CMD_SUBMIT *)malloc(sizeof(USBIP_CMD_SUBMIT));
	memcpy(cloned, cmd_submit, sizeof(USBIP_CMD_SUBMIT));
	return cloned;
}
