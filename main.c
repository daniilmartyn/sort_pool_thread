#define _BSD_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sort.h"
#include "getWord.h"

#define A_SIZE 5

/* 
	This program implements the quicksort function using a pool of threads. A set amount of threads 
	are spawned at the start of the function and as soon as a thread is done sorting its section of 
	the array, it get assigned to a new one.
*/

char **doubleList(char **oldList, unsigned int *size){
    
    int oldsize = (*size);
    *size = oldsize*2;
    
    char** newList = malloc((oldsize*2)*sizeof(char*));
    
    for(int i = 0; i < oldsize; i++){
        
        newList[i] = oldList[i];
    }
	free(oldList);
    return newList;
}

char **dupList(char** oldList, unsigned int size){
	char** newList = malloc(size*sizeof(char*));
	for(int i = 0; i < size; i++)
		newList[i] = strdup(oldList[i]);
	return newList;
}

void printArray(char** wordList, unsigned int arraysize){
    
    for(int i = 0; i < arraysize; i++){
        if( printf("%s\n", wordList[i]) < 0) printf("error\n");
    }
}

void isSorted(char** qsorted, char** threadSorted, unsigned int arraysize){
	for(int i = 0; i < arraysize; i++){
		if(strcmp(qsorted[i],threadSorted[i]) != 0){
			printf("isSorted? NO!\n");
			return;
		}
	}
	printf("isSorted? YES!\n");
}

int mystrcmp (const void * a, const void * b){
	return strcmp(*(const char**)a, *(const char**)b);
}

char** trimArray(char** wordList, unsigned int *size){

	int count = 0;
	while(wordList[count] != NULL)
		count++;
	
	char** newList = malloc(count*sizeof(char*));
	
	for(int i = 0; i< count; i++)
		newList[i] = wordList[i];
		
	*size = count;
	free(wordList);
	return newList;
}

int main(int argc, char* argv[]){
    
    FILE *fd = NULL;
    
    if(argc == 1){
        fprintf(stderr, "Needs more args\n");
        exit(1);
    }
    
    unsigned int arraysize = A_SIZE;				// set default array size
    char **wordList = NULL;							// create and malloc array to insert words into
    wordList = (char**) malloc(arraysize * sizeof(char*));
    
    for (int i = 1; i < argc; i++) {				// go through arguments (should be files to read)
        if((fd = fopen(argv[i], "r")) == NULL){
            fprintf(stderr, "error opening file %s\n", argv[i]);
            exit(1);
        }
        
        char *nextWord;
        int j = 0;
        
        while((nextWord = getNextWord(fd)) != NULL){ // insert all words from file into array
            wordList[j++] = nextWord;
            if(j == arraysize){
                wordList = doubleList(wordList, &arraysize);
            }
        }
        
    }
    
	
	wordList = trimArray(wordList,&arraysize);			// get rid of null pointers in the array
	char** wordListTwo = dupList(wordList, arraysize);	// make a copy of the first array of words

	qsort(wordList,arraysize,sizeof(char *),mystrcmp); // sort the first copy of the wordList with qsort to have a benchmark
	
	setSortThreads(8);									// the the number of max threads to spawn
	//printf("before sorting\n");
	sortThreaded(wordListTwo,arraysize);				// sort the second copy of the wordList using threads
	
	printf("after sorting\n");
	isSorted( wordList, wordListTwo, arraysize);		// compare thread-sorted list to qsorted list. See if successful sort or not.
    
	//printArray(wordListTwo,arraysize);
	
    return 0;
}

