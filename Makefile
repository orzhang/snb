OBJS=snb_protocal.o snb_lun_service.o snb_server.o snb_server_dispatch snb_session.o
CFLAGS = -I ./
all:$(OBJS)
	gcc $(OBJS) $(CFLAGS) -o server 