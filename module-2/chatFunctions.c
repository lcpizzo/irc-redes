#include<chatFunctions.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<unistd.h>
#include<signal.h>
#include<fcntl.h>
#include<netinet/in.h>
#include<stdbool.h>

int client_ping(user *client){
	return send(client->sockID, "pong", strlen("pong"), 0);
}

int change_nickname(user *client, char *new_nick){
	strncpy(client->name, new_nick, strlen(new_nick));

	return 0;
}

// Funçao que adiciona o usuario ao canal
int join_channel(channel *chnl, user *client){
	if(chnl->channel_users == MAX_CHANNELS)
		return -1;

	chnl->channel_users[chnl->n_members++] = client;

	return 0;
}

// TODO: mudar essa func para enviar a msg apenas para os users no
//		mesmo canal
// TODO: antes de chamar essa funçao checar se o remetente esta mutado
int send_msg(user **userList, char *msg, int user_count){
	for(int i=0; i < user_count; i++) {
		if(userList[i]->connected)
			// TODO: criar um while para checar se a mensagem foi enviada
			send(userList[i]->sockID, msg, MAX_MSG, 0);
	}
	return 0;
}

// Funçao de busca do canal
// TODO: criar uma funcao que busca se existe o canal desejado
//		------------------------------
//		o resto da logica da criaçao de canais / adiçao do usuario ao canal
//		fica para outras partes do codigo
//		------------------------------	
//			-> talvez simplificar a funçao de busca de usuario para poder usar
//				essa mesma funçao para as duas funcionalidades
//				Ex. a funçao recebe uma lista e uma chave e retorna o membro encontrado
//				ou -1 caso negativo
channel *search_channel(channel **channelList, char *channelName){
	int i;
	
	for(i = 0; i < MAX_CHANNELS; i++){
		if(strcmp(channelName, channelList[i]->channelName == 0))
			return channelList[i];
	}
	
	return NULL;
	}

user* search_user(char *clientName, channel *chnl){
	int i;
	
	for(i = 0; i < chnl->n_members; i++){
		if(strcmp(clientName, chnl->channel_users[i] == 0))
			return chnl->channel_users[i];
	}
	
	return NULL;
}

// Funçao que cria canais
int create_channel(channel **channelList, char* channelName, user *admin, int channel_count){
	if(channel_count == MAX_CHANNELS)
		return -1;
	
	channelList[channel_count]->admin = admin;
	strcpy(channelList[channel_count]->channelName, channelName);
	channelList[channel_count]->n_members = 1;
	channelList[channel_count]->channel_users[channelList[channel_count]->n_members] = admin;
	admin->admin = true;

	channel_count++;

	return 0;
}

// Funçoes de mutar e desmutar o cliente, se ele pertencer ao mesmo 
//	canal que o admin que requisitou o mute/unmute
int mute_user(char *clientName, channel *chnl) {
	user *client = search_user(clientName, chnl);
	if(client == NULL)
		return -1; 
	
	if(client->mute)
		return 0;
	
	client->mute = true;
	return 0;
}

int unmute_user(char *clientName, channel *chnl) {
	user *client = search_user(clientName, chnl);
	if(client == NULL)
		return -1; 
	
	if(!client->mute)
		return 0;
	
	client->mute = false;
	return 0;
}

// Funçao controladora que interpreta o comando e chama a funçao apropriada
int client_cmd(channel **channelList, user **userList, char *input, user *client, int channel_count, int user_count){
	if(strncmp(input, "/ping", strlen("/ping") == 0)){
		return client_ping(client);

	} else if (strncmp(input, "/quit", strlen("/quit") == 0)){
		//return QUIT;

	} else if (strncmp(input, "/nickname", strlen("/nickname") == 0)){
		return change_nickname(client, (input+strlen("/nickname ")));
	} else if (strncmp(input, "/join", strlen("/join") == 0)){
		channel *chnl = search_channel(channelList, (input+strlen("/join ")));
		if(chnl == NULL)
			return create_channel(channelList, (input+strlen("/join ")), client, channel_count);
		else
			return join_channel(chnl, client);

	} else if (strncmp(input, "/kick", strlen("kick") == 0)){
		//return KICK;
		
	} else if (strncmp(input, "/mute", strlen("/mute") == 0)){
		return mute_user(input+strlen("/mute "), client->conn_channel);

	} else if (strncmp(input, "/unmute", strlen("/unmute") == 0)){
		return unmute_user(input+strlen("/unmute "), client->conn_channel);

	} else if (strncmp(input, "/whois", strlen("/whois") == 0)){
		//return WHO;
	}

	// no momento se a mensagem enviada e diferente de alguma dessas
	//		a mensagem e enviada a todos
	return send_msg(userList, input, user_count);
} 

// Thread de comunicaçao com o cliente
void* clientThread(void *Client, user **userList, int user_count){
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
			send_msg(userList, msg, user_count);
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
			send_msg(userList, msg, user_count);
		}
	}

	pthread_exit(NULL);
}

// Handle 'ctrl + c' signal
void interruptionHandler(int dummy) {
    printf("\n'Ctrl + C' não é um comando válido!!\n");
}
