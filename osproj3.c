//*********************************************************
//
// Rahul Bethi
// COSC 5331, Foundations of Computer System Software (Operating Systems) - Fall 2015
// Programming Project #3: Process Synchronization using PThreads: The Producer / Consumer Problem With Prime Number Detector
// November 18, 2015
// Instructor: Dr. Ajay Katangur
//
//*********************************************************
#include <pthread.h>
#include <stdbool.h>
#include "buffer.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <semaphore.h>
#include <time.h>
#define MAX_THREADS 100

//Initialize global variables
int mSleepTm, pcSleepTm, nProducers, nConsumers, elaborate, bCount = 0, run = 1, tProduced[ MAX_THREADS ], tConsumed[ MAX_THREADS ], tP = 0, tC = 0, tFull = 0, tEmpty = 0 ;
unsigned int seed;

buffer_item buffer[ BUFFER_SIZE ];

pthread_t		ptid[ MAX_THREADS ], ctid[ MAX_THREADS ];
pthread_attr_t	attr;
pthread_mutex_t mutex;
sem_t			full, empty;

//Initializing functions
int  initLocks();
int  generate(unsigned int *, int, int );
int  displayBuffer();
int *producer_fn( void * );
int *consumer_fn( void * );

//main function
int main( int argc, char *argv[] )
{
	int retVal, i;
	if(argv[1] == NULL || argv[2] == NULL || argv[3] == NULL || argv[4] == NULL || argv[5] == NULL)
	{
		printf( "Incomplete input!!\n" );
		return( retVal );
	}
	//Store input into appropriate variables
	mSleepTm   = atoi( argv[1] )*1000; //converting milli to micro seconds
	pcSleepTm  = atoi( argv[2] )*1000;
	nProducers = atoi( argv[3] );
	nConsumers = atoi( argv[4] );
	
	if	   ( *argv[5] == 'y' && *(argv[5]+1) == 'e' && *(argv[5]+2) == 's' && *(argv[5]+3) == NULL )	{	elaborate = 1;	}
	else if( *argv[5] == 'n' && *(argv[5]+1) == 'o'	&&						  *(argv[5]+2) == NULL )	{	elaborate = 0;	}
	else
	{
		printf( "Last entry Invalid!! %s\n", argv[5] );
		return( retVal );
	}
	//Initiate mutex semaphore and buffer
	initLocks();
	//Create producer and COnsumer threads
	pthread_attr_init( &attr );
	if(elaborate == 1) { printf("\nStarting Threads...\n\n"); displayBuffer(); }
	for(i=0; i<nProducers; i++)	{	pthread_create( &ptid[i], &attr, producer_fn, (void *)i );	}
	
	for(i=0; i<nConsumers; i++)	{	pthread_create( &ctid[i], &attr, consumer_fn, (void *)i );	}
	//Sleep
	usleep( mSleepTm );
	//stop the threads from recurring and let them end their program
	run = 0;
	//join the threads
	for(i=0; i<nProducers; i++)	{	pthread_join( ptid[i], NULL );	}
	for(i=0; i<nConsumers; i++)	{	pthread_join( ctid[i], NULL );	}
	//Display statistics
	printf( "PRODUCER / CONSUMER SIMULATION COMPLETE\n" );
	printf( "=======================================\n" );
	printf( "Simulation Time:                      %d milli seconds\n", (mSleepTm/1000) );
	printf( "Maximum Thread Sleep Time             %d milli seconds\n", (pcSleepTm/1000) );
	printf( "Number of Producer Threads:           %d\n", nProducers );
	printf( "Number of Consumer Threads:           %d\n", nConsumers );
	printf( "Size of Buffer:                       %d\n", BUFFER_SIZE );
	printf( "\nTotal Number of Items Produced      %d\n", tP );
	for(i=0; i<nProducers; i++)
		printf("  Thread %d:                           %d\n", i, tProduced[i] );
	printf( "\nTotal Number of Items Consumed      %d\n", tC );
	for(i=0; i<nConsumers; i++)
		printf("  Thread %d:                           %d\n", i, tConsumed[i] );
	printf("\nNumber of Items Remaining in Buffer: %d\n", bCount);
	printf("Number of Times Buffer Was Full:       %d\n", tFull);
	printf("Number of Times Buffer Was Empty:      %d\n", tEmpty);
	
	return( retVal );
}

