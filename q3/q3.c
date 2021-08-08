#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include <pthread.h>

#define QT_THREADS 8
#define QT_ARQUIVOS 5

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

typedef struct{
    int idDaSala, qtAlunos;
    double media, moda, mediana, desvioPadrao;
    double *notaAlunos;
} Salas;

typedef struct{
    char nome[50];
    double nota;
} Aluno;

Salas dados[QT_ARQUIVOS];
char nomeDoArquivo[] = "sala_.txt";
bool arquivosRestantes[QT_ARQUIVOS];
int offset[QT_THREADS];

void *analisarDesempenho(void *arg){
    int threadID = *((int*)arg);
    Aluno temp;
    double sT, xiMa;
    int aux, max;
    if(threadID >= QT_ARQUIVOS){
        pthread_exit(NULL);
    }
    loop:
    
    aux = max = 0;

    pthread_mutex_lock(&mutex);
    threadID = offset[*((int*)arg)];
    printf("%d", offset[*((int*)arg)]);
    arquivosRestantes[threadID] = false;
    char IDdaSala = (threadID+1)+'0';
    dados[threadID].idDaSala = threadID+1;
    dados[threadID].media = dados[threadID].moda = dados[threadID].mediana = dados[threadID].desvioPadrao = dados[threadID].qtAlunos = 0;
    nomeDoArquivo[4] = IDdaSala;
    printf("Thread[%d] | Sala %s\n", *((int*)arg), nomeDoArquivo);
    dados[threadID].notaAlunos = NULL;

    FILE *arquivo;
    if((arquivo = fopen(nomeDoArquivo,"rt")) == NULL){
        printf("Nao foi possivel abrir o arquivo!\n");
        exit(-1);
    }
    pthread_mutex_unlock(&mutex);

    while((fscanf(arquivo, "%s %lf", temp.nome, &temp.nota)) != EOF){
        dados[threadID].media += temp.nota;
        dados[threadID].qtAlunos++;
        dados[threadID].notaAlunos = (double*) realloc (dados[threadID].notaAlunos, dados[threadID].qtAlunos*sizeof(double));
        dados[threadID].notaAlunos[((int)dados[threadID].qtAlunos)-1] = temp.nota;
    }
    fclose(arquivo);

    dados[threadID].media /= (double)dados[threadID].qtAlunos;

    for(int i = 0; i < dados[threadID].qtAlunos; i++){
        for(int j = 0; j < dados[threadID].qtAlunos; j++){
            if(dados[threadID].notaAlunos[i] == dados[threadID].notaAlunos[j]){
                aux++;
            }
        }
        if(aux > max){
            max = aux;
            dados[threadID].moda = dados[threadID].notaAlunos[i];
        }
        xiMa = dados[threadID].notaAlunos[i]-dados[threadID].media;
        dados[threadID].desvioPadrao += xiMa*xiMa;
        aux = 0;
    }

    dados[threadID].desvioPadrao /= (double)dados[threadID].qtAlunos;
    dados[threadID].desvioPadrao = sqrt(dados[threadID].desvioPadrao); 

    for(int i = 0; i < dados[threadID].qtAlunos-1; i++){
        for(int j = 0; j < dados[threadID].qtAlunos-1; j++){
            if(dados[threadID].notaAlunos[j+1] < dados[threadID].notaAlunos[j]){
                sT = dados[threadID].notaAlunos[j];
                dados[threadID].notaAlunos[j] = dados[threadID].notaAlunos[j+1];
                dados[threadID].notaAlunos[j+1] = sT; 
            }
        }
    }

    if(((int)dados[threadID].qtAlunos)%2 == 0){
        dados[threadID].mediana = (dados[threadID].notaAlunos[((int)dados[threadID].qtAlunos)/2] + dados[threadID].notaAlunos[((int)dados[threadID].qtAlunos)/2 - 1])/2;
    }else{
        dados[threadID].mediana = dados[threadID].notaAlunos[((int)dados[threadID].qtAlunos)/2];
    }

    int check = 1;
    pthread_mutex_lock(&mutex);
    for(int i = QT_ARQUIVOS; i >= 0; i--){
        if(arquivosRestantes[i] == true){
            arquivosRestantes[i] = false;
            offset[*((int*)arg)] = i;
            check = 0;
            break;
        }
    }
    pthread_mutex_unlock(&mutex);           
    
    if(check == 1){
        pthread_exit(NULL);
    }
    goto loop;
}

int main(){

    pthread_t threads[QT_THREADS];
    int *ids[QT_THREADS];

    memset(arquivosRestantes, true, QT_ARQUIVOS);
    for(int i = 0; i < QT_THREADS; i++){
        offset[i] = i;
    }

    for(int i = 0; i < QT_THREADS; i++){
        ids[i] = (int*) malloc(sizeof(int));
        *ids[i] = i;
        if(pthread_create(&threads[i], NULL, &analisarDesempenho, (void*)ids[i]) != 0){ //Criando uma Thread e passando a função 'decrement' como parâmetro e o ID da Thread como argumento
            perror("Failed to create thread");
        }
    }
    for(int i = 0; i < QT_THREADS; i++){ //Damos join em todas as Threads que já foram craidas
        if(pthread_join(threads[i], NULL) != 0){
            perror("Failed to join thread");
        }
    }

    for(int i = 0; i < QT_ARQUIVOS; i++){
        printf("Sala %d\nMedia: %lf\nModa: %lf\nMediana: %lf\nDesvio Padrao: %lf", i+1, dados[i].media, dados[i].moda, dados[i].mediana, dados[i].desvioPadrao);
        if(i+1 < QT_ARQUIVOS){
            printf("\n\n");
        }else{
            printf("\n");
        }
    }

    pthread_exit(NULL);
    return 0;
}