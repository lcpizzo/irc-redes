#include <arpa/inet.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <stdbool.h>
#include "chatFunctions.h"

#define MAX_MSG 4096
#define MAX_USERS 1024
#define MAX_CHANNELS 10

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

int user_count = 0;
int channel_count = 0;



// Thread de comunicaçao com o cliente
void* clientThread(void *param){
	struct thread_args *my_args = (thread_args *)param;

	channel** channelList = my_args->channelList;
	user** userList = my_args->userList;
	user* client = my_args->client;

	bool quit = false;

	char data[MAX_MSG];
	// Recebe o nome de usuario desse cliente
	//int read = recv(client->sockID, data, MAX_MSG, 0);
	//strncpy(client->name, data, read);

	int read;

	// loop de comunicaçao do cliente
	while(!quit) {
		// Recebe a mensagem do usuario
		bzero(data, MAX_MSG);
		read = recv(client->sockID, data, MAX_MSG, 0);
		data[read] = '\0';
		printf("aqui tem %s\n", data);
		int erro = client_cmd(*channelList, *userList, data, client, channel_count, user_count, &quit);
		
		printf("erro %d\n", erro);
	}

	pthread_exit(NULL);
}


// Driver Code
int main(){
	// Array for thread
	int user_count = 0;
	int channel_count = 0;
	pthread_t thread[MAX_USERS];

	// set handler function
	//signal(SIGINT, interruptionHandler);

	channel **channelList = (channel **)calloc(MAX_CHANNELS, sizeof(channel *));
	for(int i = 0; i < MAX_CHANNELS; i++) channelList[i] = (channel *)calloc(1, sizeof(channel));

	user **userList = (user **)calloc(MAX_USERS, sizeof(user *));
	for(int i = 0; i < MAX_USERS; i++) userList[i] = (user *)calloc(1, sizeof(user));

	// Initialize variables
	int serverSocket;
	serverSocket = socket(AF_INET, SOCK_STREAM, 0);
	
	struct sockaddr_in serverAddr;
	serverAddr.sin_addr.s_addr = INADDR_ANY;
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(8989);

	// Bind the socket to the
	// address and port number.
	bind(serverSocket,
		(struct sockaddr*)&serverAddr,
		sizeof(serverAddr));

	// Listen on the socket,
	// with 40 max connection
	// requests queued
	if (listen(serverSocket, 50) == 0)
		printf("Listening\n");
	else
		printf("Error\n");

	while (1) {
		// Extract the first
		// connection in the queue
		userList[user_count]->sockID = accept(serverSocket,
			(struct sockaddr*)&userList[user_count]->socketAddr,
			&userList[user_count]->len);
		userList[user_count]->index = user_count;
		//userList[user_count]->connected = 1;

		struct thread_args param;
		param.userList = userList;
		param.channelList = channelList;
		param.client = userList[user_count];

		if (pthread_create(&thread[user_count], NULL,
		clientThread, &param) != 0)
			// Error in creating thread
			printf("Failed to create thread\n");

		user_count++;
	}

	for (int i=0; i<user_count; i++)
		pthread_join(thread[i], NULL);


	for(int i = 0; i < MAX_CHANNELS; i++) free(channelList[i]);
	free(channelList);
	
	for(int i = 0; i < MAX_USERS; i++) free(userList[i]);
	free(userList);

	return 0;
}
