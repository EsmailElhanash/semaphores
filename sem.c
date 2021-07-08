#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <pthread.h>
#include <unistd.h>

#define bufferSize 1000
#define N 3 //number of threads
#define counterUpper 8000000 //8 second
#define counterLower 1000000 // 1 second

#define monitorUpper 15000000 //15 Second
#define monitorLower 8000000 //8 Seconds


#define collectorUpper 50000000 //50 second
#define collectorLower 30000000 //30 second

sem_t semC,semB;//mcounter semaphore,buffer semaphore
int mcounter=0;
int buffer[bufferSize];
int bufferLength=0;

void addMessage();
int readMessagesNumber();
void writeToBuffer(int num);
void readBuffer();
void* counterThread(void *vargp);
void* monitorThread(void *vargp);
void* collectorThread(void *vargp);
void startThreads();
unsigned long getRandomCounterSleepTime();
unsigned long getRandomMonitorSleepTime();
unsigned long getRandomCollectorSleepTime();
void printBuffer();


int main(){
    sem_init(&semC,0,1);
    sem_init(&semB,0,1);
    startThreads();
    //usleep(60000000);
    return 0;
}

void addMessage(){
    printf("Counter Thread %ld:Waiting to write to mcounter\n",pthread_self());
    sem_wait(&semC);
    printf("Counter Thread %ld:Writing..\n",pthread_self());
    mcounter++;
    printf("Counter Thread %ld:MessagesCounter=%d\n",pthread_self(),mcounter);
    sem_post(&semC);
}

int readMessagesNumber(){
    int num;
    printf("Monitor Thread %ld:Waiting to read mcounter\n",pthread_self());
    sem_wait(&semC);
    printf("Monitor Thread %ld:Reading..\n",pthread_self());
    if (mcounter>0){
        num = mcounter;
        printf("Monitor Thread %ld:mcounter=%d\n",pthread_self(),num);
        mcounter=0;
        printf("Monitor Thread %ld:mcounter set to %d\n",pthread_self(),mcounter);
    }else
    {
        printf("Monitor Thread %ld:No new Messages!\n",pthread_self());
    }
    
    sem_post(&semC);
    return num;
}

void writeToBuffer(int num){
    printf("Monitor Thread %ld:Waiting to write to buffer\n",pthread_self());
    sem_wait(&semB);
    if (bufferLength<bufferSize){
        printf("Monitor Thread %ld:Writing..\n",pthread_self());
            buffer[bufferLength++] = num;
            printf("Monitor Thread %ld:Buffer=",pthread_self());
            printBuffer();
            printf("\n");
    }else{
        printf("Monitor Thread %ld:Buffer is full\n",pthread_self());
    }
    sem_post(&semB);
}

void readBuffer(){
    printf("Collector Thread %ld: waiting to read buffer\n",pthread_self());
    sem_wait(&semB);
    if (bufferLength>0){
        printf("Collector Thread %ld: reading buffer\n",pthread_self());
        int n = bufferLength;
        for (int i = 0;i<n;i++){
            printf("Collector Thread %ld: Position%d=%d\n",pthread_self(),i,buffer[i]);
            bufferLength--;
        }
    }else
    {
        printf("Collector Thread %ld: buffer empty\n",pthread_self());
    }
    
    sem_post(&semB);
}

void* counterThread(void *vargp){
    while (1){
        printf("\nCounter Thread %ld: recieved a message\n",pthread_self());
        addMessage();
        usleep(getRandomCounterSleepTime());
    }
    
}

void* monitorThread(void *vargp){
    while (1){
        printf("\nMonitor Thread %ld started\n",pthread_self());
        int mNum = readMessagesNumber();
        if (mNum>0)
            writeToBuffer(mNum);
        usleep(getRandomMonitorSleepTime());
    }
    
}

void* collectorThread(void *vargp){
    while (1){
        printf("\nCollector Thread %ld started\n",pthread_self());
        readBuffer();
        usleep(getRandomCollectorSleepTime());
    }   
}

void startThreads(){
    int i;
    pthread_t cthread[N];
    for (i=0;i<N;i++){
        pthread_create(&cthread[i],NULL,counterThread,NULL);
    }
    pthread_t monThread;
    pthread_create(&monThread,NULL,monitorThread,NULL);


    pthread_t colThread;
    pthread_create(&colThread,NULL,collectorThread,NULL);
    pthread_join(colThread,NULL);
}

unsigned long getRandomCounterSleepTime(){
    return (rand() % (counterUpper - counterLower + 1)) + counterLower; ;
}

unsigned long getRandomMonitorSleepTime(){
    return (rand() % (monitorUpper - monitorLower + 1)) + monitorLower; ;
}

unsigned long getRandomCollectorSleepTime(){
    return (rand() % (monitorUpper - monitorLower + 1)) + monitorLower; ;
}

void printBuffer(){
    for (int i = 0; i < bufferLength; i++)
        printf("%d ", buffer[i]);
}