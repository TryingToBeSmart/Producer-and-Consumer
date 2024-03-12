#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <errno.h>
#include <pthread.h>
#include <fcntl.h>
#include <semaphore.h>
#include <sys/mman.h>

pid_t pid;
pid_t otherPid;
sem_t mutex;

// Shared Circular Buffer
struct CIRCULAR_BUFFER
{
    int count; // Number of items in the buffer
    int lower; // Next slot to read in the buffer
    int upper; // Next slot to write in the buffer
    int buffer[100]; 
};
struct CIRCULAR_BUFFER *buffer = NULL;


void producer()
{
    
}

void main()
{
    // Initialize semaphore
    sem_init(&mutex, 0, 1);

    // Create shared memory for the Circular Buffer to be shared between
    // the Parent and Child Processes
    buffer = (struct CIRCULAR_BUFFER*)mmap(0, sizeof(struct CIRCULAR_BUFFER), 
        PROT_READ|PROT_WRITE,MAP_SHARED|MAP_ANONYMOUS, -1, 0);
    buffer->count = 0;
    buffer->lower = 0;
    buffer->upper = 0;

    // Use fork()
    pid = fork();
    if (pid == -1)
    {
        // Error: if fork() returns -1, then error
        printf("Error in fork()", errno);
        exit(EXIT_FAILURE);
    }

    // OK if fork() returns 0
    if (pid == 0)
    {
        // Run Producer process logic as a child process
        otherPid = getppid();
        producer();
    }
    else
    {
        // Run Consumer process logic at Parent process
        otherPid = pid;
        consumer();
    }
}