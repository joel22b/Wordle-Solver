#include "utils.h"

#define WORD_LENGTH 5
#define GUESS_NUM 6

int main(int argc, char **argv) {
    int i, j, k;
    unsigned char** guesses = malloc(sizeof(unsigned char*) * GUESS_NUM);
    for (i = 0; i < GUESS_NUM; i++) {
        guesses[i] = malloc(WORD_LENGTH + 1);
        guesses[i][1] = '\0';
        guesses[i][WORD_LENGTH] = '\0';
    }
    
    // Queue of letters and how to filter them
    queue* filterQueueFirst = NULL, *filterQueueLast = NULL;

    // Queue of words
    int status;
    queue* wordQueueLast = NULL;
    queue* wordQueueFirst = read_file_into_queue("dict/wordle.txt", &wordQueueLast, &status);

    printf("Form for result:\nb = black (not in word)\ny = yellow (in word, wrong spot)\ng = green (correct letter)\n\n");

    for (i = 0; i < GUESS_NUM; i++) {
        // Calculate best guess
        unsigned char* bestWord = calculate_best_word (wordQueueFirst);
        printf("The best word to guess is: %s\n", bestWord);
        memcpy(guesses[i], bestWord, WORD_LENGTH + 1);
        free(bestWord);

        // Read in the guess
        status = 5;
        //printf("Please enter your %d guess: ", i + 1);
        //status = read_console(guesses[i], WORD_LENGTH + 2);
        if (guesses[i][0] == 'q' && guesses[i][1] == '\0') {
            break;
        }
        if (status == -1 || status != WORD_LENGTH) {
            i--;
            continue;
        }

        // Read in how the guess was
        int valid = -1;
        unsigned char result[WORD_LENGTH];
        while (valid == -1) {
            printf("Please enter your %d result: ", i + 1);
            status = read_console(&result[0], WORD_LENGTH + 2);
            if (result[0] == 'q' && result[1] == '\0') {
                valid = -2;
                break;
            }
            if (status == -1 || status != WORD_LENGTH) {
                continue;
            }
            if (result[0] == 'g' && result[0] == result[1] && result[1] == result[2] &&
                result[2] == result[3] && result[3] == result[4]) {
                    printf("Congrats! It took %d guesses!\n", i + 1);
                    valid = -2;
                    break;
            }
            int proper = 0;
            for (j = 0; j < WORD_LENGTH; j++) {
                if (!(result[j] == 'b' || result[j] == 'y' || result[j] == 'g')) {
                    proper = -1;
                }
            }
            if (proper == 0) {
                valid = 0;
            }
        }
        if (valid == -2) {
            break;
        }

        // Process result
        for (j = 0; j < WORD_LENGTH; j++) {
            // Check if letter already exists
            int exists = 0;
            queue* cursor = filterQueueFirst;
            letter* tempLetter = NULL;
            while (cursor != NULL) {
                tempLetter = (letter*)(cursor->data);
                if (tempLetter->letter == guesses[i][j]) {
                    exists = 1;
                    break;
                }
                cursor = cursor->next;
            }

            if (exists == 0) {
                // Create new letter
                tempLetter = malloc(sizeof(letter));
                tempLetter->letter = guesses[i][j];
                tempLetter->position = -1;
                tempLetter->isCorrect = false;
                for (k = 0; k < WORD_LENGTH; k++) {
                    tempLetter->triedPositions[k] = false;
                }
            }
            // Update letter with new info
            switch (result[j]) {
            case 'b':
                tempLetter->inWord = false;
                break;
            case 'y':
                tempLetter->inWord = true;
                tempLetter->triedPositions[j] = true;
                break;
            case 'g':
                tempLetter->inWord = true;
                tempLetter->isCorrect = true;
                tempLetter->position = j;
                break;
            default:
                printf("Error: Unknown char in result: %c\n", result[j]);
                break;
            }
            
            // Add letter to queue
            if (exists == 0) {
                filterQueueLast = queuePush(filterQueueLast, tempLetter);
                if (filterQueueFirst == NULL) {
                    filterQueueFirst = filterQueueLast;
                }
            }
        }

        // Filter word queue to new word list
        queue* newWordQueueFirst = NULL, *newWordQueueLast = NULL;

        while (wordQueueFirst != NULL) {
            unsigned char* word = (unsigned char*)queuePop(&wordQueueFirst);

            if (filter_word(word, filterQueueFirst)) {
                newWordQueueLast = queuePush(newWordQueueLast, word);
                if (newWordQueueFirst == NULL) {
                    newWordQueueFirst = newWordQueueLast;
                }
            }
            else {
                free(word);
            }
        }
        
        wordQueueFirst = newWordQueueFirst;
        wordQueueLast = newWordQueueLast;

        // Checks if only 1 or 0 words in list
        if (wordQueueFirst == NULL) {
            printf("Ummm this is awkward... there's no possible word left...\n");
            break;
        }
        if (wordQueueFirst->next == NULL) {
            printf("The word must be: %s\nCongrats!\n", (unsigned char*)wordQueueFirst->data);
            break;
        }

        // Writes filtered word list to file
        char path[16];
        sprintf(path, "guess%d.txt", i + 1);
        status = write_file_queue(&path[0], wordQueueFirst, WORD_LENGTH);
        if (status != 0) {
            printf("Error: Failed to write queue to file. Path=%s Status=%d\n", path, status);
        }
    }

    printf("Exiting...\n");

    // Clean up
    for (i = 0; i < GUESS_NUM; i++) {
        free(guesses[i]);
    }
    free(guesses);

    queueDestroy(wordQueueFirst);
    queueDestroy(filterQueueFirst);

    return 0;
}