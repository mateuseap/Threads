#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <pthread.h>

#define T_THREADS 8
char senha[] = "1234320985"; // Senha que devemos procurar
bool senhaEncontrada = false; // Flag para sabermos quando a senha foi encontrada

void *gerarSenhas(void *arg){ 
    int threadID = *((int*)arg), j; 
    long long int aux; // Número auxiliar que vamos usar para fazer a conversão em string
    char temp[] = "0000000000"; // Variável auxiliar que vai ser a senha temporária que vamos checar para ver se é a senha correta
    for(long long int i = threadID; i < 1e10; i+=T_THREADS){
        // Nesse for, cada thread irá pegar os números que começam com o ID da Thread e seguem somando pela quantidade de threads no programa
        // Exemplo: (Supondo que temos 3 Threads)
        // Thread de ID = 0 irá pegar os números 0 3 6 9 12 15 18 21...
        // Thread de ID = 1 irá pegar os números 1 4 7 10 13 16 19 22...
        // Thread de ID = 2 irá pegar os números 2 5 8 11 14 17 20 23...
        // Desse jeito, vamos pegar todos os inteiros até o limite estabelecido pelo "for" 

        if(senhaEncontrada){ // Se a senha for encontrada, damos free no ponteiro e encerramos a Thread
            free(arg);
            pthread_exit(NULL);
        }
        j = 9; // J = 9 pois a string tem 10 posições, indo de 0 a 9
        aux = i; // Colocamos o valor de 'i', que vai ser a senha a ser testada, na variável auxiliar 'aux'

        while(aux > 0){ // Nesse while, extraimos cada dígito do valor de 'aux' e colocamos na nossa string temporária
            temp[j] = 48+(aux%10);
            aux = aux/10;
            j--;
        }
        if(strcmp(senha, temp) == 0){ // Se a nossa string temporária for igual a senha desejada, mostramos o resultado
            printf("A Thread de ID [%d] encontrou a senha '%s' que bate com a senha '%s' desejada.\n", threadID, temp, senha);
            senhaEncontrada = true; // E setamos senhaEncontrada como true, para que na próxima iteração do for, a thread seja encerrada
        }
    }
    pthread_exit(NULL);
}

int main(){

    pthread_t threads[T_THREADS];
    int *ids[T_THREADS];

    for(int i = 0; i < T_THREADS; i++){ // Com esse for, criamos as threads
        ids[i] = (int*) malloc(sizeof(int)); 
        *ids[i] = i;
        if((pthread_create(&threads[i], NULL, &gerarSenhas, (void*)ids[i])) != 0){ // Criando uma Thread e passando a função 'gerarSenhas' como parâmetro e o ID da Thread como argumento
            perror("Failed to create thread");
        }
    }
    for(int i = 0; i < T_THREADS; i++){ // Damos join em todas as Threads que já foram craidas
        if(pthread_join(threads[i], NULL) != 0){
            perror("Failed to join thread");
        }
    }

    pthread_exit(NULL);
    return 0; 
}
