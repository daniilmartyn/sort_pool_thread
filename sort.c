#include "sort.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#define SORT_THRESHOLD      40

typedef struct _sortParams {
    char** array;
    int left;
    int right;
} SortParams;

typedef struct _Jobs {
	SortParams *job;
	struct _Jobs *nextJob;
} Jobs;

pthread_mutex_t mutex;
pthread_cond_t wantJob;
pthread_t *threads;
Jobs *jobsRoot;
int threadCount;

static int maximumThreads = 1;              /* maximum # of threads to be used */



static void quickSort(void* p);
/* This is an implementation of insert sort, which although it is */
/* n-squared, is faster at sorting short lists than quick sort,   */
/* due to its lack of recursive procedure call overhead.          */


void performJobs(){

	while(1){	
		//printf("performJobs whileloop\n");
		Jobs *temp = NULL;
		pthread_mutex_lock(&mutex);
		//printf("locked mutex in performJobs\n");
		threadCount--;
		if(jobsRoot == NULL){
			//printf("waiting for wantJob cond\n");
			//printf("threadCount is %d\n", threadCount);
			pthread_cond_wait(&wantJob, &mutex);
			//printf("done waiting\n");
		}
		if(jobsRoot != NULL){
			temp = jobsRoot;
			if(jobsRoot->nextJob != NULL){
				jobsRoot = jobsRoot->nextJob;
			}
			else{
				jobsRoot = NULL;
			}
			threadCount++;
		}
		//printf("mutex Unlock in performJobs\n");
		pthread_mutex_unlock(&mutex);
		
		if(temp){
			SortParams *params = temp->job;
			//free(temp);
			//printf("going into quicksort\n");
			quickSort(params);
		}
	}
}

void createPool(){
	threads = malloc(sizeof(pthread_t) * maximumThreads);
	pthread_cond_init(&wantJob,NULL);
	pthread_mutex_init(&mutex,NULL);
	
	for(int i = 0; i < maximumThreads; i++){
		if(pthread_create(&threads[i], NULL, (void *) performJobs, NULL) != 0){
			fprintf(stderr, "error creating thread %d\n", i);
			exit(1);
		}
	}
}

void queueJob(Jobs *job){
	Jobs *temp = jobsRoot;
	if(temp == NULL){
		jobsRoot = job;
	}else{
		while(temp->nextJob != NULL){
			temp = temp->nextJob;
		}
		temp->nextJob = job;
	}
	//printf("added job to queue\n");
}

static void insertSort(char** array, int left, int right) {
    int i, j;
    for (i = left + 1; i <= right; i++) {
        char* pivot = array[i];
        j = i - 1;
        while (j >= left && (strcmp(array[j],pivot) > 0)) {
            array[j + 1] = array[j];
            j--;
        }
        array[j + 1] = pivot;
    }
	//printf("should be in here\n");
}

/* Recursive quick sort, but with a provision to use */
/* insert sort when the range gets small.            */

static void quickSort(void* p) {
    SortParams* params = (SortParams*) p;
    char** array = params->array;
    int left = params->left;
    int right = params->right;
    int i = left, j = right;
	
    if (j - i > SORT_THRESHOLD) {           /* if the sort range is substantial, use quick sort */
        
        int m = (i + j) >> 1;               /* pick pivot as median of         */
        char* temp, *pivot;                 /* first, last and middle elements */
        if (strcmp(array[i],array[m]) > 0) {
            temp = array[i]; array[i] = array[m]; array[m] = temp;
        }
        if (strcmp(array[m],array[j]) > 0) {
            temp = array[m]; array[m] = array[j]; array[j] = temp;
            if (strcmp(array[i],array[m]) > 0) {
                temp = array[i]; array[i] = array[m]; array[m] = temp;
            }
        }
        pivot = array[m];
		
        for (;;) {
            while (strcmp(array[i],pivot) < 0) i++; /* move i down to first element greater than or equal to pivot */
            while (strcmp(array[j],pivot) > 0) j--; /* move j up to first element less than or equal to pivot      */
            if (i < j) {
                char* temp = array[i];      /* if i and j have not passed each other */
                array[i++] = array[j];      /* swap their respective elements and    */
                array[j--] = temp;          /* advance both i and j                  */
            } else if (i == j) {
                i++; j--;
            } else break;                   /* if i > j, this partitioning is done  */
        }
		
		pthread_mutex_lock(&mutex);
		//printf("locked mutex before job create\n");
		
		SortParams *first = (SortParams *) malloc(sizeof(SortParams));
		SortParams *second = (SortParams *) malloc(sizeof(SortParams));
				
		first->array = array; first->left = left; first->right = j;
		Jobs *job = (Jobs *) malloc(sizeof(Jobs));
		job->job = first; job->nextJob = NULL;
		queueJob(job);
		pthread_cond_signal(&wantJob);
		//printf("put job 1 on list\n");
		
		second->array = array; second->left = i; second->right = right;
		Jobs *job2 = (Jobs *) malloc(sizeof(Jobs));
		job2->job = second; job2->nextJob = NULL;
		queueJob(job2);
		pthread_cond_signal(&wantJob);
		//printf("put job 2 on list\n");
			
		pthread_mutex_unlock(&mutex);
		//printf("unlocked mutex after job create\n");
    } else insertSort(array,i,j);           /* for a small range use insert sort */
}

/* user interface routine to set the number of threads sortT is permitted to use */

void setSortThreads(int count) {
    maximumThreads = count;
	threadCount = count;
}

/* user callable sort procedure, sorts array of count strings, beginning at address array */

void sortThreaded(char** array, unsigned int count) {
    SortParams *parameters = (SortParams *) malloc(sizeof(SortParams));
    parameters->array = array; parameters->left = 0; parameters->right = count - 1;
	
	Jobs *job = (Jobs *) malloc(sizeof(Jobs));
	job->job = parameters;
	job->nextJob = NULL;
	queueJob(job);
	
	//printf("before creating pool\n");
	createPool();
	//printf("Created Pool\n");
	
    //performJobs();
	
	//printf("main thread in busy loop\n");
	while(1){ // busy loop
		if(jobsRoot == NULL && threadCount == 0){
			//printf("time to cancel\n");
			for(int i = 0; i < maximumThreads; i++)
				pthread_cancel(threads[i]);
			break;
		}
	}
}
