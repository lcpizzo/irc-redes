#include"chatFunctions.h"
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
#include <pthread.h>

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

channel *search_channel(channel **channelList, char *channelName){
	int i;

	for(i = 0; i < MAX_CHANNELS; i++){
		if(channelList[i]->channelName == NULL)
			return NULL;
		if(strcmp(channelName, channelList[i]->channelName) == 0)
			return channelList[i];
	}
	
	return NULL;
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
int create_channel(channel** channelList, char* channelName, user *admin, int channel_count){
	if(channel_count == MAX_CHANNELS)
		return -1;
	
	channel *chnl = (channel *)malloc(sizeof(channel));
	chnl->admin = admin;
	strcpy(chnl->channelName, channelName);
	chnl->n_members = 1;
	chnl->channel_users[chnl->n_members] = admin;
	admin->admin = true;

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

int kick_user(char *clientName, channel* chnl){
	search_user(clientName, chnl);

	
	return 0;
}

int remove_user(user** userList, char *userName){
	//user *client = userList()
}

int quit_server(user *client, bool* quit, int user_count, user **userList){
	// TODO: checar se o usuario eh admin do canal atual
	//		e tratar disso (deletar o canal ou passar admin para outro
	//		se for o segundo caso: qual o criterio?)
	//	--------------------
	// 		usar essa logica tambem para o join, se o admin de um canal
	//			for para outro, o q acontece com o canal que ele criou?
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
int client_cmd(channel **channelList, user *userList, char *input, user *client, int channel_count, int user_count, bool *quit){
	printf("%s\n", input);
	if(strncmp(input, "/ping", strlen("/ping")) == 0){
		return client_ping(client);

	} else if (strncmp(input, "/quit", strlen("/quit")) == 0){
		return quit_server(client, quit, user_count, userList);

	} else if (strncmp(input, "/nickname", strlen("/nickname")) == 0){
		return change_nickname(client, (input+strlen("/nickname ")));
		
	} else if (strncmp(input, "/join", strlen("/join")) == 0){
		channel *chnl = search_channel(channelList, (input+strlen("/join ")));
		if(chnl == NULL)
			return create_channel(channelList, (input+strlen("/join ")), client, channel_count);
		
		return join_channel(chnl, client);

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
		// no momento se a mensagem enviada e diferente de alguma dessas
		//		a mensagem e enviada a todos
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
