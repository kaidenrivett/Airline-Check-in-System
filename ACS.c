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
} customer_info;

typedef struct clerk_info {
  int id, available;
} clerk_info;

double getCurrentSimulationTime();
void readCustomers(char *file);
void *customer_entry(void * cus_info);

pthread_mutex_t mutex[NUMBER_OF_THREADS];
pthread_cond_t convar[NUMBER_OF_CONVAR];
// global variables
struct timeval start_time;
double overall_waiting_time;
customer_info* customers;
customer_info* queue[2];
clerk_info clerk_number[4];

int queue_length[NQUEUE];
int queue_status[NQUEUE];
int line_length[3] = {0,0,0}; // representing the line lengths of economy class, business class, and the # of total customers, respectively
float waiting_time[2] = {0,0}; // represents waiting time of economy class business class, respectively


// implemented with reference to the sample code given on connex
double getCurrentSimulationTime() {
  struct timeval cur_time;
  double cur_secs, init_secs;

  init_secs = (start_time.tv_sec + (double) start_time.tv_usec / 1000000);
  gettimeofday(&cur_time, NULL);
  cur_secs = (cur_time.tv_sec + (double) cur_time.tv_usec / 1000000);\
  return cur_secs - init_secs;
}

int main(int argc, char* argv[]) {
  int number_customers = 1;
  printf("Some stuff is happening here!!\n");

if(argc < 2){
  printf("Not enough arguments passed\n");
  exit(1);
}
// read text file from user arguments
readCustomers(argv[1]);

  return 0;
}

void readCustomers(char *file){
  FILE *fp = fopen(file, "r");
  if(fp == NULL || fscanf(fp, "%d", &line_length[2]) < 1){
    fprintf(stderr, "Failed to read file");
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


// // update the linked list that contains the information of the customer
// void customerUpdate(struct customer_info** head_ref, int customerID, int class_type, int service_time, int arrival_time){
//   customer_info* info = (customer_info*)malloc(sizeof(info));
//   customer_info* last = *head_ref;
//   info ->customerID = customerID;
//   info ->class_type = class_type;
//   info ->service_time = service_time;
//   info ->arrival_time = arrival_time;
//   info ->next = NULL;
//   if(*head_ref == NULL){
//     *head_ref = info;
//     return;
//   }
//   while(last->next != NULL){
//     last = last->next;
//   }
//   last->next = info;
//   return;
// }
//
// void displayInfo(struct customer_info *info){
//   while(info!=NULL){
//     printf("Customer ID: %d \n", info->customerID);
//     if(info->class_type == 1){
//       printf("Business class\n");
//     } else {
//       printf("Economy class \n");
//     }
//     printf("Arrival Time: %d \n ", info->arrival_time);
//     printf("Service Time: %d \n", info->service_time);
//   }
//   exit(1);
// }
