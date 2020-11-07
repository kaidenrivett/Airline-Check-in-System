#define _GNU_SOURCE

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>  // intptr_t
#include <sys/time.h>
#define NQUEUE 32
#define NUMBER_OF_THREADS 5
#define NUMBER_OF_CONVAR 6

// creating linked list to store customer information from the text file, referenced with A2-hint_detailed.c

typedef struct customer_info {
  int customerID;
  int queueID;
  int class_type;
  int service_time;
  int arrival_time;
  int clerk, index;
  float start_serv_time, end_serv_time;
}customer_info;

typedef struct clerk_info {
  int id, available;
}clerk_info;

pthread_mutex_t mutex[NUMBER_OF_THREADS];
pthread_cond_t convar[NUMBER_OF_CONVAR];
// global variables
struct timeval start_time;
double overall_waiting_time;
customer_info* customers;
customer_info* queue[2]; // two queues, one for business class and one for economy class
clerk_info clerk_number[4];

int queue_length[NQUEUE];
int queue_status[NQUEUE];
int line_length[3] = {0,0,0}; // representing the line lengths of economy class, business class, and the # of total customers, respectively
float waiting_time[2] = {0,0}; // represents waiting time of economy class business class, respectively


double getCurrentSimulationTime();
void readCustomers(char *file);
void *customerEntry(void * cus_info);
void *clerkEntry(void* clerk_num);

// queue operations
int queuePop(int i);
void queueInsertion(customer_info* cust, int i);


int main(int argc, char* argv[]) {
  int number_customers = 1;
  if(argc < 2){
    printf("Not enough arguments passed\n");
    exit(1);
  }
// read text file from user arguments
readCustomers(argv[1]);

gettimeofday(&start_time, NULL);
for(int i = 0; i < NUMBER_OF_CONVAR; i++){
  if(i < 4){
    clerk_number[i].id = i++;
    clerk_number[i].available = 0;
  }
  if(i < NUMBER_OF_THREADS && pthread_mutex_init(&mutex[i], NULL) != 0){
    printf("Error: Failed to initialize the mutex\n");
    exit(1);
  }
  if(pthread_cond_init(&convar[i], NULL) != 0){
    printf("Error: Failed to initilaize the convar\n");
  }
}
  return 0;
}



// implemented with reference to the sample code given on connex
double getCurrentSimulationTime() {
  struct timeval cur_time;
  double cur_secs, init_secs;

  init_secs = (start_time.tv_sec + (double) start_time.tv_usec / 1000000);
  gettimeofday(&cur_time, NULL);
  cur_secs = (cur_time.tv_sec + (double) cur_time.tv_usec / 1000000);\
  return cur_secs - init_secs;
}

void readCustomers(char *file){
  FILE *fp = fopen(file, "r");
  if(fp == NULL || fscanf(fp, "%d", &line_length[2]) < 1){
    fprintf(stderr, "Failed to read file\n");
  }
  if(line_length[2] < 1) {
    printf("Error: Invalid number of customers in text file.\n");
    exit(1);
  }
  queue[0] = (customer_info*) malloc(line_length[2] * sizeof(customer_info));
  queue[1] = (customer_info*) malloc(line_length[2] * sizeof(customer_info));
  customers = (customer_info*) malloc(line_length[2] * sizeof(customer_info));

  int i = 0;
  customer_info cust;
  for(int j = 0; j < line_length[2]; j++){
    if(fscanf(fp, "%d:%d,%d,%d", &cust.customerID, &cust.queueID, &cust.arrival_time, &cust.service_time) != 4){
      printf("Error: invalid layout.\n");
      continue;
    }
    cust.index = i;
    cust.clerk = -1;
    customers[i] = cust;
    i++;
    line_length[cust.queueID]++;
  }
  line_length[2] = i;
  printf("Index is: %d\n", i);
  fclose(fp);

}

// developed with reference to the A2 detailed sample code provided on Connex
void *clerkEntry(void *clerk_num){
  clerk_info* clerk = (clerk_info*) clerk_num;
  while(1){
    // error handling
    if(pthread_mutex_lock(&mutex[0]) != 0){
      printf("Error: mutex failed to lock\n");
      exit(1);
    }
    int queue_index = 1;
    if(queue_length[queue_index] <= 0){
      queue_index = 0;
    }
    if(queue_length[queue_index] > 0){
      int customer_index = queuePop(queue_index);
      customers[customer_index].clerk = clerk->id;
      clerk_number[clerk->id-1].available = 1;

      // error handling
      if(pthread_cond_broadcast(&convar[queue_index]) != 0){
        printf("Error: failed to broadcast convar\n");
        exit(1);
      }
      if(pthread_mutex_unlock(&mutex[0]) != 0){
        printf("Error: failed to unlock mutex.\n");
        exit(1);
      }
    } else{
      if(pthread_mutex_unlock(&mutex[0]) != 0){
				printf("Error: failed to unlock mutex.\n");
				exit(1);
    }
    usleep(250);
  }
  if(pthread_mutex_lock(&mutex[clerk->id]) != 0){
    printf("Error: failed to lock mutex.\n");
    exit(1);
  }
  if(clerk_number[clerk->id-1].available){
    if(pthread_cond_wait(&convar[clerk->id+1], &mutex[clerk->id]) != 0){
      printf("Error: failed to wait.\n");
      exit(1);
    }
  }
  if(pthread_mutex_unlock(&mutex[clerk->id]) != 0){
    printf("Error: failed to unlock mutex.\n");
    exit(1);
  }
}
  return NULL;
}



////////// QUEUE OPERATIONS //////////

void queueInsertion(customer_info* cust, int i){
  queue[i][queue_length[i]] = *cust;
  queue_length[i]++;
}

int queuePop(int i){
  int cust_index = queue[i][0].index;
  for(int j = 0; i < queue_length[i]-1; j++){
    queue[i][j] = queue[i][j+1];
   }
   queue_length[i]--;
   return cust_index;
}
