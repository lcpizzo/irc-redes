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


// usar lista linkada? facilitaria a remoçao de usuarios? atualmente com
//		lista estou usando o attr connected para desconectar logicamente
//		mas nao estou apagando o registro do usuario da memoria. 


/*
TODO: pensar em como implementar o mute/unmute -> pensado/aceito criticas
		preciso do attr admin aqui? tem em 2 lugares, tanto no usuario
		quanto no canal, um dos 2 eh desnecessario.
	-----------------
	Pensando melhor acho que so no canal e melhor pois facilita a checagem se o user
	eh admin. Na implementaçao atual essa checagem eh demorada
*/

// TODO: criar uma funçao de busca de usuarios que retorna o socket dele
//			recebendo como input o canal(que contem uma lista de todos os usuarios conectados)
//			e o nome de usuario dele

// TODO: criar uma funçao que recebe o nome de um usuario e o canal e 
//	retorna um ponteiro para ele caso ele pertença ao canal, NULL caso
//		contrario

// TODO: criar uma funçao de falha, cmd nao autorizado apenas o ademir pode

// TODO: modularizar o codigo



// Driver Code
int main(){
	channel **channelList = (channel **)malloc(MAX_CHANNELS * sizeof(channel *));
	for(int i = 0; i < MAX_CHANNELS; i++) (channel *)malloc(sizeof(channel));

	user **userList = (user **)malloc(MAX_USERS * sizeof(user *));
	for(int i = 0; i < MAX_USERS; i++) (user *)malloc(sizeof(user));

	// Array for thread
	int user_count = 0;
	int channel_count = 0;
	pthread_t thread[MAX_USERS];

	// set handler function
	//signal(SIGINT, interruptionHandler);

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
		userList[user_count]->connected = 1;

		if (
			// TODO: resolver o problema do userList dentro do clientThread
			pthread_create(&thread[user_count], NULL,
			clientThread, (void *) &userList[user_count]) != 0)
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
