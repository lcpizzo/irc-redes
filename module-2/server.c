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

#define MAX_MSG 4096

typedef struct user {
	struct sockaddr_in socketAddr;
	char name[51];
	int sockID;
	int len;
	int index;
	int connected;
} user;

// TODO: modularizar o codigo
// TODO: resolver o problema de Broken Pipe que acontece quando um dos clientes
//		desconecta do servidor -> acho que e pq o servidor tenta enviar a mensagem
// 		que aquele cliente saiu para ele mesmo, e como a conexao foi fechada
//		um erro e gerado

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

// Array for thread
struct user userList[1024];
int user_count = 0;
pthread_t thread[1024];

// Thread de comunicaçao com o cliente
void* clientThread(void *Client){
	user *client = (user *)Client;
	int quit = 0;

	char data[MAX_MSG];
	char msg[MAX_MSG];
	// Recebe o nome de usuario desse cliente
	int read = recv(client->sockID, data, MAX_MSG, 0);
	data[read] = '\0';
	strncpy(client->name, data, read);

	// loop de comunicaçao do cliente
	while(!quit) {
		bzero(msg, MAX_MSG);

		// Recebe a mensagem do usuario
		bzero(data, MAX_MSG);
		read = recv(client->sockID, data, MAX_MSG, 0);
		data[read] = '\0';

		// comandos especiais
		if(strncmp(data, "/quit", 5) == 0) {
			quit = 1;
			client->connected = 0;
			userList[client->index].connected = 0;
			send(client->sockID, "AK/QUIT", 7, 0);
			printf("%s saiu...\n", client->name);
			for(int i=0; i<user_count; i++) {
				strcpy(msg, client->name);
				strcat(msg, " saiu\n");
				if(userList[i].connected)
					// TODO: criar um while para checar se a mensagem foi enviada
					send(userList[i].sockID, msg, MAX_MSG, 0);
			}
		} 
		else if (strncmp(data, "/ping", 5) == 0) {
			send(client->sockID, "pong", 4, 0);
		} 
		else if (strncmp(data, "/nickname", 9) == 0) {
			strncpy(client->name, (data+10), (read-1));
			printf("user: %s\n", client->name);
		}

		// mensagem normal
		else {
			for(int i=0; i<user_count; i++) {
				strcpy(msg, client->name);
				strcat(msg, ": ");
				strcat(msg, data);
				// TODO: checar se o usuario ainda esta conectado antes de enviar a mensagem
				if(userList[i].connected)
					send(userList[i].sockID, msg, MAX_MSG, 0);
			}
		}
	}

	pthread_exit(NULL);
}

// Handle 'ctrl + c' signal
void interruptionHandler(int dummy) {
    printf("\n'Ctrl + C' não é um comando válido!!\n");
}

// Driver Code
int main()
{
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
		userList[user_count].sockID = accept(serverSocket,
			(struct sockaddr*)&userList[user_count].socketAddr,
			&userList[user_count].len);
		userList[user_count].index = user_count;
		userList[user_count].connected = 1;

		if (pthread_create(&thread[user_count], NULL,
			clientThread, (void *) &userList[user_count]) != 0)
			// Error in creating thread
			printf("Failed to create thread\n");
		

		user_count++;
	}

	for (int i=0; i<user_count; i++)
		pthread_join(thread[i], NULL);

	return 0;
}
