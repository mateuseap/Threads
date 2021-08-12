#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

pthread_mutex_t lock;

int atual_primo = 1;             // Topo da fila de números
int N_max;                        // Quantidade de números que serão testados
int qtd_threads = 0;           // Quantidade de threads definidas
int qtd_execs = 0;              // Quantidade de execuções
int n_primos = 0;            // Quantidade de números primos encontrados
int N_enesimo_Primo;

int primo(int value) {
    int i = 2;
    while (i < value && value%i != 0) i++;
    return i == value;
} 

void *thread_fn(void *threadid) {
    int *id = (int*) threadid; // Vem como argumento da thread
    int valor = 0;            // Pega do espaço compartilhado
    int res = 0;
    while (n_primos + 1 <= N_max) {
        // Tranca o mutex, pega um valor para calcular e soma
        // para a próxima thread executar o próximo numero
        pthread_mutex_lock(&lock);
        valor = atual_primo;
        N_enesimo_Primo = valor;
        atual_primo++;
        pthread_mutex_unlock(&lock);

        // Calcula se é primo ou não
        res = primo(valor);
        
        // Soma no contador de execuções e caso seja primo soma no contador de primos também 
        pthread_mutex_lock(&lock);
        if(res) {
            n_primos++;
        }
        qtd_execs++;
        pthread_mutex_unlock(&lock);

        // Espera até as outras threads executarem
        while (n_primos + 1 <= N_max && qtd_execs < valor + qtd_threads -1);
    }
}

int main () {
    int N = 0, T = 0, i = 0;

    int* args = (int *) malloc(sizeof(int));
    pthread_t* threads = (pthread_t *) malloc(sizeof(pthread_t));
    
    pthread_mutex_init(&lock, NULL);

    printf("Numero T: ");
    scanf("%d", &T);

    printf("Numero N: ");
    scanf("%d", &N);

    N_max = N;
    qtd_threads = T;
    args = (int *) realloc(args, T * sizeof(int));
    threads = (pthread_t *) realloc(threads, T * sizeof(pthread_t));
    for (i = 0; i < T; i++) {
        args[i] = i;
        pthread_create(&(threads[i]), NULL, thread_fn, &(args[i]));
    }
    for (i = 0; i < T; i++) {
        pthread_join(threads[i], NULL);
    }
    
    pthread_mutex_destroy(&lock);

    printf("N-enesimo numero primo: %d\n", N_enesimo_Primo);

    return 0;
}
