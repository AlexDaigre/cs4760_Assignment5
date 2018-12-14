#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>
#include <string.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ipc.h> 
#include <sys/msg.h>

struct mesg_buffer { 
    long mtype; 
    char mtext[100]; 
} message; 

void childClosedSignal(int sig);
int closeChild();
void closeProgramSignal(int sig);
void closeProgram();

void reciveMessages();
int checkGrant(int processLocation, int requestedResources[]);

void setupOutputFile();

void createProcesses();
int createNextProcessAt = 1;
void advanceTime();

void setupSharedClock();
#define CLOCKVAR 0
#define SHMNAME "/tmp/daigreTmp11556688"


void setupMsgQueue();
#define QUEUEVAR 0
#define QUEUENAME "/tmp/daigreTmp11556677"
int msgQueueId;

int clockShmId;
int* clockShmPtr;
// int clockShmPtr[2];
FILE* outputFile;

int currentProcesses;
#define maxProcesses 18
#define numberOfResources 20

pid_t openProcesses[18] = {0};
pid_t blockedProcesses[18] = {0};

void intilizeResourceTables();
void print2DTable(int table[18][numberOfResources], char* title);
void print1DTable(int table[numberOfResources], char* title);

int resourceLimts[numberOfResources] = {
    10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10,10
};
int resourceAllocations[18][numberOfResources] = {
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
    {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
};
int resourceMaxes[18][numberOfResources] = {
    {3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3},
    {3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3},
    {3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3},
    {3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3},
    {3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3},
    {3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3},
    {3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3},
    {3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3},
    {3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3},
    {3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3},
    {3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3},
    {3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3},
    {3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3},
    {3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3},
    {3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3},
    {3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3},
    {3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3},
    {3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3}
};
// int resourceNeeds[18][numberOfResources] = {
//     {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
//     {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
//     {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
//     {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
//     {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
//     {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
//     {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
//     {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
//     {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
//     {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
//     {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
//     {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
//     {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
//     {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
//     {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
//     {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
//     {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
//     {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
// };

int requestsGranted = 0;
int requestDenied = 0;
int deadlockAvoidanceRun = 0;

int printVerbose = 0;

int main (int argc, char *argv[]) {
    srandom( getpid() );
    //set signals
    signal(SIGCHLD, childClosedSignal);
    signal(SIGINT, closeProgramSignal);

    //set default values and get command line inputs
    int c;
    int maxRunTime = 20;
    char* logFile = "logFile.txt";

    while ((c = getopt (argc, argv, "hs:l:t:v")) != -1){
        switch (c){
            case 'h':
                printf("Options:\n-h: Help\n-l: The given argument(string) specifies the neame of the logfile.\n-t: The given number(int) specifies the max amount of time the program will run for.\n");
                fprintf(outputFile, "Options:\n-h: Help\n-l: The given argument(string) specifies the neame of the logfile.\n-t: The given number(int) specifies the max amount of time the program will run for.\n");
                exit(0);
                break;
            case 'l':
                logFile = optarg;
                break;
            case 't':
                maxRunTime = atoi(optarg);
                break;
            case 'v':
                printVerbose = 1;
                break;
            default:
                printf("there was an error with arguments");
                fprintf(outputFile, "there was an error with arguments");
                exit(1);
                break;
        }
    }

    //setup output file
    setupOutputFile();

    //display run parameters
    printf("Log file name: %s\n", logFile);
    fprintf(outputFile, "Log file name: %s\n", logFile);
    printf("Max run time: %d\n", maxRunTime);
    fprintf(outputFile, "Max run time: %d\n", maxRunTime);

    //Intilize various shared memory
    setupSharedClock();
    clockShmPtr[0] = 0;
    clockShmPtr[1] = 0;

    setupMsgQueue();

    intilizeResourceTables();

    while(clockShmPtr[0] < 80){
    // while(1==1){
        // printf("looping!\n");
        if ((currentProcesses < maxProcesses)){
            // printf("Creating process!\n");
            createProcesses();
        }
        advanceTime();
        reciveMessages();
    }

    closeProgram();
}

void childClosedSignal(int sig){
    closeChild();
}

int closeChild(){
    pid_t closedChild = wait(NULL);
    if (closedChild > 0){
        int i;
        for(i = 0; i < 18; i++){
            if (openProcesses[i] == closedChild){
                openProcesses[i] = 0;
            }
        }
        // printf("closing child %d\n", closedChild);
        // fprintf(outputFile, "closing child %d\n", closedChild);
        currentProcesses--;
    }
    return closedChild;
}

void closeProgramSignal(int sig){
    closeProgram();
}

void closeProgram(){
    printf("Final Stats:\n");
    printf("  Times Deadlock Avoidance Ran: %d\n", deadlockAvoidanceRun);
    printf("  Requests Granted: %d\n", requestsGranted);
    printf("  Requests Denied: %d\n", requestDenied);
    printf("  Percent of requests Granted: %f\n", (double)requestsGranted/(double)deadlockAvoidanceRun);

    shmctl(clockShmId, IPC_RMID, NULL);
    shmdt(clockShmPtr);
    fclose(outputFile);
    msgctl(msgQueueId, IPC_RMID, NULL);
    int i;
    for(i = 0; i < 18; i++){
        if (openProcesses[i] != 0){
            kill(openProcesses[i], SIGINT);
        }
    }
    printf("Parrent: Exiting gracefully.\n");
    fprintf(outputFile, "Parrent: Exiting gracefully.\n");
    while (closeChild() > 0){}
    exit(0);
}

void setupOutputFile(){
    char* logFile = "logFile.txt";
    outputFile = fopen(logFile, "w");
    if (outputFile == NULL){
        printf("Failed to open output file.\n");
        fprintf(outputFile, "Failed to open output file.\n");
        closeProgram();
    }
}

void setupSharedClock(){
    key_t sharedClockKey;
    if (-1 != open(SHMNAME, O_CREAT, 0666)) {
        sharedClockKey = ftok(SHMNAME, CLOCKVAR);
    } else {
        printf("ftok error in parrent: setupSharedClock\n");
        printf("Error: %d\n", errno);
        fprintf(outputFile, "ftok error in parrent: setupSharedClock\n");
        fprintf(outputFile, "Error: %d\n", errno);
        exit(1);
    }

    clockShmId = shmget(sharedClockKey, sizeof(int)*2, IPC_CREAT | 0666);
    if (clockShmId < 0) {
        printf("shmget error in parrent: setupSharedClock\n");
        printf("Error: %d\n", errno);
        fprintf(outputFile, "shmget error in parrent: setupSharedClock\n");
        fprintf(outputFile, "Error: %d\n", errno);
        exit(1);
    }

    clockShmPtr = (int *) shmat(clockShmId, NULL, 0);
    if ((long) clockShmPtr == -1) {
        printf("shmat error in parrent: setupSharedClock\n");
        printf("Error: %d\n", errno);
        fprintf(outputFile, "shmat error in parrent: setupSharedClock\n");
        fprintf(outputFile, "Error: %d\n", errno);
        shmctl(clockShmId, IPC_RMID, NULL);
        exit(1);
    }
}

void advanceTime(){
    clockShmPtr[0] += 1;
    clockShmPtr[1] += random() % 1000;
    // clockShmPtr[1] += 1000;
    while (clockShmPtr[1] >= 1000000000){
        clockShmPtr[1] -= 1000000000;
        clockShmPtr[0]++;
        printf("%d:%d\n", clockShmPtr[0], clockShmPtr[1]);
        fprintf(outputFile, "%d:%d\n", clockShmPtr[0], clockShmPtr[1]);
    }
}

void createProcesses(){
    // printf("creating child\n");

    if (createNextProcessAt < 0){
        int randNumber = (random() % 2);
        // createNextProcessAt = randNumber + clockShmPtr[0];
        createNextProcessAt = 1;
        // printf("next process at %d seconds\n", createNextProcessAt);
    }

    if ((clockShmPtr[0] > createNextProcessAt) && (createNextProcessAt > 0)){
   
        int i;
        int openSpace;
        for(i = 0; i < 18; i++){
            if (openProcesses[i] == 0){
                openSpace = i;
                break;
            }
        }
        char alocatedResourcesString[40];
        sprintf(
            alocatedResourcesString, 
            "%d/%d/%d/%d/%d/%d/%d/%d/%d/%d/%d/%d/%d/%d/%d/%d/%d/%d/%d/%d", 
            resourceMaxes[openSpace][0], 
            resourceMaxes[openSpace][1], 
            resourceMaxes[openSpace][2], 
            resourceMaxes[openSpace][3], 
            resourceMaxes[openSpace][4], 
            resourceMaxes[openSpace][5], 
            resourceMaxes[openSpace][6], 
            resourceMaxes[openSpace][7], 
            resourceMaxes[openSpace][8], 
            resourceMaxes[openSpace][9], 
            resourceMaxes[openSpace][10], 
            resourceMaxes[openSpace][11], 
            resourceMaxes[openSpace][12], 
            resourceMaxes[openSpace][13], 
            resourceMaxes[openSpace][14], 
            resourceMaxes[openSpace][15], 
            resourceMaxes[openSpace][16], 
            resourceMaxes[openSpace][17], 
            resourceMaxes[openSpace][18], 
            resourceMaxes[openSpace][19]
        );
        pid_t newForkPid;
        createNextProcessAt = -1;
        newForkPid = fork();
        if (newForkPid == 0){
            execlp("./worker","./worker", alocatedResourcesString, NULL);
            fprintf(stderr, "Failed to exec worker!\n");
            fprintf(outputFile, "Failed to exec worker!\n");
            exit(1);
        }
        openProcesses[i] = newForkPid;
        printf("Execed child %d\n", newForkPid);
        fprintf(outputFile, "Execed child %d\n", newForkPid);
        currentProcesses++;
    }
}

void setupMsgQueue(){
    key_t msgQueueKey;
    if (-1 != open(QUEUENAME, O_CREAT, 0777)) {
        msgQueueKey = ftok(QUEUENAME, QUEUEVAR);
    } else {
        printf("ftok error in parrent: setupMsgQueue\n");
        printf("Error: %d\n", errno);
        exit(1);
    }

    msgQueueId = msgget(msgQueueKey, (IPC_CREAT | 0777));
    if (msgQueueId < 0) {
        printf("msgget error in parrent: setupSharedClock\n");
        printf("Error: %d\n", errno);
        exit(1);
    }
}

void reciveMessages(){
    int msgRecived  = msgrcv(msgQueueId, &message, sizeof(message), 1, IPC_NOWAIT);
    if (msgRecived == -1){
        return;
    }

    char* requestedResourcesString = message.mtext;
    char* stringElement = strtok(requestedResourcesString, "/");
    int requestedResources[20];


    pid_t requestingPid = atoi(stringElement);
    stringElement = strtok (NULL, "/");

    int i = 0;
    while (stringElement != NULL){
        requestedResources[i++] = atoi(stringElement);
        stringElement = strtok (NULL, "/");
    }

    // printf("Parent: Recived msg from child: %d ", requestingPid);
    // printf("requesting resources: {");
    // for (i = 0; i < 20; ++i) {
    //     printf("%d,", requestedResources[i]);
    // }
    // printf("}\n");
    print1DTable(requestedResources, "Parent: Recived msg from Requesting:");

    int j;
    int processLocation;
    for(j = 0; j < 18; j++){
        if (openProcesses[j] == requestingPid){
            processLocation = j;
            break;
        }
    }

    int grantOkay = checkGrant(processLocation, requestedResources);

    if (grantOkay == 1){
        requestsGranted++;
        message.mtype = requestingPid;
        int n;
        for(n = 0; n < 20; n++){
            resourceAllocations[processLocation][n] += requestedResources[n];
            if (resourceAllocations[processLocation][n] < 0) {
                resourceAllocations[processLocation][n] = 0;
            }
        }
        int msgSent = msgsnd(msgQueueId, &message, sizeof(message), 0);
        if (msgSent < 0){
            printf("Parrent: failed to send message.\n");
            fprintf(outputFile, "Parrent: failed to send message.\n");
        }
        printf("Parent: sent msg to child %d Granting Request\n", requestingPid);
        fprintf(outputFile, "Parent: sent msg to child %d Granting Request\n", requestingPid);
    } else {
        requestDenied++;
        printf("Parent: sent msg to child %d denying Request\n", requestingPid);
        fprintf(outputFile, "Parent: sent msg to child %d denying Request\n", requestingPid);
        int n;
        int openSpace;
        for(n = 0; n < 18; n++){
            if (blockedProcesses[n] == 0){
                blockedProcesses[n] = requestingPid;
                printf("Parrent: added process %d to blocked queue.\n", requestingPid);
                fprintf(outputFile, "Parrent: added process %d to blocked queue.\n", requestingPid);
                break;
            }
        }
    }
    print1DTable(resourceLimts, "System Resources:");
    print2DTable(resourceAllocations, "Resources Alocated:");
}

int checkGrant(int processLocation, int requestedResources[]){
    advanceTime();
    deadlockAvoidanceRun++;
    int avalibleResources[numberOfResources];
    int needResources[18][numberOfResources];
    int newAlocations[18][numberOfResources];
    int allocationOrder[18];
    int processesToCheck = 0;
    int finishedProcesses = 0;

    int j;
    int i;

    //copy respource limits
    for (i = 0; i < numberOfResources; i++){
        avalibleResources[i] = resourceLimts[i];
    }

    //copy previous alocations
    for (i = 0; i < numberOfResources; i++){
         for (j = 0; j < numberOfResources; j++){
            newAlocations[i][j] = resourceAllocations[i][j];
        }
    }

    //add requested resources to allocations
    for (i = 0; i < numberOfResources; i++){
        newAlocations[processLocation][i] = newAlocations[processLocation][i] + requestedResources[i];
    }

    //calculate needed resorces and currently avalible resources
    for (i = 0; i < 18; i++){
        for (j = 0; j < numberOfResources; j++){
            //find number of avalible resources in system
            avalibleResources[j] -= resourceAllocations[i][j];   
            //find need of each process
            needResources[i][j] = resourceMaxes[i][j] - resourceAllocations[i][j];
        }
    }

    //Bankers algorythim
    int processComplete = 1;
    int currentProcessesToCheck = 0;
    int grantOkay = 0;
    while (processesToCheck < 18){
        processComplete = 1;
        currentProcessesToCheck = (currentProcessesToCheck + 1) % 18;

        for (i = 0; i < numberOfResources; i++){
            if (needResources[currentProcessesToCheck][i] > avalibleResources[i]){
                processComplete = 0;
                break;
            }
        }


        if (processComplete == 1) {
            allocationOrder[finishedProcesses] = currentProcessesToCheck;
            finishedProcesses++;
            processesToCheck = 0;
            for (i = 0; i < numberOfResources; i++){
                avalibleResources[i] = avalibleResources[i] + needResources[currentProcessesToCheck][i];
                needResources[currentProcessesToCheck][i] = 0;
            }
        }
        if (finishedProcesses == 18){
            grantOkay = 1;
            break;
        }
        processesToCheck++;
    }
    // int processesCompleted[18];
    // int safeSequence[18];
    // int newProcessCanComplete = 0;
    // int grantOkay = 1;
    // int n = 0;
    // while(){
    //     for (i = 0; i < 18; i++){
    //         int resourcesAvalibleForProcess = 1;
    //         for (j = 0; j < numberOfResources; j++){
    //             if (needResources[i][j] > avalibleResources[j]){
    //                 resourcesAvalibleForProcess = 0;
    //             }
    //         }
    //         if (resourcesAvalibleForProcess == 1){
    //             newProcessCanComplete = 1;
    //         }
    //     }
    //     if (newProcessCanComplete > 0){

    //     }
    // }

    // int grantOkay = 1;
    // for(i=0; i<20; i++){
    //     if (requestedResources[i] > avalibleResources[i]){
    //         grantOkay = 0;
    //     }
    // }
    return grantOkay;
}

// void generateNeedTable() {
//     int i;
//     for (i = 0; i < 18; i++){
//         int j;
//         for (j = 0; j < numberOfResources; j++){
//             resourceNeeds[i][j] = resourceMaxes[i][j] - resourceAllocations[i][j];
//         }
//     }
// }

void intilizeResourceTables(){
    int i;
    int j;

    for (i=0; i < numberOfResources; i++){
        resourceLimts[i] = (random() % 10) +5;
    }
    print1DTable(resourceLimts, "System Resources:");

    for (i = 0; i < 18; i++){
        for (j = 0; j < numberOfResources; j++){
            resourceMaxes[i][j] = (random() % 3) +1;  
        }
    }
    print2DTable(resourceMaxes, "Resource Maxes:");
}

void print2DTable(int table[18][numberOfResources], char* title){
    if (printVerbose == 0){
        return;
    }
    int i;
    int j;
        
    printf("%s\n", title);
    fprintf(outputFile, "%s\n", title);
    printf("    ");
    fprintf(outputFile, "    ");
    for (i=0; i < numberOfResources; i++){
        if ( i < 10){
            printf("  %d ", i); 
            fprintf(outputFile, "  %d ", i); 
        } else {
            printf(" %d ", i); 
            fprintf(outputFile, " %d ", i); 
        }
    }
    printf("\n");
    fprintf(outputFile, "\n");

    for (i = 0; i < 18; i++){
        if ( i < 10){
            printf(" P%d ", i); 
            fprintf(outputFile, " P%d ", i);
        } else {
            printf("P%d ", i); 
            fprintf(outputFile, "P%d ", i); 
        }
        for (j = 0; j < numberOfResources; j++){
            if ( table[i][j] < 10){
                printf("  %d ", table[i][j]); 
                fprintf(outputFile, "  %d ", table[i][j]); 
            } else {
                printf(" %d ", table[i][j]); 
                fprintf(outputFile, " %d ", table[i][j]);
            }
        }
        printf("\n");
        fprintf(outputFile, "\n");
    }
}

void print1DTable(int table[numberOfResources], char* title){
    if (printVerbose == 0){
        return;
    }
    int i;

    printf("%s\n", title);
    fprintf(outputFile, "%s\n", title);
    for (i=0; i < numberOfResources; i++){
        if ( i < 10){
            printf("  %d ", i); 
            fprintf(outputFile, "  %d ", i);
        } else {
            printf(" %d ", i); 
            fprintf(outputFile, " %d ", i); 
        }
    }
    printf("\n");
    fprintf(outputFile, "\n");

    for (i = 0; i < numberOfResources; i++){
        if ( table[i] < 10){
            printf("  %d ", table[i]); 
            fprintf(outputFile, "  %d ", table[i]); 
        } else {
            printf(" %d ", table[i]); 
            fprintf(outputFile, " %d ", table[i]); 
        }
    }
    printf("\n");
    fprintf(outputFile, "\n");
}