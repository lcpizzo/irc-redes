// C program for the Server Side

// inet_addr
#include <arpa/inet.h>

// For threading, link with lpthread
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <signal.h>
#include <assert.h>

#define MAX_MSG 4096

int myRead(int connfd, char *msg){
	bzero(msg, sizeof(msg));
	// recebe a mensagem
	if(recv(connfd, msg, sizeof(msg), 0) <= 0){
		printf("erro no recebimento\n");
		return -1;
	}
	// envia AK
	if(send(connfd, "AK", strlen("AK"), 0) <= 0){
		printf("erro no envio da resposta\n");
		return -1;
	}

    return 0;
}

#define MAX_MSG_LENGTH 4096

// Message type
typedef struct Message_t {
    char user[51];
    int req;
    char content[MAX_MSG_LENGTH];
} Message_t;

typedef struct messageBuffer_t{
	Message_t item[10];
	int size;
} messageBuffer_t;

messageBuffer_t buf;

// Semaphore variables
sem_t x, y;
pthread_t tid;
pthread_t writerthreads[100];
pthread_t readerthreads[100];
int readercount = 0;

// Reader Function
void* reader(void* param)
{
	// Lock the semaphore
	sem_wait(&x);
	readercount++;

	if (readercount == 1)
		sem_wait(&y);

	// Unlock the semaphore
	sem_post(&x);

	printf("%d Reader is inside\n",
		readercount);

	char input[4096];

	int newSocket = *((int*)param);

	send(newSocket,
		&buf.item[buf.size].user, sizeof(buf.item[buf.size].user), 0);
	
	send(newSocket,
		&buf.item[buf.size].content, sizeof(buf.item[buf.size].content), 0);
	
	// Lock the semaphore
	sem_wait(&x);
	readercount--;
	buf.size--;

	if (readercount == 0) {
		sem_post(&y);
	}

	// Lock the semaphore
	sem_post(&x);

	printf("%d Reader is leaving\n",
		readercount + 1);
	pthread_exit(NULL);
}

// Writer Function
void* writer(void* param)
{
	printf("Writer is trying to enter\n");

	// Lock the semaphore
	sem_wait(&y);

	printf("Writer has entered\n");

	int newSocket = *((int*)param);
	int erro = 0;
	char input[4096];

	recv(newSocket,
		&input, sizeof(input), 0);
	printf("user: %s\n", input);

	strcpy(buf.item[buf.size].user, input);

	bzero(input, sizeof(input));

	input[0] = '\0';
	recv(newSocket,
		&input, sizeof(input), 0);
	printf("content: %s\n", input);
	strcpy(buf.item[buf.size].content, input);
	

	erro = myRead(newSocket, input);
	printf("%s\n", input);
	bzero(input, sizeof(input));
	erro = myRead(newSocket, input);
	printf("%s\n", input);
	bzero(input, sizeof(input));
	erro = myRead(newSocket, input);
	printf("%s\n", input);
	bzero(input, sizeof(input));

	// Unlock the semaphore
	sem_post(&y);

	printf("Writer is leaving\n");

	buf.size++;
	
	pthread_exit(NULL);
}

// Handle 'ctrl + c' signal
void interruptionHandler(int dummy) {
    printf("\n'Ctrl + C' não é um comando válido!!\n");
}

// Auxiliary function to split strings
char** str_split(char* a_str, const char a_delim)
{
    char** result    = 0;
    size_t count     = 0;
    char* tmp        = a_str;
    char* last_comma = 0;
    char delim[2];
    delim[0] = a_delim;
    delim[1] = 0;

    /* Count how many elements will be extracted. */
    while (*tmp)
    {
        if (a_delim == *tmp)
        {
            count++;
            last_comma = tmp;
        }
        tmp++;
    }

    /* Add space for trailing token. */
    count += last_comma < (a_str + strlen(a_str) - 1);

    /* Add space for terminating null string so caller
       knows where the list of returned strings ends. */
    count++;

    result = malloc(sizeof(char*) * count);

    if (result)
    {
        size_t idx  = 0;
        char* token = strtok(a_str, delim);

        while (token)
        {
            assert(idx < count);
            *(result + idx++) = strdup(token);
            token = strtok(0, delim);
        }
        assert(idx == count - 1);
        *(result + idx) = 0;
    }

    return result;
}




// Driver Code
int main()
{
	// Initialize variables
	int serverSocket, newSocket;
	struct sockaddr_in serverAddr;
	struct sockaddr_storage serverStorage;

	// set handler function
	//signal(SIGINT, interruptionHandler);

	socklen_t addr_size;
	sem_init(&x, 0, 1);
	sem_init(&y, 0, 1);

	serverSocket = socket(AF_INET, SOCK_STREAM, 0);
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

    //printf("IP: %d\tPort:%d\n", serverAddr.sin_addr.s_addr, serverAddr.sin_port);

	// Array for thread
	pthread_t tid[60];

	int i = 0;

	while (1) {
		addr_size = sizeof(serverStorage);

		// Extract the first
		// connection in the queue
		newSocket = accept(serverSocket,
						(struct sockaddr*)&serverStorage,
						&addr_size);

		int choice = 0;
		recv(newSocket,
			&choice, sizeof(choice), 0);

		if (choice == 1) {
			// Creater readers thread
			if (pthread_create(&readerthreads[i++], NULL,
							reader, &newSocket)
				!= 0)

				// Error in creating thread
				printf("Failed to create thread\n");
		}
		else if (choice == 2) {
			// Create writers thread
			if (pthread_create(&writerthreads[i++], NULL,
							writer, &newSocket)
				!= 0)

				// Error in creating thread
				printf("Failed to create thread\n");
		} 
		else if (choice == 3) {

			char command[4096];
			recv(newSocket,
				&command, sizeof(command), 0);
			printf("%s\n", command);
			if (strncmp(command, "/ping", 5) == 0) {
				bzero(command, sizeof(command));
				strcpy(command, "pong\0");
				send(newSocket, command, strlen(command), 0);
				
			}else if (strncmp(command, "/join", 5) == 0) {
				printf("sera implementado no futuro.\n");
				send(newSocket, "WIP", strlen("WIP")+1, 0);

			}else if (strncmp(command, "/nickname", 9) == 0) {
				printf("sera implementado no futuro.\n");
				send(newSocket, "WIP", strlen("WIP")+1, 0);
				// sera implementado deposi durante a parte dos canais
			}else if (strncmp(command, "/kick", 5) == 0) {
				printf("sera implementado no futuro.\n");
				send(newSocket, "WIP", strlen("WIP")+1, 0);
				// sera implementado duante a parte dos canais
			}else if (strncmp(command, "/mute", 5) == 0) {
				printf("sera implementado no futuro.\n");
				send(newSocket, "WIP", strlen("WIP")+1, 0);
				// sera implementado durante a parte dos canais
			}else if (strncmp(command, "/unmute", 7) == 0) {
				printf("sera implementado no futuro.\n");
				send(newSocket, "WIP", strlen("WIP")+1, 0);
				// sera implementado durante a parte dos canais
			}

		}

		if (i >= 50) {
			// Update i
			i = 0;

			while (i < 50) {
				// Suspend execution of
				// the calling thread
				// until the target
				// thread terminates
				pthread_join(writerthreads[i++],
							NULL);
				pthread_join(readerthreads[i++],
							NULL);
			}

			// Update i
			i = 0;
		}
	}

	return 0;
}
