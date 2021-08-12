#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <pthread.h>
#include <unistd.h>

#define QT_TRENS 10

// Um mutex para uso geral
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

// Como cada trem só pode alertar os trens que estão na espera da sua interseção, cada interseção tem seu mutex e sua variável de condição
pthread_mutex_t mutex0 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex2 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex3 = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t mutex4 = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond0 = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond1 = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond2 = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond3 = PTHREAD_COND_INITIALIZER;
pthread_cond_t cond4 = PTHREAD_COND_INITIALIZER;

// Crio o vetor das interseções e o que vai auxiliar a posição de cada trem
int intersecoes[5] = {}, pos[QT_TRENS] = {};

void *estacaoDeTrens(void *arg){
  int tremID = *((int*)arg);

  // While(1) para os trens/threads ficarem em movimento sempre
  while(1){
    
    queue:

    // De acordo com a interseção que o trem deseja acessar, ele entra no mutex da respectiva interseção
    if(pos[tremID] == 0){
      pthread_mutex_lock(&mutex0);
    }else if(pos[tremID] == 1){
      pthread_mutex_lock(&mutex1);
    }else if(pos[tremID] == 2){
      pthread_mutex_lock(&mutex2);
    }else if(pos[tremID] == 3){
      pthread_mutex_lock(&mutex3);
    }else if(pos[tremID] == 4){
      pthread_mutex_lock(&mutex4);
    }

    while(intersecoes[pos[tremID]] >= 2){ // Se a intersecao que o trem deseja entrar já estiver lotada, o trem espera, entrando no wait da respectiva interseção
      if(pos[tremID] == 0){
        pthread_cond_wait(&cond0, &mutex0);
      }else if(pos[tremID] == 1){
        pthread_cond_wait(&cond1, &mutex1);
      }else if(pos[tremID] == 2){
        pthread_cond_wait(&cond2, &mutex2);  
      }else if(pos[tremID] == 3){
        pthread_cond_wait(&cond3, &mutex3);
      }else if(pos[tremID] == 4){
        pthread_cond_wait(&cond4, &mutex4);
      }
    }

    // De acordo com a interseção que o trem deseja acessar, ele sai do mutex da respectiva interseção
    if(pos[tremID] == 0){
      pthread_mutex_unlock(&mutex0);
    }else if(pos[tremID] == 1){
      pthread_mutex_unlock(&mutex1);
    }else if(pos[tremID] == 2){
      pthread_mutex_unlock(&mutex2);
    }else if(pos[tremID] == 3){
      pthread_mutex_unlock(&mutex3);
    }else if(pos[tremID] == 4){
      pthread_mutex_unlock(&mutex4);
    }
    
    pthread_mutex_lock(&mutex);
    if(intersecoes[pos[tremID]] + 1 >= 3){ // Checo se a quantidade de trens na interseção está mesmo dentro do limite
      pthread_mutex_unlock(&mutex);
      goto queue; // Se estiver extrapolando o limite, o trem retorna para esperar (Nunca acontece, mas por precaução optamos por fazer esse tratamento)
    }
    intersecoes[pos[tremID]]++; // Se estiver tudo OK, adiciona mais um trem na quantidade de trens da interseção

    printf("Thread [%d] -> ", tremID);
    for(int i = 0; i < 5; i++){
      printf("%d ", intersecoes[i]);
    }
    printf("\n");

    pthread_mutex_unlock(&mutex);
    
    sleep(1); // Coloco o trem para fazer sua passagem, esperando 500 milisegundos

    pthread_mutex_lock(&mutex);
    intersecoes[pos[tremID]]--; // Assim que o trem termina sua passagem, diminuo um trem na quantidade de trens da interseção
    pthread_mutex_unlock(&mutex);

    // Como uma vaga na interseção foi liberada, mando um sinal para as threads que estão esperando para entrar nessa interseção
    if(pos[tremID] == 0){
      pthread_cond_signal(&cond0);
    }else if(pos[tremID] == 1){
      pthread_cond_signal(&cond1);
    }else if(pos[tremID] == 2){
      pthread_cond_signal(&cond2);  
    }else if(pos[tremID] == 3){
      pthread_cond_signal(&cond3);
    }else if(pos[tremID] == 4){
      pthread_cond_signal(&cond4);
    }

    // Mando o trem para a próxima interseção
    if((pos[tremID]+1) < 5){
        pos[tremID]++;
    }else{ // Caso o trem já esteja na última interseção, ele retorna para a primeira
        pos[tremID] = 0;
    }
  }
}

int main(){

  pthread_t threads[QT_TRENS];
  int *ids[QT_TRENS];

  for(int i = 0; i < QT_TRENS; i++){ // Com esse for, criamos as threads
    ids[i] = (int*) malloc(sizeof(int));
    *ids[i] = i;
    if(pthread_create(&threads[i], NULL, &estacaoDeTrens, (void*)ids[i]) != 0){ //Criando uma Thread e passando a função 'estacaoDeTrens' como parâmetro e o ID da Thread como argumento
      perror("Failed to create thread");
    }
  }
  for(int i = 0; i < QT_TRENS; i++){ // Damos join em todas as Threads que já foram craidas
    if(pthread_join(threads[i], NULL) != 0){
      perror("Failed to join thread");
    }
  }

  pthread_exit(NULL);
  return 0;
}
