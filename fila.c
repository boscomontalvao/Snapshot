#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <stdbool.h>

#define MAX 10

pthread_mutex_t mutex;


pthread_cond_t condFull;
pthread_cond_t condEmpty;


typedef struct 
{
    int pid;
    int destination;
    int p[3];
} CLOCK;

typedef struct
{
   CLOCK clocks[MAX];
   int inicio;
   int fim;
   int tamanho;
} QUEUE;



// Inicializa a fila deixando-a pronta para ser utilizada.
void inicializar(QUEUE *q)
{
    q->inicio = 0;
    q->fim = -1;
    q->tamanho = 0;
}


// Retornar o tamanho da fila
int tamanho(QUEUE *q)
{
    return q->tamanho;
}


// Estara cheia quando tamanho = MAX
bool cheia(QUEUE *q)
{
    return tamanho(q) == MAX;
}


// Retorna true se a filha esta vazia (Tamanho = 0)
bool vazia(QUEUE *q)
{
    return tamanho(q) == 0;
}


/* 
  Objetivo: Insere um item no final da fila.
*/
bool inserir(CLOCK clock, QUEUE *q)
{
    
    pthread_mutex_lock(&mutex);
    while(cheia(q))
    {
        pthread_cond_wait(&condFull, &mutex);
    }
    
    q->fim = (q->fim + 1) % MAX;
    q->clocks[q->fim] = clock;
    q->tamanho++;
    
    pthread_mutex_unlock(&mutex);
    pthread_cond_signal(&condEmpty);
    return true;
}



/*
  Objetivo: Acessa a frente da Fila e atribui ao par�metro item, sem afetar
            o estado da Fila. Retorna true quando algum item foi obtido.
*/
bool frente(CLOCK *clock, QUEUE *q)
{
    
    if(vazia(q))
        return false;
    else
        *clock = q->clocks[q->inicio];
    
    return true;
  
}


/*
  Objetivo: Retira um Item do inicio da Fila (frente) e atribui
            ao parametro item.
            Retorna true quando algum item foi retornado.
*/
CLOCK retirar(QUEUE *q)
{
    CLOCK c;

    while(vazia(q))
    {
        pthread_cond_wait(&condEmpty, &mutex);
    }
    
    c = q->clocks[q->inicio];
    q->inicio = (q->inicio + 1) % MAX;
    q->tamanho--;
    
    pthread_cond_signal(&condFull);
    
    return c;
}


// Exibicao da fila
void exibir(QUEUE *q)
{
    int pos = q->inicio;

    for (int i = 0; i < tamanho(q); i++)
    {
        printf("(%d, %d, %d) ", q->clocks[pos].p[0], q->clocks[pos].p[1], q->clocks[pos].p[2]);
        pos = (pos + 1) % MAX;
    }
}


// Liberacao das variaveis dinamicas dos nos da lista, iniciando da cabeca
void destruir(QUEUE *q)
{
    q->inicio = 0; // ajusta o inicio da lista (vazia)
    q->fim = 0;    // ajusta o fim da lista (vazia)
    q->tamanho = 0;
}


//////////////////////////////////////////////////////////////

void lerItens(QUEUE*q)
{
    int n;
    scanf("%d", &n);

    // insere os valores n pares chave,valor
    CLOCK clock;
    for (int i = 0; i < n; i++)
    {
        for(int j = 0; j < 3; j++)
            scanf("%d", &clock.p[j]);
        if (!inserir(clock, q))
            printf("Nao inseriu na fila o clock: (%d, %d, %d)\n", clock.p[0], clock.p[1], clock.p[2]);
    }
}

void imprimir(QUEUE *q)
{
    printf("Tamanho = %d\n", tamanho(q));
    exibir(q);
    printf("\n");
}
