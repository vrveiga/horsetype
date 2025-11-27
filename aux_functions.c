#include "ascii_data.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/**
* @note: return pointer MUST be freed with free_ascii_art
*/
char **generate_ascii_art(const char *word) { 
    if (word == NULL) return NULL;

    int word_len = strlen(word);
    int max_width = (word_len * MAX_LETTER_WIDTH) + 1;

    char **ascii_word = (char**)malloc(sizeof(char*) * LETTER_HEIGHT);
    if ( ascii_word == NULL) {
        perror("Falha ao alocar memoria da arte ascii");
        return NULL;
    }

    for (int i = 0; i < LETTER_HEIGHT; i++) { //itera por cada linha
        ascii_word[i] = (char *)malloc(max_width );
        if (ascii_word[i] == NULL) {
            perror("Falha ao alocar memoria da arte ascii");
            return NULL;
        }
        ascii_word[i][0] = '\0';

        for (int j = 0; j < word_len; j++) {//passa pela linha i de cada letra
            char ch = word[j];
            if (ch < 'a' || ch > 'z') continue; //evita erros
            strncat(ascii_word[i], alphabet[ch - 'a'][i], MAX_LETTER_WIDTH);
            
        }
    }

    return ascii_word;
}

void free_ascii_art(char **art) {
    if (art == NULL) return;
    for (int i = 0; i < LETTER_HEIGHT; i++) {
        free(art[i]); // libera cada linha
    }
    free(art); // libera o array de ponteiros 
}
