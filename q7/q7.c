#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

typedef struct{
    int l, r; //Limite a esquerda e limite a direita do vetor, respectivamente
} Partition; 

int *vetor = NULL; //Esse é o nosso vetor que vai ser lido e depois ordenado

void *merge(void *arg){ 
    Partition dados = *((Partition*) arg);
    int m = (dados.l+dados.r)/2;
    int tam = (dados.r-dados.l)+1, i1 = 0, i2 = (m+1)-dados.l, curr = dados.l; //'tam' é o tamanho do vetor, i1 e i2 são indícies que irão servir para "varrer" o vetor 'temp', 'curr' será nossa variável para percorrer o 'vetor'
    int *temp = (int*) malloc(sizeof(int)*tam); //'temp' é o vetor temporário que utilizaremos
    for(int i = 0, j = dados.l; i <= (dados.r-dados.l); i++, j++){
        temp[i] = vetor[j];
    } //Copiamos o nosso 'vetor' para o vetor temporário 'temp'
    while(curr <= dados.r){ //Fazemos a merge
        if(i1 == (m+1)-dados.l){
            vetor[curr] = temp[i2++];
        }else if(i2 > (dados.r-dados.l)){
            vetor[curr] = temp[i1++];
        }else if(temp[i1] <= temp[i2]){
            vetor[curr] = temp[i1++];
        }else{
            vetor[curr] = temp[i2++];
        }
        curr++;
    }
    free(temp); //Liberamos a memória, já que não utilizaremos mais o vetor 'temp'
    pthread_exit(NULL);
}

void *mergeSort(void *arg){
    pthread_t threads[3]; //Criamos três threads, duas para "particionar" o nosso 'vetor' e a outra para dar o merge nas partições criadas
    Partition dadosInit = *((Partition*) arg); //Salvamos os dados que utilizaremos para realizar as partições
    Partition *dados0, *dados1, *dados2;
    int l = dadosInit.l, r = dadosInit.r, m; //'dadosInit.l' é o limite a esquerda do vetor, 'dadosInit.r' é o limite a direita no vetor e 'm' é o meio do vetor
    if(l < r){
        m = (l+r)/2; //Calculamos qual é o o meio do vetor
        dados0 = (Partition*) malloc(sizeof(Partition));
        dados0->l = l;
        dados0->r = m;
        if(pthread_create(&threads[0], NULL, &mergeSort, (void*)dados0) != 0){
            perror("Failed to create thread");
        }
        dados1 = (Partition*) malloc(sizeof(Partition));
        dados1->l = m+1;
        dados1->r = r;
        if(pthread_create(&threads[1], NULL, &mergeSort, (void*)dados1) != 0){
            perror("Failed to create thread");
        } //Usamos uma thread para realizar o 'mergeSort' na primeira metade do vetor e outra thread para realizar o 'mergeSort' na segunda metade do vetor
        if(pthread_join(threads[0], NULL) != 0){
            perror("Failed to join thread");
        }
        if(pthread_join(threads[1], NULL) != 0){
            perror("Failed to join thread");
        } //Damos joins nas duas threads que irão realizar o 'mergeSort' para garantir que o 'merge' dessas partições só aconteça quando essas duas threads realizarem todo o seu trabalho
        dados2 = (Partition*) malloc(sizeof(Partition));
        dados2->l = l;
        dados2->r = r;
        if(pthread_create(&threads[2], NULL, &merge, (void*)dados2) != 0){
            perror("Failed to create thread");
        }
        if(pthread_join(threads[2], NULL) != 0){
            perror("Failed to join thread");
        } //Esperamos a thread terminar o seu trabalho de dar 'merge' nas partições ordenadas, depois que ela finalizar o seu trabalho já teremos uma parte do vetor ordenada
    }
    pthread_exit(NULL);
}

int main(){

    pthread_t t; //Nossa thread principal
    Partition *pInicial = (Partition*) malloc(sizeof(Partition)); //Valores iniciais que serão passados para a função 'mergeSort'
    int tam, temp; //'tam' é o tamanho do vetor que vai ser dado pelo usuário, 'temp' é uma variável temporária que será usada mais abaixo

    printf("Digite o tamanho do vetor que vai ser ordenado: ");
    scanf("%d", &tam);

    printf("Digite os elementos do vetor: ");
    for(int i = 0; i < tam; i++){ //Aqui nós vamos ler os elementos e armazernar no 'vetor'
        scanf("%d", &temp);
        vetor = (int*) realloc(vetor, sizeof(int)*(i+1));
        vetor[i] = temp; 
    }

    pInicial->l = 0; //Limite esquerdo do vetor
    pInicial->r = tam-1; //Limite direito do vetor

    //'pInicial' vai ser utilizado pela thread 't' para iniciar o mergeSort no nosso vetor

    if(pthread_create(&t, NULL, &mergeSort, (void*)pInicial) != 0){ //Criamos a thread 't', que vai iniciar o mergeSort no nosso vetor
        perror("Failed to create thread");
    }

    if(pthread_join(t, NULL) != 0){ //Usamos join aqui para garantir que a thread 'main' só prossiga quando a thread 't' terminar a sua execução
        perror("Failed to join thread");
    }
    
    printf("O vetor ordenado é: "); //Exibimos o vetor já ordenado
    for(int i = 0; i < tam; i++){ 
        printf("%d ", vetor[i]);
    }

    free(pInicial);
    pthread_exit(NULL);
    return 0;
}