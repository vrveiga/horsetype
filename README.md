# Manual do Jogo: Horsetype

**Horsetype** é um jogo de corrida de cavalos baseado em digitação, desenvolvido em C e terminal-based. O projeto simula uma corrida em tempo real onde o jogador deve digitar palavras corretamente para motivar seu cavalo.

### Autores
- Felipe Camargo Cerri
- Nicolas de Sousa Maia
- Vitor Rocha Veiga

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

## Threads

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

A chamada `usleep()` faz com que cada cavalo espere um tempo aleatório entre seus avanços. Caso todos os cavalos fossem executados em uma única thread, o programa teria que processar a posição e o atraso na movimentação de cada cavalo de forma sequencial, reduzindo a responsividade do jogo. Com o uso de threads, cada cavalo dorme e avança de maneira independente, permitindo que o jogo continue fluindo de forma natural enquanto a animação e o processamento ocorram paralelamente.

Assim, o uso de threads não apenas melhora o desempenho geral, mas também permite uma modelagem mais realista do comportamento assíncrono dos cavalos na corrida.

Quando o jogo acaba, todas as threads dos cavalos são finalizadas utilizando o `pthread_join()`.

## Condições de Corrida
ara viabilizar a utilização de multithreading e evitar comportamentos indefinidos, foi necessário identificar e controlar o acesso às variáveis compartilhadas entre a thread principal (responsável pelo input e output) e as threads dos cavalos. As principais variáveis críticas identificadas foram:


```c
int horse_progress[MAX_HORSES];
//...
atomic_bool game_over = false;
int winner = -1;

```

Foram utilizadas variadas técnicas de sincronização para garantir a integridade dos dados, sendo a principal a utilização de semáforos para exclusão mútua.:


## Semáforos
Os semáforos são primitivas de sincronização que coordenam o acesso a recursos compartilhados através de um contador interno. Neste projeto, utilizamos um semáforo binário, que atua efetivamente como um Mutex.

O acesso às regiões críticas do código são protegidas pelas operações da biblioteca <semaphore.h>:

- sem_wait(): Decrementa o semáforo. Se o valor for 0, a thread é bloqueada até que o recurso esteja disponível.

- sem_post(): Incrementa o semáforo, liberando o recurso e acordando threads em espera.


### `horse_progress[]`
Para coordenar a lógica semáforica, utilizamos o semáforo `sem_progress` iniciado na função main:
```c
sem_init(&progress_sem, 0, 1);
```

O acesso a variável `horse_progress[]`ocorre em três momentos distintos: na funcão `run_horse()` executada por cada thread, na main através da função `process_input()` e `draw_all()` do input:

Sem proteção, se a thread do cavalo e a do jogador tentassem mover o mesmo cavalo ao mesmo tempo, um dos incrementos poderia ser perdido, ou a interface gráfica poderia renderizar um estado inconsistente.

#### `draw_all()` (Read-Only)

```c
void draw_all() {
    // Puxa o estado atual da corrida
    int local_progress[MAX_HORSES];

    // Entra na seção crítica para pegar o estado
    sem_wait(&progress_sem); // <--- Bloqueio do acesso a variável
    memcpy(local_progress, horse_progress, sizeof(horse_progress));
    sem_post(&progress_sem); // <--- Liberação do acesso a variável
    
    // ...
```

Nessa ocorrência, devido a característica de apenas leitura da varíavel compatilhada nessa função, optamos por realizar uma técnica de snapshot do progesso da corrida. O semáforo é retido apenas pelo tempo necessário para realizar a cópia da memória, de forma que o processo custoso de renderização na tela ocorre fora da seção crítica, minimizando o tempo de bloqueio das threads dos cavalos, garantindo fluidez ao jogo.


#### `process_input()`(Read-Write)

```c
void process_input(int ch) {
    // ... Lógica de verificação de char ...

                // Atualiza o progresso do cavalo
                sem_wait(&progress_sem); // Faz o bloqueio do semáforo
                
                if (!game_over) {
                    horse_progress[player_bet]++;
                    
                    if (horse_progress[player_bet] + 4 >= finish_line && winner == -1) {
                        game_over = true;
                        winner = player_bet;
                    }
                } 

                sem_post(&progress_sem); // Libera o semáforo
            }
        }
    }
}
```
A função de input realiza o acesso tanto para leitura como para escrita, por isso, após realizar uma lógica interna, o semáforo é utilizado. Vale ressaltar que como as funções `draw_all()` e `process_input()` ocorrem sequencialmente na main,os semáforos servem para coordenar o acesso dessas duas funções da main com as threads executando `run_horse()` 

#### `run_horse()`(Read-Write)
```c
void* run_horse(void *arg) {
    // ...

    while (!game_over) { 
        int sleep_time = (rand() % 150 + 50) * 1000; // Tempo aleatório para cavalo se mover
        usleep(sleep_time);

        sem_wait(&progress_sem); // Faz o bloqueio do semáforo

        // Se não tiver acabado a corrida, move o cavalo
        if (!game_over) {
            horse_progress[id]++;
            
            if (horse_progress[id] + 4 >= finish_line && winner == -1) {
                game_over = true;
                winner = id;
            }
        }
        
        sem_post(&progress_sem); // Libera o semáforo
    }
    return NULL;
}
```
Novamente a função realiza a leitura e escrita na variável compartilhada, o bloqueio protege a lógica central do jogo. É crucial notar que a verificação if (!game_over) é feita  novamente dentro da seção crítica para evitar que um cavalo se mova após o término oficial da corrida ter sido decretado por outra thread no tempo de sleep.

## Outros Métodos de Evitar Condições de Corrida

### `game_over`
evido a simplicidade dessa varíavel booleana, a declaramos com o tipo atomic_bool da biblioteca <stdatomic.h>. Isso garante que as operações de leitura e escrita nessa flag sejam atômicas ao nível do processador evitando a dessincronização de leitura e escrita dessa variável. Embora o fluxo lógico principal já esteja protegido por semáforos em `processo_input()` e `run_horse()`, a ausência dessa proteção contra interrupções de escalonamento na main, permitiria a validação da condição do laço while como verdadeira instantes antes de um cavalo vencer, uma vez que a decisão de entrar no laço já foi tomada, o código executaria todo o ciclo de desenho novamente com o estado antigo. Dessa forma, o erro ocorreria pela alteração da variável após o acesso, printando um frame a mais desnecessário antes de encerrar o programa.

### `winner`
Apesar da variável winner não possuir um semáforo próprio, ela é protegida indiretamente pelo mesmo semáforo de horse_progress, evitando uma condição de corrida nas funções `run_hose()` e `process_input()` a partir da seguinte sequência:

1. Dois cavalos poderiam cruzar a linha de chegada quase simultaneamente.

2. Sem o semáforo, ambos passariam pela verificação if (winner == -1).

3. O segundo cavalo a escrever sobrescreveria o vencedor legítimo.

Assim, envolver a leitura (winner == -1) em  e a escrita (winner = id) no mesmo bloco semafórico, garantimos que apenas o primeiro cavalo a cruzar a linha defina o vencedor.