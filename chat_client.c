#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <pthread.h>
#include <signal.h>

#define CHAT_MAX_SIZE 256
#define NAME_MAX_SIZE 10

pthread_t thread_send, thread_recv;
char username[NAME_MAX_SIZE];

void* sendThreadClient(void*);
void* recvThreadClient(void*);

int main()
{
	int clientfd;
	struct sockaddr_in addr;
	char msg[CHAT_MAX_SIZE];

	//  IPv4 TCP
	if ((clientfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		printf("Socket Open Failed\n");
		exit(1);
	}

	addr.sin_family = AF_INET;
	addr.sin_port = htons(36007);
	addr.sin_addr.s_addr = inet_addr("127.0.0.1");

	// Connect
	if (connect(clientfd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
		printf("Connect Failed\n");
		exit(4);
	}

	printf("Enter yourname : ");
	scanf("%s", username);
	write(clientfd, username, strlen(username));

	pthread_create(&thread_send, NULL, sendThreadClient, (void*)clientfd);
	pthread_create(&thread_recv, NULL, recvThreadClient, (void*)clientfd);
	pthread_join(thread_send, NULL);
	pthread_join(thread_recv, NULL);

	close(clientfd);
}
void* sendThreadClient(void* arg)
{
	char msg[CHAT_MAX_SIZE];
	int tmp;
	int clientfd = (int)arg;

	while (1)
	{
		memset(msg, 0, CHAT_MAX_SIZE);
		if ((tmp = read(0, msg, CHAT_MAX_SIZE)) > 0)
		{
			write(clientfd, msg, CHAT_MAX_SIZE);
			char exit[] = "exit";
			if (!strncmp(msg, exit, strlen(exit)))
			{
				pthread_kill(thread_recv, SIGINT);
				break;
			}
		}
	}
}

void* recvThreadClient(void* arg)
{
	char msg[CHAT_MAX_SIZE];
	int tmp;
	int clientfd = (int)arg;

	while (1)
	{
		memset(msg, 0, sizeof(msg));
		if ((tmp = read(clientfd, msg, sizeof(msg))) > 0)
			write(1, msg, tmp);
	}
}
