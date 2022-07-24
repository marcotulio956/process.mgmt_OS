// Marco e Julio

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <stdlib.h>

#define BUFFER_SIZE 10
#define READ_END	0
#define WRITE_END	1

#define BILLION 1000000000.0

double func(unsigned* id, unsigned num_processes){
	int ID = (*id);
	unsigned long long int i, steps = 5*BILLION;
	double sum = 0.0, x, h = 1.0/steps, result; 

	for(i = ID+1; i<= steps; i += num_processes){
		x = h*((double)i-0.5);
		sum += 4.0 / (1.0+x*x);
	}

	result=h*sum;
	printf("proc[%d] partial result: %lf\n", ID, result);
	return result;
}

void send_to_pipe(char* write_msg, int* fd){
	printf("proc[%d] +writing to the pipe: %s\n",getpid(),write_msg);
	write(fd[WRITE_END], write_msg, BUFFER_SIZE); 
}

int receive_from_pipe(char* read_msg, int* fd){
	printf("proc[%d] ...waiting data from the pipe\n",getpid());

	ssize_t s = read(fd[READ_END], read_msg, BUFFER_SIZE);
	if(s){
		printf("proc[%d] -reading from the pipe: %s\n",getpid(),read_msg);	
		return 1;
	}else{
		printf("Parent: nothing more to read\n");	
		return 0;
	}
}


int main(int argc, char *argv[]){
	unsigned num_processes;

	if(argc == 2){
		num_processes = atoi(argv[1]);
	} else{
		printf("Para executar o programa, digite: ./<prog_nome> <numero de processos>\n");
        printf("Exemplo: ./calculo_pi_pipe 10\n");
        exit(1);
	}

	int pid;
	printf("Processo Pai ID: %u\n", getpid());

	int fd[2];

	if(pipe(fd) == -1){
		fprintf(stderr, "Pipe Falhou");
		return 1;
	}

	int i;
	for(i=0; i<num_processes; i++){//cria processos
		pid = fork();
		if(pid < 0){//erro
			fprintf(stderr, "%dº Fork Falhou", i);
			return 1;
		} else if(pid == 0){
			break;//ja estamos em filho
		}
	}

	if(pid > 0) {//processo pai
		close(fd[WRITE_END]);//nao rescreveremos aqui

		//filhos terminam, tudo ja esta escrito no pipe
		for(unsigned int i = 0; i< num_processes; i++){
			printf("proc[%d] !terminou\n", wait(0));
		}

		printf("Criados %u processos.\n", num_processes);

		double pi = 0.0;
		char* read_msg = (char*)malloc(sizeof(char)*BUFFER_SIZE);//de recebimento do pipe
		int count_received = 0;
		do{
			receive_from_pipe(read_msg, fd);
			pi = pi + atof(read_msg);
			count_received++;
		}while(count_received < num_processes);

		close(fd[READ_END]);//nao havera mais leituras

		printf("Total PI = %lf\n", pi);
	} else if(pid == 0){ //filho
		close(fd[READ_END]);//sem leitura, so escrita

		unsigned new_pid = getpid();
		printf("\tNovo Processo Filho ID: %u\n", new_pid);

		double value = func(&new_pid, num_processes);

		char str_value[10];
        snprintf(str_value, 10, "%f", value);
		send_to_pipe(str_value, fd);

		close(fd[WRITE_END]);//acabamos escrita

		exit(0);
	}
	return 0;
}
