#include <stdio.h>
#include <stdlib.h>

// Global variables for system parameters
int PROCESS_ID = 1; // Used to assign a unique ID to each new process
int INTERRUPT_ID = 1001; // Used to assign a unique ID to each new I/O request
int SYSTEM_TIME = 0; // Indicates the current system time
int SCHEDULING_POLICY; // Indicates the selected scheduling policy
int TIME_QUANTUM; // Required for Round Robin Scheduling policy
int IS_CPU_IDLE; // Indicates the current CPU state (1 for idle)
FILE *fs; // File pointer for input/output


// Structure representing a process control block (PCB)
typedef struct node {
    struct node *next; // Pointer to the next PCB in the list
    int processId; // Unique ID of the process
    int arrivalTime; // Time at which the process arrives in the system
    int burstTime; // CPU time required for the process to complete its execution
    int completionTime; // Time at which the process completes or terminates
    int waitingTime; // Time the process spends waiting in the ready queue
    int turnAroundTime; // Total time taken by the process from arrival to completion
    int remainingTime; // Remaining CPU time needed by the process to complete execution
    int responseTime; // Time taken from arrival until the process starts execution
    int isTerminated; // Flag indicating whether the process has terminated (1 for true, 0 for false)
    int firstExecution; // Flag indicating if the process has been executed at least once
    int blockedTime; // Time for which the process is blocked for I/O
} node;


// Structure representing a list of processes
typedef struct {
    node *head; // Pointer to the first process in the list
} list;


// Structure representing a node in the priority queue
typedef struct queueNode {
    node *ele; // Pointer to the PCB stored in the node
    struct queueNode *next; // Pointer to the next node in the priority queue
} queueNode;


// Structure representing a priority queue of processes
typedef struct {
    queueNode *head; // Pointer to the first node in the priority queue
} priorityQueue;


// Function prototypes
void insert(list *l, int arrivalTime, int burstTime, int isInterrupt);
void display(list *l);
void enqueue(priorityQueue *q, node *data);
void priorityEnqueue(priorityQueue *q, node *data);
void priorityBlock(priorityQueue *bq, node *data);
void deduct(priorityQueue *bq);
node* dequeue(priorityQueue *q);
int isEmpty(priorityQueue *q);
void input(list *l);
void setPolicy();
void fcfs(list *l, priorityQueue *rq, priorityQueue *bq);
void sjf(list *l, priorityQueue *rq, priorityQueue *bq);
void sjfPreempt(list *l, priorityQueue *rq, priorityQueue *bq);
void roundRobin(list *l, priorityQueue *rq, priorityQueue *bq);
void schedule(list *l, priorityQueue *rq, priorityQueue *bq);
void output(list *l);


// Main function to initialize the program and execute scheduling
int main() {
    // Create lists and queues for processes
    list *l = (list*)malloc(sizeof(list));
    l->head = NULL;
    priorityQueue *rq = (priorityQueue*)malloc(sizeof(priorityQueue));
    rq->head = NULL;
    priorityQueue *bq = (priorityQueue*)malloc(sizeof(priorityQueue));
    bq->head = NULL;

    // Open input file
    fs = fopen("input.txt", "r");
    if(fs == NULL) {
        printf("Error: Unable to open the file.\n");
        return -1;
    }

    // Read input from file
    setPolicy();
    input(l);

    // Close input file
    fclose(fs);

    // Execute scheduling algorithm
    schedule(l, rq, bq);

    // Write output to file
    output(l);

    // Free memory
    free(l);
    free(rq);
    free(bq);

    return 0;
}


