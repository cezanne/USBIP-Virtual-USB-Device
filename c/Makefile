CC = gcc
CFLAGS = -g -Wall -DLINUX 
PROGS = hid-mouse hid-keyboard cdc-acm
COMMONS = vstub.o vstub_net.o

all:	${PROGS}

hid-mouse: $(COMMONS) hid-mouse.o
	$(CC) -o $@ $^

hid-keyboard: $(COMMONS) hid-keyboard.o
	$(CC) -o $@ $^

cdc-acm: $(COMMONS) cdc-acm.o
	$(CC) -o $@ $^

$(COMMONS): vstub.h usbip_proto.h

clean:
	rm -f ${PROGS} core core.* *.o temp.* *.out typescript*
