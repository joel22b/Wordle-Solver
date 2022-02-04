#include "utils.h"

unsigned char* read_file (const char* path, int* fileLength, int* statusCode){
    unsigned char* buffer = 0;
    long length;
    FILE * fileptr = fopen (path, "rb");
    /* Ensure the file is located correctly and is not empty */
    if (fileptr == NULL){
        printf("Error: Unable to open file. Is %s the correct file path?\n", path);
        *statusCode = -1;
        return NULL;
    }
    else{
        /* Determine the number of bytes within the file, save to variable length */
        fseek (fileptr, 0, SEEK_END);
        length = ftell (fileptr);
        fseek (fileptr, 0, SEEK_SET);
        buffer = malloc (length);
        if (buffer == NULL) {
            *statusCode = -2;
            return NULL;
        }
        /* Ensuring there is a value in the buffer, read the file. It is expected
         * to have the length equalling the number of reads (output of fread) */
        if (buffer) {
            int readReturnCode = fread (buffer, 1, length, fileptr);
            if (readReturnCode != length){
                *statusCode = -3;
                return NULL;
            }
        }
        fclose (fileptr);
    }
    *fileLength = length;
    *statusCode = 0;
    return buffer;
}

queue* read_file_into_queue (const char* path, queue** queueLast, int* statusCode) {
    int fileLength, status;
    unsigned char* file = read_file(path, &fileLength, &status);
    if (status != 0) {
        printf("Error reading file. Status: %d\n", status);
        return 0;
    }

    int wordsCursor = 0;
    unsigned char* cursor = file;
    queue *queueFirst = NULL;
    *queueLast = NULL;
    while (fileLength > wordsCursor) {
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

            // Add word to queue
            *queueLast = queuePush(*queueLast, word);
            if (queueFirst == NULL) {
                queueFirst = *queueLast;
            }
        }

        // Update cursors
        cursor = &cursor[length + 1];
        wordsCursor += length + 1;
    }

    free(file);

    return queueFirst;
}

int write_file (const char* path, const unsigned char* data, int length) {
    FILE* filePointer;
    filePointer = fopen(path,"w");
    fwrite(data, length, 1, filePointer);
    fclose(filePointer);
    return 0;
}

int write_file_queue (const char* path, queue* first, int wordLength) {
    int queueLength = 0;
    queue* temp = first;
    while (temp != NULL) {
        queueLength++;
        temp = temp->next;
    }

    if (queueLength == 0) {
        printf("Warning: Could not write queue to file. Queue was empty.\n");
        return -1;
    }

    int length = (wordLength + 1) * queueLength;
    unsigned char* data = malloc(length + 1);
    data[length] = '\0';

    temp = first;
    unsigned char* cursor = data;
    while (temp != NULL) {
        memcpy(cursor, temp->data, wordLength);
        cursor[wordLength] = '\n';
        cursor = &cursor[wordLength + 1];
        temp = temp->next;
    }

    write_file(path, data, length);
    free(data);

    return 0;
}

queue* queuePush (queue* last, void* data) {
    if (last != NULL) {
        while (last->next != NULL) {
            last = last->next;
        }
    }

    queue* new = malloc(sizeof(queue));
    new->next = NULL;
    new->data = data;

    if (last != NULL) {
        last->next = new;
    }
    return new;
}

void* queuePop (queue** first) {
    if ((*first) == NULL) {
        printf("Error: Could not pop queue. Queue is empty\n");
    }

    queue* temp = (*first)->next;
    unsigned char* data = (*first)->data;
    free(*first);
    *first = temp;
    return data;
}

int queueDestroy (queue* first) {
    queue* temp;
    while (first != NULL) {
        temp = first;
        first = first->next;
        free(temp->data);
        free(temp);
    }
    return 0;
}

