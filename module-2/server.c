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
#define MAX_USERS 1024

// O usuario vai estar "logado" em apenas um canal por vez, entao
typedef struct user {
	struct sockaddr_in socketAddr;
	char name[51];
	int sockID;
	int len;
	int index;
	int connected;
} user;

typedef struct channel {
	/*
	lista de usuarios conectados no canal
	usuario admin
	nome
	n_membros
	TODO: pensar em como implementar o mute/unmute
	*/
} channel;

// TODO criar uma funçao de busca de usuarios que retorna o socket dele
//			recebendo como input o canal(que contem uma lista de todos os usuarios conectados)
//			e o nome de usuario dele

// TODO: crir uma maneira de checar se o user e admin para checar se o
//		comando pedido e valido

// TODO: criar uma funçao de falha, cmd nao autorizado apenas o ademir pode

// TODO: modularizar o codigo

// TODO: criar uma funcao que busca se existe o canal desejado
//		------------------------------
//		o resto da logica da criaçao de canais / adiçao do usuario ao canal
//		fica para outras partes do codigo
//		------------------------------	
//			-> talvez simplificar a funçao de busca de usuario para poder usar
//				essa mesma funçao para as duas funcionalidades
//				Ex. a funçao recebe uma lista e uma chave e retorna o membro encontrado
//				ou -1 caso negativo

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

// Array for thread
int user_count = 0;
pthread_t thread[MAX_USERS];

// usar lista linkada? facilitaria a remoçao de usuarios? atualmente com
//		lista estou usando o attr connected para desconectar logicamente
//		mas nao estou apagando o registro do usuario da memoria. 
struct user userList[MAX_USERS];

enum CMD {
	NONE = 100,
	PING,
	NICKNAME,
	QUIT,
	MUTE,
	UNMUTE,
	WHO,
	KICK,
	JOIN
};

int client_ping(user *client){
	return send(client->sockID, "pong", strlen("pong"), 0);
}

int change_nickname(user *client, char *new_nick){
	strncpy(client->name, new_nick, strlen(new_nick));

	return 0;
}

// TODO: mudar essa func para enviar a msg apenas para os users no
//		mesmo canal
// TODO: antes de chamar essa funçao checar se o usuario esta mutado
int send_msg(char *msg){
	for(int i=0; i<user_count; i++) {
		if(userList[i].connected)
			// TODO: criar um while para checar se a mensagem foi enviada
			send(userList[i].sockID, msg, MAX_MSG, 0);
	}
	return 0;
}

// Funçao que cria o canal
void create_channel(user *client, char *channel_name){

}

// Funçao que adiciona o usuario ao canal
void join_channel(user *client, char *channel_name){

}

int client_cmd(char *input){
	if(strncmp(input, "/ping", strlen("/ping") == 0)){
		return PING;
	} else if (strncmp(input, "/quit", strlen("/quit") == 0)){
		return QUIT;
	} else if (strncmp(input, "/nickname", strlen("/nickname") == 0)){
		return NICKNAME;
	} else if (strncmp(input, "/join", strlen("/join") == 0)){
		return JOIN;
	} else if (strncmp(input, "/kick", strlen("kick") == 0)){
		return KICK;
	} else if (strncmp(input, "/mute", strlen("/mute") == 0)){
		return MUTE;
	} else if (strncmp(input, "/unmute", strlen("/unmute") == 0)){
		return UNMUTE;
	} else if (strncmp(input, "/whois", strlen("/whois") == 0)){
		return WHO;
	}

	// no momento se a mensagem enviada e diferente de alguma dessas
	//		a mensagem e enviada a todos
	return NONE;
} 

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

		//int cmd = client_cmd(data);

		// comandos especiais
		if(strncmp(data, "/quit", 5) == 0) {
			// TODO: checar se o usuario eh admin do canal atual
			//		e tratar disso (deletar o canal ou passar admin para outro
			//		se for o segundo caso: qual o criterio?)
			//	--------------------
			// 		usar essa logica tambem para o join, se o admin de um canal
			//			for para outro, o q acontece com o canal que ele criou?
			quit = 1;
			client->connected = 0;

			send(client->sockID, "AK/QUIT", 7, 0);
			printf("%s saiu...\n", client->name);

			strcpy(msg, client->name);
			strcat(msg, " saiu\n");
			send_msg(msg);
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
			strcpy(msg, client->name);
			strcat(msg, ": ");
			strcat(msg, data);
			send_msg(msg);
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