// Function to insert a process into the process list
void insert(list *l, int arrivalTime, int burstTime, int isInterrupt) {
	node *ref, *ptr;
	ref = (node*)malloc(sizeof(node)); // Allocate memory for the new process

	// Determine the ID for the process based on whether it's an interrupt or a regular process
    if(isInterrupt) ref->processId = INTERRUPT_ID++;
	else ref->processId = PROCESS_ID++;

	// Initialize process attributes
	ref->burstTime = burstTime;
	ref->remainingTime = burstTime;
	ref->arrivalTime = arrivalTime;
	ref->responseTime = 0;
	ref->isTerminated = 0;
	ref->firstExecution = 0;
	ref->blockedTime = 0;
	ref->next = NULL;

	// Insert the new process into the list
	if(l->head == NULL) {
		l->head = ref;
		return;
	}

	ptr = l->head;
	while (ptr->next != NULL) ptr = ptr->next;
	ptr->next = ref;
}


// Function to display process information
void display(list *l) {
	node *ref = l->head;

	printf("\n");
	while(ref) {
		printf("Process %d:\nWaiting Time: %d\nTurn Around Time: %d\n", ref->processId, ref->waitingTime, ref->turnAroundTime);
		ref=ref->next;
	}
}


// Function to enqueue a process into the priority queue
void enqueue(priorityQueue *q, node *data) {
	queueNode *ptr, *ref;
	ptr = (queueNode*)malloc(sizeof(queueNode));
	ptr->ele = data;
	ptr->next = NULL;

	if (q->head == NULL){
		q->head = ptr;
		return;
	}

	ref = q->head;
	while(ref->next != NULL) ref = ref->next;
	ref->next = ptr;
}


// Function to enqueue a process into the priority queue based on remaining time
void priorityEnqueue(priorityQueue *q, node *data) {
	queueNode *ptr, *ref, *temp;
	ptr = (queueNode*)malloc(sizeof(queueNode));
	ptr->ele = data;
	ptr->next = NULL;

	if(q->head == NULL) {
		q->head = ptr;
		return;
	}

	ref = q->head;
	temp = q->head;
	
	// Inserting the process in ascending order of remaining time
	if(ref->ele->remainingTime > data->remainingTime) {
		q->head = ptr;
		ptr->next = ref;
		return;
	}

    if(ref->next == NULL) {
        ref->next = ptr;
        return;
    }

	ref = q->head->next;

	while(ref->next != NULL) {
		if(ref->ele->remainingTime > data->remainingTime) {
			temp->next = ptr;
			ptr->next = ref;
			return;
		}

		temp = temp->next;
		ref = ref->next;
	}

	if(ref->ele->remainingTime > data->remainingTime) {
		temp->next = ptr;
		ptr->next = ref;
		return;
	}

	ref->next = ptr;
}


// Function to insert a process into the blocked queue based on blocked time
void priorityBlock(priorityQueue *bq, node *data) {
	queueNode *ptr, *ref, *temp;
	ptr = (queueNode*)malloc(sizeof(queueNode));
	ptr->ele = data;
	ptr->next = NULL;

	if(!bq->head) {
		bq->head = ptr;
		return;
	}

	ref = bq->head;
	temp = bq->head;

	// Inserting the process in ascending order of blocked time
	if(ref->ele->blockedTime > data->blockedTime) {
		bq->head = ptr;
		ptr->next = ref;
		return;
	}

    if(ref->next == NULL) {
        ref->next = ptr;
        return;
    }

	ref = bq->head->next;

	while(ref->next) {
		if(ref->ele->blockedTime > data->blockedTime){
			temp->next = ptr;
			ptr->next = ref;
			return;
		}
		temp = temp->next;
		ref = ref->next;
	}

	if(ref->ele->blockedTime > data->blockedTime) {
		temp->next = ptr;
		ptr->next = ref;
		return;
	}

	ref->next = ptr;
}


// Function to decrement blocked time for processes in the blocked queue
void deduct(priorityQueue *bq) {
	queueNode *ref;
	ref = bq->head;

	while(ref) {
		ref->ele->blockedTime--;
		ref = ref->next;
	}
}


// Function to dequeue a process from the priority queue
node* dequeue(priorityQueue *q) {
	node *ref = q->head->ele;
	q->head = q->head->next;
	return ref;
}


// Function to check if the priority queue is empty
int isEmpty(priorityQueue *q) {
	if(q->head == NULL) return 1;
	return 0;
}


