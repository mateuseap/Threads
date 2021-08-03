#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <pthread.h>

#define T_THREADS 8
char senha[] = "4444444444";
bool senhaEncontrada = false;

void *gerarSenhas(void *arg){
    int threadID = *((int*)arg), j;
    long long int aux;
    char temp[] = "0000000000";
    for(long long int i = threadID; i < 1e10; i+=T_THREADS){
        if(senhaEncontrada){
            free(arg);
            pthread_exit(NULL);
        }
        j = 9;
        aux = i;
        while(aux > 0){
            temp[j] = 48+(aux%10);
            aux = aux/10;
            j--;
        }
        if(strcmp(senha, temp) == 0){
            printf("A Thread de ID [%d] encontrou a senha '%s' que bate com a senha '%s' desejada.\n", threadID, temp, senha);
            senhaEncontrada = true;
        }
    }
}

int main(){

    pthread_t threads[T_THREADS];
    int *ids[T_THREADS];

    for(int i = 0; i < T_THREADS; i++){
        ids[i] = (int*) malloc(sizeof(int));
        *ids[i] = i;
        if((pthread_create(&threads[i], NULL, &gerarSenhas, (void*)ids[i])) != 0){
            perror("Failed to create thread");
        }
    }   
    for(int i = 0; i < T_THREADS; i++){
        if(pthread_join(threads[i], NULL) != 0){
            perror("Failed to join thread");
        }
    }

    pthread_exit(NULL);
    return 0; 
}
