#include <stdio.h>
#include <pthread.h>
#define MAX_THREADS 10

/**
 * Data passed to thread task
 */
typedef struct threadData{
	int position;
} t_threadData;


/**
 * Task executed by each thread
 */
void* threadTask(void *args){

	t_threadData *data;

		// Casting
		data = (t_threadData *) args;

		printf("Thread [%d] with ID %ld working...\n", data->position, pthread_self());
		sleep(3);
		printf("Thread [%d] with ID %ld Done!\n", data->position, pthread_self());

	pthread_exit(NULL);
}


/**
 * Main function
 */
int main () {

	pthread_t threadVector[MAX_THREADS];
	t_threadData threadDataVector [MAX_THREADS];
	int currentThread;

		// Create threads...
		for(currentThread=0; currentThread<MAX_THREADS; currentThread++){

			printf ("Creating thread number: %d\n",currentThread);

			// Set data
			threadDataVector[currentThread].position = currentThread;

			if (pthread_create(&threadVector[currentThread], NULL, threadTask, &threadDataVector[currentThread]) == -1){
				printf("Error creating thread number:%d\n", currentThread);
				exit(1);
			}
		}

		// Wait for threads execution (joinable)
		for(currentThread=0; currentThread<MAX_THREADS; currentThread++){
			pthread_join (threadVector[currentThread], NULL);
			exit(0);
		}
}
