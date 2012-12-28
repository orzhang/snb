OBJS=snb_protocal.o snb_lun_service.o snb_server_dispatch.o snb_session.o
CFLAGS = -pthread -DDEBUG -I ./ -g
LIBS=-lpthread
TEST_OBJS+=$(OBJS)
TEST_OBJS+=client_test.o

SERVER_OBJS+=$(OBJS)
SERVER_OBJS+=snb_server.o

all:server test

test:$(TEST_OBJS) 
	gcc $(TEST_OBJS) $(CFLAGS) -o client_test
server:$(SERVER_OBJS) 
	gcc $(SERVER_OBJS) $(CFLAGS) $(LIBS) -o snb_server 
clean:
	rm *.o client_test snb_server
