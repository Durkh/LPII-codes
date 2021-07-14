#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#ifdef __linux__ 
#include <bsd/string.h>

#else
size_t strlcat(char* dest, const char* src, size_t siz){
    size_t dlen, n = siz;
    char* d = dest;
    const char* s = src;

    while(n-- != 0 && *d != '\0')
        d++;

    dlen = d - dest; 
    n = siz - dlen;

    if(n == 0)
        return(dlen + strlen(s));

    for(; *s != '\0'; s++){
        if(n != 1){
            *d++ = *s;
            n--;
        }
    }
    *d = '\0';

    return (dlen + (s-src));
}
#endif


#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include <pthread.h>
#include <semaphore.h>

#include <assert.h>

#define SERVER_PORT 9000
#define QTD_CLIENTS 10
#define QTD_EXIT    5


#define PRINT_ERROR(msg)    perror(msg);\
                            exit(EXIT_FAILURE)

#define PUTS_AND_LINE(x)    puts(x);\
                            puts("...")

#define PRINT_AND_LINE(x, ...)  printf(x "\n", ##__VA_ARGS__); \
                                puts("...")

#ifdef VERBOSE
    unsigned closingReq = 0;
#endif


typedef struct {

    char buf[1024];
    pthread_mutex_t bufferMutex;

}tBuffer;

typedef struct {
    
    long  clientFD;
    sem_t *client_sem;

}tClient;

tBuffer shrdBuffer = { {'\0'}, PTHREAD_MUTEX_INITIALIZER};
pthread_barrier_t finishingBarrier;


void* ClientHandler(void* args);


int main(){

    int serverFD;
    tClient clients[QTD_CLIENTS];
    sem_t clientQtdLimit;
    
    struct sockaddr_in serverAddr, clientAddr[QTD_CLIENTS];

    unsigned sizeClientAddr[QTD_CLIENTS];

    pthread_t handlers[QTD_CLIENTS];

    //zera os FDs para sinalizar vaga aberta
    for(int i=0; i<QTD_CLIENTS; i++){
        clients[i].clientFD = 0;
    }

    //inicializa barreira e semáforo
    sem_init(&clientQtdLimit, 0, 10);

    if(pthread_barrier_init(&finishingBarrier, NULL, QTD_EXIT)){
        PRINT_ERROR("BARRIER INIT ERROR");   
    }


    //cria o socket
    serverFD  = socket(AF_INET, SOCK_STREAM, 0);

    if(serverFD == -1){
        PRINT_ERROR("SOCKET CREATION ERROR");
    }
    
    memset(&serverAddr, 0, sizeof(serverAddr));

    //configura o endereço do socket
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddr.sin_port = htons(SERVER_PORT);

    //liga o socket ao endereço configurado
    if(bind(serverFD, (struct sockaddr*)&serverAddr, sizeof(serverAddr))){
        PRINT_ERROR("SOCKET BIND ERROR");
    }

    //escuta a porta do socket
    if(listen(serverFD, QTD_CLIENTS)){
       PRINT_ERROR("SOCKET LISTEN ERROR"); 
    }

    printf("Listening port %d\n", SERVER_PORT);


    int t_idx;
    while(1){
        
        sem_wait(&clientQtdLimit);

        //seta o t_idx na vaga vazia, por conta do semáforo garantimos que sempre que chegue aqui terá ao menos 1 vaga aberta
        for(t_idx = 0; clients[t_idx].clientFD == 0; t_idx++);

        //garante que t_idx não estoura o array
        assert(t_idx <= QTD_CLIENTS);

        //aceita a conexão
        clients[t_idx].clientFD = accept(serverFD, (struct sockaddr*)&clientAddr[t_idx], &sizeClientAddr[t_idx]);
        clients[t_idx].client_sem = &clientQtdLimit;
        PUTS_AND_LINE("connection received");

        pthread_create(&handlers[t_idx], NULL, ClientHandler, (void*)&clients[t_idx]);
    }

}


_Noreturn void* ClientHandler(void* args){

    tClient* client = (tClient*)args;
    char* greeting = "START\n";
    char buf[1026];

    //envia a saudação ao cliente
    write(client->clientFD, greeting, strlen(greeting));

    while(1){

        memset(buf, '\0', 1026);

#ifdef VERBOSE

        PUTS_AND_LINE("reading from client");

#endif
        //lê o que foi enviado pelo cliente
        if(read(client->clientFD, buf, 1024) < 0){
            PRINT_ERROR("READ ERROR");
        }

        PRINT_AND_LINE("read from client %ld: %s", client->clientFD, buf);

        //checa se é uma mensagem válida
        if(strlen(buf) > 0){
            if(strstr(buf, "SHUTDOWN")){

#ifdef VERBOSE

                PRINT_AND_LINE("%d/4 closing requests", __atomic_add_fetch(&closingReq, 1, __ATOMIC_SEQ_CST));

#endif
                //fecha a conexão com o cliente
                close(client->clientFD);
                //seta o clientfFD como zero para abrir uma vaga e libera o semaforo
                client->clientFD = 0;
                sem_post(client->client_sem);

                //barreira de espera de saída
                pthread_barrier_wait(&finishingBarrier);
                PUTS_AND_LINE("CLOSING THE SERVER");
                pthread_exit(EXIT_SUCCESS);         

            }else{

#ifdef VERBOSE
                PUTS_AND_LINE("concatenating the string");
#endif

                //trava o mutex para acesso ao buffer
                pthread_mutex_lock(&shrdBuffer.bufferMutex);

                //garante que a string lida termina com o '/0' para o funcionamento ideal do strlcat
                assert(buf[1025] == '\0');

                if(strlcat(shrdBuffer.buf, buf , 1024) >= 1024){
                    errno = ENOMEM;                    
                    //chamada válida pois está protegido pelo mutex
                    PRINT_ERROR("SHARED BUFFER OVERFLOW");
                }

                write(client->clientFD, shrdBuffer.buf, strlen(shrdBuffer.buf)); 
                //responde ao cliente com o buffer compartilhado

                pthread_mutex_unlock(&shrdBuffer.bufferMutex);

#ifdef VERBOSE

                PUTS_AND_LINE("end of response");

#endif
            }       
        } 
    }
    
    //esse código nunca é alcançado mas é bom deixar explícito
    pthread_exit(EXIT_SUCCESS);
}
