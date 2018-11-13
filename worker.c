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

void closeProgramSignal(int sig);
void closeProgram();

int* setupSharedClock();

#define CLOCKVAR 0
#define SHMNAME "/tmp/daigreTmp43648"

int clockShmId;
int* clockShmPtr;

int main (int argc, char *argv[]) {
    signal(SIGINT, closeProgramSignal);

    char* maxResourcesString = argv[1];
    int i = 0;
    char* stringElement = strtok(maxResourcesString, "/");
    char* maxResources[20];

    while (stringElement != NULL){
        maxResources[i++] = stringElement;
        stringElement = strtok (NULL, "/");
    }

    setupSharedClock();

    printf("time: %d:%d\n", clockShmPtr[0], clockShmPtr[1]);

    printf("{");
    for (i = 0; i < 20; ++i) {
        printf("%s,", maxResources[i]);
    }
    printf("}\n");

    // for(;;){}
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

int* setupSharedClock(){
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

    return clockShmPtr;
}