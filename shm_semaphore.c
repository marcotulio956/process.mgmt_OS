#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <unistd.h>
#include <pthread.h>

#define NUM_PROCESS  100
#define NUM_STEPS 100000
#define MEM_NAME "mymem"

typedef struct mem_struct{
	int sum;
	sem_t sem;
}mem_struct;

int processBody(int id){
	int shm_fd;
	mem_struct* mem_ptr;
	shm_fd = shm_open(MEM_NAME, O_CREAT | O_RDWR, 0666);
	ftruncate(shm_fd,sizeof(mem_struct));
	mem_ptr = mmap(0,sizeof(mem_struct), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
	
	printf("\tantigo proc[%d].soma: %d\n", id, mem_ptr->sum);
	
	for(int i=0; i< NUM_STEPS; i++){
		sem_wait(&mem_ptr->sem);
		mem_ptr->sum += 1 ;   // critical section
		sem_post(&mem_ptr->sem);
	}
	
	printf("\tnovo proc[%d].soma: %d\n", id, mem_ptr->sum);

	return 0;
}

int main (int argc, char *argv[]){
	// criação do segmento de memória compartilhada com soma e semafaro
	int shm_fd;
	shm_fd = shm_open(MEM_NAME, O_CREAT | O_RDWR, 0666);
	ftruncate(shm_fd, sizeof(mem_struct));
	mem_struct* mem_ptr = mmap(NULL, sizeof(mem_struct), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
	if (mem_ptr == MAP_FAILED) {
		printf("Map failed\n");
		return -1;
	}
	
	// iniciacao do semafaro e soma na estrutura
	sem_init(&mem_ptr->sem, 1, 1);
	mem_ptr->sum = 0;
	
	// criação dos processos filhos
	printf("Sou o processo pai: %d\n", getpid());
	for(int i = 0; i < NUM_PROCESS; i++){
		int pid = fork();
		if (pid!= 0)
			printf("Criando: %d[%d]\n", pid,i);
		if(pid == 0){
			processBody(i);
			exit(0);
		}
	}
	
	// encerramentos
	for(int i = 0; i < NUM_PROCESS; i++){
		wait(0);
 	}
 	// resultado
	printf("Soma: %d\n", mem_ptr->sum);
 	
 	if (shm_unlink(MEM_NAME) == -1) {
		printf("Error removing %s\n",MEM_NAME);
		exit(-1);
	}
	
	return 0;
}
