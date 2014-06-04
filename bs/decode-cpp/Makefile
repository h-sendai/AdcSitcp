CC = g++
PROG = main
CXXFLAGS = -g -O2 -Wall

# LDLIBS += -L/usr/local/lib/ -lmylib

OBJS += $(PROG).o
OBJS += AdcSitcpPacket.o

all: $(PROG)

$(PROG): $(OBJS)

clean:
	rm -f *.o $(PROG)
