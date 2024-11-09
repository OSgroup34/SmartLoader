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
int fd;
int pagefaults=0;
int numOfPages=0;
int mappedSize;
Elf32_Phdr *mappedPhdr;
int PAGESIZE=4096;
size_t fragmentation;
void* pageAllocated;
void* mappedPages[100];
char* heapmemalloc;
//assuming max pages required 100

void cleanup();

void loadAndRunElf(char** exe){
    fd=open(*exe, O_RDONLY);
    off_t size=lseek(fd,0,SEEK_END);
    //error handling
    if (size==-1){
        printf("Error in getting file size\n");
        exit(1);
    }
    lseek(fd,0,SEEK_SET);
    //entire file memory allocation
    heapmemalloc=(char*)malloc(size);
    //error handling
    if (!heapmemalloc){
        printf("error in memory allocation\n");
        exit(1);
    }
    ssize_t readfile=read(fd, heapmemalloc,size);

    //error handling
    if (readfile<0 || (size_t)readfile!=size){
    perror("Error in reading file");
    free(heapmemalloc);
    exit(1);
    }

    ehdr=(Elf32_Ehdr*)heapmemalloc;
    phdr=(Elf32_Phdr*)(heapmemalloc+(*ehdr).e_phoff);

    int x;
    if(!ehdr)
        x=1;
    else if((*ehdr).e_ident[EI_MAG0]!=ELFMAG0 || (*ehdr).e_ident[EI_MAG1]!=ELFMAG1 || (*ehdr).e_ident[EI_MAG2]!=ELFMAG2 || (*ehdr).e_ident[EI_MAG3]!=ELFMAG3)
        x=1;    
    if(x==1){
        printf("Invalid ELF file.\n");
        cleanup();
        exit(0);
    }

    int (*_start)(void) = (int (*)(void))(uintptr_t)(*ehdr).e_entry;
    int result = _start();
    printf("User _start return value = %d\n", result);
    printf("Total page faults: %d\n",pagefaults);
    printf("Pages Allocated: %d\n",numOfPages);
    printf("Total fragmentation (in KB): %0.4f bytes\n",(double)fragmentation);
    }


void sigsegvHandler(int signo, siginfo_t *info, void *context){
    if (signo==SIGSEGV){
        pagefaults++;
        void* addr=(*info).si_addr;
        size_t pageStart=((size_t)addr/PAGESIZE)*PAGESIZE;
        int segment=-1;
        for(int i=0;i<(*ehdr).e_phnum;i++){
            if ((*(phdr+i)).p_type==PT_LOAD && (uintptr_t)((*(phdr + i)).p_vaddr) <= (uintptr_t)addr && ((uintptr_t)((*(phdr + i)).p_vaddr)+(*(phdr + i)).p_memsz)>=(uintptr_t)(info->si_addr)){
                segment=i;
                break;
            }
        }
        if (segment==-1){
            printf("fault not occured in given segments.");
            exit(1);
        }
        void *pageAddr=(void *)((uintptr_t)((*(phdr+segment)).p_vaddr)+numOfPages*PAGESIZE);
        pageAllocated=mmap(pageAddr,PAGESIZE, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_PRIVATE | MAP_FIXED, fd,((((uintptr_t)(*(phdr+segment)).p_offset)/PAGESIZE)*PAGESIZE+numOfPages*PAGESIZE));
        mappedPages[numOfPages]=pageAllocated;
        numOfPages++;
        if (pageAllocated == MAP_FAILED){
        printf("mmap error");
        exit(1);}
        fragmentation=PAGESIZE-(*(phdr+segment)).p_memsz;
    }
}

void cleanup(){
    free(heapmemalloc);
    if(mappedPages){
        for(int i=0; i<numOfPages;i++){
            if (munmap(mappedPages[i],PAGESIZE)==-1){
            perror("munmap error");
            exit(0);
        }}}
    if (close(fd)==-1){
        perror("Error in closing elf");
        exit(0);
    }
}

int main(int argc,char** argv){
    if(argc!=2){
        printf("Incorrect or missing arguments");
        exit(1);
    }
    struct sigaction sig;
    memset(&sig,0,sizeof(sig));
    sig.sa_sigaction=sigsegvHandler;
    sig.sa_flags=SA_SIGINFO;
    if(sigaction(SIGSEGV,&sig,NULL)==-1){
        perror("Error: sigaction");
        exit(0);
    }

    loadAndRunElf(&argv[1]);
    cleanup();
    return 0;
}