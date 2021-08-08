#define _XOPEN_SOURCE 600 
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <pthread.h>

pthread_barrier_t barrier;
int n, qtdT, offset = 0, *check, i = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void *encontrarPrimo(void *arg){
    int threadID = *((int*)arg), count = 0, primo = 1, liberado = 0;
    while(n > i){
        check[threadID] = -1;
        for(int k = 1; k < sqrt(threadID+offset); k++){
            if((threadID+offset)%k == 0){
                count++;                                                             
            }
            if(count > 2){
                primo = 0;
                break;
            }
        }
        if(primo){
            check[threadID] = threadID+offset;
        }else{
            check[threadID] = -777;
        }
        if(check[threadID] == threadID+offset){
            pthread_mutex_lock(&mutex);
            i++;
            if(i == n){
                printf("%d\n", check[i]);
                pthread_exit(NULL);
            }
            pthread_mutex_unlock(&mutex);
        }
        pthread_barrier_wait(&barrier);
        pthread_mutex_lock(&mutex);
        offset += 1;
        pthread_mutex_unlock(&mutex);
        pthread_barrier_wait(&barrier);
    }
}

int main(){

    printf("Numero T: ");
    scanf("%d", &qtdT);
    printf("Numero N: ");
    scanf("%d", &n);

    check = (int*) malloc(sizeof(int)*qtdT);
    pthread_t threads[qtdT];
    pthread_barrier_init(&barrier, NULL, qtdT);
    int *ids[qtdT];

    for(int i = 0; i < qtdT; i++){
        ids[i] = (int*) malloc(sizeof(int));
        *ids[i] = i;
        if((pthread_create(&threads[i], NULL, &encontrarPrimo, (void*)ids[i])) != 0){
            perror("Failed to create thread");
        }
    }

    pthread_exit(NULL);
    return 0; 
}