// Function to read input from file and populate the process list
void input(list *l) {
    int burstTime, arrivalTime, isInterrupt;
	while(fscanf(fs, "%d%d%d", &arrivalTime, &burstTime, &isInterrupt) != EOF){
        insert(l, arrivalTime, burstTime, isInterrupt);
    }
}


// Function to set the scheduling policy based on input from file
void setPolicy() {
	fscanf(fs, "%d", &SCHEDULING_POLICY);
    if(SCHEDULING_POLICY == 4) fscanf(fs, "%d", &TIME_QUANTUM);
}


// Function to execute First Come First Serve (FCFS) scheduling algorithm
void fcfs(list *l, priorityQueue *rq, priorityQueue *bq) {
	node *data, *ref = NULL, *temp = NULL;
	data = l->head;
	IS_CPU_IDLE = 1;

	while(data != NULL || ref != NULL || isEmpty(rq) == 0 || isEmpty(bq) == 0) {
		if(ref != NULL) ref->remainingTime--;

		if(isEmpty(bq) == 0) deduct(bq);

		while(data != NULL && IS_CPU_IDLE == 1 && isEmpty(rq) == 1) {
			if(data->processId > 1000) data = data->next;
			else if(data != NULL && data->arrivalTime == SYSTEM_TIME) {
				IS_CPU_IDLE = 0;
				ref = data;
				data = data->next;
			}
			else {
				SYSTEM_TIME++;
			}
		}

		while(data != NULL && SYSTEM_TIME == data->arrivalTime && IS_CPU_IDLE == 0) {
			if(data->processId > 1000) {
				if(ref->remainingTime != 0) {
					ref->blockedTime = data->burstTime;
					priorityBlock(bq, ref);
					printf("Process %d was blocked at %d!\n", ref->processId, SYSTEM_TIME);
					IS_CPU_IDLE = 1;
				} 
				else{
					printf("Process %d completed at %d\n", ref->processId, SYSTEM_TIME);
					ref->completionTime = SYSTEM_TIME;
					ref->turnAroundTime = SYSTEM_TIME - ref->arrivalTime;
					ref->isTerminated = 1;
					IS_CPU_IDLE = 1;
					ref = NULL;
				}

				if(isEmpty(rq) == 0) {
					ref = dequeue(rq);
					IS_CPU_IDLE = 0;
				}
				else{
					ref = NULL;
					IS_CPU_IDLE = 1;
				}
			} 
			else enqueue(rq, data);

			data = data->next;
		}

		if(ref != NULL && ref->remainingTime == 0){
			printf("Process %d completed at %d\n", ref->processId, SYSTEM_TIME);
			ref->completionTime = SYSTEM_TIME;
			ref->turnAroundTime = SYSTEM_TIME - ref->arrivalTime;
			ref->isTerminated = 1;
			IS_CPU_IDLE = 1;
			ref = NULL;
		}

		if(IS_CPU_IDLE == 1 && isEmpty(rq) == 0){
			ref = dequeue(rq);
			IS_CPU_IDLE = 0;
		}

		while(isEmpty(bq) == 0 && bq->head->ele->blockedTime == 0){
			temp = dequeue(bq);
			printf("Process %d was unblocked at %d!\n", temp->processId, SYSTEM_TIME);
			enqueue(rq, temp);
		}

		SYSTEM_TIME++;
	}
}


