#include <pthread.h>    // "-lpthread" for compilation
#include <stdio.h>

pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
int ready = 0;

void *provider_func(void *arg)
{
    int i = *(int *)arg;
    while (i)
    {
        pthread_mutex_lock(&lock);          
        if (ready == 1)
        {
            pthread_mutex_unlock(&lock);
            continue;
        }
        ready = 1;
        printf("provided\n");
        i--;
        pthread_cond_signal(&cond);
        pthread_mutex_unlock(&lock);        
    }
}


void *consumer_func(void *arg)
{
    while (1)
    {
        pthread_mutex_lock(&lock);
        while (ready == 0)
        {
            pthread_cond_wait(&cond, &lock);
            printf("awoke\n");
        }
        ready = 0;
        printf("consumed\n\n");
        pthread_mutex_unlock(&lock);
    }

}


int main()
{
    /////////////////////////////
    // initialize ID's of threads
    pthread_t provider;
    pthread_t consumer;
    /////////////////////////////

    ////////////////////////////////
    // limit settings
    int itr;
    printf("CTRL+C FOR EXIT\n");
    printf("How many iterations: ");
    scanf("%d", &itr);
    printf("\n");
    ////////////////////////////////

    //////////////////////////////////////////////////////////
    // creating threads with ID's provider and consumer  
    // with default attributes
    // provider_func and consumer_func - entry points
    // give provider limits
    pthread_create(&provider,NULL,provider_func,(void *)&itr);
    pthread_create(&consumer,NULL,consumer_func,NULL);
    //////////////////////////////////////////////////////////

    pthread_exit(NULL);     // safe main exit
}