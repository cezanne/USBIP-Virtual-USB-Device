#ifndef _USBIP_VSTUB_H_
#define _USBIP_VSTUB_H_

#ifdef LINUX

#include<sys/types.h>
#include<sys/socket.h>
#include<sys/un.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#define        min(a,b)        ((a) < (b) ? (a) : (b))
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

#define        TCP_SERV_PORT        3240

typedef struct sockaddr	sockaddr;

typedef struct {
	BOOL	attached;
	int	sockfd;
} vstub_t;

void error(const char *fmt, ...);

BOOL send_data(vstub_t *vstub, char *buf, unsigned len);
BOOL recv_data(vstub_t *vstub, char *buf, unsigned len);
	
BOOL recv_cmd_submit(vstub_t *vstub, USBIP_CMD_SUBMIT *cmd_submit);
BOOL send_ret_submit(vstub_t *vstub, USBIP_RET_SUBMIT *ret_submit, char *data, unsigned int size, unsigned int status);

BOOL init_vstub_net(void);
void fini_vstub_net(void);

BOOL accept_vstub(vstub_t *vstub);
void close_vstub(vstub_t *vstub);

//implemented by user
extern const USB_DEVICE_DESCRIPTOR dev_dsc;
extern const USB_DEVICE_QUALIFIER_DESCRIPTOR  dev_qua;
extern const char * configuration;
extern const USB_INTERFACE_DESCRIPTOR *interfaces[];
extern const unsigned char *strings[];

void handle_data(vstub_t *vstub, USBIP_RET_SUBMIT *ret_submit);
void handle_unknown_control(vstub_t *vstub, setup_pkt_t *setup_pkt, USBIP_RET_SUBMIT *ret_submit);

void usbip_run(const USB_DEVICE_DESCRIPTOR *dev_dsc);

#endif
