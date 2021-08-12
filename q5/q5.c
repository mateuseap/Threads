#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>

#define thread_users 5 //declaração número de threads usuário
#define thread_desp 3  //declaração número de threads despachantes
#define tamBuffer 5    //tamanho tanto do vetor buffer quanto do vetor de resultados

typedef struct{     //struct do vetor buffer
  int ID_thread, ID_operacao, val1, val2;
}Buffer;

typedef struct{    //struct do vetor de resultado
  int resultado, ID_operacao;
}Resultado;

//declaração de mutex e variável de condição
pthread_mutex_t buffer_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t resultado_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t full = PTHREAD_COND_INITIALIZER;
pthread_cond_t empty = PTHREAD_COND_INITIALIZER;
pthread_cond_t resFull = PTHREAD_COND_INITIALIZER;
pthread_cond_t resEmpty= PTHREAD_COND_INITIALIZER;
pthread_cond_t resInteract = PTHREAD_COND_INITIALIZER;
pthread_cond_t finalizou = PTHREAD_COND_INITIALIZER;

int count=0;    //contador para saber quantas vezes a despachante rodou
int ID_operacao = 0; //o ID da operação que será passado pra resgatar o valor a ser executado 
bool isOver=false; //flag para verificar se todas as t_despachantes foram executadas
Buffer buffer[tamBuffer]; 
Resultado resultados[tamBuffer];
Buffer parametros[thread_users]; //vetor auxiliar onde vai ser guardado os parametros a serem passados pela funexec

//função que verifica se buffer esta cheio, se estiver retorna -1, se não, retorna a posição livre
int bufferIsFull(){ 
  for(int i=0; i < tamBuffer; i++){
    if(buffer[i].ID_thread == -1){
      return i;
    }
  }
  return -1;
}

//função que verifica se buffer esta vazio, se estiver retorna -1, se não, retorna a posição ocupada
int bufferIsEmpty(){
  for(int i = 0; i < tamBuffer; i++){
    if(buffer[i].ID_thread != -1){
      return i;
    }
  }
  return -1;
}

//função que verifica se o resultados esta cheio, se estiver retorna -1, se não, retorna a posição livre
int resultadoIsFull(){
  for(int i = 0; i < tamBuffer; i++){
    if(resultados[i].ID_operacao == -1){
      return i;
    }
  }
  return -1;
}

//função que verifica se resultados esta vazio, se estiver retorna -1, se não, retorna a posição ocupada
int resultadoIsEmpty(){
  for(int i = 0; i < tamBuffer; i++){
    if(resultados[i].ID_operacao != -1){
      return i;
    }
  }
  return -1;
}

//função funexec a ser executada
int soma(int val1, int val2){
  return val1 + val2;
}


int agendarExecucao(Buffer valores){
  int pos;
  pthread_mutex_lock(&buffer_mutex); //torno o acesso ao buffer restrito
  if(bufferIsFull() == -1){ // Buffer está cheio, então tem que esperar uma posição vazia
    pthread_cond_wait(&full, &buffer_mutex);
  }
  pos = bufferIsFull(); //se o buffer nao estiver cheio, retorna a posição em que ele esta vazio
  buffer[pos] = valores; //coloca os valores a serem executados da soma no buffer
  buffer[pos].ID_operacao = ID_operacao; //passa o ID da operacao daqueles valores para aquela posição do buffer
  int ID_retornado=ID_operacao; //coloca o ID em uma variável auxiliar para retornar, e mais tarde ser passado para a função pegarResultadoExecucao
  ID_operacao++; //incrementa o ID da operacao
  printf("A thread de ID [%d] chegou! Agora o buffer tem a requisicao de ID <%d> e os valores sao %d %d\n", valores.ID_thread, buffer[pos].ID_operacao, buffer[pos].val1, buffer[pos].val2);
  pthread_mutex_unlock(&buffer_mutex); 
  pthread_cond_broadcast(&empty); //dou um sinal para as threads despachantes avisando que agora podem retirar um valor do buffer
  return ID_retornado; //retorno o ID da operação colocada no buffer
}

void pegarResultadoExecucao(int id){
    pthread_mutex_lock(&resultado_mutex); //torno o vetor de resultados restrito
    int resultado_id=-777;  //resultado_id sera usado para ver se achamos nosso resultado correto entre o vetor de resultados, entao seto ele para um valor absurdo
    while(id!=resultado_id){
        if(resultadoIsEmpty && !isOver){ //se o vetor de resultados estiver vazio, e as despachantes ainda não tiver terminado de trabalhar, espera ele ser preenchido
            pthread_cond_wait(&resEmpty, &resultado_mutex);
        }
        for(int i=0; i<tamBuffer; i++){  //checo todas as posições do vetor de resultados
            if(resultados[i].ID_operacao==id){  
                resultado_id=id;    //se achar o ID, digo que resultado_id=id para sair do while 
                printf("O resultado da operação de ID <%d> foi: %d\n", resultados[i].ID_operacao, resultados[i].resultado);
                resultados[i].ID_operacao=-1; //printo o valor achado e seto o id_operacao para -1, para mostrar que aquela posição agora está vazia
                if(isOver && (bufferIsEmpty()==-1) && (resultadoIsEmpty()==-1)){
                    exit(0); //se acabou todo o processamento de resultados, eu mato todas as threads terminando o processo
                } 
                break; 
            }
        }
        if(resultado_id!=id){ //se não achar o ID entre o vetor resultados, espera que o vetor resultados seja preenchido novamente para evitar espera ocupada
            pthread_cond_wait(&resInteract, &resultado_mutex); 
        }
    }
    pthread_mutex_unlock(&resultado_mutex);
    pthread_cond_broadcast(&resFull); //aviso para a despachante que pode inserir no vetor resultados
    return;
}

