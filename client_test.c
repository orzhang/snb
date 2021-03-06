#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <snb_session.h>
#include <snb_protocal.h>
#include <sys/select.h>
#include <unistd.h>
#include <snb_common.h>
#define DEFAULT_SERVER "127.0.0.1"
int sock_fd;
struct sockaddr_in addr;
int snb_count = 0;
uint16_t snb_LUNs[100];

int snb_connect(uint16_t port)
{
	int rc = 0;
	sock_fd = socket(AF_INET, SOCK_STREAM, 0);
	bzero((char *) &addr, sizeof(addr));

	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	inet_aton(DEFAULT_SERVER, &addr.sin_addr);
	rc = connect(sock_fd, (struct sockaddr*)&addr, sizeof(addr));
	return rc;
}
void test_ping()
{
	int expect;
	int ret = 0;
	uint8_t buf[1024];
	uint8_t *pbuf = buf;
	snb_command_t* cmd = snb_create_command_ping(server);
	snb_command_t* ack_cmd;
	expect = cmd->length + SNB_COMMAND_HEAD_SIZE;
	snb_pack_command(buf, sizeof(buf), cmd);
	do {
		ret = send(sock_fd, pbuf, expect, 0);
		SNB_TRACE("send %d bytes\n", ret);
		expect -= ret;
		pbuf += ret;
	} while (expect != 0);
	SNB_TRACE("ping sent\n");

	ret = recv(sock_fd, buf, SNB_COMMAND_HEAD_SIZE, 0);
	snb_unpack_command(buf, sizeof(buf), &ack_cmd);
	if(ack_cmd != NULL)
	{
		SNB_TRACE("id = %d\n", ack_cmd->id);
		SNB_TRACE("type = %d\n", ack_cmd->type);
		SNB_TRACE("proxy = %d\n", ack_cmd->proxy);
		SNB_TRACE("rc = %d\n", ack_cmd->rc);
	}	
}

void test_login()
{
	int expect;
	int ret = 0;
	uint8_t buf[1024];
	uint8_t *pbuf = buf;
	snb_command_t* cmd = snb_create_command_login("root", "root");
	snb_command_t* ack_cmd;
	expect = cmd->length + SNB_COMMAND_HEAD_SIZE;
	snb_pack_command(buf, sizeof(buf), cmd);
	do {
		ret = send(sock_fd, pbuf, expect, 0);
		SNB_TRACE("send %d bytes\n", ret);
		expect -= ret;
		pbuf += ret;
	} while (expect != 0);
	SNB_TRACE("login sent\n");

	ret = recv(sock_fd, buf, SNB_COMMAND_HEAD_SIZE, 0);
	snb_unpack_command(buf, sizeof(buf), &ack_cmd);
	if(ack_cmd != NULL)
	{
		SNB_TRACE("id = %d\n", ack_cmd->id);
		SNB_TRACE("type = %d\n", ack_cmd->type);
		SNB_TRACE("proxy = %d\n", ack_cmd->proxy);
		SNB_TRACE("rc = %d\n", ack_cmd->rc);
	}

}

