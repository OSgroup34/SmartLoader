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
int fd;
int pagefaults=0;

void sigsegvHandler(int signo, siginfo_t *info, void *context){
    if (signo==SIGSEGV){
        void* addr=(*info).si_addr;
        pagefaults++;
        for(int i=0;i<(*ehdr).e_phnum;i++){

        }
    }
}

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
    int a=Check_Magic_Num(ehdr);
    if(a==1){
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
    loader_cleanup();
    return 0;
}
