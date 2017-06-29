#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>

#define LISTEN_PORT 12226
#define MAX_PENDING 1000
#define MAX_LINE 256
#define ESQUINA 11
#define TEMPO_SEGURANCA 3
#define TEMPO_SEMAFORO 5
#define MAX_CLIENTS 100000

long long current_timestamp() {
    struct timeval te;
    gettimeofday(&te, NULL); // get current time
    long long milliseconds = te.tv_sec * 1000LL + te.tv_usec / 1000; // caculate milliseconds
    // printf("milliseconds: %lld\n", milliseconds);
    return milliseconds;
}

void main() {
    struct sockaddr_in socket_address;
    char buf[MAX_LINE];
    unsigned int len;
    int s, new_s, i, k, j ;
    int semaforo = 0; // ccomeça com o eixo 0 aberto
    int altera = 0;
    long il, max, min, pos1, pos2, velocidade, eixo, direcao, delta, inverso, esquina, cruzamento[2];
    char numero[2];
    long long hora, saida, inicio, entrou[MAX_CLIENTS], saiu[MAX_CLIENTS];
    char *lixo;
    inicio = current_timestamp();


    /* criaÃ§Ã£o da estrutura de dados de endereÃ§o */
    bzero((char *) &socket_address, sizeof(socket_address));
    socket_address.sin_family = AF_INET;
    socket_address.sin_addr.s_addr = INADDR_ANY;
    socket_address.sin_port = htons(LISTEN_PORT);
    /* criaÃ§Ã£o de socket passivo */
    s = socket(AF_INET, SOCK_STREAM, 0);
    if (s < 0) {
        printf("[WARN] FALHA AO ABRIR SOCKET.\n");
        return;
    }
    /* Associar socket ao descritor */
    if (bind(s, (struct sockaddr *) &socket_address, sizeof(socket_address)) < 0) {
        printf("[WARN] FALHA AO ASSOCIAR SOCKET.\n");
        return;
    }
    /* Criar escuta do socket para aceitar conexÃµes */
    listen(s, MAX_PENDING);
    /* aguardar/aceita conexÃ£o, receber e imprimir texto na tela, enviar eco */
    while (1) {
        len = sizeof(socket_address);
        printf("[INFO] AGUARDANDO CCONEXAO.\n");
        new_s = accept(s, (struct sockaddr *) &socket_address, &len);
        if (new_s < 0) {
            printf("[WARN] FALHA DE CONEXAO.\n");
            continue;
        }
        bzero(buf, MAX_LINE);
        len = read(new_s, buf, MAX_LINE - 1);

        if (len < 0) {
            printf("[WARN] FALHA DE RECEBIMENTO.\n");
        } else {
            numero[0] = buf[0];
            numero[1] = buf[1];
            pos1 = strtol(numero, &lixo, 10);
            numero[0] = buf[3];
            numero[1] = buf[4];
            pos2 = strtol(numero, &lixo, 10);
            numero[0] = buf[6];
            numero[1] = buf[7];
            velocidade = strtol(numero, &lixo, 10);
            eixo = buf[9] - '0';
            direcao = buf[11] - '0';
            hora = current_timestamp();
            printf("[INFO] VEICULO\n POSICAO: [%ld,%ld]\n VELOCIDADE: %ld u.d/s\n EIXO: %ld\n DIRECAO: %ld\n HORA: %lld\n", pos1,
                   pos2, velocidade, eixo, direcao, hora - inicio);
            bzero(buf, MAX_LINE);
            if (pos1 > pos2) {
                max = pos1;
                min = pos2;
            } else {
                max = pos2;
                min = pos1;
            }
            // primeiro verifica se foi colisao
            esquina = 0;
            for (il = min; il <= max; ++il) {
                if (il == ESQUINA) {
                    esquina = 1;
                    // esta no cruzamento pelo eixo dele
                    cruzamento[eixo] = 1;
                    // ve quantass u.d faltam para ele sair do eixo
                    delta = pos2 - il;
                    saida = hora + delta*1000; // soma 1 egundo pra cada u.d faltante
                    printf("[INFO] PASSANDO PELA ESQUINA NO EIXO %ld ATÉ %lld\n",eixo, saida - inicio);
                    if (cruzamento[eixo] < saida) {
                        cruzamento[eixo] = saida;
                    }
                    inverso = eixo == 0 ? 1 : 0;
                    // ve se no outro eixo ja foi desocupado, se sim zera ele,  e verifica se o veiculo
                    // esta andando se não manda avançar
                    if (cruzamento[inverso] < hora) {
                        printf("[INFO] OUTRO EIXO VAZIO\n");
                        cruzamento[inverso] = 0;
                        if (velocidade == 0) {
                            strcpy(buf, "acelere");
                        } else {
                            strcpy(buf, "ignore");
                        }
                        break;
                    } else {
                        // se tem alguem no cruzamento no outro eixo deve chamar a ambulancia
                        printf("[WARN] COLISÃO\n");
                        strcpy(buf, "ambulancia");
                        cruzamento[0] = cruzamento[1] = 0;
                        break;
                    }
                }
            }

            // se nao esta na esquina ve se deve acelerar, frear ou ignorar
            if (!esquina) {
                // se o semaforo ta fechado para essa via e o carro esta a menos de 6 u.d freia
                delta = pos1 - ESQUINA;
                if (delta < 0) {
                    delta = delta * -1;
                }
                if (eixo != semaforo && delta < TEMPO_SEGURANCA) {
                    strcpy(buf, "freia");
                } else {
                    // se nao se ta parado acelera, se ja ta andando nao faz nada
                    if (velocidade == 0) {
                        strcpy(buf, "acelere");
                    } else {
                        strcpy(buf, "ignore");
                    }
                }
                printf("[INFO] SEMAFORO\n ABERTO: %d\n DISTANCIA ATE: %ld\n", semaforo, delta);
            }

            len = write(new_s, buf, strlen(buf));
            printf("[INFO] RESPOSTA: %s\n", buf);
            if (len < 0) {
                printf("[WARN] FALHA DE ENVIO\n");
            }
            printf("========================================================\n");

            // checa se é hora de trocar o semaforo
            if (altera++ % TEMPO_SEMAFORO == 0) {
                printf("[INFO] SEMAFORO %d FECHOU ", semaforo);
                if (semaforo == 1) {
                    semaforo = 0;
                } else {
                    semaforo = 1;
                }
                printf("e %d ABRIU\n", semaforo);
                printf("========================================================\n");
            }
        }
        close(new_s);
    }
}