/**
 * Código base (incompleto) para implementação de relógios vetoriais.
 * Meta: implementar a interação entre três processos ilustrada na figura
 * da URL:
 *
 * https://people.cs.rutgers.edu/~pxk/417/notes/images/clocks-vector.png
 *
 * Compilação: mpicc -o rvet rvet.c
 * mpicc -o rvet rvet.c -Wall -lpthread
 *
 * Execução: mpiexec -n 3 ./rvet
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>
#include <pthread.h>
#include <semaphore.h>
#include "fila.c"

/*
snapshot: P1 -> <- P0 -> <- P2 -> <- P1
1. P0 faz uma copia do estado local;
2. P0 entrada um <marcador> para os canais de saida;
3. Quando um processo receber um <marcador> ele deve fazer um snapshot local e entrada um <marcador> para seus canais de saida;
4. Salvar os estados dos canais de entrada em cada processo, as mensagens recebidas antes do <marcador>;
5. Snapshot esta concluido quando recebe um <marcador> em todos os canais de entrada;
6. Marcador = (-1, -1, -1)
*/

sem_t semFullInput;
sem_t semEmptyInput;

sem_t semFullOutput;
sem_t semEmptyOutput;

// pthread_mutex_t mutexQueueInput;
// pthread_mutex_t mutexQueueOutput;
pthread_mutex_t mutexClock;

// pthread_cond_t condFullInput;
// pthread_cond_t condEmptyInput;

// pthread_cond_t condFullOutput;
// pthread_cond_t condEmptyOutput;


typedef struct 
{
	int inicializador;
    CLOCK clock;
    CLOCK snapClock;
    QUEUE inputQueue;
    QUEUE outputQueue;
	int markerCheck[2]; // 0 para aguardando e 1 para concluido 
} pData;

// void snapshot(Status *st)
// {
//     // usar um semaforo para bloquear o acesso ao relogios (ou mutex)

//     // fazer uma copia do relogio atual e dos canais de entrada

//     printf("> Snapshot: %d , Clock: (%d, %d, %d)\n", st->pid, st->clock.p[0], st->clock.p[1], st->clock.p[2]);
    
//     if(st->pid == 0)
//     {
//         if(st->inicializador == 1)
//         {
//             st->snapClock = st->clock;
//                 //armazenar na fila de saída os marcadores para ou outros processos
//         }
//         else
//         {
//             if (st->recebeuMarcador[0] == 0 && st->recebeuMarcador[1] == 0)
//             {
//                 st->recebeuMarcador[0] = 1;
//                 st->snapClock = st->clock;
//                 //MPI_Send(marca, 3, MPI_INT, 1, 0, MPI_COMM_WORLD);
//                 //MPI_Send(marca, 3, MPI_INT, 2, 0, MPI_COMM_WORLD);
//             }
//         }
//     }
// }

void event(pData *data)
{
    pthread_mutex_lock(&mutexClock);
    data->clock.p[data->clock.pid]++;
    pthread_mutex_unlock(&mutexClock);
    
    printf("Processo: %d, Clock: (%d, %d, %d)\n", data->clock.pid, data->clock.p[0], data->clock.p[1], data->clock.p[2]); 
}

void clocksComparison(CLOCK *clockE, pData *data)
{
    if (data->clock.pid == 0)
    {
        if (clockE->p[1] >= data->clock.p[1])
            data->clock.p[1] = clockE->p[1];

        if (clockE->p[2] >= data->clock.p[2])
            data->clock.p[2] = clockE->p[2];
    }
    else if (data->clock.pid == 1)
    {
        if (clockE->p[0] >= data->clock.p[0])
            data->clock.p[0] = clockE->p[0];

        if (clockE->p[2] >= data->clock.p[2])
            data->clock.p[2] = clockE->p[2];
    }
    else if (data->clock.pid == 2)
    {
        if (clockE->p[0] >= data->clock.p[0])
            data->clock.p[0] = clockE->p[0];

        if (clockE->p[1] >= data->clock.p[1])
            data->clock.p[1] = clockE->p[1];
    }
}

void sender(int destinationP, pData *data)
{
    
    // sem_wait(&semEmptyOutput);
    // pthread_mutex_lock(&mutexQueueOutput);
    
    // while(cheia(&data->outputQueue))
    // {
    //     pthread_cond_wait(&condFullOutput, &mutexQueueOutput);
    // }
    
    event(data);
    data->clock.destination = destinationP;
    if(!inserir(data->clock, &data->outputQueue))
        printf("as");
    
    
    // pthread_mutex_unlock(&mutexQueueOutput);
    // sem_post(&semFullOutput);
    // // pthread_cond_signal(&condEmptyOutput);
}

void receive(int pid, int pidE, pData *data)
{
    
    // sem_wait(&semFullInput);
    // pthread_mutex_lock(&mutexQueueInput);
    
    // while(vazia(&data->inputQueue))
    // {
    //     pthread_cond_wait(&condEmptyInput, &mutexQueueInput);
    // }
    
    CLOCK clockR = retirar(&data->inputQueue);
    clocksComparison(&clockR, data);
    event(data);
    
    // pthread_mutex_unlock(&mutexQueueInput);
    // sem_post(&semEmptyInput);
    
    // pthread_cond_signal(&condFullInput);
    
}   

