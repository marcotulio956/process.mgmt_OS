#include <unistd.h>
#include <stdio.h>
#include <inttypes.h>
#include <stdlib.h>
#include <sys/wait.h>

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
		printf(" %" PRIxPTR "", paddr);
}

int main(){
    int arr[1025];
    printf("Primeiro endereco:\n");
    vtop((intptr_t)&arr[0]);
    printf("\n");

    printf("Ultimo endereco:\n");
    vtop((intptr_t)&arr[1024]);
    printf("\n");

    printf("Endereco do Array:");
    vtop((intptr_t)&arr);

    printf("\n\n");

    for(int i = 0; i < 1025; i++){
        printf("%d: ",i);vtop((intptr_t)&arr[i]);
        printf("\n");
    } 
    printf("\n");
} 
