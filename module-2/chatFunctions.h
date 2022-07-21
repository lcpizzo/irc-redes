#ifndef _CHATFUNCTIONS_H_
#define _CHATFUNCTIONS_H_

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

#define MAX_MSG 4096
#define MAX_USERS 1024
#define MAX_CHANNELS 10

// enum?
#define UN_AUTH -100
#define MUTED -101

// O usuario vai estar "logado" em apenas um canal por vez, entao
//  podemos adicionar dois attr a mais, um bool admin e um bool mute,
// 	para checar se o usuario eh um admin e se esta mutado 
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
	char *channelName;
} channel;

typedef struct thread_args {
	user* client;
	channel** channelList;
	user** userList;
}thread_args;

int client_ping(user *client);
int change_nickname(user *client, char *new_nick);
int join_channel(channel *chnl, user *client);
int send_msg(user* client, char* msg);
channel* search_channel(channel **channelList, char *channelName);
int search_user(char *clientName, channel *chnl);
int create_channel(channel** channelList, char* channelName, user *admin, int channel_count);
int mute_user(char *clientName, channel *chnl);
int unmute_user(char *clientName, channel *chnl);
int client_cmd(channel **channelList, user *userList, char *input, user *client, int channel_count, int user_count, bool *quit);
void* clientThread(void *args);
void interruptionHandler(int dummy);
int kick_user(char *clientName, channel* chnl);
int remove_user(user** userList, char *userName);
int quit_server(user *client, bool* quit, int user_count, user **userList);
int whois(user *client, char *input);

#endif