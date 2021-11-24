#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <thread>
#define BUFLEN 65536

void usage() {
	printf("syntax : syntax : echo-client <ip> <port>\n");
	printf("sample : echo-client 192.168.10.2 1234\n");
	exit(1);
}

void recvThread(int fd) {
	printf("Thread start...\n");
	char buf[BUFLEN];
	while(1) {
		ssize_t res = recv(fd, buf, BUFLEN - 1, 0);
		if(res <= 0) {
			perror("Failed recv\n");
			break;
		}
		buf[res] = '\0';
		printf("%s\n", buf);
		fflush(stdout);
	}
	close(fd);
	exit(0);
}

int main(int argc, char *argv[]) {
	if(argc != 3)
		usage();

	int fd = socket(AF_INET, SOCK_STREAM, 0);
	if(fd == -1) {
		perror("cannot create socket\n");
		return 0;
	}
	
	struct sockaddr_in addr;
	memset(&addr, 0, sizeof addr);
	addr.sin_family = AF_INET;
	addr.sin_port = htons(atoi(argv[2]));
	if(inet_pton(AF_INET, argv[1], &addr.sin_addr) == -1) {
		perror("inappropriate ip address\n");
		close(fd);
		return 0;
	}

	if(connect(fd, (struct sockaddr *)&addr, sizeof addr) == -1) {
		perror("connect failed\n");
		close(fd);
		return 0;
	}

	printf("Connected...\n");

	std::thread t(recvThread, fd);
	t.detach();

	char buf[BUFLEN];
	while(1) {
		scanf(" %[^\n]s", buf);
		ssize_t res = send(fd, buf, strlen(buf) + 1, 0);
		if(res <= 0) {
			perror("Failed send\n");
			break;
		}
	}
	close(fd);
	return 0;
}
