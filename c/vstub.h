#ifndef _USBIP_VSTUB_H_
#define _USBIP_VSTUB_H_

#ifdef LINUX

#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>

typedef int	BOOL;
#define TRUE	1
#define FALSE	0

#else

#include <winsock.h>

#endif

//system headers independent
#include <errno.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "usbip_proto.h"

typedef struct sockaddr	sockaddr;

typedef struct {
	BOOL	attached;
	int	sockfd;
	struct _vstubmod	*mod;
} vstub_t;

typedef BOOL (*handler_t)(vstub_t *, USBIP_CMD_SUBMIT *cmd_submit);

typedef struct _vstubmod {
	const char	*code;
	const char	*desc;
	const USB_DEVICE_DESCRIPTOR	*dev_dsc;
	const USB_DEVICE_QUALIFIER_DESCRIPTOR	*dev_qua;
	const CONFIG_GEN	*conf;
	const char	**strings;
	handler_t	handler_get_status;
	handler_t	handler_control_transfer;
	handler_t	handler_non_control_transfer;
} vstubmod_t;

#define MAX_STUBS	32

extern unsigned		n_mods_bound;
extern vstubmod_t	*mods_bound[MAX_STUBS];

void error(const char *fmt, ...);

USBIP_RET_SUBMIT *create_ret_submit(USBIP_CMD_SUBMIT *cmd_submit);
USBIP_CMD_SUBMIT *clone_cmd_submit(USBIP_CMD_SUBMIT *cmd_submit);

BOOL send_data(vstub_t *vstub, char *buf, unsigned len);
BOOL recv_data(vstub_t *vstub, char *buf, unsigned len);

USBIP_CMD_SUBMIT *recv_cmd_submit(vstub_t *vstub);
BOOL reply_cmd_submit(vstub_t *vstub, USBIP_CMD_SUBMIT *cmd_submit, char *data, unsigned int size);
BOOL reply_cmd_submit_err(vstub_t *vstub, USBIP_CMD_SUBMIT *cmd_submit, int errcode);

BOOL setup_vstubmods(int argc, char *argv[]);

BOOL init_vstub_net(void);
void fini_vstub_net(void);

vstub_t *accept_vstub(void);
void close_vstub(vstub_t *vstub);

vstubmod_t *find_vstubmod(unsigned devno);
BOOL handle_unattached_devlist(vstub_t *vstub);

#endif
