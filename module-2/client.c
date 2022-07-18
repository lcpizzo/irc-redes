// C program for the Client Side
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <signal.h>


// inet_addr
#include <arpa/inet.h>
#include <unistd.h>

// For threading, link with lpthread
#include <pthread.h>
#include <semaphore.h>

#include "aux.h"

// Message type
#define MAX_MSG 4096
#define READLINE_BUFFER 4096

typedef struct Message_t {
    char user[51];
    int req;
    char content[MAX_MSG];
}Message_t;

#define CONTINUE 100
#define QUIT 200

int myWrite(int connfd, char *msg){
	char rec[3];
	bzero(rec, sizeof(rec));

	// Sends the message
	if(send(connfd, msg, sizeof(msg), 0) <= 0){
		printf("erro no envio\n");
		return -1;
	}

	// Waits for the AK
	if(recv(connfd, rec, sizeof(rec), 0) <= 0){
		printf("erro no recebimento da resposta\n");
		return -1;
	}
	if(strncmp(rec, "AK", 2) != 0){
		printf("erro, resposta nao e AK\n");
		return -1;
	}

    return 0;
}


// Function to send data to
// server socket.
/*
void* clienthread_read(void *param)
{
	//Message_t msg = *((Message_t*) param[0]);
	//int network_socket = *((int*) param[1]);
	
	// receive data from the server
	char input[4096];
	send(network_socket, &msg.req,
		sizeof(msg.req), 0);

	recv(network_socket, &input, sizeof(input), 0);
	printf("user: %s\n", input);
	
	bzero(input, sizeof(input));

	recv(network_socket, &input, sizeof(input), 0);
	printf("content: %s\n", input);

	pthread_exit(NULL);

	return 0;
}

// Function to send data to
// server socket.
void* clienthread_write(void *param)
{
	//Message_t msg = *((Message_t*) param[0]);
	//int network_socket = *((int*)param[1]);

	// Send data to the socket
	send(network_socket, &msg.req,
		sizeof(msg.req), 0);

	//int erro = myWrite(networkSocket,);
	
	pthread_exit(NULL);

	return 0;
}*/

char *readline(FILE *stream) {
    char *string = (char *)calloc(MAX_MSG + 1, sizeof(char));
    int pos = 0;
    
    do{
        if (pos % MAX_MSG == 0) {
            string = (char *)realloc(string, (pos / MAX_MSG + 1) * MAX_MSG);
        }
        string[pos] = (char)fgetc(stream);
    }while(string[pos++] != '\n' && !feof(stream));
    string[strlen(string)-1] = '\n';

    return string;
}

int quit_server(int conn, int network_socket){
	if (conn){
		conn = 0;
		close(network_socket);
		printf("Connection Closed.\n");
	}
	
	return QUIT;
}

int conct(int *network_socket){
	// Create a stream socket
	*network_socket = socket(AF_INET,
							SOCK_STREAM, 0);

	// Initialise port number and address
	struct sockaddr_in server_address;
	server_address.sin_family = AF_INET;
	server_address.sin_addr.s_addr = INADDR_ANY;
	server_address.sin_port = htons(8989);

	// Initiate a socket connection
	int connection_status = connect(*network_socket,
									(struct sockaddr*)&server_address,
									sizeof(server_address));

	// Check for connection error
	if (connection_status < 0) {
		puts("Error\n");
		return 0;
	}

	printf("Connection established\n");
	return 1;
}

// Handle 'ctrl + c' signal
void interruptionHandler(int dummy) {
    printf("\n'Ctrl + C' não é um comando válido!!\n");
}

void* clienthread_send(void** params){
	int network_socket = *((int*)params[0]);
	char user[51];
	char input[4096];
	strcpy(user, ((char*)params[1]));
	strcpy(input, ((char*)params[2]));
	int erro = 0;
	
	// Envia a mensagem em 3 partes -> a flag de identificaçao 
	//		da msg, o usuario que enviou a mensagem e o conteudo dela
	erro = myWrite(network_socket, "2");
	erro = myWrite(network_socket, user);
	erro = myWrite(network_socket, input);

	
	pthread_exit(NULL);
}

// Driver Code
int main() {	
	// set handler function
	//signal(SIGINT, interruptionHandler);
	
	pthread_t tid;
    int cmd = CONTINUE, conn = 0, network_socket, user_set = 0, exit = 0, erro = 0;
	char user[51] = "", input[4096];

	void* params[5];

    while(cmd != QUIT) {
		bzero(input, sizeof(input));
        strcpy(input, readline(stdin));

		if (feof(stdin)){
			cmd = quit_server(conn, network_socket);
			continue;
		}
    

        // input that starts with / is likely a command
        if (input[0] == '/') {
			if(strncmp(input, "/quit", 5) == 0){
				// Closes the connection
				cmd = quit_server(conn, network_socket);
			} else if (strncmp(input, "/ping", 5) == 0){
				// Envia ping para o servidor 
				// TODO botar em uma funcao
				if(conn){
					int req = 3;
					send(network_socket, &req, sizeof(req), 0);
					send(network_socket, "/ping", strlen("/ping")+1, 0);
					bzero(input, sizeof(input));
					recv(network_socket, &input, sizeof(input), 0);
					printf("%s\n", input);
				} else {
					printf("Voce deve estar conectado ao servidor para enviar mensagens. Use o comando /connect.\n");
				}
			} else if (strncmp(input, "/connect", 8) == 0){
				// Conecta ao server                    
				conn = conct(&network_socket);
			} else if (strncmp(input, "/nickname", 9) == 0){
				// Muda o nick
				strncpy(user, (input+10), strlen(input) - 11);
				user_set = 1;
			} else {
				//Comando nao reconhecido
				printf("Comando nao reconhecido.\n");
				int req = 3;
				send(network_socket, &req, sizeof(req), 0);
				bzero(input, sizeof(input));
				recv(network_socket, &input, sizeof(input), 0);
				printf("%s\n", input);
			}
        } else if (input[0] != '\0'){
			// texto eh uma mensagem
			if(!conn){
				printf("Voce deve estar conectado ao servidor para enviar mensagens. Use o comando /connect.\n");
				continue;
			}
			
			if(!user_set){
				printf("Voce deve definir um nome de usuario antes de poder enviar mensagens. Use o comando /nickname [nome] para mudar seu nome.\n");
				continue;
			}
			
			params[0] = (void*)(&network_socket);
			params[1] = (void*)(&user);
			params[2] = (void*)(&input);

			pthread_create(&tid, NULL,
				clienthread_send,
				&params);
		}
    }
	// Suspend execution of
	// calling thread
	pthread_join(tid, NULL);
}

