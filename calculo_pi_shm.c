#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>

#define BILLION 1000000000.0 //for steps

int producer(int id, int num_processes){
 printf("\tSou o proc[%d] (%d)  executando producer\n", getpid(), id);

 const int SIZE = 4096;
 const char *name = "piMem";

 int shm_fd;
 double *ptr;

 shm_fd = shm_open(name, O_CREAT | O_RDWR, 0666);
 ftruncate(shm_fd,SIZE);

 ptr = mmap(0,SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
 if (ptr == MAP_FAILED) {
  printf("Map failed\n");
  return -1;
 }

 double sum = 0.0, x;
 unsigned long long int i;
 unsigned long long int passos = 5 * BILLION;
  
 double h = 1.0/passos;
 for (i = id + 1; i <= passos; i += num_processes) {
  x = h * ((double)i - 0.5); 
  sum += 4.0 / (1.0 + x * x); 
 }

 double resultado = h * sum;
 printf("Processo %d - Resultado parcial: %lf\n", id, resultado);

 *(ptr+8*id) = resultado;

 printf("Resultado em Shared: %lf\n", *(ptr+8*id));
}

int main(int argc, char *argv[]){
 const char *name = "piMem";
	const int SIZE = 4096;

 int num_processes;

 if(argc == 2){
   num_processes = atoi(argv[1]);
 }
 else{
   printf("Para executar o programa informe 1 parâmetro.\n");
   printf("O parâmetro deve ser o número de processos que irá calcular o valor de pi.\n");
   printf("\n *Exemplo:   \n$./a.out 2\n");
   return 1;
 } 

	int shm_fd;
	double* ptr;

 // criação do segmento de memória compartilhada
 shm_fd = shm_open(name, O_CREAT | O_RDWR, 0666);
 ftruncate(shm_fd, SIZE);
 ptr = mmap(NULL, SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);

 printf("Sou o processo pai: %d\n", getpid());
 // criação dos processos filhos
 for(int i = 0; i < num_processes; i++){
  int pid = fork();
  if(pid == 0){
   printf("Producer Write @ %ld\n", ptr+i);
   producer(i, num_processes);
   exit(0);
  }
 }
 for(int i = 0; i < num_processes; i++){
   wait(NULL);
 }

 //printando a soma de todos os calculos parciais localizado na memoria compartilhada
 double pi = 0.0; 

 for(int i=0, adrs = 0; i<num_processes; i++, adrs+=8){
  printf("%ld %ld -> %lf %lf\n", ptr, adrs, *(ptr), *(ptr+adrs));
  pi += *(ptr+adrs);
 }

 printf("Soma de todo calculo parcial: %lf\n", pi);

  // remove segmento de memória compartilhada
	if (shm_unlink(name) == -1) {
		printf("Error removing %s\n",name);
		exit(-1);
	}
  
  return 0;
}
