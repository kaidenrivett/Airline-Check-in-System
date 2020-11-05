#define _GNU_SOURCE

#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>  // intptr_t
#include <time.h>
#define NQUEUE 32

// creating linked list to store customer information from the text file, referenced with A2-hint_detailed.c

typedef struct customer_info {
  int customerID;
  int class_type;
  int service_time;
  int arrival_time;
  struct customer_info* next;
} customer_info;
customer_info* customer_list = NULL;

// method to search for customer id in the linked list
customer_info* searchInfo(int customerID){
  customer_info* info = customer_list;
  while(info != NULL){
    if(info->customerID == customerID){
      return info;
    }
    info = info->next;
  }
  return NULL;
}

void customerUpdate(struct customer_info** head_ref,int customerID, int class_type, int service_time, int arrival_time);
void displayInfo(struct customer_info *info);

// global variables
struct timeval init_time;
double overall_waiting_time;
int queue_length[NQUEUE];
int queue_status[NQUEUE];
// int winner_selected[NQUEUE] = FALSE;


int main(int argc, char* argv[]) {
  int number_customers = 1;

if(argc < 2){
  printf("Not enough arguments passed\n");
  exit(1);
}
// read file
const char *filename = argv[1];
const char *delimeter = ",";

FILE* fp = fopen(filename, "r");
char line[256];

int customerID = 5;
int class_type = 0;
int service_time = 60;
int arrival_time = 20;

struct customer_info* head = NULL;

customerUpdate(&head,customerID,class_type,service_time,arrival_time);
displayInfo(head);
// while(fgets(line, sizeof(line), fp) != NULL) {
//     char *t = strtok(line, delimeter);
//     while(t != NULL){
//       printf("%s", t);
//       t = strtok(NULL,delimeter);
//       customerUpdate(t)
//
//     }
//   }

fclose(fp);
  return 0;
}

// update the linked list that contains the information of the customer
void customerUpdate(struct customer_info** head_ref, int customerID, int class_type, int service_time, int arrival_time){
  customer_info* info = (customer_info*)malloc(sizeof(info));
  customer_info* last = *head_ref;
  info ->customerID = customerID;
  info ->class_type = class_type;
  info ->service_time = service_time;
  info ->arrival_time = arrival_time;
  info ->next = NULL;
  if(*head_ref == NULL){
    *head_ref = info;
    return;
  }
  while(last->next != NULL){
    last = last->next;
  }
  last->next = info;
  return;
}

void displayInfo(struct customer_info *info){
  while(info!=NULL){
    printf("Customer ID: %d \n", info->customerID);
    if(info->class_type == 1){
      printf("Business class\n");
    } else {
      printf("Economy class \n");
    }
    printf("Arrival Time: %d \n ", info->arrival_time);
    printf("Service Time: %d \n", info->service_time);
  }
  exit(1);
}