int initLocks()
{
	int i=0;
	
	pthread_mutex_init( &mutex, NULL );
	sem_init( &full,  0, BUFFER_SIZE );
	sem_init( &empty, 0, BUFFER_SIZE );
	tFull = 0;
	tEmpty = 0;
	tP = 0;
	tC = 0;
	for( i=0; i<BUFFER_SIZE; i++)
	{
		buffer[i] = -1;
	}
}
//function to insert item in buffer
bool buffer_insert_item( buffer_item  item )
{
	if( bCount < BUFFER_SIZE )
	{
		buffer[ bCount ] = item;
		bCount++;
		return true;
	}
	else	{	return false;	}
}
//function to remove item from buffer
bool buffer_remove_item( buffer_item *item )
{
	int i = 0;
	if( bCount > 0 )
	{
	   *item = buffer[ 0 ];
		bCount--;
		for( i=0; i<bCount; i++)
		{
			buffer[i] = buffer[i+1];
		}
		buffer[i] = -1;
		
		return true;
	}
	else	{	return false;	}
}
//generate random number
int generate(unsigned int *seed, int min, int max)
{
    int value = rand_r(seed);
    int length = max - min + 1;
    return min + (value % length);
}
//function to display buffer only when yes is given in the last variable of input
int displayBuffer()
{
	int i=0, j=-1;
	
	printf( "(buffers occupied: %d)\nbuffers:  ", bCount );
	for( i=0; i<BUFFER_SIZE; i++)
	{
		printf("%d  ", buffer[i]);
	}
	printf("\n\n");
	return 0;
}
//producer threads function
int *producer_fn( void *ii )
{
	int t = (int)ii, randTm, randNum;
	buffer_item item;
	
	tProduced[t] = 0;
	//loop runs until main function sleeps
	while(run)
	{
		//sleep for random time in maxtime
		randTm = generate(&seed, 0, pcSleepTm );
		usleep(randTm);
		//generate random number
		item = generate(&seed, 0, 100 );
		
		sem_wait(&empty);
		pthread_mutex_lock(&mutex);
		
		if( buffer_insert_item(item) == false )
		{
			tFull++;
			if(elaborate == 1) { printf( "Producer %d : Buffer Full\n", t );	}
		}
		else
		{
			tProduced[t]++;
			tP++;
			if(elaborate == 1) { printf("Producer %d : produced %d\n", t, item); }
		}
		if(elaborate == 1) { displayBuffer(); }
		
		pthread_mutex_unlock(&mutex);
		sem_post(&full);
	}
	pthread_exit(0);
}
//function for consumer threads
int *consumer_fn( void *ii )
{
	int t = (int)ii, randTm, i, flag = 0, n;
	buffer_item item;
	
	tConsumed[t] = 0;
	//loop runs until main function sleeps
	while(run)
	{
		//sleep for randim time in maxtime
		randTm = generate(&seed, 0, pcSleepTm );
		usleep(randTm);
		
		sem_wait(&full);
		pthread_mutex_lock(&mutex);
		//retrieve item
		if( buffer_remove_item(&item) == false)
		{
			tEmpty++;
			if(elaborate == 1) { printf("Consumer %d : Buffer Empty\n", t); }
		}
		else
		{
			tConsumed[t]++; tC++;
			if(elaborate == 1)
			{
				//determining prime number
				n = item;
				flag = 0;
				for( i=2; i<=n/2; i++)
				{
					if( n%i == 0)
					{
						flag = 1;
						i = n;
					}
				}
				if(flag == 0)
					printf("Consumer %d : consumed %d   * * * PRIME * * *\n", t, n);
				else
					printf("Consumer %d : consumed %d   * * * NOT PRIME * * *\n", t, n);
			}
		}
		if(elaborate == 1) { displayBuffer(); }
		pthread_mutex_unlock(&mutex);
		sem_post(&empty);
	}
	pthread_exit(0);
}