#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <stdbool.h>

#define WORD_LENGTH 5
#define ALPHABET_LENGTH 26

/* ========================================================
   Data Types and Structs
=========================================================*/

typedef struct queue {
    void* data;
    struct queue* next;
} queue;

typedef struct letter {
   unsigned char letter;
   bool inWord;
   bool isCorrect;
   int position;
   bool triedPositions[WORD_LENGTH];
} letter;

typedef struct letterTable {
   int countPos[ALPHABET_LENGTH][WORD_LENGTH];
   int countTotal[ALPHABET_LENGTH];
   double fracPos[ALPHABET_LENGTH][WORD_LENGTH];
   double fracTotal[ALPHABET_LENGTH];
} letterTable;

/* ========================================================
   Function Prototypes
=========================================================*/

// Reads in a file at path and return a pointer to a char array of the entire file
unsigned char* read_file (const char* path, int* fileLength, int* statusCode);

// Reads in a file of words and returns a queue of the words
queue* read_file_into_queue (const char* path, queue** queueLast, int* statusCode);

// Write to file
int write_file (const char* path, const unsigned char* data, int length);

// Write queue to file
int write_file_queue (const char* path, queue* first, int wordLength);

// Inserts data into queue
queue* queuePush (queue* last, void* data);

// Removes and returns data from queue
void* queuePop (queue** first);

// Destroys queue
int queueDestroy (queue* first);

// Reads in console input
int read_console (unsigned char *buf, int n);

// Filter word, returns whether word should be included or not
bool filter_word (unsigned char* word, queue* filter);

// Initializes and builds the letter table
letterTable* build_letter_table (queue* wordsQueue);

// Calculates the score of any given word based on the letter table
double calculate_word_score (unsigned char* word, letterTable* letterDataTable);

// Calculates the best word for any word list
unsigned char* calculate_best_word (queue* wordQueue);
