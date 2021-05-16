#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* TYPEDEF */

typedef char * String;

typedef struct Student {
    unsigned int id;
    String first_name;
    String last_name;
    String department;
    float gpa;
} Student;

typedef struct Node {
    Student * student;
    struct Node * next;
    struct Node * prev;
} Node;

typedef struct List {
    Node * head;
    Node * tail;
} List;

/* LINKED LIST FUNCTIONS */

Node * create_node(unsigned int id, String firstname, String lastname, String department, float gpa){
    // Create memory for the Node
    Node * node = malloc(sizeof(Node));

    if (node != NULL) {
        // Create memory for the Student
        Student * student = malloc(sizeof(Student));

        if (student != NULL){
            node->student = student;

            node->student->id = id;
            node->student->first_name = strdup(firstname);
            node->student->last_name = strdup(lastname);
            node->student->department = strdup(department);
            node->student->gpa = gpa;
        } else {
            printf("ERROR: Couldn't allocate memory for student with id %d\n", id);
            exit(-1);
        }
        
        node->next = NULL;
        node->prev = NULL;
    } else {
        printf("ERROR: Couldn't allocate memory for node with student id %d\n", id);
        exit(-1);
    }

    return node;
}

Node * copy(Node * node){
    return create_node(node->student->id,  
                       node->student->first_name,
                       node->student->last_name,
                       node->student->department,
                       node->student->gpa);
}

List * create_list(){
    List * list = malloc(sizeof(List));
    
    if (list != NULL){
        list->head = NULL;
        list->tail = NULL;
    } else {
        printf("Error: Couldn't allocate memory for list\n");
        exit(-1);
    }

    return list;
}

void destroy_list(List * list){    
    Node * node = list->head;

    while (node != NULL){
        // Free node's student
        free(node->student->first_name);
        free(node->student->last_name);
        free(node->student->department);
        free(node->student);
        
        Node * old_node = node;
        node = node->next;

        free(old_node);
    }

    free(list);
}

void add_to(List * list, Node * node){
    if (node != NULL && list != NULL){
        // If list is empty
        if (list->head == NULL && list->tail == NULL){
            list->head = node;
            list->tail = node;
        } else {
            // Insert into end
            list->tail->next = node;
            node->prev = list->tail;
            list->tail = node;
        }
    } else {
        printf("ERROR: Attempted add_to with empty list or with NULL node\n");
        exit(-1);
    }
}

Node * find_greater_in(List * list, int id){
    if (list != NULL){
        Node * node = list->head;

        while (node != NULL){
            if (node->student->id > id){
                return node;
            }

            node = node->next;
        }

        // Return NULL if id not found
        return NULL;
    } else {
        printf("ERROR: Attempted find_greater_in with empty list\n");
        exit(-1);
    }
}

void print(List * list){
    Node * node = list->head;

    while (node != NULL){
        printf("Student %d: %s %s, %s, %.2f GPA\n", 
            node->student->id,
            node->student->first_name,
            node->student->last_name,
            node->student->department,
            node->student->gpa);

        node = node->next;
    }
}

/* I/O FUNCTIONS */

List * read(String file_name){
    FILE * file = fopen(file_name, "r");
    
    if (file != NULL){
        List * list = create_list();

        Node * node;
        unsigned int id;
        String firstname, lastname, department;
        float gpa;

        while(fscanf(file, "%d %ms %ms %ms %f", &id, &firstname, &lastname, &department, &gpa) != EOF){
            node = create_node(id, firstname, lastname, department, gpa);
            add_to(list, node);

            // Cleanup memory
            free(firstname); 
            free(lastname); 
            free(department);
        }

        fclose(file);
        
        return list;
    } else {
        printf("ERROR: File not found. %s does not exist\n", file_name);
        exit(-1);
    }
}

