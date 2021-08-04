#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define N 4
#define M 8

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
int *bancoDeDados = NULL, flag = 1, size = 0, max = 1e5;

void *escrever(void *arg){
    int threadID = *((int*)arg), i = 0;
    while(1){
        pthread_mutex_lock(&mutex);
        printf("A Thread escritora de ID [%d] está escrevendo no Banco de Dados\n", threadID);
        flag = 1;
        if(i <= -max){
            i = 0;
        }else{
            i--;
        }
        bancoDeDados = (int*) realloc(bancoDeDados, sizeof(int)*(size+1));
        bancoDeDados[size] = i;
        size++;
        flag = 0;
        pthread_mutex_unlock(&mutex);
    }
}

void *ler(void *arg){
    int threadID = *((int*)arg), pos;
    while(1){
        if(flag == 0){
            pos = rand()%(size+1);
            printf("A Thread leitora de ID [%d] leu a posição [%d] do Banco de Dados: %d\n", threadID, pos, bancoDeDados[pos]);
        }
    }
}

int main(){

    pthread_t escritoras[M];
    pthread_t leitoras[N];
    int *idsE[M], *idsL[N];

    for(int i = 0; i < M; i++){
        idsE[i] = (int*) malloc(sizeof(int));
        *idsE[i] = i;
        if(pthread_create(&escritoras[i], NULL, &escrever, (void*)idsE[i]) != 0){
            perror("Failed to create thread");
        }
    }
    for(int i = 0; i < N; i++){
        idsL[i] = (int*) malloc(sizeof(int));
        *idsL[i] = i;
        if(pthread_create(&leitoras[i], NULL, &ler, (void*)idsL[i]) != 0){
            perror("Failed to create thread");
        }
    }

    pthread_exit(NULL);
    return 0;
}