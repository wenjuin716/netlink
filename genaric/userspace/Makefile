ROOT_DIR=$(shell pwd)
LIB_DIR=$(ROOT_DIR)/../../libs
EXEC=netlinkUser
OBJS=netlinkUser.o

#USE_LIBNL:=y
ifneq ($(USE_LIBNL), )
CFLAGS+=-I$(LIB_DIR)/install/include
LDFLAGS+=-L$(LIB_DIR)/install/lib/ -lnl-3 -lnl-genl-3 -lnl-idiag-3 -lnl-nf-3 -lnl-route-3
endif

CFLAGS+=-DDEBUG
all: $(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $(EXEC)

clean:
	rm -rf *.o $(EXEC)

