CC = gcc
CFLAGS = -g -Wall -DLINUX 
PROGS = vstub-server
COMMONS = vstub.o vstubmod.o vstub_list.o vstub_net.o usbip_pkt.o
MODULES = hid-mouse.o hid-keyboard.o cdc-acm.o bth.o cp210x.o arduino.o avrmkii.o

all:	${PROGS}

vstub-server: $(COMMONS) $(MODULES)
	$(CC) -o $@ $^ -lpthread

$(COMMONS) $(MODULES): vstub.h usbip_proto.h

clean:
	rm -f ${PROGS} core core.* *.o temp.* *.out typescript*
