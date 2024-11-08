#include <stdio.h>
#include <elf.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <signal.h>

Elf32_Ehdr *ehdr;
Elf32_Phdr *phdr;
Elf32_Phdr **phdr_arr;
int fd;
int pagefaults=0;

typedef struct mapped_phdr {
    Elf32_Phdr* m_phdr;
    int size;} 

mapped_phdr;
mapped_pdhr **delete_arr;
int no_phdr;

void sigsegvHandler(int signo, siginfo_t *info, void *context){
    if (signo==SIGSEGV){
        void* addr=(*info).si_addr;
        pagefaults++;
        for(int i=0;i<(*ehdr).e_phnum;i++){

        }
    }
}

void free_mapped_phdr(mapped_phdr* phdr){
    if(phdr){
        if (munmap((*phdr).m_phdr,(*phdr).size)==-1){
            perror("Error: munmap");
            exit(0);
        }
        free(phdr);
}}
void clean_up(){
    free(ehdr);
    for(int i=0; i<no_phdr; i++){
        free(phdr_arr[i];)
    }
    free(phdr_arr);

    if(delete_arr){
        for(int i=0; i<no_phdr;i++){
            free_mapped_phdr(delete_arr[i]);
        }
        free(delete_arr);
}}

int main(int argc,char** argv){
    if(argc!=2){
        printf("Incorrect or missing arguments");
        exit(1);
    }
    //error handling for ELF file
    fd=open(argv[1], O_RDONLY);
    if(fd==-1){ 
        perror("Error while opening");
        exit(0);
    }
    int e_size=sizeof(Elf32_Ehdr);
    ehdr=(Elf32_Ehdr*)malloc(e_size*sizeof(char));                
    if(ehdr==NULL){ 
        perror("Error allocating malloc");
        exit(0);
    }
    lseek(fd,0,SEEK_SET);
    read(fd,ehdr,e_size*sizeof(char));
    //checking magic numbers
    int x;
    if(!ehdr)
        x=1;
    else if((*ehdr).e_ident[EI_MAG0]!=ELFMAG0 || (*ehdr).e_ident[EI_MAG1]!=ELFMAG1 || (*ehdr).e_ident[EI_MAG2]!=ELFMAG2 || (*ehdr).e_ident[EI_MAG3]!=ELFMAG3)
        x=1;    
    if(x==1){
        printf("Invalid ELF file.\n");
        free(ehdr);
        exit(0);
    }
    free(ehdr);
    int close_error=close(fd);
    if (close_error==-1){
        perror("Error in closing elf");
        exit(0);
    }

    struct sigaction sig;
    memset(&sig,0,sizeof(sig));
    sig.sa_sigaction=sigsegvHandler;
    sig.sa_flags=SA_SIGINFO;
    if(sigaction(SIGSEGV,&sig,NULL)==-1){
        perror("Error: sigaction");
        exit(0);
    }

    load_and_run_elf(&argv[1]);
    clean_up();
    return 0;
}
