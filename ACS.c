#define _GNU_SOURCE

///////////////////////////////////////////////////////////////////////////
/////////////////////// IMPORTED LIBRARIES ///////////////////////////////
/////////////////////////////////////////////////////////////////////////
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/time.h>


// data structure to store the customer information
typedef struct customerInfo{
	int customerID;				//customer id
	int queueID;				//queue id (0 = economy class, 1 = business class)
	int arrival_time;				//arrival time
	int service_time;				//service time
	int clerk, index;				//where customer is stored in customers
	float start_serv_time, end_serv_time;		//start time of service
		//end time of service
}customerInfo;

// data structure to store the clerk information
typedef struct clerkInfo{
	int id, available;
}clerkInfo;


struct timeval init_time; //s used to record simulation start
customerInfo* customers;
customerInfo* queue[2];
clerkInfo NClerks[4];
int queue_length[2] = {0,0};
int line_length [3] = {0,0,0}; // representing the line lengths of economy class, business class, and the # of total customers, respectively
float waiting_time[2] = {0,0};	// represents waiting time of economy class business class, respectively

//////////////////////////////////////////////////////////////////////////
/////////////////////// FUNCTION PROTOTYPES /////////////////////////////
////////////////////////////////////////////////////////////////////////
float getCurrentSimulationTime();
void readCustomers(char* input_file);
void* customerEntry(void* cus_info);
void* clerkEntry(void* clerk_num);
void insertQueue(customerInfo* customer, int i);
int popQueue(int i);

pthread_mutex_t mutex[4];
pthread_cond_t convar[4];

int main(int argc, char* argv[]){
	if(argc != 2){
		printf("Invalid Input. \n");
    printf("Please enter input as ./ACS <file>.txt \n");
		exit(1);
	}
	readCustomers(argv[1]);
	gettimeofday(&init_time, NULL);
	for(int i = 0; i < 6; i++){
		if(i < 4){
			NClerks[i].id = i++;
      // all clerks are idle at the beginning
			NClerks[i].available = 0;
		}
    int i;
		if(i < 5 && pthread_mutex_init(&mutex[i], NULL) != 0){
			printf("Error: failed to initialize mutex.\n");
			exit(1);
		}
		if(pthread_cond_init(&convar[i], NULL) != 0){
			printf("Error: failed to initialize convar.\n");
			exit(1);
		}
	}
  // created with reference to the a2-hint file provided on connex
	for(int i = 0; i < 4; i++){
		pthread_t clerk_thread;
		if(pthread_create(&clerk_thread, NULL, clerkEntry, (void*)&NClerks[i]) != 0){
			printf("Error: failed to create thread.\n");
			exit(1);
		}
	}
	pthread_t customer_thread[line_length [2]];
  // create threads for the customers
	for(int i = 0; i < line_length[2]; i++){
		if(pthread_create(&customer_thread[i], NULL, customerEntry, (void*)&customers[i]) != 0){
			printf("Error: failed to create thread.\n");
			exit(1);
		}
	}
	for(int i = 0; i < line_length[2]; i++){
    // joining the customer threads
		if(pthread_join(customer_thread[i], NULL) != 0){
			printf("Error: failed to join threads.\n");
			exit(1);
		}
	}
  // destroy mutex and convars here
	for(int i = 0; i < 6; i++){
		if(i < 5 && pthread_mutex_destroy(&mutex[i]) != 0){
			printf("Error: failed to destroy mutex.\n");
			exit(1);
		}
		if(pthread_cond_destroy(&convar[i]) != 0){
			printf("Error: failed to destroy convar.\n");
			exit(1);
		}
	}
	printf("The average waiting time for all customers in the system is: %.2f seconds.\n", (waiting_time[0]+waiting_time[1])/line_length [2]);
	printf("The average waiting time for all business-class customers is: %.2f seconds.\n", waiting_time[1]/line_length [1]);
	printf("The average waiting time for all economy-class customers is: %.2f seocnds.\n", waiting_time[0]/line_length [0]);
	free(queue[0]);
	free(queue[1]);
  free(customers);
	return 0;
}


// created with reference to sample code given on Connex
float getCurrentSimulationTime(){
  struct timeval cur_time;
	double cur_secs, init_secs;
	init_secs = (init_time.tv_sec + (double) init_time.tv_usec / 1000000);
	gettimeofday(&cur_time, NULL);
	cur_secs = (cur_time.tv_sec + (double) cur_time.tv_usec / 1000000);
	return cur_secs - init_secs;
}

