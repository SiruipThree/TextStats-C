#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include <ctype.h>
#include <limits.h> // For PATH_MAX

#define BUFFER_SIZE 4096 //common size for buffer 
#define WORD_SIZE 128 //common size of word 
#define OUTPUT_SIZE 256

typedef struct {
    char word[WORD_SIZE];
    int count;
} WordEntry;

WordEntry wordTable[10000];  // A fixed-size table for storing words
int wordCount = 0;

//this function is used to add a word or update its count in the word table
void addWordToTable(const char *word) { //using constant inorder to protect the completely of input data
    for (int i = 0; i < wordCount; i++) {
        if (strcmp(wordTable[i].word, word) == 0) {
            wordTable[i].count++;
            return;
        }
    }
    strcpy(wordTable[wordCount].word, word);
    wordTable[wordCount].count = 1;
    wordCount++;
}


// This is a function in order to figure out "-", it must be 
//s1:such-that -> such-that
//s2:-that -> that
//s3:that- -> that 

void processFile(const char *filename) {
    int fd = open(filename, O_RDONLY);
    if (fd == -1) {
        printf("Error: cannot open the file: %s\n", filename);
        return;
    }

    unsigned char buffer[BUFFER_SIZE];
    char word[WORD_SIZE];
    int word_len = 0;
    int bytes_read;
    int state = 0;

    while ((bytes_read = read(fd, buffer, BUFFER_SIZE)) > 0) {
        for (int i = 0; i < bytes_read; i++) {
            char currChar = buffer[i];
            switch(state){
                case 0: // not in any word
                    if(isalpha(currChar)){ //we are not in a word, but we meet a letter
                        word_len = 1; 
                        word[0] = currChar;
                        state = 3;
                    }
                    else if( currChar == '\''){
                        word_len = 1; 
                        word[0] = currChar;
                        state = 2;
                    }
                    //we dont need else since if we are not in any word 
                    break; 
                
                case 1: //we are in a word, and we have read '-'
                    if (isalpha(currChar)){ //if the next char is a letter  --> since we are in case 1, which means we already have a letter before the currChar, otherwise we wont enter to case1
                        word[word_len++] = currChar;
                        state = 2;
                    }
                    else{// - is not part of the word
                        word[-- word_len] = '\0';
                        addWordToTable(word); //so if we have word-something we need to decide what something is, if it is word-'apple 
                        if(currChar == '\''){
                            word_len = 1;
                            word[0] = currChar;
                            state = 2;
                        }
                        else{
                            state = 0; 
                        }
                        }
                break;  

                case 2: // we are in a word, ending with '
                    if(isalpha(currChar)){
                        word[word_len ++] = currChar;
                        state = 3;
                    } // since'good we are adding good in word array
                    else if(currChar == '\''){ //since''
                        word[word_len++] = '\'';
                    }
                    else{
                        word[word_len] = '\0';
                        addWordToTable(word);
                        state = 0;
                        printf("Temp\n");
                    }
                    break;

                case 3://we are in a word, ending with a letter eg: apple ending with e
                    if(isalpha(currChar)){
                        word[word_len++] = currChar; //we are still in a word ending with a letter,so we dont need to change state
                    }
                    else if(currChar == '-'){
                        word[word_len++] = currChar;
                        state = 1;
                    }
                    else if(currChar == '\''){
                        word[word_len++] = '\'';
                        state  = 2;
                    }
                    else{
                        word[word_len] = '\0';
                        addWordToTable(word);
                        state = 0;
                    }
                    break;
            }
        }
    }
    if (state == 1){
        word_len--;
    }
    if(state != 0){
        word[word_len] = '\0';
        addWordToTable(word);
    }
    close(fd);
}

//this is a function to process directories, recursively and read files.
void processDirectory(const char *directName) {
    DIR *dir = opendir(directName);
    if (dir == NULL) {
        printf("Error: cannot open the directory: %s\n", directName);
        return;
    }

    struct dirent *entry;
    struct stat entry_stat;
    char path[PATH_MAX];

    while ((entry = readdir(dir)) != NULL) {
        //this function is used to check if files or directories'names starting with '.'
        if (entry->d_name[0] == '.') {
            printf("Hidden file or directory is ignored: %s/%s\n", directName, entry->d_name);
            continue;
        }

        snprintf(path, PATH_MAX, "%s/%s", directName, entry->d_name);
        
        if (stat(path, &entry_stat) == -1) {
            printf("Error getting file stats: %s\n", path);
            continue;
        }        
        if (S_ISDIR(entry_stat.st_mode)) {
            processDirectory(path);  
        } 
        // Process regular `.txt` files only
        else if (S_ISREG(entry_stat.st_mode) && strstr(entry->d_name, ".txt")) {
            processFile(path);
        } else {
            printf("Ignore the non-.txt file: %s/%s\n", directName, entry->d_name);
        }
    }

    closedir(dir);
}

// Sort the word entries by count and lexicographically
void sortWords() {
    for (int i = 0; i < wordCount - 1; i++) {
        for (int j = 0; j < wordCount - i - 1; j++) {
            if (wordTable[j].count < wordTable[j + 1].count || 
                (wordTable[j].count == wordTable[j + 1].count && strcmp(wordTable[j].word, wordTable[j + 1].word) > 0)) {
                WordEntry temp = wordTable[j];
                wordTable[j] = wordTable[j + 1];
                wordTable[j + 1] = temp;
            }
        }
    }
}

//Output the sorted words and their counts
void outputResults() {
    for (int i = 0; i < wordCount; i++) {
        printf("%s %d\n", wordTable[i].word, wordTable[i].count);
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: ./words <file|directory>\n");
        return 1;
    }
    
    for (int i = 1; i < argc; i++) {
        // Check if the file or directory name starts with a period (hidden)
        if (argv[i][0] == '.') {
            printf("Hidden file or directory is ignored: %s\n", argv[i]);
            continue;
        }
        struct stat path_stat;
        if (stat(argv[i], &path_stat) == -1) {
            printf("Error with path of the file: %s\n", argv[i]);
            continue;
        }

        if (S_ISDIR(path_stat.st_mode)) {
            processDirectory(argv[i]);
        } else if (S_ISREG(path_stat.st_mode)) {
            if (strstr(argv[i], ".txt")) {
                processFile(argv[i]);
            } else {
                printf("the non-.txt file is ignored: %s\n", argv[i]); 
            }
        }
    }

    sortWords();
    outputResults();

    return 0;
}
