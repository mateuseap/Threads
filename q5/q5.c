#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>

#define thread_users 5
#define thread_desp 3
#define tamBuffer 5

typedef struct{
  int ID_thread, ID_operacao, val1, val2;
}Buffer;

typedef struct{
  int resultado, ID_operacao;
}Resultado;

pthread_mutex_t cont_mutex =  PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t buffer_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t resultado_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t full = PTHREAD_COND_INITIALIZER;
pthread_cond_t empty = PTHREAD_COND_INITIALIZER;
pthread_cond_t resFull = PTHREAD_COND_INITIALIZER;
pthread_cond_t resEmpty= PTHREAD_COND_INITIALIZER;
pthread_cond_t resInteract= PTHREAD_COND_INITIALIZER;

int count = 0;
int ID_operacao = 0;
int posResultado = 0;
bool isOver=false;
Buffer buffer[tamBuffer];
Resultado resultados[tamBuffer];
Buffer parametros[thread_users];

int is_full(){
  for(int i=0; i < tamBuffer; i++){
    if(buffer[i].ID_thread == -1){
      return i;
    }
  }
  return -1;
}

int is_empty(){
  for(int i = 0; i < tamBuffer; i++){
    if(buffer[i].ID_thread != -1){
      return i;
    }
  }
  return -1;
}

int resultadoIsFull(){
  for(int i = 0; i < tamBuffer; i++){
    if(resultados[i].ID_operacao == -1){
      return i;
    }
  }
  return -1;
}

int resultadoIsEmpty(){
  for(int i = 0; i < tamBuffer; i++){
    if(resultados[i].ID_operacao != -1){
      return i;
    }
  }
  return -1;
}

int soma(int val1, int val2){
  //printf("Somou %d e %d igual a %d\n", val1, val2, val1 + val2);
  return val1 + val2;
}

int agendarExecucao(Buffer valores){
  int pos;
  pthread_mutex_lock(&buffer_mutex);
  pos = is_full();
  if(pos == -1){ // Buffer está cheio, então tem que esperar uma posição vazia
    pthread_cond_wait(&full, &buffer_mutex);
  }
  buffer[pos] = valores; // Buffer tem espaço
  buffer[pos].ID_operacao = ID_operacao;
  int ID_retornado=ID_operacao;
  ID_operacao++;
  printf("A thread [%d] chegou! Buffer tem ID [%d] e os valores %d %d\n\n", valores.ID_thread, buffer[pos].ID_operacao, buffer[pos].val1, buffer[pos].val2);
  pthread_mutex_unlock(&buffer_mutex);
  pthread_cond_broadcast(&empty);
  return ID_retornado;
}

void pegarResultadoExecucao(int id){
    pthread_mutex_lock(&resultado_mutex);
    int resultado_id=-777;
    while(id!=resultado_id){
        if(resultadoIsEmpty){
            pthread_cond_wait(&resEmpty, &resultado_mutex);
        }
        for(int i=0; i<tamBuffer; i++){
            if(resultados[i].ID_operacao==id){
                resultado_id=id;
                printf("resultado da operacao de ID[%d] foi: %d\n\n", resultados[i].ID_operacao, resultados[i].resultado);
                resultados[i].ID_operacao=-1;
                break;
            }
        }
        if(resultado_id!=id){
            pthread_cond_wait(&resInteract, &resultado_mutex);
        }
    }
    pthread_mutex_unlock(&resultado_mutex);
    pthread_cond_broadcast(&resFull);

    return;
}

void *func_despachante(void *arg){
    int pos;
    int threadID = *((int*)arg);

    despachar:

    pthread_mutex_lock(&cont_mutex);
    if(count+1 == thread_users){
      printf("thread_despachante [%d] vai fazer a ultima execucao\n", threadID);
      count++;
      isOver = true;
    }else if(count+1 > thread_users){
      printf("thread_despachante [%d] terminou\n", threadID);
      count++;
      pthread_exit(NULL);
    }else{
      printf("thread_despachante [%d] vai executar\n", threadID);
      if(isOver){
        printf("thread_despachante [%d] terminou\n", threadID);       
        pthread_exit(NULL);
      }
      count++;
    }
    pthread_mutex_unlock(&cont_mutex);      

    pthread_mutex_lock(&buffer_mutex);
    
    if(is_empty() == -1){ // Buffer está vazio, thread despachante dorme
        pthread_cond_wait(&empty, &buffer_mutex); /* despachante_mutex não está sendo usado em canto nenhum */
    }
    pos=is_empty();
    Buffer temp;
    temp=buffer[pos];
    buffer[pos].ID_thread=-1;

    pthread_mutex_unlock(&buffer_mutex);
    
    pthread_cond_broadcast(&full);

    pthread_mutex_lock(&resultado_mutex); // Mutex para bloquear o uso do array resultados 
    
    if(resultadoIsFull() == -1){ // Array de resultados cheio, espera liberar espaço
        pthread_cond_wait(&resFull, &resultado_mutex);
    }
    posResultado=resultadoIsFull();
    resultados[posResultado].resultado = soma(buffer[pos].val1, buffer[pos].val2);
    resultados[posResultado].ID_operacao = buffer[pos].ID_operacao;  
    pthread_mutex_unlock(&resultado_mutex);
        
    pthread_cond_broadcast(&resEmpty);
    pthread_cond_broadcast(&resInteract);
    
    goto despachar;
}

void *func_user(void *arg){
    int threadID = *((int*)arg);

    parametros[threadID].ID_thread = threadID;
    parametros[threadID].val1 = rand()%100;
    parametros[threadID].val2 = rand()%100;

    int id = agendarExecucao(parametros[threadID]);
    pegarResultadoExecucao(id); 
    printf("thread_user[%d] terminou\n\n", threadID);       
    pthread_exit(NULL);
}

int main(){

    pthread_t usuario[thread_users];
    pthread_t despachantes[thread_desp];
    int *ids_users[thread_users], *ids_desp[thread_desp];

    for(int i = 0; i < tamBuffer; i++){
      resultados[i].ID_operacao = -1;
      buffer[i].ID_thread = -1;
      buffer[i].ID_operacao = buffer[i].val1 = buffer[i].val2 = 0;
    }

    for(int i = 0; i < thread_users; i++){
      ids_users[i] = (int*) malloc(sizeof(int));
      *ids_users[i] = i;
      if(pthread_create(&usuario[i], NULL, &func_user, (void*)ids_users[i]) != 0){
          perror("Failed to create thread");
      }
    }

    for(int i = 0; i < thread_desp; i++){
      ids_desp[i] = (int*) malloc(sizeof(int));
      *ids_desp[i] = i;
      if(pthread_create(&despachantes[i], NULL, &func_despachante, (void*)ids_users[i]) != 0){
          perror("Failed to create thread");
      }
    }

    pthread_exit(NULL);
    return 0;
}