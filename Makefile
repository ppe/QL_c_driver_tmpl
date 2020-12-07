CC = qdos-gcc
CFLAGS = -nostartfiles -c -O3
OBJS = heap.o echodrv.o
RM = /bin/rm
DRIVER_BIN = echodrv
$(DRIVER_BIN): $(OBJS)
	ld -o$(DRIVER_BIN) -ms -screspr.o $(OBJS) -lgcc
clean:
	$(RM) -f *.o *.s *.MAP $(DRIVER_BIN)
.c.o:
	$(CC) $(CFLAGS) $C$*.c
heap.o: heap.c heap.h
echodrv.o: echodrv.c heap.h
