#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <pthread.h>
#include <unistd.h>

#define QT_TRENS 10

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

int intersecoes[5] = {}, pos[QT_TRENS] = {};

void *estacaoDeTrens(void *arg){
    int tremID = *((int*)arg);
    int flag = 1;
    while(1){
        //pthread_mutex_lock(&mutex);
        while(intersecoes[pos[tremID]] >= 2){
            printf("Trem [%d] esperando intersecao %d liberar\n", tremID, pos[tremID]);
            pthread_cond_wait(&cond, &mutex);
        }
        //pthread_mutex_unlock(&mutex);
        if(flag == 1){
            flag = 0;
            pthread_mutex_lock(&mutex);
            if(intersecoes[pos[tremID]] < 2){
                //printf("Trem [%d] acessando a intersecao %d\n", tremID, pos[tremID]);
                intersecoes[pos[tremID]]++;
                sleep(1);
                intersecoes[pos[tremID]]--;
                pthread_mutex_unlock(&mutex);   
                pthread_cond_signal(&cond);
                if((pos[tremID]+1) < 5){
                    pos[tremID]++;
                }else{
                    pos[tremID] = 0;
                }
                //printf("\n");
            }
        }else{
            //pthread_mutex_lock(&mutex);
            if(intersecoes[pos[tremID]] < 2){
                //printf("Trem [%d] acessando a intersecao %d\n", tremID, pos[tremID]);
                intersecoes[pos[tremID]]++;
                sleep(1);
                intersecoes[pos[tremID]]--;
                //pthread_mutex_unlock(&mutex);  
                pthread_cond_signal(&cond);
                if((pos[tremID]+1) < 5){
                    pos[tremID]++;
                }else{
                    pos[tremID] = 0;
                }
                //printf("\n");
            }
        }
    }
}

int main(){

    pthread_t threads[QT_TRENS];
    int *ids[QT_TRENS];

    for(int i = 0; i < QT_TRENS; i++){
        ids[i] = (int*) malloc(sizeof(int));
        *ids[i] = i;
        if(pthread_create(&threads[i], NULL, &estacaoDeTrens, (void*)ids[i]) != 0){ //Criando uma Thread e passando a função 'decrement' como parâmetro e o ID da Thread como argumento
            perror("Failed to create thread");
        }
    }
    for(int i = 0; i < QT_TRENS; i++){ //Damos join em todas as Threads que já foram craidas
        if(pthread_join(threads[i], NULL) != 0){
            perror("Failed to join thread");
        }
    }

    pthread_exit(NULL);
    return 0;
}