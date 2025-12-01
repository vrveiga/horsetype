# Manual do Jogo: Horsetype

**Horsetype** é um jogo de corrida de cavalos baseado em digitação, desenvolvido em C e terminal-based. O projeto simula uma corrida em tempo real onde o jogador deve digitar palavras corretamente para motivar seu cavalo.

## Pré-requisitos

Para compilar e executar o jogo, você precisará de um ambiente Linux com os seguintes pacotes instalados:

  * **GCC** (Compilador C)
  * **Make** (Automação de compilação)
  * **Biblioteca Ncurses** (Interface gráfica de terminal)
  * **Pthread Library** (Para uso de threads)

Para instalá-las no ubuntu/debian, execute:

```bash
sudo apt update
sudo apt install build-essential libncurses5-dev libncursesw5-dev
```

## Compilação

Montamos um Makefile para auxiliar na compilação do jogo. Ele automaticamente gera os objetos necessários linkados nas bibliotecas, utilizando as diretivas -lncurses -lpthread, e gera um único binário executável.

Para compilar, basta rodar:

```bash
make
```

Se quiser limpar os arquivos gerados (*.o e ./horsetype), basta rodar:

```bash
make clean
```

## Execução

Após a compilação, você pode iniciar o jogo rodando:

```bash
make run
```

## Como Jogar

O jogo combina sorte (movimentação autônoma dos cavalos) com habilidade (digitação rápida).

### Configuração da Corrida

Ao iniciar o jogo, siga as instruções na tela:

1.  **Defina os participantes:** Digite quantos cavalos irão correr (Mínimo: 2, Máximo: 9).
2.  **Faça sua aposta:** Escolha o número do cavalo em que deseja apostar (ex: 1 para o Cavalo \#1).

### A Corrida

Assim que a corrida começar:

  * Todos os cavalos se moverão automaticamente em direção à linha de chegada (lado direito da tela).
  * Palavras aleatórias aparecerão na parte inferior da tela.
  * Digite as letras da palavra corretamente.
  * Ao completar uma palavra, seu cavalo ganha um impulso.
  * Quanto mais palavras você completar, maiores as chances de seu cavalo vencer!

### Fim de Jogo

A corrida termina quando o primeiro cavalo cruzar a linha de chegada. Uma tela de resultados será exibida informando o vencedor e se você ganhou ou perdeu a aposta.

## Threads:

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