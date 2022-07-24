//gcc -Wall me4-tsl.c -o me4 -lpthread

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

#define NUM_THREADS  30
#define NUM_STEPS  1000

int sum  = 0 ;
int lock = 0 ;

void enter_cs (int *lock)
{
  // atomic OR (Intel macro for GCC)
  while (__sync_fetch_and_or (lock, 1)) ;   // busy waiting
  if (sum % 500 == 0)
     printf ("=Sum: %d\n", sum) ;

}

void leave_cs (int *lock)
{
  (*lock) = 0 ;
}

void *threadBody(void *id)
{
   int i ;

   for (i=0; i< NUM_STEPS; i++)
   {
      enter_cs (&lock) ;
      sum += 1 ;   // critical section
      leave_cs (&lock) ;
   }

   pthread_exit (NULL) ;
}

int main (int argc, char *argv[])
{
   pthread_t thread [NUM_THREADS] ;
   pthread_attr_t attr ;
   long i, status ;

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
