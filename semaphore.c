//gcc -Wall semaphore.c -lpthread

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>

#define NUM_THREADS  100
#define NUM_STEPS 100000

int sum = 0 ;
sem_t s ;

void *threadBody(void *id)
{
   int i ;

   for (i=0; i< NUM_STEPS; i++)
   {
      sem_wait (&s) ;
      sum += 1 ;   // critical section
      sem_post (&s) ;
   }

   pthread_exit (NULL) ;
}

int main (int argc, char *argv[])
{
   pthread_t thread [NUM_THREADS] ;
   pthread_attr_t attr ;
   long i, status ;

   sem_init (&s, 0, 1) ;

   pthread_attr_init (&attr) ;
   pthread_attr_setdetachstate (&attr, PTHREAD_CREATE_JOINABLE) ;

   // create threads
   for(i=0; i<NUM_THREADS; i++)
   {
      status = pthread_create (&thread[i], &attr, threadBody, (void *) i) ;
      if (status)
      {
         perror ("pthread_create") ;
         exit (1) ;
      }
   }
   
   // wait all threads to finish   
   for (i=0; i<NUM_THREADS; i++)
   {
      status = pthread_join (thread[i], NULL) ;
      if (status)
      {
         perror ("pthread_join") ;
         exit (1) ;
      }
   }

   printf ("Sum should be %d and is %d\n", NUM_THREADS*NUM_STEPS, sum) ;

   pthread_attr_destroy (&attr) ;
   pthread_exit (NULL) ;
}