// Function to execute Shortest Job First (SJF) scheduling algorithm
void sjf(list *l, priorityQueue *rq, priorityQueue *bq) {
	node *data, *ref = NULL, *temp = NULL;
	data = l->head;
	IS_CPU_IDLE = 1;

	while(data != NULL || ref != NULL || isEmpty(rq) == 0 || isEmpty(bq) == 0) {
		if(ref != NULL) ref->remainingTime--;

		if(isEmpty(bq) == 0) deduct(bq);

		while(data != NULL && IS_CPU_IDLE == 1 && isEmpty(rq) == 1) {
			if(data->processId > 1000) data = data->next;
			else if(data != NULL && data->arrivalTime == SYSTEM_TIME) {
				IS_CPU_IDLE = 0;
				ref = data;
				data = data->next;
			}
			else {
				SYSTEM_TIME++;
			}
		}

		while(data != NULL && SYSTEM_TIME == data->arrivalTime && IS_CPU_IDLE == 0) {
			if(data->processId > 1000) {
				if(ref->remainingTime != 0) {
					ref->blockedTime = data->burstTime;
					priorityBlock(bq, ref);
					printf("Process %d was blocked at %d!\n", ref->processId, SYSTEM_TIME);
					IS_CPU_IDLE = 1;
				} 
				else {
					printf("Process %d completed at %d\n", ref->processId, SYSTEM_TIME);
					ref->completionTime = SYSTEM_TIME;
					ref->turnAroundTime = SYSTEM_TIME - ref->arrivalTime;
					ref->isTerminated = 1;
					IS_CPU_IDLE = 1;
					ref = NULL;
				}

				if(isEmpty(rq) == 0) {
					ref = dequeue(rq);
					IS_CPU_IDLE = 0;
				}
				else {
					ref = NULL;
					IS_CPU_IDLE = 1;
				}
			} 
			else priorityEnqueue(rq, data);

			data = data->next;
		}

		if(ref != NULL && ref->remainingTime == 0) {
			printf("Process %d completed at %d\n", ref->processId, SYSTEM_TIME);
			ref->completionTime = SYSTEM_TIME;
			ref->turnAroundTime = SYSTEM_TIME - ref->arrivalTime;
			ref->isTerminated = 1;
			IS_CPU_IDLE = 1;
			ref = NULL;
		}

		if(IS_CPU_IDLE == 1 && isEmpty(rq) == 0) {
			ref = dequeue(rq);
			IS_CPU_IDLE = 0;
		}

		while(isEmpty(bq) == 0 && bq->head->ele->blockedTime == 0){
			temp = dequeue(bq);
			printf("Process %d was unblocked at %d!\n", temp->processId, SYSTEM_TIME);
			priorityEnqueue(rq, temp);
		}

		SYSTEM_TIME++;
	}
}


// Function to execute Preemptive Shortest Job First (SJF) / Shortest Remaining Time Next (SRTN) scheduling algorithm
void sjfPreempt(list *l, priorityQueue *rq, priorityQueue *bq) {
	node *data, *ref = NULL, *temp = NULL;
	data = l->head;
	IS_CPU_IDLE = 1;

	while(data != NULL || ref != NULL || isEmpty(rq) == 0 || isEmpty(bq) == 0){
		if(ref != NULL) ref->remainingTime--;

		if(isEmpty(bq) == 0) deduct(bq);

		while(data != NULL && IS_CPU_IDLE == 1 && isEmpty(rq) == 1) {
			if(data->processId > 1000) data = data->next;
			else if(data != NULL && data->arrivalTime == SYSTEM_TIME) {
				IS_CPU_IDLE = 0;
				ref = data;
				data = data->next;
			}
			else {
				SYSTEM_TIME++;
			}
		}

		while(data != NULL && SYSTEM_TIME == data->arrivalTime && IS_CPU_IDLE == 0) {
			if(data->processId > 1000) {
				if(ref->remainingTime != 0) {
					ref->blockedTime = data->burstTime;
					priorityBlock(bq, ref);
					printf("Process %d was blocked at %d!\n", ref->processId, SYSTEM_TIME);
					IS_CPU_IDLE = 1;
				} 
				else {
					printf("Process %d completed at %d\n", ref->processId, SYSTEM_TIME);
					ref->completionTime = SYSTEM_TIME;
					ref->turnAroundTime = SYSTEM_TIME - ref->arrivalTime;
					ref->isTerminated = 1;
					IS_CPU_IDLE = 1;
					ref = NULL;
				}

				if(isEmpty(rq) == 0) {
					ref = dequeue(rq);
					IS_CPU_IDLE = 0;
				}
				else {
					ref = NULL;
					IS_CPU_IDLE = 1;
				}
			} 
			else {
				priorityEnqueue(rq, data);

				if(ref->remainingTime > rq->head->ele->remainingTime) {
					priorityEnqueue(rq, ref);
					ref = dequeue(rq);
				}
			}

			data = data->next;
		}

		if(ref != NULL && ref->remainingTime == 0) {
			printf("Process %d completed at %d\n", ref->processId, SYSTEM_TIME);
			ref->completionTime = SYSTEM_TIME;
			ref->turnAroundTime = SYSTEM_TIME - ref->arrivalTime;
			ref->isTerminated = 1;
			IS_CPU_IDLE = 1;
			ref = NULL;
		}

		if(IS_CPU_IDLE == 1 && isEmpty(rq) == 0) {
			ref = dequeue(rq);
			IS_CPU_IDLE = 0;
		}

		while(isEmpty(bq) == 0 && bq->head->ele->blockedTime == 0) {
			temp = dequeue(bq);
			printf("Process %d was unblocked at %d!\n", temp->processId, SYSTEM_TIME);
			priorityEnqueue(rq, temp);

			if(ref->remainingTime > rq->head->ele->remainingTime) {
				priorityEnqueue(rq, ref);
				ref = dequeue(rq);
			}
		}

		SYSTEM_TIME++;
	}
}


