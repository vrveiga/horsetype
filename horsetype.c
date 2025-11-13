#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <ncurses.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

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

void draw_all_horses() {
    clear();

    for (int y = 2; y < num_horses + 2; y++) {
        mvaddch(y, finish_line, '|');
    }

    for (int i = 0; i < num_horses; i++) {
        pthread_mutex_lock(&progress_mutex);
        int pos = horse_progress[i];
        pthread_mutex_unlock(&progress_mutex);

        int y_pos = 2 + i;

        if (i == player_bet) {
            attron(A_BOLD);
        }
        
        mvprintw(y_pos, pos, "[#%d]", i + 1);

        if (i == player_bet) {
            attroff(A_BOLD);
        }
    }
    
    mvprintw(0, 0, "Press 'q' to quit");
    refresh();
}

void init_screen() {
    initscr();
    cbreak();
    noecho();
    curs_set(0);
    
    
    int y = (LINES - TITLE_HEIGHT) / 4;
    int x = (COLS - TITLE_WIDTH) / 2;

    for (int i = 0; i < TITLE_HEIGHT; i++) {
        mvprintw(y + i, x, "%s", TITLE[i]);
    }
    
    mvprintw(y + TITLE_HEIGHT + 2, (COLS - 16) / 2, "press any key...");
    refresh();
    getch();
}

void show_results() {
    nodelay(stdscr, FALSE);
    clear();
    
    int y = LINES / 2;
    int x = (COLS - 40) / 2;

    mvprintw(y - 1, x, "****************************************");
    mvprintw(y,     x, "* The winner is... HORSE #%d!           *", winner + 1);
    
    if (winner == player_bet) {
        mvprintw(y + 1, x, "* You won your bet!                    *");
    } else {
        mvprintw(y + 1, x, "* You lost your bet.                   *");
    }
    mvprintw(y + 2, x, "****************************************");
    mvprintw(y + 4, x, "     (Press any key to exit)");
    
    refresh();
    getch();
}

int main(int argc, char *argv[]) {
    
    srand(time(NULL));
    
    init_screen();

    echo();
    curs_set(1);
    clear();
    
    do {
        mvprintw(2, 2, "How many horses to race? (2-%d): ", MAX_HORSES);
        scanw("%d", &num_horses);
    } while (num_horses < 2 || num_horses > MAX_HORSES);
    
    do {
        mvprintw(4, 2, "Place your bet! (1-%d): ", num_horses);
        scanw("%d", &player_bet);
    } while (player_bet < 1 || player_bet > num_horses);

    player_bet--;
    
    noecho();
    curs_set(0);
    nodelay(stdscr, TRUE);
    
    finish_line = COLS - 2;
    memset(horse_progress, 0, sizeof(horse_progress));
    pthread_mutex_init(&progress_mutex, NULL);

    pthread_t threads[num_horses];

    for (int i = 0; i < num_horses; i++) {
        int *id = malloc(sizeof(int));
        *id = i;
        if (pthread_create(&threads[i], NULL, run_horse, (void*)id) != 0) {
            perror("Failed to create thread");
            return 1;
        }
    }

    while (!game_over) {
        draw_all_horses();
        
        int ch = getch();
        if (ch == 'q') {
            game_over = true;
        }
        
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