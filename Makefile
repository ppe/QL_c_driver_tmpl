CC = qdos-gcc
CFLAGS = -nostartfiles -c -O3
OBJS = heap.o chan_ops.o echodrv.o
RM = /bin/rm
DRIVER_BIN = echodrv_bin
SRCS = heap.c chan_ops.c echodrv.c
$(DRIVER_BIN): $(OBJS)
	ld -o$(DRIVER_BIN) -ms -screspr.o $(OBJS) -lgcc
clean:
	$(RM) -f *.o *.s *.MAP $(DRIVER_BIN)

depend: .depend

.depend: $(SRCS)
	$(RM) -f ./.depend
	$(CC) $(CFLAGS) -MM $^ > ./.depend;

include .depend
