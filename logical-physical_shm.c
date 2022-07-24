#include <unistd.h>
#include <stdio.h>
#include <inttypes.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/mman.h>

void vtop(uintptr_t vaddr) {
    FILE *pagemap;
    intptr_t paddr = 0;
    unsigned long offset = (vaddr / sysconf(_SC_PAGESIZE)) * sizeof(uint64_t);
    uint64_t e;
    if ((pagemap = fopen("/proc/self/pagemap", "r"))) {
        if (lseek(fileno(pagemap), offset, SEEK_SET) == offset) {
            if (fread(&e, sizeof(uint64_t), 1, pagemap)) {                
                if (e & (1ULL << 63)) { // page present ?
                    paddr = e & ((1ULL << 54) - 1); // pfn mask
                    paddr = paddr * sysconf(_SC_PAGESIZE);
                    paddr = paddr | (vaddr & (sysconf(_SC_PAGESIZE) - 1));
                }
            }
        }
        fclose(pagemap);
    }
        printf(" %" PRIxPTR , vaddr);
		printf(" %" PRIxPTR " \n", paddr);
}

int main(){
  const int SIZE = 4096;
  const char *name = "mem";

  int shm_fd;
  int *a;

  //print a address
  printf("[Processo 1] Enderecos 'a':\n");
  vtop((intptr_t)&a);
  printf("_______________________________________________\n");

  shm_fd = shm_open(name, O_CREAT | O_RDWR, 0666);
  ftruncate(shm_fd,SIZE);

  a = mmap(0,SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
  if (a == MAP_FAILED) {
    printf("Map failed\n");
    return -1;
  }

  int pid = fork();
  if(pid == 0){
    printf("[Processo 2] Enderecos 'a':\n");
    vtop((intptr_t)&a);
    exit(0);
  }
}