void *func_despachante(void *arg){
    int posBuffer;
    int posResultado;
    int threadID = *((int*)arg);
    while(!isOver){ //enquanto todas as despachantes não terminaram de executar
        pthread_mutex_lock(&buffer_mutex);

        if(bufferIsEmpty() == -1){ // Buffer está vazio, thread despachante dorme
            if(isOver){
                pthread_cond_broadcast(&resEmpty);
            }
            pthread_cond_wait(&empty, &buffer_mutex); /* despachante_mutex não está sendo usado em canto nenhum */ 
        }

        posBuffer=bufferIsEmpty(); //guarda a posição ocupada do buffer
        Buffer temp;        
        temp=buffer[posBuffer];    
        buffer[posBuffer].ID_thread=-1; //depois de passar os valores para temp, diz que aquela posição está vazia

        pthread_mutex_unlock(&buffer_mutex);
        
        pthread_cond_broadcast(&full);  //digo para agendar funçao que agora pode preencher o buffer

        pthread_mutex_lock(&resultado_mutex); // Mutex para bloquear o uso do array resultados 
        
        if(resultadoIsFull() == -1){ // Array de resultados cheio, espera liberar espaço
            pthread_cond_wait(&resFull, &resultado_mutex);
        }
        posResultado=resultadoIsFull(); //pega o espaço vazio de resultados
        resultados[posResultado].resultado = soma(temp.val1, temp.val2); //executa a funexec e guarda o valor
        resultados[posResultado].ID_operacao = temp.ID_operacao;  
        count++; //adiciona no contador de execuções da despachante
        if(count>=thread_users) isOver=true;  //se as despachantes despacharam todos os usários, então seta isOver para true
        pthread_mutex_unlock(&resultado_mutex);
        
        pthread_cond_broadcast(&resEmpty); //avisa para pegarResultadoExecucao que resultados nao esta mais vazia
        pthread_cond_broadcast(&resInteract);//e avisa tambem que um valor novo foi colocado em resultados, então se pode fazer uma nova checagem do ID

    }
    pthread_exit(NULL);
}

void *func_user(void *arg){
    int threadID = *((int*)arg);

    //gerar os valores a serem passados para o buffer
    parametros[threadID].ID_thread = threadID;
    parametros[threadID].val1 = rand()%100;
    parametros[threadID].val2 = rand()%100;

    int id = agendarExecucao(parametros[threadID]);
    pegarResultadoExecucao(id); 
    pthread_exit(NULL);
}


int main(){

    //declaração das threads
    pthread_t usuario[thread_users];
    pthread_t despachantes[thread_desp];
    pthread_t sair;
    int *ids_users[thread_users], *ids_desp[thread_desp];

    //setar todas as posições de Buffer e Resultados como desocupadas
    for(int i = 0; i < tamBuffer; i++){
      resultados[i].ID_operacao = -1;
      buffer[i].ID_thread = -1;
      buffer[i].ID_operacao = buffer[i].val1 = buffer[i].val2 = 0;
    }

    //crio as threads usuario
    for(int i = 0; i < thread_users; i++){
      ids_users[i] = (int*) malloc(sizeof(int));
      *ids_users[i] = i;
      if(pthread_create(&usuario[i], NULL, &func_user, (void*)ids_users[i]) != 0){
          perror("Failed to create thread");
      }
    }

    //crio as threads despachantes
    for(int i = 0; i < thread_desp; i++){
      ids_desp[i] = (int*) malloc(sizeof(int));
      *ids_desp[i] = i;
      if(pthread_create(&despachantes[i], NULL, &func_despachante, (void*)ids_desp[i]) != 0){
          perror("Failed to create thread");
      }
    }

    for(int i = 0; i < thread_users; i++){ //Damos join em todas as Threads que já foram craidas
        if(pthread_join(usuario[i], NULL) != 0){
            perror("Failed to join thread");
        }
    }

    for(int i = 0; i < thread_desp; i++){ //Damos join em todas as Threads que já foram craidas
        if(pthread_join(despachantes[i], NULL) != 0){
            perror("Failed to join thread");
        }
    }
    pthread_exit(NULL);
    return 0;
}
