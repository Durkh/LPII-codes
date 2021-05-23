#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <stdlib.h>

void gastarProcessamento();

int main(){

    pid_t descendente[2], mPid; 

    descendente[0] = fork();
    
    if( descendente[0] < 0){
        abort();
    }else if(descendente[0] == 0){ //primeiro filho
        
        gastarProcessamento();

        exit(0);
    }
    
    descendente[1] = fork();
    
    if(descendente[1] < 0){
        abort();
    }else if(descendente[1] == 0){ //segundo filho
        mPid = getpid();
        printf("[%d]: Descendente 1 foi criado", mPid);
        puts("..");
        
        for(int i = 100; i <= 200; i++){
            printf("[%d]: %d\n", mPid, i);
            puts("..");
            sleep(1);
        }
        printf("[%d]: Descendente 1 vai morrer", mPid);
        puts("..");
    
        exit(0);
    }
    
    mPid = getpid();
    for(int i = 1; i <= 50; i++){
        printf("[%d]: %d\n", mPid, i);
        puts("..");
        sleep(1);
    }
    printf("[%d]: Processo original parou de contar\n", mPid);
    puts("..");
    kill(descendente[0], SIGTERM);

    exit(0);

}

void gastarProcessamento(){
    int x, y;
    
    for (x = 0; x < 99999999; x++)
        for (y = 0; y < 60; y++);

}
