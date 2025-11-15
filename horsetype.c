#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <ncurses.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include "funcoes_aux.h"

const char *TITLE[] = {
    " _   _                    _____                 ",
    "| | | | ___  _ __ ___  __|_   _|   _ _ __   ___ ",
    "| |_| |/ _ \\| '__/ __|/ _ \\| || | | | '_ \\ / _ \\",
    "|  _  | (_) | |  \\__ \\  __/| || |_| | |_) |  __/",
    "|_| |_|\\___/|_|  |___/\\___||_| \\__, | .__/ \\___|",
    "                               |___/|_|         "
};
const int TITLE_HEIGHT = 6;
const int TITLE_WIDTH = 48;

#define MAX_HORSES 9

int horse_progress[MAX_HORSES];
int finish_line;
int num_horses = 0;
int player_bet = -1;

pthread_mutex_t progress_mutex;
volatile bool game_over = false;
int winner = -1;

char palavras[3][10] = {
    "vai!",
    "corre!",
    "forca!"
};
char *current_word = NULL;
int idx_current_char;



void* run_horse(void *arg) {
    int id = *(int*)arg;
    free(arg);

    while (!game_over) {
        int sleep_time = (rand() % 150 + 50) * 1000;
        usleep(sleep_time);

        pthread_mutex_lock(&progress_mutex);

        if (!game_over) {
            horse_progress[id]++;
            
            if (horse_progress[id] + 4 >= finish_line && winner == -1) {
                game_over = true;
                winner = id;
            }
        }
        
        pthread_mutex_unlock(&progress_mutex);
    }
    return NULL;
}








void process_input(int ch) {

    //So processa se houver uma palavra ativa
    if (current_word != NULL) {
        if (ch == current_word[idx_current_char]) {
            idx_current_char++;

            int len_atual = strlen(current_word);
            if (idx_current_char >= len_atual) {
                // Palavra completada! Limpa a palavra
                current_word = NULL;

                //Atualiza o progresso do cavalo
                pthread_mutex_lock(&progress_mutex);
                if (!game_over) {
                    horse_progress[player_bet]++;
                    
                    if (horse_progress[player_bet] + 4 >= finish_line && winner == -1) {
                        game_over = true;
                        winner = player_bet;
                    }
                }
                pthread_mutex_unlock(&progress_mutex);

            }
        }
    }

    
}


void draw_all() {
    //Limpa a tela
    erase();

    //Coloca as barras da linha de chegada
    for (int y = 2; y < num_horses + 2; y++) {
        mvaddch(y, finish_line, '|');
    }

    //Desenha cada cavalo
    for (int i = 0; i < num_horses; i++) {
        //Trava o mutex e pega a posicao atual do cavalo
        pthread_mutex_lock(&progress_mutex);
        int pos = horse_progress[i];
        pthread_mutex_unlock(&progress_mutex);

        //Desenha o cavalo
        int y_pos = 2 + i;

        if (i == player_bet) {
            attron(A_BOLD);
        }
        
        mvprintw(y_pos, pos, "[#%d]", i + 1);

        if (i == player_bet) {
            attroff(A_BOLD);
        }
    }
    
    //Desenha as palavras
    if (current_word != NULL) {
        int y = 15;
        int x;
        const char **arte = map_palavras(current_word);
        
        start_color();
        init_pair(1, COLOR_RED, COLOR_BLACK);

        //Depois, na parte do desenho:
        if(arte != NULL){
            float progress = (float)idx_current_char / strlen(current_word);
            
            for (int j = 0; j < 11; j++) {
                int len_line = strlen(arte[j]);
                int chars_red = (int)(len_line * progress);
                x = (COLS - len_line) / 2;

                //Parte em vermelho
                if (chars_red > 0) {
                    attron(COLOR_PAIR(1));
                    mvprintw(y + j, x, "%.*s", chars_red, arte[j]);
                    attroff(COLOR_PAIR(1));
                }
                
                //Parte normal
                mvprintw(y + j, x + chars_red, "%s", arte[j] + chars_red);
            }
        }
    }
    

    refresh();
}






