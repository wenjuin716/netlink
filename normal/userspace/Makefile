EXEC=netlinkUser
OBJS=netlinkUser.o

CFLAGS += -DNETLINK_UNICAST
CFLAGS += -DNETLINK_MULTICAST

all: $(OBJS)
	$(CC) $(CFLAGS) $^ -o $(EXEC)

clean:
	rm -rf *.o $(EXEC)

