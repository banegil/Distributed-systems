#include <stdio.h>
#include <pthread.h>

// Size of buffer
#define DATA_BUFFER 128

// Maximum data to produce
#define MAX_DATA_TO_PRODUCE 1000

// Mutex
pthread_mutex_t mutex;

// Condition variables
pthread_cond_t bufferFull;
pthread_cond_t bufferEmpty;

// Number of element currently produced
int producedElements;

// Buffer where produced data is placed
int buffer[DATA_BUFFER];


/**
 * Task executed by the producer thread
 */
void *threadProducer(void *args){

	int producedData;
	int i, index;

		// Init...
		index = 0;

		// For each data to be produced
		for(i=0; i<MAX_DATA_TO_PRODUCE; i++ ){

			// Current data to be produced
			producedData = i;

			// Enter the critical section
			pthread_mutex_lock(&mutex);

				// Block if buffer is full
				while (producedElements == DATA_BUFFER)
					pthread_cond_wait(&bufferFull, &mutex);

				// Produce current data
				buffer[index] = i;

				// Update index
				index = (index + 1) % DATA_BUFFER;

				// Increase number of current produced elements
				producedElements += 1;

				// Signal
				if (producedElements == 1)
					pthread_cond_signal(&bufferEmpty);

				printf ("Produce data: %d\n", i);

			// End of critical section
			pthread_mutex_unlock(&mutex);
		}

	pthread_exit(0);
}

/**
 * Task executed by the consumer thread
 */
void *threadConsumer(void *args){

	int consumedData;
	int i, index;

		// Init...
		index = 0;

		// For each data to be consumed
		for(i=0; i<MAX_DATA_TO_PRODUCE; i++ ){

			// Enter the critical section
			pthread_mutex_lock(&mutex);

				// While the buffer is empty... block!
				while (producedElements == 0)
					pthread_cond_wait(&bufferEmpty, &mutex);

				// Consume current data
				consumedData = buffer[index];

				// Update index
				index = (index + 1) % DATA_BUFFER;

				// Update number of current produced elements
				producedElements = producedElements - 1 ;

				// There is a gap to produce data... signal!
				if (producedElements == (DATA_BUFFER - 1))
					pthread_cond_signal(&bufferFull);

				printf ("Consume data: %d\n", i);

			// Exit critical section
			pthread_mutex_unlock(&mutex);
		}

	pthread_exit(0);
}


/**
 * Main function
 */
int main () {

	pthread_t threadProducerID;
	pthread_t threadConsumerID;

		// Init...
		producedElements = 0;

		// Init mutex and condition variables
		pthread_mutex_init(&mutex, NULL);
		pthread_cond_init(&bufferFull, NULL);
		pthread_cond_init(&bufferEmpty, NULL);

		// Create producer and consumer threads
		pthread_create(&threadProducerID, NULL, threadProducer, NULL);
		pthread_create(&threadConsumerID, NULL, threadConsumer, NULL);

		// Join!
		pthread_join(threadProducerID, NULL);
		pthread_join(threadConsumerID, NULL);

		// Free mutex and condition variables
		pthread_mutex_destroy(&mutex);
		pthread_cond_destroy(&bufferFull);
		pthread_cond_destroy(&bufferEmpty);

	return 0;
}
