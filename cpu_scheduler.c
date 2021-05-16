#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef char * string;
typedef unsigned int uint;

typedef enum bool {
    FALSE = 0,
    TRUE = 1
} bool;

typedef enum algorithm {
    INVALID_ARGUMENT = -1,
    SJF = 0,
    SRTF = 1
} algorithm;

typedef enum sort {
    BY_BURST_TIME = 0,
    BY_FINISH_TIME = 1
} sort;

typedef struct process {
    uint id;
    uint arrival_time;
    uint finish_time;
    uint waiting_time;
    uint burst_time;
    
    struct process * next;
} process;

typedef struct queue {
    uint size;
    process * head;
} queue;

/* LINKED queue */

process * create_process(uint id, uint arrival_time, uint burst_time){
    process * new_process = malloc(sizeof(process));

    if (new_process != NULL){
        new_process->id = id;
        new_process->arrival_time = arrival_time;
        new_process->burst_time = burst_time;

        new_process->finish_time = 0;
        new_process->waiting_time = 0;

        new_process->next = NULL;

        return new_process;
    } else {
        printf("ERROR: Could not allocate memory for process %d\n", id);
        exit(-1);
    }
}

queue * create_queue(){
    queue * new_queue = malloc(sizeof(queue));

    if (new_queue != NULL){
        new_queue->size = 0;

        new_queue->head = NULL;

        return new_queue;
    } else {
        printf("ERROR: Could not allocate memory for queue\n");
        exit(-1);
    }
}

void add_to(queue * queue, process * new_process){
    if (queue != NULL && new_process != NULL){
        if (queue->head == NULL){ // Insert first element
            queue->head = new_process;
        } else { // Insert into end
            // Start at head
            process * curr_process = queue->head;

            // Navigate to last process in queue
            while (curr_process->next != NULL){
                curr_process = curr_process->next;
            }

            // Insert at end
            curr_process->next = new_process;
        }

        // Increment size
        queue->size++;
    } else {
        printf("ERROR: Attempted add_to with NULL queue and/or NULL process\n");
        exit(-1);
    }
}

void add_sorted_to(queue * queue, process * new_process, sort sort_method){
    if (queue != NULL && new_process != NULL){
        if (queue->head == NULL){
            // If first process, add to end
            queue->head = new_process;
        } else if ((sort_method == BY_BURST_TIME && queue->head->burst_time > new_process->burst_time)
                    || (sort_method == BY_FINISH_TIME && queue->head->finish_time > new_process->finish_time)){
            // If less than head, add as head
            new_process->next = queue->head;
            queue->head = new_process;
        } else {
            // Start at head
            process * curr_process = queue->head;
            process * prev_process;

            // Navigate to a process before a greater process in queue
            while (curr_process != NULL) {
                prev_process = curr_process;
                curr_process = curr_process->next;

                if (curr_process != NULL){
                    if (sort_method == BY_BURST_TIME
                        && curr_process->burst_time > new_process->burst_time){
                        // Stop navigating if greater burst time is found
                        break;
                    } else if (sort_method == BY_FINISH_TIME
                               && curr_process->finish_time > new_process->finish_time){
                        // Stop navigating if greater finish time is found
                        break;
                    }
                }
            }

            // Insert after prev process
            prev_process->next = new_process;
            new_process->next = curr_process;
        }

        // Increment size
        queue->size++;
    } else {
        printf("ERROR: Attempted add_sorted_to with NULL queue and/or NULL process\n");
        exit(-1);
    }
}

void remove_from(queue * queue){
    if (queue != NULL){
        // Get first process
        process * old_head = queue->head;

        // Update queue
        queue->head = old_head->next;
        
        // Isolate removed process
        old_head->next = NULL;

        // Decrement size
        queue->size--;
    } else {
        printf("ERROR: Attempted remove_from with NULL queue\n");
        exit(-1); 
    }
}   