int read_console (unsigned char *buf, int n) {
    int count = 0;
    char c;

    if (__glibc_unlikely(n <= 0))
        return -1;

    while (--n && (c = fgetc(stdin)) != '\n')
        buf[count++] = c;
    buf[count] = '\0';

    return (count != 0 || errno != EAGAIN) ? count : -1;
}

bool filter_word (unsigned char* word, queue* filter) {
    int i;
    while (filter != NULL) {
        letter* letterFilter = (letter*)filter->data;

        if (letterFilter->inWord) {
            if (letterFilter->isCorrect) {
                if (word[letterFilter->position] != letterFilter->letter) {
                    return false;
                }
            }
            else {
                bool seenLetter = false;
                for (i = 0; i < WORD_LENGTH; i++) {
                    if (word[i] == letterFilter->letter) {
                        if (letterFilter->triedPositions[i]) {
                            return false;
                        }
                        seenLetter = true;
                    }
                }
                if (!seenLetter) {
                    return false;
                }
            }
        }
        else {
            for (i = 0; i < WORD_LENGTH; i++) {
                if (word[i] == letterFilter->letter) {
                    return false;
                }
            }
        }

        filter = filter->next;
    }

    return true;
}

letterTable* build_letter_table (queue* wordsQueue) {
    letterTable* letterDataTable = malloc(sizeof(letterTable));
    int i, j;
    // Initalize letter table
    for (i = 0; i < ALPHABET_LENGTH; i ++) {
        for (j = 0; j < WORD_LENGTH; j++) {
            letterDataTable->countPos[i][j] = 0;
            letterDataTable->fracPos[i][j] = 0;
        }
        letterDataTable->countTotal[i] = 0;
        letterDataTable->fracTotal[i] = 0;
    }

    // Build count tables
    int wordCount = 0;
    while (wordsQueue != NULL) {
        for (i = 0; i < WORD_LENGTH; i++) {
            unsigned char* word = (unsigned char*)wordsQueue->data;
            int letterIndex = (int)(word[i] - 'a');
            letterDataTable->countPos[letterIndex][i]++;
            letterDataTable->countTotal[letterIndex]++;
        }
        wordCount++;
        wordsQueue = wordsQueue->next;
    }
    
    // Build fraction tables
    for (i = 0; i < ALPHABET_LENGTH; i ++) {
        for (j = 0; j < WORD_LENGTH; j++) {
            if (letterDataTable->countPos[i][j] == 0) {
                letterDataTable->fracPos[i][j] = 0;
            }
            else {
                letterDataTable->fracPos[i][j] = (double)(letterDataTable->countPos[i][j]) / (double)wordCount;
            }
        }
        letterDataTable->fracTotal[i] = (double)(letterDataTable->countTotal[i]) / (double)(wordCount * WORD_LENGTH);
    }

    return letterDataTable;
}

double calculate_word_score (unsigned char* word, letterTable* letterDataTable) {
    double score = 0;
    int i, letterIndex;
    for (i = 0; i < WORD_LENGTH; i++) {
        letterIndex = (int)(word[i] - 'a');
        score += letterDataTable->fracPos[letterIndex][i];
        score += letterDataTable->fracTotal[letterIndex];
    }
    return score;
}

unsigned char* calculate_best_word (queue* wordQueue) {
    // Create letter table
    letterTable *letterDataTable = build_letter_table(wordQueue);

    // Calculate word scores and get largest score
    unsigned char* bestWord;
    double bestScore = 0;
    while (wordQueue != NULL) {
        unsigned char* word = (unsigned char*)wordQueue->data;
        double score = calculate_word_score(word, letterDataTable);
        if (score > bestScore) {
            bestWord = word;
            bestScore = score;
        }
        wordQueue = wordQueue->next;
    }

    printf("Best score=%f\n", bestScore);
    free(letterDataTable);
    unsigned char* bestWordCopy = malloc(WORD_LENGTH + 1);
    memcpy(bestWordCopy, bestWord, WORD_LENGTH + 1);
    return bestWordCopy;
}