void init_screen() {
    initscr();
    cbreak();
    noecho();
    curs_set(0);
    
    int title_y = (LINES - TITLE_HEIGHT) / 4-5;
    int title_x = (COLS - TITLE_WIDTH) / 2;
    
    //Printa o titulo
    for (int i = 0; i < TITLE_HEIGHT; i++) {
        mvprintw(title_y + i, title_x, "%s", TITLE[i]);
    }
    
    //Posição da animacao do cavalo
    int horse_y = title_y + TITLE_HEIGHT +1;
    
    int frame_count = 0;
    const char **current_frame;

    while(1){
        // Alterna entre os 4 frames
        if (frame_count % 4 == 0) {
            current_frame = frame1;
        } else if (frame_count % 4 == 1) {
            current_frame = frame2;
        } else if (frame_count % 4 == 2) {
            current_frame = frame3;
        } else {
            current_frame = frame4;
        }
        
        for (int i = 0; i < 19; i++) {
            int len_line = strlen(current_frame[i]);
            int x_centered = (COLS - len_line) / 2;
            mvprintw(horse_y + i, x_centered-1, "%s", current_frame[i]);
        }
        
        mvprintw(title_y + TITLE_HEIGHT + 22, (COLS - 16) / 2, "Aperte qualquer tecla...");
        refresh();
        
        // Verifica se alguma tecla foi pressionada
        nodelay(stdscr, TRUE);
        if (getch() != ERR) {
            break;  // Sai da animação se apertar qualquer tecla
        }
        
        usleep(220000);  // ~3 fps (333ms por frame)
        frame_count++;
    }
    
    // Restaura modo bloqueante e espera tecla final
    nodelay(stdscr, FALSE);
}






void show_results() {
    nodelay(stdscr, FALSE);
    clear();

    flushinp();
    
    int y = LINES / 2;
    int x = (COLS - 40) / 2;

    mvprintw(y - 1, x, "****************************************");
    mvprintw(y,     x, "* O vencedor é... CAVALO #%d!           *", winner + 1);
    
    if (winner == player_bet) {
        mvprintw(y + 1, x, "* Você venceu sua aposta!                    *");
    } else {
        mvprintw(y + 1, x, "* Você perdua sua aposta.                   *");
    }
    mvprintw(y + 2, x, "****************************************");

    refresh();
    sleep(3);
    flushinp();

    mvprintw(y + 4, x, "     (Aperte qualquer tecla para sair)");
    refresh();

    getch();
}

int main(int argc, char *argv[]) {
    
    srand(time(NULL));
    
    //Tela inicial
    init_screen();

    //Perimite o usuario escrever no terminal
    echo();
    //Faz o cursor aparecer
    curs_set(1);
    //Limpa a tela incial do terminal
    clear();
    
    //Faz a primeira pergunta ate ser um valor valido
    do {
        mvprintw(2, 2, "Quantos cavalos irão correr? (2-%d): ", MAX_HORSES);
        scanw("%d", &num_horses);
    } while (num_horses < 2 || num_horses > MAX_HORSES);
    //Faz a segunda pergunta ate ser um valor valido
    do {
        mvprintw(4, 2, "Faça sua aposta! (1-%d): ", num_horses);
        scanw("%d", &player_bet);
    } while (player_bet < 1 || player_bet > num_horses);

    //Ajuda para ficar indexado em 0
    player_bet--;
    
    noecho();
    curs_set(0);
    //Faz com que o input do usuario (mecanica de torcer) nao bloquei o programa
    nodelay(stdscr, TRUE);
    
    finish_line = COLS - 2;
    //Enche o vetor de progresso dos cavalos com 0s
    memset(horse_progress, 0, sizeof(horse_progress));

    //Inicia os semaforos
    pthread_mutex_init(&progress_mutex, NULL);

    //Cria o vetor que contem as threads
    pthread_t threads[num_horses];

    //Cria uma thread para cada cavalo e aloca elas no vetor 
    for (int i = 0; i < num_horses; i++) {
        int *id = malloc(sizeof(int));
        *id = i;
        //Thread eh criada para seguir a rotina "run_horse"
        if (pthread_create(&threads[i], NULL, run_horse, (void*)id) != 0) {
            perror("Failed to create thread");
            return 1;
        }
    }




    //Main Loop
    while (!game_over) {

        //Escolhe a proxima palavra
        if(current_word == NULL && rand()%10==0){
            int idx_palavra = rand() % 3;
            current_word = palavras[idx_palavra];
            idx_current_char = 0;
        }
        int ch = getch();
        if (ch != ERR) {
            process_input(ch);
        }

        draw_all();
        
        usleep(33000);
    }






    for (int i = 0; i < num_horses; i++) {
        pthread_join(threads[i], NULL);
    }
    
    pthread_mutex_destroy(&progress_mutex);

    show_results();
    
    endwin();
    return 0;
}