void destroy(queue * queue){    
    process * curr_process = queue->head;

    while (curr_process != NULL){
        // Store old process for freeing before iterating
        process * old_process = curr_process;
        curr_process = curr_process->next;

        free(old_process);
    }

    free(queue);
}

/* IO */

algorithm parse_algorithm_from(string argument) {    
    if (strcmp(argument, "SJF") == 0){
        return SJF;
    } else if (strcmp(argument, "SRTF") == 0){
        return SRTF;
    } else {
        return INVALID_ARGUMENT;
    }
}

queue * read_until(int depth, string file_name) {
    FILE * file = fopen(file_name, "r");

    if (file != NULL) {
        queue * result = create_queue();

        uint id, arrival_time, burst_time;

        // Scan each line of file
        // Stop at depth limit if it's specified 
        for(int i = 0; fscanf(file, "%d %d %d", &id, &arrival_time, &burst_time) != EOF && (depth == -1 || i < depth); i++){
            process * process = create_process(id, arrival_time, burst_time);
            add_to(result, process);
        }

        fclose(file);

        return result;        
    } else {
        printf("ERROR: File not found; %s does not exist\n", file_name);
        exit(-1);
    }
}

void write_to(string file_name, queue * queue) {
    FILE * file = fopen(file_name, "w+");

    if (queue != NULL) {
        process * curr_process = queue->head;

        // Write each line of file
        while (curr_process != NULL){
            fprintf(file, "%d %d %d %d\n",
                curr_process->id, 
                curr_process->arrival_time,
                curr_process->finish_time,
                curr_process->waiting_time);

            curr_process = curr_process->next;
        }

        fclose(file);
    } else {
        printf("ERROR: Could not write NULL queue to output\n");
        exit(-1);
    }
}

/* SCHEDULE LOGIC */

queue * sjf_schedule(queue * ready_queue){
    queue * result = create_queue(), * scheduling_queue = create_queue();
    uint curr_time = ready_queue->head->arrival_time; // Time starts at arrival of first item
    process * curr_process;
    
    // Iterate through each item in both queues
    while (ready_queue->size > 0 || scheduling_queue->size > 0) {
        // Schedule any remaining items in ready queue
        if (ready_queue->size > 0){
            // Go to beginning of ready queue
            curr_process = ready_queue->head;

            // Simulate scheduling of each process
            while (curr_process != NULL){
                process * next_process = curr_process->next;

                // Add all process that arrived at/before the current time to the scheduling queue
                if (curr_process->arrival_time <= curr_time){
                    remove_from(ready_queue);
                    add_sorted_to(scheduling_queue, curr_process, BY_BURST_TIME);
                } else {
                    // Stop if the process arrives in the future
                    break;
                }

                // Iterate to next process in ready queue
                curr_process = next_process;
            }
        }

        // Simulate any remaining items in scheduling queue
        if (scheduling_queue->size > 0){
            // Go to beginning of scheduling queue
            curr_process = scheduling_queue->head;

            // Simulate process
            curr_process->finish_time = curr_time + curr_process->burst_time;
            curr_process->waiting_time = curr_process->finish_time - curr_process->arrival_time - curr_process->burst_time;

            // Increment time
            curr_time += curr_process->burst_time;

            // Add process to results
            remove_from(scheduling_queue);
            add_sorted_to(result, curr_process, BY_FINISH_TIME);
        }
    }

    // Cleanup memory
    destroy(scheduling_queue);

    return result;
}   

