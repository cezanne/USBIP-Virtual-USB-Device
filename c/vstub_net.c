#include "vstub.h"

#ifndef LINUX
WORD wVersionRequested = 2;
WSADATA wsaData;
#endif

static int	fd_accept;

static void
pack(USBIP_RET_SUBMIT *ret_submit)
{
	int	*data = (int *)ret_submit;
	int	i;

	for (i = 0; i < 10; i++) {
		data[i] = htonl(data[i]);
	}
}

static void
unpack(USBIP_CMD_SUBMIT *cmd_submit)
{
	int	*data = (int *)cmd_submit;
	int	i;

	for (i = 0; i < 10; i++) {
		data[i] = ntohl(data[i]);
	}
}

BOOL
send_data(vstub_t *vstub, char *buf, unsigned len)
{
	if (send(vstub->sockfd, buf, len, 0) != len) {
		error("failed to send: %s \n", strerror(errno));
		return FALSE;
	}
	return TRUE;
}

BOOL
recv_data(vstub_t *vstub, char *buf, unsigned len)
{
	int	nrecvs = 0, nrecv;

	while (nrecvs < len) {
		if ((nrecv = recv(vstub->sockfd, buf + nrecvs, len - nrecvs, 0)) <= 0) {
			error("failed to recv: %s \n", strerror(errno));
			return FALSE;
		}
		nrecvs += nrecv;
	}
	return TRUE;
}

BOOL
recv_cmd_submit(vstub_t *vstub, USBIP_CMD_SUBMIT *cmd_submit)
{
	if (!recv_data(vstub, (char *)cmd_submit, sizeof(USBIP_CMD_SUBMIT))) {
		return FALSE;
	}

	unpack(cmd_submit);
	return TRUE;
}

BOOL
send_ret_submit(vstub_t *vstub, USBIP_RET_SUBMIT *ret_submit, char *data, unsigned int size, unsigned int status)
{
        ret_submit->command = 0x3;
        ret_submit->status = status;
        ret_submit->actual_length = size;
        ret_submit->start_frame = 0x0;
        ret_submit->number_of_packets = 0x0;
	
        memset(ret_submit->setup, 0, 8);
        ret_submit->devid = 0x0;
	ret_submit->direction = 0x0;
        ret_submit->ep = 0x0;

        pack(ret_submit);
 
        if (!send_data(vstub, (char *)ret_submit, sizeof(USBIP_RET_SUBMIT))) {
		return FALSE;
        }

        if (size > 0) {
		if (!send_data(vstub, data, size)) {
			return FALSE;
		}
        }

	return TRUE;
}

BOOL
accept_vstub(vstub_t *vstub)
{
	struct sockaddr_in	cli;
#ifdef LINUX
	unsigned int clilen;
#else
	int clilen;
#endif

	clilen = sizeof(cli);
	if ((vstub->sockfd = accept(fd_accept, (sockaddr *)&cli, &clilen)) < 0) {
		printf("accept error : %s \n", strerror(errno));
		return FALSE;
	}

	printf("Connection address:%s\n", inet_ntoa(cli.sin_addr));

	vstub->attached = FALSE;
	return TRUE;
}

void
close_vstub(vstub_t *vstub)
{
	close(vstub->sockfd);
}

BOOL
init_vstub_net(void)
{
	struct sockaddr_in	serv;

#ifndef LINUX
	WSAStartup (wVersionRequested, &wsaData);
	if (wsaData.wVersion != wVersionRequested) {
		fprintf(stderr, "\n Wrong version\n");
		return FALSE;
	}
#endif

	if ((fd_accept = socket (PF_INET, SOCK_STREAM, 0)) < 0) {
		printf("socket error : %s \n", strerror (errno));
		return FALSE;
	}

	int reuse = 1;
	if (setsockopt(fd_accept, SOL_SOCKET, SO_REUSEADDR, (const char*)&reuse, sizeof(reuse)) < 0)
		perror("setsockopt(SO_REUSEADDR) failed");

	memset (&serv, 0, sizeof(serv));
	serv.sin_family = AF_INET;
	serv.sin_addr.s_addr = htonl(INADDR_ANY);
	serv.sin_port = htons(TCP_SERV_PORT);

	if (bind(fd_accept, (sockaddr *)&serv, sizeof(serv)) < 0) {
		printf ("bind error : %s \n", strerror(errno));
		return FALSE;
	}

	if (listen(fd_accept, SOMAXCONN) < 0) {
		printf ("listen error : %s \n", strerror (errno));
		return FALSE;
	}

	return TRUE;
}

void
fini_vstub_net(void)
{
#ifndef LINUX
	WSACleanup();
#endif
}
