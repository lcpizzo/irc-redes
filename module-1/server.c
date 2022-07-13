#include <stdio.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>

#define MAX 4096
#define PORT 8080
#define SA struct sockaddr
#define FALSE 0
#define TRUE 1

char *readline(FILE *stream) {
    char *string = (char *)calloc(MAX + 1, sizeof(char));
    int pos = 0;
    
    do{
        if (pos % MAX == 0) {
            string = (char *)realloc(string, (pos / MAX + 1) * MAX);
        }
        string[pos] = (char)fgetc(stream);
    }while(string[pos++] != '\n' && !feof(stream));
    string[strlen(string)-1] = '\n';
    
    return string;
}

int myRead(int connfd, char *msg, int *exit){
    // loop para receber mensagens de varias partes
    for(;;){
        bzero(msg, MAX);
        read(connfd, msg, MAX);
        if(strncmp(msg, "AK", 2) == 0){
            //termina comm
            write(connfd, "AK", 2);
            return FALSE;
        }
        if(strncmp(msg, "sair", 4) == 0)
            *exit = TRUE;
        if(msg[strlen(msg)-1] !="\n") strcat(msg, "\n");
        printf("Mensagem do cliente: %s", msg);
        write(connfd, "AK", 2);
    }

    // confirma que recebeu o fim de msg
    bzero(msg, sizeof(msg));
    read(connfd, msg, sizeof(msg));
    if(strncmp(msg, "AK", 2) !=0){
        printf("erro\n");
        return TRUE;
    }
    write(connfd, "AK", 2);

    return FALSE;
}

int myWrite(int connfd, char *msg, int *exit){
    char temp[MAX+1];

    bzero(msg, MAX);
    printf("Para o servidor: ");
    msg = readline(stdin);

    // checa se o servidor saiu
    if(strncmp(msg, "sair", 4) == 0)
        *exit = TRUE;

    int tam = strlen(msg)+1;
    // loop para dividir a msg em varias partes
    for(int i=0; i < tam/MAX; i++){
        bzero(temp, MAX + 1);
        strncpy(temp, msg, MAX);
        write(connfd, temp, MAX + 1);
        msg += MAX;
        write(connfd, temp, MAX);
        bzero(temp, sizeof(temp));
        read(connfd, temp, sizeof(temp));
        if(strncmp(temp, "AK", 2) != 0){
            printf("erro\n");
            break;
        }
    }
    write(connfd, msg, tam%MAX);

    read(connfd, temp, sizeof(temp));
    if(strncmp(temp, "AK", 2)!= 0){
        printf("erro\n");
        return TRUE;
    }

    // envia a ultima confirmaçao indicando que acabou a mensagem
    write(connfd, "AK", 2);

    bzero(temp, sizeof(temp));
    read(connfd, temp, sizeof(temp));
    if(strncmp(temp, "AK", 2) != 0){
        printf("erro\n");
        return TRUE;
    }

    return FALSE;
}


// Função de chat.
void myChat(int connfd){
    int exit = FALSE, erro = FALSE;
    char *msg = (char *)calloc(MAX + 1, sizeof(char));

    for (;;) {
        // cliente envia a msg, server envia confirmaçao que recebeu:
        //   se acabou a msg cliente confirma a confirmaçao
        //   cc: cliente envia o resto da mensagem e volta pra cima
        erro = myRead(connfd, msg, &exit);
        if(erro)
            break;

        if(exit){
            printf("Cliente saiu...\n");
            break;
        }

        // le a msg do terminal, se for maior que MAX separa em varias
        //  partes.
        // Envia uma parte, quando confirmar continua ate a msg acabar
        //  envia uma ultima confirmaçao indicando que a msg acabou
        erro = myWrite(connfd, msg, &exit);
        if(erro)
            break;

        if(exit){
            printf("Servidor saiu...\n");
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

    // Fecha o socket
    close(sockfd);
}