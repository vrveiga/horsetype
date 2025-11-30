To Compile:
gcc -c ascii_data.c;
gcc -c aux_functions.c;
gcc -c horsetype.c;

gcc -o horsetype horsetype.o aux_functions.o ascii_data.o -lncurses -lpthread;

To Run:
./horsetype

To Debug:
valgrind --leak-check=full --track-origins=yes --show-leak-kinds=definite  ./horsetype

Threads:

No desenvolvimento do jogo, as threads foram utilizadas para controlar a movimentação dos cavalos durante a corrida, tarefa realizada pela função `run_horse()`. Cada cavalo é representado por uma thread independente, armazenada no vetor `threads`. A criação das threads é feita atribuindo a cada uma delas um identificador próprio, como mostra o trecho abaixo:

```c
// Cria o vetor que contem as threads
pthread_t threads[num_horses];

// Cria uma thread para cada cavalo e aloca elas no vetor 
for (int i = 0; i < num_horses; i++) {
    int *id = malloc(sizeof(int));
    *id = i;
    // Thread eh criada para seguir a rotina "run_horse"
    if (pthread_create(&threads[i], NULL, run_horse, (void*)id) != 0) {
        perror("Failed to create thread");
        return 1;
    }
}
```

Cada cavalo recebe assim um identificador exclusivo, correspondente à sua posição nos vetores `threads` e `horse_progress`. Dessa forma, cada thread executa de forma independente a lógica responsável pela atualização do progresso do cavalo.

Um exemplo prático dessa independência pode ser observado no trecho a seguir:

```c
void* run_horse(void *arg) {
    // Função usada pelas threads que representam os cavalos
    
    int id = *(int*)arg;
    free(arg);

    while (!game_over) { 
        int sleep_time = (rand() % 150 + 50) * 1000; // Tempo aleatório para cavalo se mover

        usleep(sleep_time); // <-------------
    
    // ...

```

A chamada `usleep()` faz com que cada cavalo espere um tempo aleatório entre seus avanços. Caso todos os cavalos fossem executados em uma única thread, o jogo teria que aguardar o tempo total somado de cada atraso antes de atualizar todos os cavalos, o que reduziria drasticamente a performance e tornaria o jogo lento e pouco responsivo. Com o uso de threads, cada cavalo dorme e avança de maneira independente, permitindo que o jogo continue fluindo de forma natural enquanto a animação e o processamento ocorram paralelamente.

Assim, o uso de threads não apenas melhora o desempenho geral, mas também permite uma modelagem mais realista do comportamento assíncrono dos cavalos na corrida.