// read the customer inmformation from the text file
void readCustomers(char* input_file){
	FILE* fp = fopen(input_file, "r");
	if(fp == NULL || fscanf(fp, "%d", &line_length [2]) < 1){
		printf("Error: failed to read input_file\n");
		exit(1);
	}
	if(line_length [2] < 1){
		printf("Error: invalid number of customers.\n");
		exit(1);
	}
	queue[0] = (customerInfo*) malloc(line_length [2] * sizeof(customerInfo));
	queue[1] = (customerInfo*) malloc(line_length [2] * sizeof(customerInfo));
	customers = (customerInfo*) malloc(line_length [2] * sizeof(customerInfo));
	int j = 0;
	customerInfo customer;
	for(int i = 0; i < line_length [2]; i++){
		if(fscanf(fp, "%d:%d,%d,%d", &customer.customerID, &customer.queueID, &customer.arrival_time, &customer.service_time) != 4){
			printf("Error: customer info is an invalid format\n");
      printf("Customer %d will be skipped. \n", customer.customerID);
			continue;
		}
		if(customer.customerID < 0 || customer.queueID < 0 || customer.queueID > 1 || customer.arrival_time < 0 || customer.service_time < 0){
			printf("Error: customer information is not correct. \n");
      printf("Please check information values for this customer and try them again. \n");
			continue;
		}
		customer.index = j;
		customer.clerk = -1;
		customers[j] = customer;
		j++;
		line_length [customer.queueID]++;
	}
	line_length [2] = j;
	fclose(fp);
}


// store the customer information
// created with reference to the structure of the "customer_entry" method in the provided A2-hint_detailed.c file provided on connex
void* customerEntry(void* cus_info){
	customerInfo* customer = (customerInfo*) cus_info;
	usleep(customer->arrival_time * 100000);
	printf("A customer arrives: customer ID %2d.\n", customer->customerID);
	if(pthread_mutex_lock(&mutex[0]) != 0){
		printf("Error: mutex failed to lock.\n");
		exit(1);
	}
	insertQueue(customer, customer->queueID);
	printf("A customer enters a queue: the queue ID %1d, and length of the queue %2d. \n", customer->queueID, queue_length[customer->queueID]);
	customer->start_serv_time = getCurrentSimulationTime();
	while(customer->clerk == -1){
		if(pthread_cond_wait(&convar[customer->queueID], &mutex[0]) != 0){
			printf("Error: failed to wait.\n");
			exit(1);
		}
	}
	customer->end_serv_time = getCurrentSimulationTime();
	waiting_time[customer->queueID] += customer->end_serv_time - customer->start_serv_time;
	printf("A clerk starts serving a customer: start time %.2f, the customer ID %2d, the clerk ID %1d.\n", customer->end_serv_time, customer->customerID, customer->clerk);
	if(pthread_mutex_unlock(&mutex[0]) != 0){
		printf("Error: mutex failed to unlock.\n");
		exit(1);
	}
	usleep(customer->service_time * 100000);
	printf("A clerk finishes serving a customer: end time %.2f, the customer ID %2d, the clerk ID %1d.\n", getCurrentSimulationTime(), customer->customerID, customer->clerk);
	int clerk = customer->clerk;
	if(pthread_mutex_lock(&mutex[clerk]) != 0){
		printf("Error: mutex failed to lock. \n");
		exit(1);
	}
	NClerks[clerk-1].available = 0;
	if(pthread_cond_signal(&convar[clerk+1]) != 0){
		printf("Error: failed to signal convar. \n");
		exit(1);
	}
	if(pthread_mutex_unlock(&mutex[clerk]) != 0){
		printf("Error: mutex failed to unlock. \n");
		exit(1);
	}
	return NULL;
}

// store the clerk information
// // created with reference to the structure of the "clerk_entry" method in the provided A2-hint_detailed.c file provided on connex
void* clerkEntry(void* clerk_num){
	clerkInfo* customer = (clerkInfo*) clerk_num;
	while(1){
		if(pthread_mutex_lock(&mutex[0]) != 0){
			printf("Error: mutex failed to lock. \n");
			exit(1);
		}
		int queue_index = 1;
		if(queue_length[queue_index]<=0){
			queue_index = 0;
		}
		if(queue_length[queue_index]>0){
			int customer_index = popQueue(queue_index);
			customers[customer_index].clerk = customer->id;
			NClerks[customer->id-1].available = 1;
			if(pthread_cond_broadcast(&convar[queue_index])!=0){
				printf("Error: failed to broadcast convar. \n");
				exit(1);
			}
			if(pthread_mutex_unlock(&mutex[0])!=0){
				printf("Error: failed to unlock mutex. \n");
				exit(1);
			}
		} else{
			if(pthread_mutex_unlock(&mutex[0])!=0){
				printf("Error: mutex failed to unlock. \n");
				exit(1);
			}
			usleep(250);
		}
		if(pthread_mutex_lock(&mutex[customer->id])!=0){
			exit(1);
		}
		if(NClerks[customer->id-1].available){
			if(pthread_cond_wait(&convar[customer->id+1], &mutex[customer->id])!=0){
				printf("Error: failed to wait. \n");
				exit(1);
			}
		}
		if(pthread_mutex_unlock(&mutex[customer->id])!=0){
			printf("Error: mutex failed to unlock. \n");
			exit(1);
		}
	}
	return NULL;
}

//////// Queue Operations //////////

void insertQueue(customerInfo* customer, int i){
	queue[i][queue_length[i]] = *customer;
	queue_length[i]++;
}

int popQueue(int i){
  int cus_index = queue[i][0].index;
  for(int j = 0; j < queue_length[i]-1; j++){
    queue[i][j] = queue[i][j+1];
  }
  queue_length[i]--;
  return cus_index;
}
