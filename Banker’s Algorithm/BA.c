/*
 * Banker's Algorthm
 * Designed by Dai Junyan
 * E-mail: daiju@kean.edu
 * Wenzhou-Kean University
 * COURSE: CPS 3250
 */


#include <stdio.h>
#include <stdlib.h>//atoi rand
#include <pthread.h>
#include <time.h>
#include <unistd.h>//sleep

/* these may be any values >= 0 */
#define NUMBER_OF_CUSTOMERS 5
#define NUMBER_OF_RESOURCES 4
/* the available amount of each resource */
int available[NUMBER_OF_RESOURCES];
/*the maximum demand of each customer */
int maximum[NUMBER_OF_CUSTOMERS][NUMBER_OF_RESOURCES];
/* the amount currently allocated to each customer */
int allocation[NUMBER_OF_CUSTOMERS][NUMBER_OF_RESOURCES];
/* the remaining need of each customer */
int need[NUMBER_OF_CUSTOMERS][NUMBER_OF_RESOURCES];
pthread_mutex_t mutex_lock;
int customerID[NUMBER_OF_CUSTOMERS]={0,1,2,3,4};

int request_resources(int customer_num, int request[]);
int release_resources(int customer_num, int release[]);
void* customer_thread(void* customerID);
void printTable();
int check_safe(); // return 0 if safe, return 1 if unsafe


int main(int argc, char const *argv[]) {
    if(argc != NUMBER_OF_RESOURCES+1){
        printf("Quantity of parameter is not correct.\n");
        return -1;
    }
    //initialize available[] (the number of resources of each type)
    int i,j,num_of_instance;
    for(i = 0; i<NUMBER_OF_RESOURCES; i++ ){
        num_of_instance = atoi(argv[i+1]);//argv[0] is name of program
        if(num_of_instance>15){
            printf("The maximum number should not exceed 15.\n");
            return -1;
        }
        available[i] = num_of_instance;
    }
    //initialize maximum[NUMBER_OF_CUSTOMERS][NUMBER_OF_RESOURCES]
    // (the number of the maximum demand of each customer)
    srand(time(NULL));
    for(i=0; i<NUMBER_OF_CUSTOMERS;i++){
        for(j=0;j<NUMBER_OF_RESOURCES ;j++){
            //randmonly create maximum demand but not exceed maximum resources number of each
            num_of_instance = rand()%(atoi(argv[j+1])+1);
            maximum[i][j] = num_of_instance;
            need[i][j] =num_of_instance;
        }
    }

    printTable();

    //initialize mutex
    pthread_mutex_init(&mutex_lock, NULL);
    //initialize and create customer thread with customerID
    pthread_t customer[NUMBER_OF_CUSTOMERS];
    for(i=0; i<NUMBER_OF_CUSTOMERS; i++){
        pthread_create(&customer[i],NULL,customer_thread,(void*)&customerID[i]);
    }
    //join threads
    for(i=0; i<NUMBER_OF_CUSTOMERS; i++){
        pthread_join(customer[i],NULL);
    }

    return 0;
}

void* customer_thread(void* customerID){
    int customer_num = *(int*)customerID;
    int i,request[NUMBER_OF_RESOURCES],release[NUMBER_OF_RESOURCES];//this flag is used for preventing request(0,0,0,0)
    int loop = 1;//thread may need to terminate later
    while(loop){
        //generate customer's request bounded by its need array
        int flag = 1;
        while(flag) {
            for (i = 0; i < NUMBER_OF_RESOURCES; i++) {
                if (need[customer_num][i] != 0) {
                    for(request[i] = rand() % (need[customer_num][i] + 1);request[i]>available[i];request[i] = rand() % (need[customer_num][i] + 1));
                } else {
                    request[i] = 0;
                }
                //if request(0,0,0,0), it will continue looping and generate customer's request again
                //if there is a request[i] != 0, then break the second while loop
                if (request[i] != 0) {
                    flag = 0;
                }
            }
        }
        sleep((unsigned)rand()%3);
        //mutex lock to prevent race condition
        pthread_mutex_lock(&mutex_lock);

        //print request array
        printf("customer %d request:",customer_num);
        sleep(1);
        for(i=0;i<NUMBER_OF_RESOURCES;i++){
            printf(" %d",request[i]);
        }
        int condition = request_resources(customer_num,request);
        //return 0 if request successful
        if(condition == 0 ){
            printf("Safe state! Request successful!\n");
            sleep(1);
            printTable();
        }
        //return 1 if request unsuccessful
        else if(condition == 1){
            printf("Unsafe state! Request Denied!\n");
        }
        //check if need array is 0
        flag=0;
        for(i=0;i<NUMBER_OF_RESOURCES;i++){
            if(need[customer_num][i]!=0){
                flag=1;
            }
        }
        //if need array is 0, release the allocation
        if(flag==0){
            sleep(1);
            printf("customer %d is releasing resources\n", customer_num );
            for(i=0;i<NUMBER_OF_RESOURCES;i++){
                release[i] = allocation[customer_num][i];
            }
            release_resources(customer_num,release);
            sleep(1);
            printTable();
            //stop this thread
            loop = 0;
        }
        pthread_mutex_unlock(&mutex_lock);
    }

}

