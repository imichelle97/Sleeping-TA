/*
 * sleepingTA_MichelleLuong_960.c
 *
 *  Created on: Oct 21, 2017
 *      Author: michelle
 */

#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <string.h>
#include <stdlib.h>

#define MAX_SLEEP_TIME 3
#define NUM_OF_STUDENTS 4
#define NUM_OF_HELPS 2
#define NUM_OF_SEATS 2

/*
 * Function Prototypes
 */
void* TA_Routine(void* arg);
void* Student_Routine(void* arg);

typedef struct {
	int student_id;
	unsigned int seed;
} param;

/*
 * Mutex Lock
 */
pthread_mutex_t mutex_lock;

/*
 * Semaphores
 */
sem_t students_sem;
sem_t ta_sem;

int waiting_students;

/*
 * Thread routine for the TA
 */
void* TA_Routine(void* arg) {
	unsigned int helping_time;
	unsigned int ta_seed = 5;
	int student_wait;

	while(1) {
		sem_post(&ta_sem); 			/* TA is ready to help student */
		helping_time = (unsigned int)(rand_r(&ta_seed)%3) + 1;
		sem_wait(&students_sem); 	/* TA is waiting for students to be available for TA to help */

		pthread_mutex_lock(&mutex_lock);
		waiting_students--;
		student_wait = waiting_students;
		pthread_mutex_unlock(&mutex_lock);

		printf("Helping a student for %d seconds, # of waiting students = %d\n", helping_time, student_wait);
		sleep(helping_time);
	}
}

/*
 * Thread routine for the Student
 */
void* Student_Routine(void* arg) {
	unsigned int program_time;
	int num_help = 0;
	int student_ID;
	unsigned int seed;
	param *p = (param *)arg;

	student_ID = p->student_id;
	seed = p->seed;

	while(num_help < NUM_OF_HELPS) {
		program_time = (unsigned int)(rand_r(&seed)%3) + 1;
		printf("\tStudent %d programming for %d seconds\n", student_ID, program_time);
		sleep(program_time);

		pthread_mutex_lock(&mutex_lock);
		if(waiting_students < NUM_OF_SEATS) {
			waiting_students++;
			printf("\t\tStudent %d takes a seat, # of waiting students = %d\n", student_ID, waiting_students);
			sem_post(&students_sem);	/* Student is ready for TA's help and notifies TA */
			pthread_mutex_unlock(&mutex_lock);
			sem_wait(&ta_sem);			/* Student waits for TA's help */
			printf("Student %d receiving help\n", student_ID);
			num_help++;
		}
		else {
			printf("\t\t\tStudent %d will try later\n", student_ID);
			pthread_mutex_unlock(&mutex_lock);
		}
	}
	pthread_exit(NULL);
	return NULL;
}

int main(void) {
	pthread_t *threads;
	pthread_attr_t pthread_my_attr;
	param * arg;

	printf("CS149 Sleeping TA from Michelle Luong\n");

	/*
	 * initialize semaphores
	 */
	if(sem_init(&students_sem, 0, 0) != 0) {
		printf("Error, initialization failed for student semaphore\n");
	}

	if(sem_init(&ta_sem, 0, 0) != 0) {
			printf("Error, initialization failed for TA semaphore\n");
	}

	/*
	 * initialize mutex
	 */
	pthread_mutex_init(&mutex_lock, NULL);

	threads = (pthread_t *)malloc(sizeof(pthread_t)*(NUM_OF_STUDENTS + 1));
	arg = (param*)malloc(sizeof(param)*NUM_OF_STUDENTS);

	pthread_attr_init(&pthread_my_attr);

	/*
	 * Spawn student thread
	 */
	for(int i = 0; i < NUM_OF_STUDENTS; i++) {
		arg[i].student_id = i;
		arg[i].seed = (unsigned int)i;
		pthread_create(&threads[i+1], &pthread_my_attr, Student_Routine, (void *)(arg + i));
	}

	/*
	 * Spawn TA thread
	 */
	pthread_create(&threads[0], &pthread_my_attr, TA_Routine, NULL);

	/*
	 * Join threads
	 */
	for(int i = 1; i < NUM_OF_STUDENTS; i++) {
		pthread_join(threads[i], NULL);
	}

	pthread_cancel(threads[0]);

	sem_destroy(&students_sem);
	sem_destroy(&ta_sem);

	pthread_mutex_destroy(&mutex_lock);

	free(arg);
	free(threads);

	return 0;
}

