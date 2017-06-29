/**************************
****** Cliente TCP *******
**************************/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>

#define SERVER_PORT_SEGURANCA 12226

#define SERVER_PORT_MIDIA 12005
#define MAX_LINE 256
#define TEMPO_ENVIO_DE_DADOS 1
#define TEMPO_ENVIO_DE_DADOS_M 100000
#define LADO_TABULEIRO 22

long long current_timestamp() {
    struct timeval te;
    gettimeofday(&te, NULL); // get current time
    long long milliseconds = te.tv_sec * 1000LL + te.tv_usec / 1000; // caculate milliseconds
    // printf("milliseconds: %lld\n", milliseconds);
    return milliseconds;
}
int main(int argc, char *argv[]) {

    /* variaveis para host e socket 1 */
    struct hostent *host_address; /* host adress de seguranca */
    struct sockaddr_in socket_address;
    char *host; /*host de seguranca */
    char buf[MAX_LINE], eco[MAX_LINE + 10];
    int s;
    int len;
    int bytessent, bytesreceived;

    /* variaveis para host e socket 2 */
    struct hostent *host_address2; /* host adress de midia/conforto */
    struct sockaddr_in socket_address2; /* socket adress de midia/conforto */
    char *host2; /* host de media/conforto */
    char buf2[MAX_LINE], eco2[MAX_LINE + 10]; /* buffer para o host de midia/conforto */
    int s2;
    int len2;
    int bytessent2, bytesreceived2;
    long long now;

    /* variaveis de controle das condicoes do automovel */
    int tipo; /* moto, carro, onibus */
    int tamanho; /* tamanho do veiculo */
    int velocidade; /* velocidade do veiculo */
    int posicao_frente; /* posicao inicial da frente do veiculo no cruzamento */
    int posicao_tras; /* posicao inicial da parte de tras do veiculo no cruzamento */
    int eixo; /* em qual das ruas o veiculo se encontra (X(0) ou Y(1)) */
    int sentido; /* 0: cima para baixo ou esquerda para direita; 1: baixo para cima ou direita para esquerda */
    int contador = 1; /* contador do numero de mensagens trocadas com o servidor de midia */
    int gerador_pedido;

    /* variaveis para manutencao de strings */
    char posicao_frente_str[10];
    char posicao_tras_str[10];
    char velocidade_str[10];
    char *tipo_str;
    char *eixo_str;
    char *sentido_str;
    char *lixo;


    /* verificação de argumentos */
    if (argc != 6) {
        fprintf(stderr, "Numero de argumentos invalido\n");
        fprintf(stderr, "Use:./client [hostname1][hostname2][Tipo][Eixo][Sentido]\n");
        return -1;
    }

    /* atribuicao dos argumentos as devidas variaveis */
    host = argv[1];
    host2 = argv[2];
    tipo_str = argv[3];
    eixo_str = argv[4];
    sentido_str = argv[5];

    /* conversao dos paramentros em string para int */
    tipo = strtol(tipo_str, &lixo, 10);
    eixo = strtol(eixo_str, &lixo, 10);
    sentido = strtol(sentido_str, &lixo, 10);

    /*determinacao dos paramentos iniciais */
    if (tipo == 0)
        tamanho = 1;
    else if (tipo == 1)
        tamanho = 2;
    else
        tamanho = 3;

    if (sentido == 0) {
        posicao_tras = 0;

        if (tamanho == 1)
            posicao_frente = 0;
        else if (tamanho == 2)
            posicao_frente = 1;
        else
            posicao_frente = 3;
    } else {
        posicao_tras = LADO_TABULEIRO - 1;

        if (tamanho == 1)
            posicao_frente = LADO_TABULEIRO - 1;
        else if (tamanho == 2)
            posicao_frente = LADO_TABULEIRO - 2;
        else
            posicao_frente = LADO_TABULEIRO - 3;

    }

    velocidade = 1;

    printf("Estabelecidas as condicoes iniciais do veiculo\n");

    /* tradução de nome para endereço IP */
    host_address = gethostbyname(host);
    host_address2 = gethostbyname(host2);

    if (host_address == NULL) {
        fprintf(stderr, "Falha na resolucao do nome de servidor de seguranca.\n");
        return -1;
    }

    printf("Consegui hostbyname de seguranca\n");

    if (host_address2 == NULL) {
        fprintf(stderr, "Falha na resolucao do nome de servidor de midia/conforto.\n");
        return -1;
    }

    printf("Consegui hostbyname de midia/conforto\n");

    /* criação da estrutura de dados de endereço */

    /* relacionadas ao socket 1*/
    bzero(&buf, MAX_LINE);

    bzero((char *) &socket_address, sizeof(socket_address));
    socket_address.sin_family = AF_INET;
    bcopy(host_address->h_addr, (char *) &socket_address.sin_addr, host_address->h_length);
    socket_address.sin_port = htons(SERVER_PORT_SEGURANCA);
    len = sizeof(socket_address);

    /* relacionadas ao socket 2*/
    bzero(&buf2, MAX_LINE);

    bzero((char *) &socket_address2, sizeof(socket_address2));
    socket_address2.sin_family = AF_INET;
    bcopy(host_address2->h_addr, (char *) &socket_address2.sin_addr, host_address2->h_length);
    socket_address2.sin_port = htons(SERVER_PORT_MIDIA);
    len2 = sizeof(socket_address2);

    /* criação de socket ativo de midia/conforto*/
    s2 = socket(AF_INET, SOCK_STREAM, 0);
    if (s2 == -1) {
        fprintf(stderr, "Falha na criacao de socket ativo de midia/conforto.\n");
        return -1;
    }

    printf("Socket de midia/conforto Criado\n");

    /* estabelecimento da conexão com servidor de midia/conforto*/
    if (connect(s2, (struct sockaddr *) &socket_address2, len2) == -1) {
        fprintf(stderr, "Falha no estabelecimento da conexao com servidor de midia.\n");
        return -1;
    }

    printf("Conectado ao servidor de midia/conforto\n");

    /* informacoes do socket de midia/conforto local */
    getsockname(s2, (struct sockaddr *) &socket_address2, len2);
    printf("\nInformacoes do socket de midia/conforto local:\nIP: %s\nPorta: %d\n\n",
           inet_ntoa((struct in_addr) socket_address2.sin_addr), socket_address2.sin_port);

    /* ler e enviar linhas de texto, receber eco */
    now = current_timestamp();
    while (1) {

        /* envio de informacoes ao servidor de seguranca */
        sprintf(posicao_frente_str, "%02d", posicao_frente);
        sprintf(posicao_tras_str, "%02d", posicao_tras);
        sprintf(velocidade_str, "%02d", velocidade);

        strcat(posicao_frente_str, " ");
        strcat(posicao_frente_str, posicao_tras_str);
        strcat(posicao_frente_str, " ");
        strcat(posicao_frente_str, velocidade_str);
        strcat(posicao_frente_str, " ");
        strcat(posicao_frente_str, eixo_str);
        strcat(posicao_frente_str, " ");
        strcat(posicao_frente_str, sentido_str);


        strcpy(buf, posicao_frente_str);

        /* estabelecimento da conexão com servidor de seguranca*/

        /* criação de socket ativo de seguranca*/
        s = socket(AF_INET, SOCK_STREAM, 0);
        if (s == -1) {
            fprintf(stderr, "Falha na criacao de socket ativo de seguranca.\n");
            //return -1;
        }

        printf("Socket de Seguranca Criado\n");

        if (connect(s, (struct sockaddr *) &socket_address, len) == -1) {
            fprintf(stderr, "Falha no estabelecimento da conexao com servidor de seguranca.\n");
            // return -1;
        }

        printf("Conectado ao servidor de seguranca\n");

        /* informacoes do socket de seguranca local */
        getsockname(s2, (struct sockaddr *) &socket_address2, len2);
        printf("\nInformacoes do socket de seguranca local:\nIP: %s\nPorta: %d\n\n",
               inet_ntoa((struct in_addr) socket_address.sin_addr), socket_address.sin_port);


        bytessent = send(s, &buf, MAX_LINE, 0);
        bytesreceived = recv(s, &eco, MAX_LINE + 10, 0);
        printf("\nMensagem recebida do servidor de seguranca:\n%s\n", eco);

        /* ajustes de acordo com a resposta do servidor de seguranca */
        if (strcmp(eco, "freie") == 0)
            velocidade = 0;
        if (strcmp(eco, "acelere") == 0)
            velocidade = 1;
        if (strcmp(eco, "ambulancia") == 0) {
            printf("chame a ambulancia!!!\n");
            break;
        }

        /* fechamento da conexao com o servidor de seguranca */
        printf("fechando conexao com servidor de seguranca\n");
        close(s);


        /* envio de informacoes ao servidor de midia/conforto */
        gerador_pedido = rand() % 2;

        if (gerador_pedido == 0)
            strcpy(buf2, "M: me ve uma musica daora ai som!");
        else
            strcpy(buf2, "C: me ve uma massagem relaxante!");
        bytessent2 = send(s2, &buf2, MAX_LINE, 0);
        bytesreceived2 = recv(s2, &eco2, MAX_LINE + 10, 0);

        printf("\nMensagem recebida do servidor de midia/conforto:\n%d: %s\n", contador, eco2);

        contador++;

        /* faz o envio de dados a cada TEMPO_ENVIO_DE_DADOS segundos */
        usleep(TEMPO_ENVIO_DE_DADOS_M);

        /* atualizacao da posicao do veiculo */
        if (velocidade == 1 && sentido == 0) {
            posicao_frente += TEMPO_ENVIO_DE_DADOS;
            posicao_tras += TEMPO_ENVIO_DE_DADOS;
        }
        if (velocidade == 1 && sentido == 1) {
            posicao_frente -= TEMPO_ENVIO_DE_DADOS;
            posicao_tras -= TEMPO_ENVIO_DE_DADOS;
        }

        printf("Minha posicao = %d\n", posicao_frente);

        /* se o veiculo terminar a rua, fechamos o cliente */
        if (posicao_frente >= LADO_TABULEIRO || posicao_frente < 0) {
            printf("fechou\n");
            break;
        }

    }
    close(s2);

    return 0;
}
