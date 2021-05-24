#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>


//  MACROS

#define PRINT_AND_LINE(x, ...) printf(x "\n", ##__VA_ARGS__); \
                            puts("...")

#define PUTS_AND_LINE(x)    puts(x); \
                            puts("...")

//  DEFINES YOU CAN TWEAK

#define N_THREADS   25  //numero de passageiros
#define N_SEATS     10  //numero de assentos no carro
#define N_TRIPS     10  //numero de viagens até o parque fechar
#define TRIP_TIME   10  //tempo em segundos da viagem do carro

//velocidade da chegada dos visitantes
//OBS: deixe só 1 descomentado
#define FAST
//#define MEDIUM
//#define REALISTIC

//  TYPE DEFINITIONS

#ifdef FAST
#define VISIT_RATE 1
#endif

#ifdef MEDIUM
#define VISIT_RATE 2
#endif

#ifdef REALISTIC
#define VISIT_RATE ((rand()%5)+5)
#endif

typedef unsigned uint;

//  GLOBAL VARIABLES

uint ticketNumber = 1, ticketCalled = 1, wallet[N_THREADS];

pthread_mutex_t carMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t boardingMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_barrier_t lineBarrier;
pthread_barrier_t boardingBarrier;
pthread_cond_t tripsCond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t tripsMutex = PTHREAD_MUTEX_INITIALIZER;

//   FUNCTIONS HEADERS

void* Passenger(void* args);
void* Car();
void* ParkManager();


int main() {

    srand(time(NULL));

    //barreira para controlar se existem passageiros suficientes para o carro sair
    if(pthread_barrier_init(&lineBarrier, NULL, N_SEATS)){
        PUTS_AND_LINE("LINE LOCK INIT ERROR");
        exit(EXIT_FAILURE);
    }

    //barreira inicializada com a quantidade de passageiros e o carro
    if(pthread_barrier_init(&boardingBarrier, NULL, N_SEATS + 1)){
        PUTS_AND_LINE("BOARDING LOCK INIT ERROR");
        exit(EXIT_FAILURE);
    }


    pthread_t passengers[N_THREADS], car, manager;

    pthread_create(&manager, NULL, ParkManager, NULL);

    pthread_create(&car, NULL, Car, NULL);

    PUTS_AND_LINE(" *** Parque aberto, visitantes estão entrando ***");
    for(long i=1; i<=N_THREADS; i++){
        pthread_create(&passengers[i], NULL, Passenger, (void*)i);
        //aos poucos os visitantes vão chegando no parque
        //coloquei esse sleep porque fica melhor de ver as coisas, mas se quiser colocar 0 no define, sem problemas
        sleep(VISIT_RATE);
    }

    pthread_join(manager, NULL);

}


//  FUNCTIONS DECLARATIONS


_Noreturn void* Passenger(void* args){

    long ID = (long)args;

    while (1){
        //pega um ticket
        wallet[ID] = __sync_fetch_and_add(&ticketNumber, 1);
        PRINT_AND_LINE("-- Cliente %ld: estou na fila com o ticket No %u --", ID, wallet[ID]);

        //espera seu número ser chamado
        while(wallet[ID] != ticketCalled);

        //se o carro está em movimento o embarque deve parar
        pthread_mutex_lock(&boardingMutex);
        //se pôde entrar na fila do embarque, libera a entrada do próx. passageiro
        __sync_fetch_and_add(&ticketCalled, 1);
        pthread_mutex_unlock(&boardingMutex);

        PRINT_AND_LINE("-- Cliente %ld: estou esperando para embarcar no carro --", ID);
        //barreira só abre se N_SEATS pessoas chegarem aqui
        pthread_barrier_wait(&lineBarrier);

        PRINT_AND_LINE("-- Cliente %ld: estou embarcando no carro --", ID);
        pthread_barrier_wait(&boardingBarrier);

        //espera a volta acabar, o carro lockou o mutex, então o passageiro só prossegue após o carro desbloquear
        //"depois que entrou no carro só sai quando der a volta" esse é o ditado
        pthread_mutex_lock(&carMutex);
        pthread_mutex_unlock(&carMutex);

        //sai do carro
        pthread_barrier_wait(&boardingBarrier);

        PRINT_AND_LINE("Cliente %ld: saí do carro e vou passear no parque", ID);
        sleep((rand()%30) + 5); //numero entre 5 e 34
    }

}

_Noreturn void* Car(){

    while(1) {

        PUTS_AND_LINE("// Carro pronto na estação, passageiros podem embarcar //");
        //impede os passageiros que entram de prosseguir
        pthread_mutex_lock(&carMutex);

        //espera os passageiros entrarem
        pthread_barrier_wait(&boardingBarrier);

        //carro cheio e pronto para sair, fecha o embarque
        pthread_mutex_lock(&boardingMutex);

        PUTS_AND_LINE("// Carro cheio e pronto para dar uma volta //");
        sleep(TRIP_TIME);

        PUTS_AND_LINE("// Volta do carro conluída, liberando os passageiros //");
        pthread_mutex_unlock(&carMutex);

        //incrementa o contador de volta
        pthread_mutex_lock(&tripsMutex);
        pthread_cond_signal(&tripsCond);
        pthread_mutex_unlock(&tripsMutex);

        //espera os passageiros saírem
        pthread_barrier_wait(&boardingBarrier);
        //libera o embarque
        pthread_mutex_unlock(&boardingMutex);
    }
}

_Noreturn void* ParkManager(){
    pthread_mutex_lock(&tripsMutex);
    //fica no for esperando sinal, quando recebe um sinal incrementa a variável e espera outro sinal
    //quando recebeu sinais suficientes, retorna
    for (uint trips = 0; trips < N_TRIPS; trips++){
        pthread_cond_wait(&tripsCond, &tripsMutex);
    }
    pthread_mutex_unlock(&tripsMutex);

    PUTS_AND_LINE("*** Parque fechando, todos os visitantes devem deixar o local ***");

    exit(EXIT_SUCCESS);
}