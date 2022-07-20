#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <signal.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <semaphore.h>

// Message type
#define MAX_MSG 4096

int quit = 0, t_run = 0, ak_quit = 0;

// thread de recebimento de mensagens do servidor
void* clientThread(void *sockID){
	int user_socket = *((int*) sockID);
	int cont = 0;
	t_run = 1;

	while(!quit) {
		char data[MAX_MSG];
		int read = recv(user_socket, data, MAX_MSG, 0);
		data[read] = '\0';
		if(strncmp(data, "AK/QUIT", 7) == 0){
			quit = 1;
			ak_quit = 1;	
			break;
		}
		printf("%s\n", data);
	}

	pthread_exit(NULL);
}

// Handle 'ctrl + c' signal
void interruptionHandler(int dummy) {
    printf("\n'Ctrl + C' não é um comando válido!!\n");
}

// Driver Code
int main() {	
	// set handler function
	//signal(SIGINT, interruptionHandler);
	
	int user_socket = socket(PF_INET, SOCK_STREAM, 0);

	struct sockaddr_in serverAddr;

	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(8989);
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);

	int conn = 0, user_set = 0, tries = 0;
	char input[MAX_MSG];

	// cria uma thread para receber respostas do servidor
	pthread_t tid;

	while(1) {
		tries = 0;
		bzero(input, sizeof(input));
		scanf("%[^\n]s", input);
		int ch;
		while ((ch = getchar()) != '\n' && ch != EOF);

		// conecta ao servidor
		if(strncmp(input, "/connect", 8) == 0){
			if(conn){
				printf("Voce ja esta conectado ao servidor...\n");
				continue;
			}
			if(connect(user_socket, (struct sockaddr*) &serverAddr, 
									sizeof(serverAddr)) == 0){
				printf("Conection established...\n");
				conn = 1;
				pthread_create(&tid, NULL, clientThread, (void *) &user_socket);
			}
			else
				printf("Conection error...\n");

		}

		// fecha a conexao
		else if (strncmp(input, "/quit", 5) == 0){
			quit = 1;
			if(conn){
				send(user_socket, "/quit", 5, 0);
				conn = 0;
				while(!ak_quit);
				close(user_socket);
				printf("Connection closed...\n");
			}
			break;
		}
		// muda o nickname e envia para o servidor o novo nome
		else if (strncmp(input, "/nickname", 9) == 0){
			if(!conn) {
				printf("Voce precisa estar conectado para definir um nome de usuario.\n");
				continue;
			}
			printf("New Username: %s\n", (input+10));
			send(user_socket, (input+10), strlen(input), 0);
			user_set = 1;
		}
		// envia a mensagem -> deve tentar 5 vezes no maximo antes de desistir
		else if (input[0] != '\0') {
			// checa se o cliente esta conectado ao servidor
			if(!conn){
				printf("Nao e possivel enviar uma mensagem sem estar conectado ao servidor. Use o comando /connect para se conectar.\n");
				continue;
			}
			// checa se o usuario ja possui um nome
			if(!user_set){
				printf("Nao e possivel enviar uma mensagem sem definir um nome de ususario. Use o comando /nickname [nome] para mudar seu nome.\n");
				continue;
			}
			// tenta enviar a mensagem ate 5 vezes se falhar desconecta o cliente
			while(send(user_socket, input, MAX_MSG, 0) == -1 && tries++ <= 5);
			if(tries == 5){
				printf("Erro ao enviar a mensagem.\n");
				if(conn)
					close(user_socket);
				break;
			}
		}
	}
	// Suspend execution of
	// calling thread
	if(t_run)
		pthread_join(tid, NULL);
	pthread_exit(NULL);
}

