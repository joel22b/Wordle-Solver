#include "utils.h"

#define WORD_LENGTH 5

int main(int argc, char **argv) {
    int wordsLength, status, i, j;
    char* wordsPath = "dict/wordle-double.txt";

    unsigned char* wordsFile = read_file(wordsPath, &wordsLength, &status);
    if (status != 0) {
        printf("Error reading file. Status: %d\n", status);
        return 0;
    }

    int wordsCursor = 0;
    unsigned char* cursor = wordsFile;
    queue *queueFirst = NULL, *queueLast = NULL;
    while (wordsLength > wordsCursor) {
        // Find word length
        int length = 0, good = 0;
        while (cursor[length] != '\n') {
            if (cursor[length] == '\'') {
                good = -1;
            }
            length++;
        }

        if (length == WORD_LENGTH && good == 0) {
            // Copy word into new memory location
            unsigned char* word = malloc(length + 1);
            memcpy(word, cursor, length);
            word[length] = '\0';

            char seen[WORD_LENGTH];
            status = 0;
            for (i = 0; i < WORD_LENGTH; i++) {
                for (j = 0; j < i; j++) {
                    if (word[i] == seen[j]) {
                        status = -1;
                    }
                }
                seen[i] = word[i];
            }

            if (status == 0) {
                // Add word to queue
                queueLast = queuePush(queueLast, word);
                if (queueFirst == NULL) {
                    queueFirst = queueLast;
                }
            }
            else {
                free(word);
            }
        }

        // Update cursors
        cursor = &cursor[length + 1];
        wordsCursor += length + 1;
    }

    free(wordsFile);

    write_file_queue("dict/wordle.txt", queueFirst, WORD_LENGTH);

    queueDestroy(queueFirst);
}