void *despachante(void *d)
{
    // tem de incluir um controle para comecar a enviar os clocks para os
    // snapshots apartir do primeiro marcador recebido
    while (1)
    {
        pData *data = (pData*) d;
    
        CLOCK clockR = {.p[0] = 0, .p[1] = 1, .p[2] = 0};
        
        MPI_Recv(&clockR, 3, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        printf("Thread Despachante  ---  Processo: %d, Clock: (%d, %d, %d)\n", data->clock.pid, clockR.p[0], clockR.p[1], clockR.p[2]); 
        if (clockR.p[0] == -1 && clockR.p[0] == -1 && clockR.p[0] == -1) // testa se e um marcador
        {
            printf("\t >>> Chamar snapshot");
            //snapshot();
        }
        else
        {
            // sem_wait(&semEmptyInput);
            // pthread_mutex_lock(&mutexQueueInput);
            
            inserir(clockR, &data->inputQueue);
            // pthread_mutex_unlock(&mutexQueueInput);
            // sem_post(&semFullInput);
            
            
        }
    }
    return NULL;
}
    

void *transmissao(void *d)
{
    while(1)
    {
        pData *data = (pData*) d;
    
        // sem_wait(&semFullOutput);    
        // pthread_mutex_lock(&mutexQueueOutput);
        // while(vazia(&data->outputQueue))
        // {
        //     pthread_cond_wait(&condEmptyOutput, &mutexQueueOutput);
        // }
        if(!vazia(&data->outputQueue))
        {
            printf("Thread transmissao ----  ");
            imprimir(&data->outputQueue);
        }
        CLOCK clockS = retirar(&data->outputQueue);
        MPI_Send(&clockS, 3, MPI_INT, clockS.destination , 0, MPI_COMM_WORLD);
        // pthread_mutex_unlock(&mutexQueueOutput);
        // sem_post(&semEmptyOutput);
        
        // pthread_cond_signal(&condFullOutput);
    }
    return NULL;
}

// Representa o processo de rank 0
void process0()
{
    pthread_t t1, t2;
    pData *data;
    data = malloc(sizeof(pData));
    
    
    inicializar(&data->inputQueue);
    inicializar(&data->outputQueue);
    data->clock.pid = 0;
    data->clock.p[0] = 0;
    data->clock.p[1] = 0;
    data->clock.p[2] = 0;
    
    pthread_create(&t1, NULL, despachante, (void*) data);
    pthread_create(&t2, NULL, transmissao, (void*) data);
    
    printf("Processo: %d, Clock: (%d, %d, %d)\n", 0, data->clock.p[0], data->clock.p[1], data->clock.p[2]);
    
    event(data);
    sender(1, data);
    receive(0, 1, data);
    sender(2, data);
    receive(0, 2, data);
    // data.inicializador = 1;
    //snapshot(0, st0); // iniciando o snapshot

    sender(1, data);
    event(data);
    
    //printf(" \nFinal: % d , Clock: (%d, %d, %d) \n ", 0, st0->clock.p[0], st0->clock.p[1], st0->clock.p[2]);
    free(data);
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
}

// Representa o processo de rank 1
void process1()
{
    pthread_t t1, t2;
    pData *data;
    data = malloc(sizeof(pData));
    
    inicializar(&data->inputQueue);
    inicializar(&data->outputQueue);
    data->clock.pid = 1;
    data->clock.p[0] = 0;
    data->clock.p[1] = 0;
    data->clock.p[2] = 0;
    
    pthread_create(&t1, NULL, despachante, (void *)data);
    pthread_create(&t2, NULL, transmissao, (void *)data);
    
    printf("Processo: %d, Clock: (%d, %d, %d)\n", 1, data->clock.p[0], data->clock.p[1], data->clock.p[2]);
    
    sender(0, data);
    receive(1, 0, data);
    receive(1, 0, data);
    //printf(" \nFinal: %d , Clock: ( %d , %d , %d ) \n ", 0, st1->clock.p[0], st1->clock.p[1], st1->clock.p[2]);
    free(data);
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
}

// Representa o processo de rank 2
void process2()
{
    pthread_t t1, t2;
    pData *data;
    data = malloc(sizeof(pData));
    
    inicializar(&data->inputQueue);
    inicializar(&data->outputQueue);
    data->clock.pid = 2;
    data->clock.p[0] = 0;
    data->clock.p[1] = 0;
    data->clock.p[2] = 0;
    
    pthread_create(&t1, NULL, despachante, (void *)data);
    pthread_create(&t2, NULL, transmissao, (void *)data);
    
    printf("Processo: %d, Clock: (%d, %d, %d)\n", 2, data->clock.p[0], data->clock.p[1], data->clock.p[2]);

    
    event(data);
    sender(0, data);
    receive(2, 0, data);
    
    free(data);
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
}

int main(void)
{
    
    pthread_cond_init(&condFull, NULL);
    pthread_cond_init(&condEmpty, NULL);


    // sem_init(&semFullInput, 1, 0);
    // sem_init(&semEmptyOutput, 1, 10);
    
    // sem_init(&semFullInput, 1, 0);
    // sem_init(&semFullOutput, 1, 10);
    
    pthread_mutex_init(&mutex, NULL);
    pthread_mutex_init(&mutexClock, NULL);
    
    int my_rank;

    MPI_Init(NULL, NULL);

    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    if (my_rank == 0)
    {
        process0();
    }
    else if (my_rank == 1)
    {
        process1();
    }
    else if (my_rank == 2)
    {
        process2();
    }
    // Finaliza MPI
    MPI_Finalize();

    // sem_destroy(&semFullInput);
    // sem_destroy(&semEmptyInput);
    
    // sem_destroy(&semFullOutput);
    // sem_destroy(&semEmptyOutput);
    
    pthread_mutex_destroy(&mutex);
    pthread_mutex_destroy(&mutexClock);
    
    // pthread_cond_destroy(&condEmptyInput);
    // pthread_cond_destroy(&condFullInput);
   
    // pthread_cond_destroy(&condEmptyOutput);
    // pthread_cond_destroy(&condFullOutput);
    return 0;
} // main