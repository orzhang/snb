#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <snb_session.h>
#include <snb_protocal.h>
#include <sys/select.h>
#include <unistd.h>
#define DEFAULT_SERVER "127.0.0.1"
int sock_fd;
struct sockaddr_in addr;
int snb_connect(uint16_t port)
{
	int rc = 0;
	sock_fd = socket(AF_INET, SOCK_STREAM, 0);
	bzero((char *) &addr, sizeof(addr));

	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	inet_aton(DEFAULT_SERVER, &addr.sin_addr);
	rc = connect(sock_fd, (struct sockaddr*)&addr, sizeof(addr));
}

int test_login()
{

}

int main()
{
	snb_connect(8086);
	getchar();
	close(sock_fd);
	return 0;
}