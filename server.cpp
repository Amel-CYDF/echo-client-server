#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <thread>
#include <mutex>
#include <vector>
#define BUFLEN 65536

void usage() {
	printf("syntax : echo-server <port> [-e[-b]]\n");
	printf("sample : echo-server 1234 -e -b\n");
	exit(1);
}

bool ise, isb;
std::vector<int> clientfd;
std::mutex m;

void recvThread(int fd) {
	char buf[BUFLEN];
	while(1) {
		ssize_t res = recv(fd, buf, BUFLEN - 1, 0);
		if(res <= 0) {
			perror("Failed recv\n");
			break;
		}
		buf[res] = '\0';
		printf("%s\n", buf);

		if(isb) {
			bool br=0;
			m.lock();
			for(auto &i: clientfd)
				if(send(i, buf, res, 0) <= 0) {
					perror("broadcast failed\n");
					br=1;
					break;
				}
			m.unlock();
			if(br) break;
		}
		else if(ise) {
			if(send(fd, buf, res, 0) <= 0) {
				perror("echo failed\n");
				break;
			}
		}
	}
	m.lock();
	for(auto it=clientfd.begin(); it!=clientfd.end(); ++it)
		if(*it == fd) {
			clientfd.erase(it);
			m.unlock();
			break;
		}
	close(fd);
	return;
}

int main(int argc, char *argv[]) {
	if(argc < 2)
		usage();
	
	int port = atoi(argv[1]);
	
	for(int res; (res = getopt(argc, argv, "eb")) !=-1; )
		switch (res)
		{
			case 'e': ise = true; break;
			case 'b': isb = true; break;
			default: perror("flag error\n"); usage(); return 0;
		}
	if(!ise && isb) {
		perror("flag error\n");
		usage();
		return 0;
	}
	printf("%d %d\n",ise, isb);

	int fd = socket(AF_INET, SOCK_STREAM, 0);
	if(fd == -1) {
		perror("cannot create socket\n");
		return 0;
	}

	int optval = 1;
	if(setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) == -1) {
		perror("setsockopt failed\n");
		return 0;
	}

	struct sockaddr_in addr;
	memset(&addr, 0, sizeof addr);
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = INADDR_ANY;

	if(bind(fd, (struct sockaddr *)&addr, sizeof addr) == -1) {
		perror("bind failed\n");
		close(fd);
		return 0;
	}

	if(listen(fd, 10) == -1) {
		perror("listen failed\n");
		close(fd);
		return 0;
	}
	
	printf("Listening...\n");

	while(1) {
		struct sockaddr_in cli_addr;
		socklen_t len = sizeof(cli_addr);
		int cli_sd = accept(fd, (struct sockaddr *)&cli_addr, &len);
		if (cli_sd == -1) {
			perror("accept failed");
			break;
		}

		m.lock();
		clientfd.push_back(cli_sd);
		m.unlock();

		std::thread t(recvThread, cli_sd);
		t.detach();
	}

	close(fd);
	return 0;
}
