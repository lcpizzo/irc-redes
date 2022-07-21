#ifndef _CHATFUNCTIONS_H_
#define _CHATFUNCTIONS_H_

#define MAX_MSG 4096
#define MAX_USERS 1024
#define MAX_CHANNELS 10

// O usuario vai estar "logado" em apenas um canal por vez, entao
//  podemos adicionar dois attr a mais, um bool admin e um bool mute,
// 	para checar se o usuario eh um admin e se esta mutado 
typedef struct user {
	struct sockaddr_in socketAddr;
	char name[51];
	int sockID;
	int len;
	int index;
	bool connected;
	bool mute;
	bool admin;
	struct channel *conn_channel;
} user;

typedef struct channel {
	user* channel_users[MAX_CHANNELS];
	user* admin;
	int n_members;
	char *channelName;
} channel;

int client_ping(user *client);
int change_nickname(user *client, char *new_nick);
int join_channel(channel *chnl, user *client);
int send_msg(user **userList, char *msg, int user_count);
channel *search_channel(channel **channelList, char *channelName);
user* search_user(char *clientName, channel *chnl);
int create_channel(channel **channelList, char* channelName, user *admin, int channel_count);
int mute_user(char *clientName, channel *chnl);
int unmute_user(char *clientName, channel *chnl);
int client_cmd(channel **channelList, user **userList, char *input, user *client, int channel_count, int user_count);
void* clientThread(void *Client, user **userList, int user_count);
void interruptionHandler(int dummy);

#endif