// Function to execute Round Robin scheduling algorithm
void roundRobin(list *l, priorityQueue *rq, priorityQueue *bq){
	node *data, *ref = NULL, *temp = NULL;
	int quantum = TIME_QUANTUM;
	data = l->head;
	IS_CPU_IDLE = 1;

	while(data != NULL || ref != NULL || isEmpty(rq) == 0 || isEmpty(bq) == 0) {
		if(IS_CPU_IDLE == 0) quantum--;
		
		if(ref != NULL) ref->remainingTime--;

		if(isEmpty(bq) == 0) deduct(bq);

		while(data != NULL && IS_CPU_IDLE == 1 && isEmpty(rq) == 1) {
			if(data->processId > 1000) data = data->next;
			else if(data->arrivalTime == SYSTEM_TIME && data->isTerminated == 0) {
				IS_CPU_IDLE = 0;
				ref = data;
				ref->responseTime = 0;
				data = data->next;
			}
			else {
				SYSTEM_TIME++;
			}
		}

		while(data != NULL && SYSTEM_TIME == data->arrivalTime && IS_CPU_IDLE == 0) {
			data->firstExecution = 1;

			if(data->processId > 1000) {
				if(ref->remainingTime != 0) {
					ref->blockedTime = data->burstTime;
					priorityBlock(bq, ref);
					printf("Process %d was blocked at %d!\n", ref->processId, SYSTEM_TIME);
					IS_CPU_IDLE = 1;
				} 
				else{
					printf("Process %d completed at %d\n", ref->processId, SYSTEM_TIME);
					ref->completionTime = SYSTEM_TIME;
					ref->turnAroundTime = SYSTEM_TIME - ref->arrivalTime;
					ref->isTerminated = 1;
					IS_CPU_IDLE = 1;
					ref = NULL;
				}

				if(isEmpty(rq) == 0) {
					ref = dequeue(rq);
					ref->responseTime = SYSTEM_TIME - ref->arrivalTime;
					ref->firstExecution = 0;
					quantum = TIME_QUANTUM;
					IS_CPU_IDLE = 0;
				}
				else{
					ref = NULL;
					IS_CPU_IDLE = 1;
				}
			} 
			else enqueue(rq, data);
			
			data = data->next;
		}

		if(ref != NULL && ref->remainingTime == 0) {
			printf("Process %d completed at %d\n", ref->processId, SYSTEM_TIME);
			ref->completionTime = SYSTEM_TIME;
			ref->turnAroundTime = SYSTEM_TIME - ref->arrivalTime;
			ref->isTerminated = 1;
			IS_CPU_IDLE = 1;
			ref = NULL;
		}

		if(IS_CPU_IDLE == 1 && isEmpty(rq) == 0) {
			ref = dequeue(rq);

			if(ref != NULL && ref->firstExecution == 1) {
				ref->responseTime = SYSTEM_TIME - ref->arrivalTime;
				ref->firstExecution = 0;
			}

			IS_CPU_IDLE = 0;
			quantum = TIME_QUANTUM;
		}
		else if(quantum == 0 && isEmpty(rq) == 0) {
			enqueue(rq, ref);
			ref = dequeue(rq);

			if(ref != NULL && ref->firstExecution == 1) {
				ref->responseTime = SYSTEM_TIME - ref->arrivalTime;
				ref->firstExecution = 0;
			}

			IS_CPU_IDLE = 0;
			quantum = TIME_QUANTUM;
		}
		else if(quantum == 0 && isEmpty(rq) == 1) quantum = TIME_QUANTUM;

		while(isEmpty(bq) == 0 && bq->head->ele->blockedTime == 0) {
			temp = dequeue(bq);
			printf("Process %d was unblocked at %d!\n", temp->processId, SYSTEM_TIME);
			enqueue(rq, temp);
		}

		SYSTEM_TIME++;
	}
}


