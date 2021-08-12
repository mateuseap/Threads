#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define N 4
#define M 8

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; //Único mutex que vai ser utilizado para garantir a exclusão mútua na região crítica ('bancoDeDados' será a nossa região crítica)
int *bancoDeDados = NULL, flag = 1, size = 0;
//'bancoDeDados' é um array dinâmico que servirá como nosso banco de dados
//'flag' vai servir como uma variável de checagem para saber se o banco de dados está sendo acessado por uma thread escritora ou não
//'size' é o tamanho do array 'bancoDeDados'

void *escrever(void *arg){
    int threadID = *((int*)arg), i = 0; //Salvamos o ID da thread escritora em 'threadID' e inicializamos nossa variável auxiliar 'i' como zero
    while(1){ //Todas threads escritoras funcionaram dentro de um while para que assim elas possam escrever no array 'bancoDeDados' continuamente
        pthread_mutex_lock(&mutex); //Travamos o mutex para garantir que uma única thread escritora está escrevendo no banco de Dados
        printf("A Thread escritora de ID [%d] está escrevendo no Banco de Dados\n", threadID); //Exibimos qual thread escritora está acessando o banco de dados
        flag = 1; //Colocamos a flag = 1 para sinalizar que o banco de dados está sendo acessado por uma thread escritora
        if(i <= (-1e5)){
            i = 0;
        }else{
            i--;
        } //Geramos um valor qualquer para armazenar no banco de dados
        bancoDeDados = (int*) realloc(bancoDeDados, sizeof(int)*(size+1)); //Criamos uma nova posição no banco dados
        bancoDeDados[size] = i; //Armazenamos o valor que geramos na nova posição do banco de dados
        size++; //Aumentamos a variável que indica qual o tamanho do 'bancoDeDados'
        flag = 0; //Colocamos a flag = 0 para sinalizar que o banco de dados está livre, ou seja, não está sendo alterado por nenhuma thread escritora
        pthread_mutex_unlock(&mutex); //Liberamos o mutex para que outra thread escritora possa acrescentar algo no banco de dados
    }
}

void *ler(void *arg){
    int threadID = *((int*)arg), pos; //'threadID' é o ID da thread escritora e 'pos' será uma variável que indicará qual a posição do banco de dados que iremos acessar e exibir
    while(1){ //Assim como as threads escritoras, todas threads leitoras funcionaram dentro de um while para que assim elas possam ler o array 'bancoDeDados' continuamente
        if(flag == 0){ //Se a flag = 0, sabemos que nenhuma thread escritora está alterando o banco de dados, então podemos liberar o acesso para que uma thread leitora acesse o banco de dados
            pos = rand()%(size+1); //Geramos uma posição aleatoria que a thread leitora acessará no vetor 'bancoDeDados'
            printf("A Thread leitora de ID [%d] leu a posição [%d] do Banco de Dados: %d\n", threadID, pos, bancoDeDados[pos]); //Exibimos qual valor está contido naquela posição do nosso banco de dados 
        }
    }
}

int main(){

    pthread_t escritoras[M]; //Threads escritoras 
    pthread_t leitoras[N]; //Threads leitoras
    int *idsE[M], *idsL[N]; //idsE[M] -> ID's das threads escritoras, idsL[N] -> ID's das trheads leitoras

    for(int i = 0; i < M; i++){
        idsE[i] = (int*) malloc(sizeof(int));
        *idsE[i] = i;
        if(pthread_create(&escritoras[i], NULL, &escrever, (void*)idsE[i]) != 0){ //Criamos as threads escritoras passando a função 'escrever' como parârametro e o ID da thread escritora como argumento
            perror("Failed to create thread");
        }
    } 
    for(int i = 0; i < N; i++){
        idsL[i] = (int*) malloc(sizeof(int));
        *idsL[i] = i;
        if(pthread_create(&leitoras[i], NULL, &ler, (void*)idsL[i]) != 0){ //Criamos as threads leitoras passando a função 'ler' como parârametro e o ID da thread leitora como argumento
            perror("Failed to create thread");
        }
    }

    pthread_exit(NULL);
    return 0;
}
