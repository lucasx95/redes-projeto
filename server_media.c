/**************************
****** Servidor TCP *******
**************************/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>

#define LISTEN_PORT 12005
#define MAX_PENDING 1000
#define MAX_LINE 256
#define SLEEP_CONSTANT 1000000

int main() {

    struct sockaddr_in socket_address, cliaddr;
    char buf[MAX_LINE], eco[MAX_LINE + 10];
    unsigned int len;
    int s, new_s;
    int bytessent, bytesreceived, gerador_musical;
    pid_t forkresult;

    /* criação da estrutura de dados de endereço */
    bzero((char *) &cliaddr, sizeof(cliaddr));
    bzero(&buf, MAX_LINE);

    len = sizeof(socket_address);
    bzero((char *) &socket_address, sizeof(socket_address));
    socket_address.sin_family = AF_INET;
    socket_address.sin_addr.s_addr = INADDR_ANY;
    socket_address.sin_port = htons(LISTEN_PORT);

    /* criação de socket passivo */
    s = socket(AF_INET, SOCK_STREAM, 0);
    if (s == -1) {
        printf("[WARN] FALHA AO ABRIR SOCKET.\n");
        return -1;
    }

    /* Associar socket ao descritor */
    if (bind(s, (struct sockaddr *) &socket_address, len) == -1) {
        printf("[WARN] FALHA AO ASSOCIAR SOCKET.\n");
        return -1;
    }

    /* Criar escuta do socket para aceitar conexões */
    if (listen(s, MAX_PENDING) == -1) {
        printf("[WARN] FALHA AO ASSOCIAR SOCKET A ESCUTA.\n");
        return -1;
    }

    /* aguardar/aceitar conexão, receber e imprimir texto na tela, enviar eco */
    while (1) {

        new_s = accept(s, (struct sockaddr *) &cliaddr, &len);
        if (new_s == -1) {
            printf("[WARN] FALHA DE CONEXAO.\n");
            return -1;
        }

        forkresult = fork();
        if (forkresult < 0) {
            printf("[WARN] FALHA NO FORK.\n");
            return -1;

        } else if (forkresult == 0) {
            getpeername(s, (struct sockaddr *) &cliaddr, &len);
            printf("[INFO] AGUARDANDO MENSAGEM.\n");
            while (recv(new_s, &buf, MAX_LINE, 0) > 0) {
                usleep(SLEEP_CONSTANT);
                printf("MENSAGEM RECEBIDA:\nIP %s: %s\n", inet_ntoa((struct in_addr) cliaddr.sin_addr), buf);
                if (buf[0] == 'M') {

                    /* o que sera que faremos tocar na radio deste cliente */
                    gerador_musical = rand() % 3;
                    if (gerador_musical == 0)
                        strcpy(eco, "M: du bi du bi duuuuu");
                    if (gerador_musical == 1)
                        strcpy(eco, "M: obladi oblada lalalaaaaaa");
                    if (gerador_musical == 2)
                        strcpy(eco, "M: ah oh ih uh eeeeeeeee");
                }

                    /* se o cliente quiser conteudo de conforto */
                else {
                    strcpy(eco, "C: vrum vrum costas relaxadas vrum vrum");
                }

                bytessent = send(new_s, &eco, MAX_LINE + 10, 0);

                printf("========================================================\n");
                printf("[INFO] AGUARDANDO MENSAGEM.\n");
            }
            printf("[INFO] IP: %s DESCONECTADO!\n", inet_ntoa((struct in_addr) cliaddr.sin_addr));
            close(new_s);
            close(s);
            return 0;

        } else {
            getpeername(s, (struct sockaddr *) &cliaddr, &len);
            printf("[INFO] CONEXAO ACEITA:\nInformacoes do socket remoto:\nIP: %s\nPorta: %d\n\n",
                   inet_ntoa((struct in_addr) cliaddr.sin_addr), cliaddr.sin_port);
            continue;
        }
    }

    close(new_s);
    close(s);
    return 0;
}