// Function to select and execute the appropriate scheduling algorithm
void schedule(list *l, priorityQueue *rq, priorityQueue *bq) {
    switch (SCHEDULING_POLICY) {
        case 1:
            printf("Executing First Come First Serve (FCFS) Scheduling Policy\n");
            fcfs(l, rq, bq);
            break;
        case 2:
            printf("Executing Shortest Job First (SJF) Scheduling Policy\n");
            sjf(l, rq, bq);
            break;
        case 3:
            printf("Executing Preemptive Shortest Job First (SJF) Scheduling Policy\n");
            sjfPreempt(l, rq, bq);
            break;
        case 4:
            printf("Executing Round Robin Scheduling Policy with Time Quantum of %d\n", TIME_QUANTUM);
            roundRobin(l, rq, bq);
            break;
        default:
            printf("Invalid Scheduling Policy Selected!\n");
            break;
    }
}


// Function to write output to a file
void output(list *l) {
	FILE *fs;
	node *ref;
	int count = 0;
	float avgTurnAround = 0, avgWaiting = 0;

	fs = fopen("./output.txt", "w");

	if(fs == NULL) {
		printf("Error! Output file did not open.");
		exit(1);
	}

	ref = l->head;

	switch (SCHEDULING_POLICY) {
        case 1:
            fprintf(fs, "First Come First Serve (FCFS) Scheduling Policy\n\n");
            break;
        case 2:
            fprintf(fs, "Shortest Job First (SJF) Scheduling Policy\n\n");
            break;
        case 3:
            fprintf(fs, "Preemptive Shortest Job First (SJF) Scheduling Policy\n\n");
            break;
        case 4:
            fprintf(fs, "Round Robin Scheduling Policy with Time Quantum of %d\n\n", TIME_QUANTUM);
    }

	fprintf(fs, "Process Id    Arrival Time    Burst Time    Completion Time    Waiting Time    Turn Around Time");

	if(SCHEDULING_POLICY == 4) fprintf(fs, "   Response Time\n");
	else fprintf(fs, "\n");

	while(ref != NULL) {
		if(ref->processId > 1000) {
			ref = ref->next;
			continue;
		}

		ref->waitingTime = ref->turnAroundTime - ref->burstTime;
		fprintf(fs, "%6d%15d%14d%18d%17d%18d", ref->processId, ref->arrivalTime, ref->burstTime, ref->completionTime, ref->waitingTime, ref->turnAroundTime);

		if(SCHEDULING_POLICY == 4) fprintf(fs, "%22d\n", ref->responseTime);
		else fprintf(fs, "\n");

		avgTurnAround += (float) ref->turnAroundTime;
		avgWaiting += (float) ref->waitingTime;
		count++;
		ref = ref->next;
	}

	fprintf(fs, "\nAverage Waiting Time: %.2f\nAverage Turn Around Time: %.2f\n", avgWaiting / count, avgTurnAround / count);
	fclose(fs);
}