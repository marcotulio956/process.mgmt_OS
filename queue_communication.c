// por Marco Túlio & Júlio Cesar, Lab SO, Tp8
/*
Em anexo: “tp8.c”, “common.h”
Compilação: “$ gcc tp8.c -lrt -o <nome>”
Execução: “./<nome> <num_processos>"
*/
#include <pthread.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/resource.h>

#include <string.h>
#include <sys/stat.h>
#include <errno.h>
#include <mqueue.h>

#include "common.h"

#define QUEUE_NAME  "/test_queue"
#define MAX_SIZE    1024
#define MSG_STOP    "exit"

#define BILLION 1000000000.0

double func(unsigned* id, unsigned num_processes){
    int ID=(*id);
    double sum =0.0,x;
    unsigned long long int i;
    unsigned long long int passos = 15*BILLION;
    double h=1.0/passos;
    for (i = ID + 1; i <= passos; i += num_processes) {
        x = h * ((double)i - 0.5); 
        sum += 4.0 / (1.0 + x*x); 
    }
    double result = h*sum;
    printf("proc[%d] partial result: %lf\n", ID, result);
    return result;
}

int send_to_queue(char* value){
    mqd_t mq;
    char buffer[MAX_SIZE];//buffer tera o dado value*(double) em chars
    struct mq_attr attr;
    /* Abre a fila de mensagens.*/
    mq = mq_open(QUEUE_NAME, O_WRONLY);
    CHECK((mqd_t)-1 != mq);
    //printf("Send to server:\n");
    //do {
        //printf(": ");
        //fflush(stdout);
        memset(buffer, 0, MAX_SIZE);
        //fgets(buffer, MAX_SIZE, stdin); //Leitura da mensagem
        memcpy(buffer, value, 50);
        /* Envia mensagem */
            //'0' -> prioridade
        CHECK( mq_send(mq, buffer, MAX_SIZE, 0) >= 0);
          //Verifica quantas mensagens existem na fila.
        CHECK( mq_getattr(mq, &attr) != -1);
        printf("%ld messages on queue.\n",attr.mq_curmsgs);
    //} while (strncmp(buffer, MSG_STOP, strlen(MSG_STOP)));
    /* Encerra a conexão com a fila */
    CHECK((mqd_t)-1 != mq_close(mq));
    return 0;
}

int main(int argc, char *argv[]){
    unsigned num_processes;
    if(argc == 2){
        num_processes = atoi(argv[1]);

    } else {
        printf("Para executar o programa, digite: ./<prog_nome>.out <numero de processos>\n");
        printf("Exemplo: ./calculo_pi_fila 10\n");
        exit(1);
    }

    // criação dos processos
    int pid;
    for(int i = 0; i < num_processes; i++){
        pid = fork();
        if(pid == 0){// executado pelos p_filhos
            unsigned new_pid = getpid();
            printf("New proc: %d\n", new_pid);
            double value = func(&new_pid, num_processes);
            char str_value[50];
            snprintf(str_value, 50, "%f", value);
            send_to_queue(str_value);
            //char* sair = "exit"; 
            //send_to_queue(sair);
            exit(0);
        }
    }

    //tratamento do resultado dos processos
    if(pid > 0){ // estamos em p_pai
        double pi = 0.0;

        int receive_count = 0;

        mqd_t mq;
        struct mq_attr attr;
        char buffer[MAX_SIZE + 1];
        int must_stop = 0;

        /* Inicializa os atributos da fila*/
        /*"0": o processo será bloqueado caso não haja 
        mensagens na fila. "O_NONBLOCK": a função de 
        leitura retorna código de erro e não é bloqueada.*/ 
        attr.mq_flags = 0; 
        //Número máximo de mensagens na fila. 
        attr.mq_maxmsg = 10;
        //Tamanho máximo de cada mensagem.
        attr.mq_msgsize = MAX_SIZE;
        //Número de mensagens que estão atualmente na fila.
        attr.mq_curmsgs = 0;

        /* Abre a fila de mensagens.*/
        mq = mq_open(QUEUE_NAME, O_CREAT | O_RDONLY, 0644, &attr);

        //Continua a execução do programa se a condição 
        //abaixo for verdadeira.
        CHECK(mq !=(mqd_t)-1); //se (mq != -1)

        do {
            ssize_t bytes_read;
            /* Recebe a mensagem. O último parâmetro pode ser
             usado para ler a prioridade da mensagem recebida.
             Se a fila estiver vazia, o comando pode bloquear
             o processo ou retornar uma mensagem de erro 
            (dependendo do parâmetro: attr.mq_flags)*/
            bytes_read = mq_receive(mq, buffer, MAX_SIZE, NULL);
            CHECK(bytes_read >= 0);
            //Adiciona um '\0' ao final da mensagem lida.
            buffer[bytes_read] = '\0';

            //Verifica se a mensagem contém o texto "exit"
            // if (! strncmp(buffer, MSG_STOP, strlen(MSG_STOP)))
            //     must_stop = 1;
            // else
            //     //Exibe a mensagem recebida.
            printf("Received: %s\n", buffer);
            pi = pi + atof(buffer);//msgs lidas correspondem ao calculo parcial de pi
            printf("pi = %lf\n", pi);
            //O processo aguarda 5seg. para ler a próxima mensagem.
            receive_count++;
            printf("receive_count: %d\n", receive_count);
            if (receive_count >= num_processes){//queremos uma mensagem apenas dos n processos
                must_stop = 1;
            }
            sleep(0.5);
        } while (!must_stop);
        //Fecha a fila. 
        CHECK((mqd_t)-1 != mq_close(mq));
        //Remove a fila.
        CHECK((mqd_t)-1 != mq_unlink(QUEUE_NAME));
    }
    return 0;
}
