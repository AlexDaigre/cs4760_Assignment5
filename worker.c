#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/msg.h> 

void closeProgramSignal(int sig);
void closeProgram();

void setupSharedClock();
#define CLOCKVAR 0
#define SHMNAME "/tmp/daigreTmp43648"

void setupMsgQueue();
#define QUEUEVAR 0
#define QUEUENAME "/tmp/daigreTmp48083"
int msgQueueId;

int clockShmId;
int* clockShmPtr;

struct mesg_buffer { 
    long mtype; 
    char mtext[10]; 
} message; 

int main (int argc, char *argv[]) {
    signal(SIGINT, closeProgramSignal);

    char* maxResourcesString = argv[1];
    char* stringElement = strtok(maxResourcesString, "/");
    char* maxResources[20];

    int i = 0;
    while (stringElement != NULL){
        maxResources[i++] = stringElement;
        stringElement = strtok (NULL, "/");
    }

    setupSharedClock();
    setupMsgQueue();

    // printf("time: %d:%d\n", clockShmPtr[0], clockShmPtr[1]);

    // printf("{");
    // for (i = 0; i < 20; ++i) {
    //     printf("%s,", maxResources[i]);
    // }
    // printf("}\n");

    message.mtype = 1;
    sprintf(message.mtext, "%d", getpid());
    // int j = 0;
    // for(j=0; j<20; j++){
    //     newResourceRequest.mtext[j] = 3;
    // }
    // newResourceRequest.mtext[20] = getpid();
    // newResourceRequest.mtext[0] = "a";
    
    printf("Child %d: sending message to parent\n", getpid());
    int msgSent = msgsnd(msgQueueId, &message, sizeof(message), 0);
    if (msgSent < 0){
        printf("Child %d: failed to send message.\n", getpid());
        printf("Error: %d\n", errno);
        // closeProgram();
    }
    printf("Child %d: SENT message to parent\n", getpid());

    msgrcv(msgQueueId, &message, sizeof(message), getpid(), 0);
    printf("Child %d: recived message from parent\n", getpid());

    closeProgram();
}

void closeProgramSignal(int sig){
    closeProgram();
}

void closeProgram(){
    shmctl(clockShmId, IPC_RMID, NULL);
    // shmdt(clockShmPtr);
    printf("Child %d Exiting gracefully.\n", getpid());
    exit(0);
}

void setupSharedClock(){
    key_t sharedClockKey;
    if (-1 != open(SHMNAME, O_CREAT, 0777)) {
        sharedClockKey = ftok(SHMNAME, CLOCKVAR);
    } else {
        printf("ftok error in child: setupSharedClock\n");
        printf("Error: %d\n", errno);
        exit(1);
    }

    clockShmId = shmget(sharedClockKey, sizeof(int)*2, IPC_CREAT | 0666);
    if (clockShmId < 0) {
        printf("shmget error in child: setupSharedClock\n");
        printf("Error: %d\n", errno);
        exit(1);
    }

    clockShmPtr = (int *) shmat(clockShmId, NULL, 0);
    if ((long) clockShmPtr == -1) {
        printf("shmat error in child: setupSharedClock\n");
        printf("Error: %d\n", errno);
        shmctl(clockShmId, IPC_RMID, NULL);
        exit(1);
    }
}

void setupMsgQueue(){
    key_t msgQueueKey;
    if (-1 != open(QUEUENAME, O_CREAT, 0777)) {
        msgQueueKey = ftok(SHMNAME, QUEUEVAR);
    } else {
        printf("ftok error in parrent: setupMsgQueue\n");
        printf("Error: %d\n", errno);
        exit(1);
    }

    msgQueueId = msgget(msgQueueKey, 0777 |IPC_CREAT);
    if (msgQueueId < 0) {
        printf("msgget error in parrent: setupMsgQueue\n");
        printf("Error: %d\n", errno);
        exit(1);
    }
}