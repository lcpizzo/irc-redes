#include <stdio.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>


#define MAX 5
#define PORT 8080
#define SA struct sockaddr

char *readline(FILE *stream) {
    char *string = (char *)calloc(MAX + 1, sizeof(char));
    int pos = 0;
    do{
		if(pos % MAX == 0){
			string = (char *)realloc(string, (pos / MAX + 1) * MAX);
        }
    	string[pos] = (char) fgetc(stream);
    }while (string[pos++] != '\n' && !feof(stream));
    string[strlen(string)-1] = '\n';
    
    return string;
}

void myWrite(int connfd, char *msg){
    int tam = strlen(msg);
    char temp[MAX];

    for(int i=0; i < tam/MAX; i++){
        bzero(temp, MAX);
        strncpy(temp, msg, MAX);
        write(connfd, temp, MAX);
        //printf("debug: %s\n", temp);
        msg += MAX;
    }
    write(connfd, msg, tam%MAX);
    //printf("debug: %s\n", msg);
    msg -= (tam/MAX) * MAX;
    //printf("original: %s", msg);
}

// Função de chat.
void myChat(int connfd){
    char *msg = (char *)calloc(MAX + 1, sizeof(char));
    int n;
    for (;;) {
        do{    
            // zera o buffer
            bzero(msg, MAX);
        
            // lê a mensagem do cliente e salva no buffer
            read(connfd, msg, sizeof(msg));
            printf("Mensagem do cliente: %s\n", msg);
            if((strncmp(msg, "sair", 4)) == 0){
                printf("Cliente saiu...\n");
                break;
            }
        }while(msg[strlen(msg)-1] != '\n');
        
        printf("\t Para o cliente: ");
        // zera o buffer
        bzero(msg, MAX);
        n = 0;
        // Recebendo nova mensagem
        msg = readline(stdin);
   
        // escreve a mensagem do cliente no terminal
        myWrite(connfd, msg);
        // checa se a mensagem do cliente é "sair" e termina a conexão.
        if (strncmp("sair", msg, 4) == 0) {
            printf("Servidor saiu......\n");
            break;
        }
    }
}
   

int main(){
    int sockfd, connfd, len;
    struct sockaddr_in servaddr, cli;
   
    // criação e verificação do socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        printf("Criacao do socket falhou...\n");
        exit(0);
    }
    else
        printf("Socket criado com sucesso...\n");
    bzero(&servaddr, sizeof(servaddr));
   
    // definição do IP, PORT
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(PORT);
   
    // Define sockfd como o "nome" do socket
    if ((bind(sockfd, (SA*)&servaddr, sizeof(servaddr))) != 0) {
        printf("Coneccao do socket falhou...\n");
        exit(0);
    }
    else
        printf("Coneccao bem sucedida..\n");
   
    // Server pronto para ouvir mensagens
    if ((listen(sockfd, 5)) != 0) {
        printf("Recebimento de mensagens falhou...\n");
        exit(0);
    }
    else
        printf("Servidor ouvindo..\n");
    len = sizeof(cli);
   
    // Aceita e verifica o pacote de dados do cliente
    connfd = accept(sockfd, (SA*)&cli, &len);
    if (connfd < 0) {
        printf("Servidor nao aceitou pacote do cliente...\n");
        exit(0);
    }
    else
        printf("Servidor aceitou pacote do cliente...\n");
   
    // Inicialização do chat
    myChat(connfd);

    close(sockfd);
}