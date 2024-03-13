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
#include <asm/signal.h>

int WAKEUP = SIGUSR1;
sigset_t sigSet;

pid_t otherPid;
pthread_mutex_t mutex;

// Shared Circular Buffer. Each element holds an int 
struct CIRCULAR_BUFFER
{
    int count; // Number of items in the buffer
    int lower; // Next slot to read in the buffer
    int upper; // Next slot to write in the buffer
    int buffer[100]; 
};
struct CIRCULAR_BUFFER *buffer = NULL;

// Sleep function
void sleepUntilWoken()
{
    int nSig;

    //put to sleep until notified to wake up
    printf("sleeping...\n");
    sigwait(&sigSet, &nSig);
    printf("Awoken\n");
}

// Responsible for creating numbers and passing them to 
// the Consumer via a shared circular buffer
void producer(int args[])
{
    // Add all elements to the buffer
    for(int i = 0; i < sizeof(args); i++)
    {
        // Go to sleep if the buffer is full
        if (buffer->count == 100) sleepUntilWoken();

        set(args[i]);

        // Signal the consumer
        printf("Waking Consumer...\n");
        kill(otherPid, WAKEUP); 
    }
}

// Consumer will always be behind the producer
// and will not have to wait for the producer
// got this idea from: stackoverflow.com/questions/11656532/returning-an-array-using-c
// return an array
char *consumer(int size)
{
    char *ret = malloc(size);
    for(int i = 0; i < size; i++)
    {
        // Set up a Signal set
        sigemptyset(&sigSet);
        sigaddset(&sigSet, WAKEUP);
        sigprocmask(SIG_BLOCK, &sigSet, NULL);
        // Go to sleep if there is no data in buffer
        if (buffer->count == 0) sleepUntilWoken();

        // Get the next int element in the buffer
        // and place it into the ret array at element location i
        ret[i] = get();

        // Signal the producer
        printf("Waking Producer...\n");
        kill(otherPid, WAKEUP);
    }
    return ret;
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
    // If no arguments passed, end program with message
    if(argc <= 1)
    {
        printf("Try again. Must pass arguments");
        return -1;
    }

    int (*result)[argc-1];

    pid_t pid;

    // Initialize mutex
    pthread_mutex_init(&mutex, 0);

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

        // send array of arguments to the producer
        producer(argv);
    }
    else
    {
        // Run Consumer process logic at Parent process
        // result pointer points to array that consumer returns
        otherPid = pid;
        result = consumer(argc);

        // print the result
        for (int i = 0; i < sizeof(result); i++)
        {
            printf("%d ", result[i]);
        }
    }

    // TODO Print result
    for(int i = 0; i < sizeof(result); i++)
    {
        printf("%s ", result[i]);
    }

    pthread_mutex_destroy(&mutex);
    pthread_exit(NULL);
    return 0;
}