void printTable(){
    printf("\tMAXIMUM\t\tALLOCATION\t NEED  \t\tAVALIABLE");
    printf("\n");
    int i,j;
    for(i=0;i<NUMBER_OF_CUSTOMERS;i++){
        printf("%d\t",customerID[i]);
        for(j=0;j<NUMBER_OF_RESOURCES;j++){
            printf("%d ",maximum[i][j]);
        }
        printf("\t");
        for(j=0;j<NUMBER_OF_RESOURCES;j++){
            printf("%d ",allocation[i][j]);
        }
        printf("\t");
        for(j=0;j<NUMBER_OF_RESOURCES;j++){
            printf("%d ",need[i][j]);
        }
        printf("\t");
        if(i==0){
            for(j=0;j<NUMBER_OF_RESOURCES;j++){
                printf("%d ",available[j]);
            }
        }
        printf("\n");
    }
}

int request_resources(int customer_num, int request[]){
    //pretend to allocate
    int i,flag=0;
    for(i=0;i<NUMBER_OF_RESOURCES;i++){
        available[i] -= request[i];
        allocation[customer_num][i] += request[i];
        need[customer_num][i] -=request[i];
        if(need[customer_num][i]!=0){
            flag=1;
        }
    }
    printf("\nChecking if it is still safe...\n");
    //if need array is all 0, it means it's safe because it will release then
    if(flag==0){
        return 0;
    }
    //if it is unsafe
    if(check_safe()==1){
        for(i=0;i<NUMBER_OF_RESOURCES;i++) {
            available[i] += request[i];
            allocation[customer_num][i] -= request[i];
            need[customer_num][i] += request[i];
        }
        return 1;//return 1 if unsafe
    }
    //return 0 if safe
    return 0;
}

int check_safe(){
    //initialize work and finishh
    int work[NUMBER_OF_RESOURCES];
    int finish[NUMBER_OF_CUSTOMERS]={0,0,0,0,0};
    int i,j,k;
    for(i=0;i<NUMBER_OF_RESOURCES;i++){
        work[i] = available[i];
    }
/*1.	Let Work and Finish be vectors of length m and n, respectively.  Initialize:
 *      Work = Available
 *      Finish[i] = false for i = 0, 1, …, n- 1
 *
 *      2.	Find an index i such that both:
 *      (a) Finish[i] = false
 *      (b) Need[i] <= Work
 *      If no such i exists, go to step 4
 *
 *      3.  Work = Work + AllocationiFinish[i] = truego to step 2
 *
 *      4.	If Finish [i] == true for all i, then the system is in a safe state
*/
    for(i=0;i<NUMBER_OF_CUSTOMERS;i++){
        if(finish[i]==0){
            for(j=0;j<NUMBER_OF_RESOURCES;j++){
                if(need[i][j]>work[j]){
                    break;
                }
                if(j==NUMBER_OF_RESOURCES - 1){//need[i]<=work
                    finish[i] = 1;
                    for(k=0;k<NUMBER_OF_RESOURCES;k++){
                        work[k] += allocation[i][k];
                        //printf("work[%d] = %d ",k,work[k]);
                    }
                    //printf("\n");
                    i=-1;
                }
            }
        }
    }

    //if there exist a finish[i] == 0, it is unsafe
    //if finish[i] == 1 for all i, then the system is in a safe state
    for(i=0;i<NUMBER_OF_CUSTOMERS;i++){
        if(finish[i]==0){//unsafe state
            return 1;
        }
    }
    return 0;//safe state
}

int release_resources(int customer_num, int release[]){
    int i;
    for(i=0;i<NUMBER_OF_RESOURCES;i++){
        allocation[customer_num][i] -= release[i];
        available[i] += release[i];
    }
    return 0;
}