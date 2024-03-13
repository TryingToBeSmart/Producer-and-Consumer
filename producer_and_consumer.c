#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <errno.h>
#include <pthread.h>
#include <fcntl.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <signal.h>

int WAKEUP = SIGUSR1;

pid_t otherPid;
sem_t mutex;

// Shared Circular Buffer. Each element holds an int 
struct CIRCULAR_BUFFER
{
    int count; // Number of items in the buffer
    int lower; // Next slot to read in the buffer
    int upper; // Next slot to write in the buffer
    int buffer[100]; 
};
struct CIRCULAR_BUFFER *buffer = NULL;

// Responsible for creating numbers and passing them to 
// the Consumer via a shared circular buffer
void producer()
{
    // Go to sleep if the buffer is full
    if (buffer->count == 100) pause();

    kill(otherPid, WAKEUP);
    // Signal the consumer
}

// Consumer will always be behind the producer
// and will not have to wait for the producer
void consumer()
{
    // Go to sleep if there is no data in buffer
    if (buffer->count == 0) pause();


    // Signal the producer
}

// put function to write to the buffer
void put(int number) 
{
    if(buffer->count < 100)
    {
        buffer->buffer[buffer->upper] = number; // assign the number to the buffer's next upper array location
        ++buffer->upper; // increment the upper(write) location slot
        ++buffer->count; // increment the buffer count
    }
    else printf("Buffer is full, did not add");
}

// get function to read from the buffer
int get()
{
    if(buffer->count > 0)
    {
        // return the number that is in the buffer's lower slot location
        int returnNumber = buffer->buffer[buffer->lower]; 
        --buffer->lower; // decrement the lower(read) location
        --buffer->count; // decrement the buffer count
        printf("Returning: %d from location: %d", returnNumber, buffer->lower);
        return returnNumber;
    }
    else printf("Buffer is empty, nothing to get");
    return -1;
}



int main(int argc, char* argv[])
{
    pid_t pid;
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