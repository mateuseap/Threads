#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

int contador = 5000000;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void *decrement(void *arg){
    int threadID = *((int*)arg); //Salvamos o ID da Thread, esse ID foi passado como argumento na hora da criação da respectiva Thread
    while(contador > 0){
        pthread_mutex_lock(&mutex); //O mutex é travado pela Thread que está sendo executada, isso garante que nenhuma outra Thread além dela possa decrementar o contador naquele dado instante 
        contador--;
        if(contador == 0){ //Checamos se o contador é igual 0 para printarmos na tela qual foi a Thread que fez o contador alcançar o valor 0
            printf("A thread de ID [%d] fez o contador atingir o valor %d!\n", threadID, contador);
        }
        pthread_mutex_unlock(&mutex); //O mutex é destravado, isso significa que alguma outra Thread pode agora decrementar o contador, isso caso ele seja ainda maior do que 0
    }
    free(arg); //Quando chegamos aqui, significa que o contador chegou a 0, então podemos finalizar a execução da Thread, mas antes de fazer isso damos free no seu ID (que é basicamente um ponteiro para inteiro) pois não vamos precisar mais dele, garantimos dessa maneira que não haja um vazamento de memória
    pthread_exit(NULL); //Finalizamos a execução da Thread
}

int main() {
    int qtdT;

    printf("Escolha quantas threads você deseja criar: ");
    scanf("%d", &qtdT);

    pthread_t threads[qtdT];
    int *ids[qtdT]; //Vetor com o ID das Threads
  
    for(int i = 0; i < qtdT; i++){
        ids[i] = (int*) malloc(sizeof(int));
        *ids[i] = i;
        if(pthread_create(&threads[i], NULL, &decrement, (void*)ids[i]) != 0){ //Criando uma Thread e passando a função 'decrement' como parâmetro e o ID da Thread como argumento
            perror("Failed to create thread");
        }
    }
    for(int i = 0; i < qtdT; i++){ //Damos join em todas as Threads que já foram craidas
        if(pthread_join(threads[i], NULL) != 0){
            perror("Failed to join thread");
        }
    }

    pthread_exit(NULL);
    return 0;
}
