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

