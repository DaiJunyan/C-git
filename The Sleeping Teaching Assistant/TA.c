/*
 * The Sleeping Teaching Assistant
 * Designed by Dai Junyan
 * E-mail: daiju@kean.edu
 * Wenzhou-Kean University
 * COURSE CPS 3250
 */

#include <stdio.h>
#include <semaphore.h>
#include <pthread.h>
#include <stdlib.h>//srand
#include <time.h>
#include <unistd.h>//sleep


#define NUM_OF_SEATS 5   //4 waiting chairs, 1 TA desk
#define NUM_OF_STUDENTS 8

sem_t students_sem;
sem_t ta_sem;
pthread_mutex_t mutex_lock;
pthread_mutex_t mutex_lock2;
sem_t ta_nap;

int waiting_students = 0;
int chair[NUM_OF_SEATS];
int seat = 0;
int teach = 0;

int randtime();
int ta_help_randime();
void* student_thread(void* studentID);
void* TA_thread();



int main() {
    //initialize
    pthread_mutex_init(&mutex_lock, NULL);
    sem_init(&students_sem, 0, 0);
    sem_init(&ta_sem, 0, 0);
    pthread_mutex_init(&mutex_lock2, NULL);
    sem_init(&ta_nap, 0, 0);

    pthread_t students[NUM_OF_STUDENTS];
    pthread_t ta;
    int studentID[NUM_OF_STUDENTS];

    //create ta thread
    pthread_create(&ta,NULL,TA_thread,NULL);
    //create student_thread
    int i;
    for(i=0; i<NUM_OF_STUDENTS;i++){
        studentID[i] = i+1;
        pthread_create(&students[i],NULL,student_thread,(void*)&studentID[i]);
    }

    pthread_join(ta,NULL);

    for(i=0; i<NUM_OF_STUDENTS;i++){
        pthread_join(students[i],NULL);
    }



    return 0;
}

void* student_thread(void* studentID){
    int id = *(int*)studentID;
    printf("[s]Student %d is programming\n",id);
    int count = 0;
    randtime();
    while (count < 6) {
        pthread_mutex_lock(&mutex_lock2);//if no mutex_lock2, student threads will always happen prior to TA
        randtime();//students come randomly
        /* acquire the mutex lock */
        pthread_mutex_lock(&mutex_lock);
        printf("[s]student %d is coming\n", id);
        if (waiting_students < NUM_OF_SEATS) {
            ++waiting_students;
            chair[seat]= id;
            seat = (seat + 1) % NUM_OF_SEATS;
            if(waiting_students == 1){
                printf("[s]TA is taking a nap. Student %d awaken the TA to ask for help\n",id);
                sem_post(&ta_nap);//awaken TA
                sleep(1);
            }
            else if(waiting_students > 1){
                printf("[s]Student %d sits in a chair. %d students are waiting\n",id, waiting_students-1);
                sleep(1);
            }

            //send a signal to notify TA
            sem_post(&students_sem);

            pthread_mutex_unlock(&mutex_lock);

            pthread_mutex_unlock(&mutex_lock2);

            //wating for TA idle
            sem_wait(&ta_sem);
            count++;//student got help time+1
            //this student got help
        } else {
            //no seats, student has to leave and come back later
            printf("[s]No chairs are available, student %d is leaving and will come back at a later time\n",id);
            sleep(1);
            pthread_mutex_unlock(&mutex_lock);
            pthread_mutex_unlock(&mutex_lock2);
        }
    }
}

void* TA_thread(){
    sleep(1);
    while (1) {
        if(chair[teach]==0){
            printf("[ta]No students come. TA is taking a nap\n");
            sem_wait(&ta_nap);//wait for a student coming
            printf("[ta]TA has been awakened\n");
        }
        /* wait for a student to show up */
        sem_wait(&students_sem);
        /* acquire the mutex lock */
        pthread_mutex_lock(&mutex_lock);
        int id = chair[teach];
        printf("[ta]TA is teaching student %d now.",id);
        chair[teach]=0;
        teach = (teach + 1) % NUM_OF_SEATS;
        printf("%d students are waiting\n",waiting_students-1);

        /* release mutex lock */
        pthread_mutex_unlock(&mutex_lock);

        int help_student_time = ta_help_randime();//time that TA spend in helping a student
        pthread_mutex_lock(&mutex_lock);
        --waiting_students;
        printf("[ta]TA finished teaching student %d, cost %d seconds. %d students are waiting\n",id,help_student_time,waiting_students);
        if(waiting_students>0){
            printf("[ta]Student %d will get help next\n",chair[teach]);
        }
        else if(waiting_students==0){
            printf("[ta]No students are waiting, TA takes a nap\n");
            randtime();
        }
        // indicate the TA is idle and ready to help a student
        sem_post(&ta_sem);
        pthread_mutex_unlock(&mutex_lock);

    }

}

int randtime(){
    srand(time(NULL));
    int time = rand() % 5+1;
    sleep(time);
    return time;
}

int ta_help_randime(){
    srand(time(NULL));
    int time = rand() % 10+1;
    sleep(time);
    return time;
}
