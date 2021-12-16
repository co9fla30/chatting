#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <pthread.h>

#define CLIENT_MAX_SIZE 50
#define CHAT_MAX_SIZE 256
#define NAME_MAX_SIZE 20

typedef struct Client {
	int socket;
	char name[NAME_MAX_SIZE];
}Client;

Client client_list[CLIENT_MAX_SIZE];

pthread_t thread;
pthread_mutex_t mutex;

void* recvThread(void* arg);
void sendMsg(int user_idx, char* msg);
int client_add(int clientfd);
int client_delete(int clientfd);

int main()
{
	int serverfd, clientfd;
	int i, len, tmp;
	int th_id;
	struct sockaddr_in addr, c_addr;

	// default portno 36007
	addr.sin_family = AF_INET;
	addr.sin_port = htons(36007);
	addr.sin_addr.s_addr = INADDR_ANY;

	// IPv4 TCP
	if ((serverfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		printf("Socket Open Failed\n");
		exit(1);
	}

	if (bind(serverfd, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
		printf("Bind Failed\n");
		exit(2);
	}
	if (listen(serverfd, 5) == -1) {
		printf("Listen Failed\n");
		exit(3);
	}
	
	for (i = 0; i < CLIENT_MAX_SIZE; i++)
		client_list[i].socket = -1;


	while (1)
	{
		len = sizeof(c_addr);
		clientfd = accept(serverfd, (struct sockaddr*)&c_addr, &len);

		tmp = client_add(clientfd);
		if (tmp < 0)
		{
			printf("fail");
			close(clientfd);
		}
		else
		{
			th_id = pthread_create(&thread, NULL, recvThread, (void*)tmp);
			if (th_id < 0) {
				printf("Receive Thread Creation Failed\n");
				exit(1);
			}
		}
	}
}
void sendMsg(int user_idx, char* msg) //write send
{
	int i, len;
	char sndMsg[CHAT_MAX_SIZE];

	len = sprintf(sndMsg, "[%s]: %s", client_list[user_idx].name, msg);

	for (i = 0; i < CLIENT_MAX_SIZE; i++)
	{
		if (client_list[i].socket == -1) continue;
		//if (client_list[i].socket == client_list[user_idx].socket) 
			//continue;
		write(client_list[i].socket, sndMsg, len);
	}
}
void* recvThread(void* arg) //read recv
{
	int user_idx = (int)arg;
	char msg[CHAT_MAX_SIZE];
	int i;

	while (1)
	{
		memset(msg, 0, sizeof(msg));
		if ((read(client_list[user_idx].socket, msg, CHAT_MAX_SIZE)) > 0)
		{
			if (!strcmp(msg, "exit"))
			{
				client_delete(client_list[user_idx].socket);
				break;
			}
			sendMsg(user_idx, msg);
		}
	}
}

int client_add(int clientfd)
{
	int i;

	for (i = 0; i < CLIENT_MAX_SIZE; i++)
	{
		pthread_mutex_lock(&mutex);
		if (client_list[i].socket == -1)
		{
			client_list[i].socket = clientfd;
			read(clientfd, client_list[i].name, NAME_MAX_SIZE);
			printf("[%s] connected\n", client_list[i].name);
			sendMsg(i, "connected\n");
			pthread_mutex_unlock(&mutex);
			return i;
		}
		pthread_mutex_unlock(&mutex);
	}

	if (i == CLIENT_MAX_SIZE) return -1;
}

int client_delete(int clientfd)
{
	int i;

	close(clientfd);

	for (i = 0; i < CLIENT_MAX_SIZE; i++)
	{
		pthread_mutex_lock(&mutex);
		if (client_list[i].socket == clientfd)
		{
			printf("[%s] disconnected\n", client_list[i].name);
			sendMsg(i, " disconnected\n");
			client_list[i].socket = -1;
			pthread_mutex_unlock(&mutex);
			break;
		}
		pthread_mutex_unlock(&mutex);
	}
	return 0;
}
