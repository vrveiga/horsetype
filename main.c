#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <ncurses.h>
#include <string.h>

const char *ASCII = 
        "            _M/\n"
        "             (}\n"
        "            //\\_,,;;;;/,\n"
        "        .--(~<';;~  ,  '\\\n"
        "  ,;;;~(    >>     / ``~-'\n"
        " `~''  `\\ )-<-.< ) )__\n"
        "        //'>/'  \\\\~-~\\\n"
        "        |  `\\  =/'    \"\n"
        "        \"   \"\n";

const char *TITLE[] = {
    " _   _                    _____                 ",
    "| | | | ___  _ __ ___  __|_   _|   _ _ __   ___ ",
    "| |_| |/ _ \\| '__/ __|/ _ \\| || | | | '_ \\ / _ \\",
    "|  _  | (_) | |  \\__ \\  __/| || |_| | |_) |  __/",
    "|_| |_|\\___/|_|  |___/\\___||_| \\__, | .__/ \\___|",
    "                               |___/|_|         "
};

void init_screen() {
    initscr();
    
    int title_line_len = strlen(TITLE[0]);
    int title_line_cnt = sizeof(TITLE) / sizeof(TITLE[0]);
    
    int y = LINES / 4;
    int x = (COLS - title_line_len) / 2;

    for (int i = 0; i < title_line_cnt; i++) {
        mvprintw(y + i, x, "%s", TITLE[i]);
    }

    refresh();
}

int main(int argc, char *argv[]) {
    
    init_screen();
    getch();
    endwin();

    return 0;
}