void test_list()
{
	snb_command_t* cmd = snb_create_command_list();
	snb_command_t * ack_cmd;
	int expect = 0;
	int i  = 0;
	int ret;
	uint8_t buf[1024];
	uint8_t *pbuf = buf;
	expect = cmd->length + SNB_COMMAND_HEAD_SIZE;
	snb_pack_command(buf, sizeof(buf), cmd);
	do {
		ret = send(sock_fd, pbuf, expect, 0);
		SNB_TRACE("send %d bytes\n", ret);
		expect -= ret;
		pbuf += ret;
	} while (expect != 0);

	SNB_TRACE("list sent\n");
	pbuf = buf;
	ret = recv(sock_fd, pbuf, SNB_COMMAND_HEAD_SIZE, 0);
	expect = SNB_COMMAND_HEAD_ATTR_LENGTH(buf);
	pbuf += ret;
	ret = recv(sock_fd, pbuf, expect, 0);
	snb_unpack_command(buf, sizeof(buf), &ack_cmd);
	if(ack_cmd != NULL)
	{
		snb_count = ((snb_command_list_ack_t*)ack_cmd)->size;
		SNB_TRACE("id = %d\n", ack_cmd->id);
		SNB_TRACE("type = %d\n", ack_cmd->type);
		SNB_TRACE("proxy = %d\n", ack_cmd->proxy);
		SNB_TRACE("rc = %d\n", ack_cmd->rc);
		SNB_TRACE("size = %d\n", snb_count);

		for (i = 0; i < snb_count; i++)
		{
			SNB_TRACE("snb block id[%d]\n", 
				((snb_command_list_ack_t*)ack_cmd)->LUNs[i]);
			snb_LUNs[i] = ((snb_command_list_ack_t*)ack_cmd)->LUNs[i];
		}
	}
}

void test_info()
{
	snb_command_t* cmd = snb_create_command_info(snb_LUNs, snb_count);
	snb_command_t * ack_cmd;
	snb_command_info_ack_t * ack_info_cmd;
	int expect = 0;
	int i  = 0;
	int ret;
	uint8_t buf[1024];
	uint8_t *pbuf = buf;
	expect = cmd->length + SNB_COMMAND_HEAD_SIZE;
	snb_pack_command(buf, sizeof(buf), cmd);
	do {
		ret = send(sock_fd, pbuf, expect, 0);
		SNB_TRACE("send %d bytes\n", ret);
		expect -= ret;
		pbuf += ret;
	} while (expect != 0);

	SNB_TRACE("info request sent\n");
	pbuf = buf;
	ret = recv(sock_fd, pbuf, SNB_COMMAND_HEAD_SIZE, 0);
	expect = SNB_COMMAND_HEAD_ATTR_LENGTH(buf);
	pbuf += ret;
	ret = recv(sock_fd, pbuf, expect, 0);
	snb_unpack_command(buf, sizeof(buf), &ack_cmd);
	ack_info_cmd = (snb_command_info_ack_t*)ack_cmd;
	if(ack_cmd != NULL)
	{
		SNB_TRACE("id = %d\n", ack_cmd->id);
		SNB_TRACE("type = %d\n", ack_cmd->type);
		SNB_TRACE("proxy = %d\n", ack_cmd->proxy);
		SNB_TRACE("rc = %d\n", ack_cmd->rc);
		SNB_TRACE("size = %d\n", ack_info_cmd->size);
		for (i = 0; i < ack_info_cmd->size; i++)
		{
			SNB_TRACE("block id[%d] block_num:%d block_size:%d\n", 
				ack_info_cmd->LUNs[i],
				ack_info_cmd->block_num[i],
				ack_info_cmd->block_size[i]);

		}
	}
}

void test_rw()
{
	char info[] = "hello world!\n";
	snb_command_t* cmd = snb_create_command_rw(snb_LUNs[0],
		info, 0, sizeof(info), SNB_CMD_ID_RW_MASK_W);

	int expect = 0;
	int i  = 0;
	int ret;
	uint8_t buf[1024];
	uint8_t *pbuf = buf;
	expect = cmd->length + SNB_COMMAND_HEAD_SIZE;
	snb_pack_command(buf, sizeof(buf), cmd);
	do {
		ret = send(sock_fd, pbuf, expect, 0);
		SNB_TRACE("send %d bytes\n", ret);
		expect -= ret;
		pbuf += ret;
	} while (expect != 0);
	SNB_TRACE("rw request sent\n");

}

int main()
{
	snb_connect(8086);
	test_login();
	SNB_TRACE("\n");
	test_ping();
	SNB_TRACE("\n");
	test_list();
	SNB_TRACE("\n");
	test_info();
	SNB_TRACE("\n");
	test_rw();
	getchar();
	close(sock_fd);
	return 0;
}