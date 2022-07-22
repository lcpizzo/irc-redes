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
//#include "chatFunctions.h"

#define MAX_MSG 4096
#define MAX_USERS 1024
#define MAX_CHANNELS 10

#define UN_AUTH -100
#define MUTED -101

typedef struct user {
	struct sockaddr_in socketAddr;
	char name[51];
	int sockID;
	int len;
	int index;
	//bool connected;
	bool mute;
	bool admin;
	struct channel *conn_channel;
} user;

typedef struct channel {
	user* channel_users[MAX_USERS / MAX_CHANNELS];
	user* admin;
	int n_members;
	char channelName[51];
} channel;

user userList[MAX_USERS];
channel channelList[MAX_CHANNELS];

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

int user_count = 0;
int channel_count = 0;

int client_ping(user *client){
	return send(client->sockID, "pong", strlen("pong"), 0);
}

int change_nickname(user *client, char *new_nick){
	strncpy(client->name, new_nick, strlen(new_nick));

	return 0;
}

// Funçao que adiciona o usuario ao canal
int join_channel(channel *chnl, user *client){
	if(chnl->n_members == MAX_CHANNELS)
		return -1;

	chnl->channel_users[chnl->n_members++] = client;
	client->conn_channel = chnl;

	return 0;
}

int send_msg(user *client, char *msg){
	int tries = 0;

	for(int i=0; i < client->conn_channel->n_members; i++) {
		while(tries++ <= 5)
			send(client->conn_channel->channel_users[i]->sockID, msg, MAX_MSG, 0);
	}

	return 0;
}

int whois(user *client, char *input){
	char *subj = (input+strlen("/whois "));

	for (int i=0; i<client->conn_channel->n_members; i++){
		if(strcmp(client->conn_channel->channel_users[i]->name, subj) == 0){
			return client->conn_channel->channel_users[i]->sockID;
		}
	}

	return -1;
}

int search_channel(char *channelName){
	int i;
	printf("channelname: %s\n", channelName);

	for(i = 0; i < channel_count; i++){
		printf("channel name: %s\n", channelList[i].channelName);
		if(strcmp(channelName, channelList[i].channelName) == 0)
			return i;
	}
	
	return -1;
}

int search_user(char *clientName, channel *chnl){
	int i;
	
	for(i = 0; i < chnl->n_members; i++){
		if(strcmp(clientName, chnl->channel_users[i]->name) == 0)
			return i;
	}
	
	return -1;
}

// Funçao que cria canais
int create_channel(char* channelName, user *admin, int channel_count){
	if(channel_count == MAX_CHANNELS)
		return -1;

	channelList[channel_count].admin = admin;
	strcpy(channelList[channel_count].channelName, channelName);
	channelList[channel_count].n_members = 1;
	channelList[channel_count].channel_users[channelList[channel_count].n_members] = admin;
	admin->admin = true;
	admin->conn_channel = &channelList[channel_count];

	channel_count++;

	return 0;
}

// Funçoes de mutar e desmutar o cliente, se ele pertencer ao mesmo 
//	canal que o admin que requisitou o mute/unmute
int mute_user(char *clientName, channel *chnl) {
	user *client = chnl->channel_users[search_user(clientName, chnl)];
	if(client == NULL)
		return -1; 
	
	if(client->mute)
		return 0;
	
	client->mute = true;
	return 0;
}

int unmute_user(char *clientName, channel *chnl) {
	user *client = chnl->channel_users[search_user(clientName, chnl)];
	if(client == NULL)
		return -1; 
	
	if(!client->mute)
		return 0;
	
	client->mute = false;
	return 0;
}

int remove_user(char *userName){
	//user *client = userList();
	return 0;
}

int kick_user(char* userName, channel* chnl){
	return 0;
}

int quit_server(user *client, bool* quit){
	*quit = true;
	//client->connected = 0;

	send(client->sockID, "AK/QUIT", 7, 0);
	printf("%s saiu...\n", client->name);

	char msg[4096];
	strcpy(msg, client->name);
	strcat(msg, " saiu\n");
	send_msg(client, msg); 

	return 0;
}

// Funçao controladora que interpreta o comando e chama a funçao apropriada
int client_cmd(char *input, user *client, int channel_count, int user_count, bool *quit){
	printf("%s\n", input);
	if(strncmp(input, "/ping", strlen("/ping")) == 0){
		return client_ping(client);

	} else if (strncmp(input, "/quit", strlen("/quit")) == 0){
		return quit_server(client, quit);

	} else if (strncmp(input, "/nickname", strlen("/nickname")) == 0){
		return change_nickname(client, (input+strlen("/nickname ")));
		
	} else if (strncmp(input, "/join", strlen("/join")) == 0){
		int index = search_channel((input+strlen("/join ")));
		if(index == -1)
			return create_channel((input+strlen("/join ")), client, channel_count);
		
		return join_channel(&channelList[index], client);

	} else if (strncmp(input, "/kick", strlen("kick")) == 0){
		if(client->admin)
			return kick_user(input+strlen("/kick "), client->conn_channel);
		char msg[128] = "Operacao nao permitida. Apenas o administrador pode executar essa funcao.\n";			
		send(client->sockID, msg, strlen(msg), 0);
		return UN_AUTH;
	} else if (strncmp(input, "/mute", strlen("/mute")) == 0){
		if(client->admin)
			return mute_user(input+strlen("/mute "), client->conn_channel);
		
		char msg[128] = "Operacao nao permitida. Apenas o administrador pode executar essa funcao.\n";		
		send(client->sockID, msg, strlen(msg), 0);
		return UN_AUTH;

	} else if (strncmp(input, "/unmute", strlen("/unmute")) == 0){
		if(client->admin)
			return unmute_user(input+strlen("/unmute "), client->conn_channel);
		char msg[128] = "Operacao nao permitida. Apenas o administrador pode executar essa funcao.\n";
		send(client->sockID, msg, strlen(msg), 0);
		return UN_AUTH;

	} else if (strncmp(input, "/whois", strlen("/whois")) == 0){
		if(client->admin){
			char msg[128] = "O usuario ";
			strcat(msg, (input+strlen("/whois ")));
			strcat(msg, " tem IP: ");
			char temp[64];
			sprintf(temp, "%d\n", whois(client, input));
			strcat(msg, temp);
			return send(client->sockID, msg, strlen(msg), 0);
		}

		char msg[128] = "Operacao nao permitida. Apenas o administrador pode executar essa funcao.\n";
		send(client->sockID, msg, strlen(msg), 0);
		return UN_AUTH;
	} else {
		if(client->conn_channel == 0)
			return -1;
		if(!client->mute)
			return send_msg(client, input);
		return MUTED;
	}
	return 0;
}

// Handle 'ctrl + c' signal
void interruptionHandler(int dummy) {
    printf("\n'Ctrl + C' não é um comando válido!!\n");
}

// Thread de comunicaçao com o cliente
void* clientThread(void *param){
	user* client = (user*)param;

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
		int erro = client_cmd(data, client, channel_count, user_count, &quit);
		
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
		//userList[user_count]->connected = 1;

		pthread_create(&thread[user_count], NULL, clientThread, (void*)&userList[user_count]);
		//if (pthread_create(&thread[user_count], NULL, 
		//	clientThread, (void*)userList[user_count]) != 0)
		//	// Error in creating thread
		//	printf("Failed to create thread\n");

		user_count++;
	}

	for (int i=0; i<user_count; i++)
		pthread_join(thread[i], NULL);


	return 0;
}
