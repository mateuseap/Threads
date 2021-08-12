#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>
#include <pthread.h>

#define QT_THREADS 8 //Quantidade de threads que irá ser utilizada para os cálculos 
#define QT_ARQUIVOS 5 //Quantidades de arquivos de entrada

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

typedef struct{
    int idDaSala, qtAlunos;
    double media, moda, mediana, desvioPadrao;
    double *notaAlunos;
} Salas; //Struct que representa uma sala e os dados que queremos calcular em relação as notas dos alunos daquela sala

typedef struct{
    char nome[50];
    double nota;
} Aluno; //Struct que representa um aluno de um sala

Salas dados[QT_ARQUIVOS]; //Vetor que irá conter todos os dados de todas as salas (nome dos alunos, notas, moda, mediana, etc)
char nomeDoArquivo[] = "sala_.txt"; //String que será utilizada pelas threads na hora de abrir o arquivo a ser lido
bool arquivosRestantes[QT_ARQUIVOS]; //Vetor que "indica" se um arquivo já foi aberto e processado ou não
int offset[QT_THREADS]; //Vetor que "indica" qual o próximo arquivo que uma thread irá ler (caso seja necessário)

void *analisarDesempenho(void *arg){
    int threadID = *((int*)arg); //Salvamos o ID da Thread que está executando
    Aluno temp;
    double sT, xiMa;
    int aux, max;
    if(threadID >= QT_ARQUIVOS){
        pthread_exit(NULL);
    } /*Se a quantidade de threads for maior que a quantidade de arquivos (no caso, maior que a quantidade de "sala_.txt"),
    nós eliminaremos as threads sobrelasentes antes que elas executem, pois sabemos que um arquivo só podera ser processado por uma única thread. Por exemplo,
    se tivermos 5 arquivos e 8 threads, eliminamos as threads com ID's 5, 6 e 7, pois as threads com ID's 0, 1, 2, 3 e 4 já irão realizar o trabalho
    de calcular os dados de todos os arquivos.
    */
    loop:
    
    aux = max = 0; //Variáveis temporárias que utilizaremos durante alguns cálculos

    pthread_mutex_lock(&mutex); //Travamos o mutex para evitar uma condição de disputa e garantir que uma única thread tenha acesso a um arquivo
    threadID = offset[*((int*)arg)]; //O ID da thread somado a 1 vai equivaler ao número da sala que ele deve abrir o arquivo e processar, o offset[[*((int*)arg)] contém essa informação ([*((int*)arg) é o ID real da Thread)
    arquivosRestantes[threadID] = false; //Setamos como false 'arquivosRestante' na posição threadID para indicar que aquele respectivo arquivo já vai ser processado por uma thread
    char IDdaSala = (threadID+1)+'0'; //Salvamos o ID da sala em forma de char
    dados[threadID].idDaSala = threadID+1; //Salvamos qual é o ID da sala que vai ser processada
    dados[threadID].media = dados[threadID].moda = dados[threadID].mediana = dados[threadID].desvioPadrao = dados[threadID].qtAlunos = 0; //Inicializamos a media, moda, mediana, desvio padrão e a quantidade de alunos da sala como 0, pois ainda iremos ler e armazenar os alunos daquela sala e depois calcular os respectivos dados
    nomeDoArquivo[4] = IDdaSala; //Modificamos a string 'nomeDoArquivo' para saber qual sala iremos ler agora (ex.: se ID da Sala = 1, após essa modificação nomeDoArquivo = sala1.txt)
    printf("Thread[%d] | Sala %s\n", *((int*)arg), nomeDoArquivo); //Exibimos qual thread está processando os dados de qual sala
    dados[threadID].notaAlunos = NULL; //Inicializamos o vetor das notas dos alunos como NULL

    FILE *arquivo;
    if((arquivo = fopen(nomeDoArquivo,"rt")) == NULL){
        printf("Nao foi possivel abrir o arquivo!\n");
        exit(-1);
    } //Abrimos o arquivo
    pthread_mutex_unlock(&mutex); //Como uma thread já acabou de abrir o arquivo que ela irá processar, então podemos liberar o mutex para que outra thread possa fazer o seu trabalho, pois a variável 'nomeDoArquivo' (que é uma região crítica) já está livre para uso

    while((fscanf(arquivo, "%s %lf", temp.nome, &temp.nota)) != EOF){
        dados[threadID].media += temp.nota; //Adicionamos as notas de cada aluno da sala na média da sala
        dados[threadID].qtAlunos++;
        dados[threadID].notaAlunos = (double*) realloc (dados[threadID].notaAlunos, dados[threadID].qtAlunos*sizeof(double));
        dados[threadID].notaAlunos[((int)dados[threadID].qtAlunos)-1] = temp.nota;
    } //Salvamos todos os dados (notas, nomes e quantidade de alunos de uma sala) no nosso vetor 'dados'.
    fclose(arquivo); 

    dados[threadID].media /= (double)dados[threadID].qtAlunos; //Dividimos o somatório das notas dos alunos da sala pela quantidade de alunos para assim obtermos a média da sala

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
    } //Calculamos a moda e fazemos parte do calculo do desvio padrão ao mesmo tempo

    dados[threadID].desvioPadrao /= (double)dados[threadID].qtAlunos;
    dados[threadID].desvioPadrao = sqrt(dados[threadID].desvioPadrao); //Finalizamos o cálculo de desvio padrão de uma sala

    for(int i = 0; i < dados[threadID].qtAlunos-1; i++){
        for(int j = 0; j < dados[threadID].qtAlunos-1; j++){
            if(dados[threadID].notaAlunos[j+1] < dados[threadID].notaAlunos[j]){
                sT = dados[threadID].notaAlunos[j];
                dados[threadID].notaAlunos[j] = dados[threadID].notaAlunos[j+1];
                dados[threadID].notaAlunos[j+1] = sT; 
            }
        }
    } //Ordenamos o vetor de notas dos alunos de uma sala para depois podermos calcular a mediana 

    if(((int)dados[threadID].qtAlunos)%2 == 0){
        dados[threadID].mediana = (dados[threadID].notaAlunos[((int)dados[threadID].qtAlunos)/2] + dados[threadID].notaAlunos[((int)dados[threadID].qtAlunos)/2 - 1])/2;
    }else{
        dados[threadID].mediana = dados[threadID].notaAlunos[((int)dados[threadID].qtAlunos)/2];
    } //Calculamos a mediana de uma respectiva sala 
    
    int check = 1; //Ao chegar aqui, a thread já encerrou seu trabalho e calculou os dados de uma sala (moda, mediana, média e desvio padrão), então temos que checar se ainda há mais arquivos para serem lido ou se o trabalho já acabou
    pthread_mutex_lock(&mutex); //Travamos o mutex para poder verificar se todos os arquivos já foram lidos
    for(int i = QT_ARQUIVOS; i >= 0; i--){
        if(arquivosRestantes[i] == true){ //Caso alguma posição do vetor de 'arquivosRestante' seja true, quer dizer que ainda há um arquivo (no caso, uma sala) para ser processado e calculado
            arquivosRestantes[i] = false; //Setamos como false 'arquivosRestantes' para dizer que aquele arquivo pendente referente a posição 'i' será lido e processado por uma thread
            offset[*((int*)arg)] = i; //Atualizamos o offset para a thread saber qual o próximo arquivo que ela deverá abrir e fazer os cálculos
            check = 0; //Dizemos que check = 0 para a thread saber que o seu trabalho ainda não acabou
            break;
        }
    }
    pthread_mutex_unlock(&mutex); //Liberamos o mutex para que outra thread possa fazer a verificação
    
    if(check == 1){ //Caso check seja = 1, significa que todos os arquivos já foram lidos e processados, então a thread pode encerrar a sua execução
        pthread_exit(NULL);
    }
    goto loop; //Caso check não seja igual a 1, significa que a thread ainda tem mais uma sala para processar, então damos um goto para o inicío da função para que a thread possa fazer o seu trabalho 
}

int main(){

    pthread_t threads[QT_THREADS];
    int *ids[QT_THREADS]; //Vetor de ID's da threads

    memset(arquivosRestantes, true, QT_ARQUIVOS); //Inicializamos o vetor 'arquivosRestantes' como true em todas as posições, para indicar que nenhum arquivo ainda foi aberto e processado
    for(int i = 0; i < QT_THREADS; i++){
        offset[i] = i;
    } //Inicialmente o vetor de offset vai conter o ID de cada thread;

    for(int i = 0; i < QT_THREADS; i++){
        ids[i] = (int*) malloc(sizeof(int));
        *ids[i] = i;
        if(pthread_create(&threads[i], NULL, &analisarDesempenho, (void*)ids[i]) != 0){ //Criando uma Thread e passando a função 'analisarDesempenho' como parâmetro e o ID da Thread como argumento
            perror("Failed to create thread");
        }
    }
    for(int i = 0; i < QT_THREADS; i++){ //Damos join em todas as Threads que já foram craidas para garantir que a thread 'main' só continue sua execução quando todas essas threads terminarem seus respectivos trabalhos
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
    } //Exibimos a media, moda, mediana e o desvio padrão de cada sala

    pthread_exit(NULL);
    return 0;
}