queue * srtf_schedule(queue * ready_queue){
    queue * result = create_queue(), * scheduling_queue = create_queue();
    uint curr_time = ready_queue->head->arrival_time; // Time starts at arrival of first item
    process * curr_process;

    while (ready_queue->size > 0 || scheduling_queue->size > 0){    
        // Schedule any remaining items in ready queue
        if (ready_queue->size > 0){
            // Go to beginning of ready queue
            curr_process = ready_queue->head;

            // Simulate scheduling of each process
            while (curr_process != NULL){
                process * next_process = curr_process->next;

                // Add all process that arrived at/before the current time to the scheduling queue
                if (curr_process->arrival_time <= curr_time){
                    remove_from(ready_queue);
                    add_sorted_to(scheduling_queue, curr_process, BY_BURST_TIME);
                } else {
                    // Stop if the process arrives in the future
                    break;
                }

                // Iterate to next process in ready queue
                curr_process = next_process;
            }
        }
        
        // Simulate any remaining items in scheduling queue
        if (scheduling_queue->size > 0){
            // Go to beginning of scheduling queue
            curr_process = scheduling_queue->head;

            // Update wait time
            // If process->wait != 0, then the process was preempted, and a different calculation is needed
            curr_process->waiting_time = curr_process->waiting_time == 0 ? curr_time - curr_process->arrival_time : curr_time - curr_process->waiting_time;
            
            // When beginning simulation, there's no preemption yet
            bool preempted = FALSE;

            // Simulate 1 ms at a time until done or preempted
            while (curr_process->burst_time > 0 && preempted == FALSE){
                // Simulate 1 ms of process
                curr_process->burst_time--;
                curr_time++;
                
                if (curr_process->burst_time > 0){
                    // Process isn't finished yet                
                    // Check if any new processes arrived that have a lesser burst time
                    process * ready_process = ready_queue->head;
                    while (ready_process != NULL) {
                        if (ready_process->arrival_time > curr_time){
                            // In future, so stop checking
                            break;
                        } else if (ready_process->burst_time < curr_process->burst_time){ 
                            // Pre-empted, so stop checking
                            curr_process->waiting_time = curr_time - curr_process->waiting_time; // Update wait time for use in calculation
                            preempted = TRUE;
                            break;
                        }
                    
                        // Go to next process in ready queue
                        ready_process = ready_process->next;
                    }
                } else {
                    // Update process as finished
                    curr_process->finish_time = curr_time;

                    // Add process to results
                    remove_from(scheduling_queue);
                    add_sorted_to(result, curr_process, BY_FINISH_TIME);
                }         
            }
        }
    }

    // Cleanup memory
    destroy(scheduling_queue);

    return result;
}

/* MAIN */

int main(int argc, string argv[]){
    if (argc == 4 || argc == 5){
        // Parse arguments from command line
        string input = strdup(argv[1]);
        string output = strdup(argv[2]);
        algorithm algorithm = parse_algorithm_from(argv[3]);
        int depth = argc == 5
                    ? atoi(argv[4])
                    : -1;

        // Read file to queue
        queue * ready_queue = read_until(depth, input);

        // Schedule
        queue * result;
        
        switch(algorithm){
            case SJF:
                result = sjf_schedule(ready_queue);
                break;
            case SRTF:
                result = srtf_schedule(ready_queue);
                break;
            default:
                printf("ERROR: Improper algorithm entered; %s not valid\n", argv[3]);
                exit(-1);
        }

        // Write result to output
        write_to(output, result);

        // Calculate metrics
        double avg_wait_time = 0, avg_turnaround_time = 0;

        process * p = result->head;
        while (p != NULL){
            avg_wait_time += p->waiting_time;
            avg_turnaround_time += p->waiting_time + p->burst_time;

            p = p->next;
        }

        avg_wait_time /= result->size;
        avg_turnaround_time /= result->size;

        // Log results
        printf("scheduled \"%s\" using %s", input, algorithm == SJF ? "SJF" : "SRTF");
        if (depth != -1) printf(" with depth %d", depth);
        printf("\n....avg wait time = %.03f ms\n....avg turn time = %.03f ms\n", avg_wait_time, avg_turnaround_time);

        // Free memory
        free(input);
        free(output);
        destroy(ready_queue);
        destroy(result);
    } else {
        printf("Invalid call. Follow format:\n\t./cpu_scheduler <INPUT_FILE> <OUTPUT_FILE> <ALGORITHM   > [LIMIT]\n\t<ALGORITHM> can be SRTF or SJF\n");
    }

    return 0;
}
