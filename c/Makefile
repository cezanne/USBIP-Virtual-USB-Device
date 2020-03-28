CC = gcc
CFLAGS = -g -Wall -DLINUX 
PROGS = hid-mouse hid-keyboard cdc-acm bth
COMMONS = vstub.o vstub_net.o usbip_pkt.o

all:	${PROGS}

hid-mouse: $(COMMONS) hid-mouse.o
	$(CC) -o $@ $^

hid-keyboard: $(COMMONS) hid-keyboard.o
	$(CC) -o $@ $^

cdc-acm: $(COMMONS) cdc-acm.o
	$(CC) -o $@ $^

bth: $(COMMONS) bth.o
	$(CC) -o $@ $^

$(COMMONS): vstub.h usbip_proto.h

clean:
	rm -f ${PROGS} core core.* *.o temp.* *.out typescript*