void write_to(String file_name, List * list){
    FILE * file = fopen(file_name, "w+");

    if (list != NULL){
        Node * node = list->head;
        
        while (node != NULL){
            fprintf(file, "%d;%s;%s;%s;%.2f\n",
                node->student->id,
                node->student->first_name,
                node->student->last_name,
                node->student->department,
                node->student->gpa);

            node = node->next;
        }

        fclose(file);
    } else {
        printf("ERROR: Couldn't write_to file; passed list was null\n");
        exit(-1);
    }
}

/* SORTING */

// Insertion sort
void sort(List * list){
    if (list != NULL){
        // Split the list into two parts...
        // ...an unsorted list, starting at the 2nd element...
        Node * unsorted_head = list->head->next;
        // ...and a sorted list, starting at the 1st element
        Node * sorted_tail = list->head;
        // Break sorted_tail->next since it should eventually be the tail
        sorted_tail->next = NULL;

        while (unsorted_head != NULL){
            Node * curr_node = unsorted_head;

            // Get next node
            unsorted_head = unsorted_head->next;

            if (curr_node->student->id < list->head->student->id){
                // Make curr_node the list->head
                curr_node->prev = NULL;
                curr_node->next = list->head;
                list->head->prev = curr_node;
                list->head = curr_node;
            } else if (curr_node->student->id > sorted_tail->student->id){
                // Make curr_node the sorted_tail
                curr_node->next = NULL;
                curr_node->prev = sorted_tail;
                sorted_tail->next = curr_node;
                sorted_tail = curr_node;
            } else {
                // Search for where to put the node
                Node * greater_node = find_greater_in(list, curr_node->student->id);

                // Insert in front of found node
                curr_node->next = greater_node;
                curr_node->prev = greater_node->prev;
                greater_node->prev->next = curr_node;
                greater_node->prev = curr_node;
            }
        }
    } else {
        printf("ERROR: Couldn't sort NULL list\n");
        exit(-1);
    }
}

List * merge(List * list_1, List * list_2){
    if (list_1 != NULL && list_2 != NULL){
        // List that will contain the merged items
        List * merged_list = create_list();

        // Node to be inserted into the merged list
        Node * new_node;

        // Starting place for both lists
        Node * node_1 = list_1->head;
        Node * node_2 = list_2->head;
        
        // Increment through both nodes
        while (node_1 != NULL && node_2 != NULL){
            if (node_1->student->id < node_2->student->id){
                // List 1 had smaller node
                new_node = copy(node_1);
                node_1 = node_1->next;
            } else {
                // List 2 had smaller node
                new_node = copy(node_2);
                node_2 = node_2->next;
            }

            add_to(merged_list, new_node);
        }

        // If list_1 isn't empty, but list_2 is
        while (node_1 != NULL){
            new_node = copy(node_1);
            add_to(merged_list, new_node);
            node_1 = node_1->next;
        }

        // If list_2 isn't empty, but list_1 is
        while (node_2 != NULL){
            new_node = copy(node_2);
            add_to(merged_list, new_node);
            node_2 = node_2->next;
        }

        return merged_list;
    } else {
        printf("ERROR: Can't merge lists; one or both was NULL\n");
        exit(-1);
    }
}

/* MAIN */

int main(int argc, String argv[]){
    int rc = 0;
    
    if (argc == 4){
        // Get file names from command line arguments
        String input_file_1 = strdup(argv[1]);
        String input_file_2 = strdup(argv[2]);
        String output_file = strdup(argv[3]);

        // Create lists from files
        List * list_1 = read(input_file_1);
        free(input_file_1);
        
        List * list_2 = read(input_file_2);
        free(input_file_2);

        // Insertion sort lists
        sort(list_1);      
        sort(list_2);

        // Merge lists together
        List * list_out = merge(list_1, list_2);

        // Output final list to file
        write_to(output_file, list_out);
        free(output_file);

        // Free memory
        destroy_list(list_1);
        destroy_list(list_2);
        destroy_list(list_out);
    } else {
        printf("Invalid arguments given: %d arguments.\nFollow format: ./mergestudents input1.txt input2.txt output.txt\n", argc);
        
        rc = -1;
    }

    return rc;
}
