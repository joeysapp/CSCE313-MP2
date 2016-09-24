/* 
    File: my_allocator.c

    Author: Joseph Sapp
            Department of Computer Science
            Texas A&M University
    Date  : 9/18/2016

    Modified: 

    This file contains the implementation of the module "MY_ALLOCATOR".

*/

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

    /* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <math.h>
#include "my_allocator.h"

/*--------------------------------------------------------------------------*/
/* DATA STRUCTURES */ 
/*--------------------------------------------------------------------------*/

// 16 Bytes long 
// 8 bytes for next, 4 for size, 1 for free
struct Node {
	struct Node* next;
	int size;
	bool free;
};

/*--------------------------------------------------------------------------*/
/* CONSTANTS */
/*--------------------------------------------------------------------------*/

	Addr main_block;
	Addr main_block_start;

	struct Node** free_list = NULL;

	int size_of_nodes;
	int total_size;
	int free_list_length;
	
/*--------------------------------------------------------------------------*/
/* FORWARDS */
/*--------------------------------------------------------------------------*/

    /* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* FUNCTIONS FOR MODULE MY_ALLOCATOR */
/*--------------------------------------------------------------------------*/

/* Don't forget to implement "init_allocator" and "release_allocator"! */
unsigned int init_allocator(unsigned int _basic_block_size, unsigned int _length){
	
	printf("\nInitializing main memory block!\n");

	// Make sure to initialize with space for nodes
	// This is the size of the basic block/node
	if (_basic_block_size <= sizeof(struct Node)){
		_basic_block_size += sizeof(struct Node);
	}
	size_of_nodes = ceil(log2(_basic_block_size));
	size_of_nodes = pow(2, size_of_nodes);

	// This is the total amount of space we need to allocate
	// (given from length, raised to nearest power of 2.)
	total_size = ceil(log2(_length));
	total_size = pow(2, total_size);

	free_list_length = total_size/size_of_nodes;
	free_list_length = log2(free_list_length) + 1;

	printf("%i, %i, %i\n", size_of_nodes, total_size, free_list_length);

	// Main block (where we will be storing blocks)
	main_block = (Addr)malloc((total_size * sizeof(char)));
	main_block_start = main_block;

	
	// Free list for storing of addresses of blocks

	//free_list = (struct Node**)malloc(free_list_length * sizeof(struct Node));
	
	// I use Calloc() here because otherwise I was getting garbage values
	free_list = (struct Node**)calloc(free_list_length, sizeof(struct Node));

	int to_allocate = total_size;
	int allocate_in = free_list_length - 1;
	// I was doing this backwards and as a result would start with
	// smallest to largest nodes here.. After a bunch of googling
	// I came up with this solution from StackOverflow
	// So I start with the largest possible free size this way.
	while (to_allocate >= size_of_nodes){
		int current_block_size = pow(2, allocate_in) * size_of_nodes;
		if (current_block_size <= to_allocate){
			to_allocate -= current_block_size;
			struct Node* header = (struct Node*)main_block;
			header->size = current_block_size;
			header->next = NULL;
			main_block = (Addr)(header + current_block_size);
			free_list[allocate_in] = header;
			printf("Free space of size %i going into position %p, free_list[%i]. Beginning of list is %p.\n",
			       current_block_size, free_list[allocate_in], allocate_in, main_block_start);
		}
		allocate_in--;
	}

	printf("\n== The free list is %i entries. This is the only used one:\n", free_list_length);
	for (int i = 0; i < free_list_length; i++){
		if ((struct Node*)free_list[i] != NULL){
			printf("== free_list[%i] - %p ==\n", i, free_list[i]);
		}
	}
	return total_size;
}

int release_allocator(){
	printf("\nFreeing main memory block!\n\n");	
	//free(main_block);
	free(main_block_start);
	return 0;
}

extern Addr my_malloc(unsigned int _length) {
	/* This preliminary implementation simply hands the call over the 
	the C standard library! 
	Of course this needs to be replaced by your implementation.
	*/

	int use = -1;
	int size_needed = _length + sizeof(struct Node);
	printf("* We need a space of %i\n", size_needed);
	for (int i = 0; i < free_list_length; i++){
		if (pow(2, i)*size_of_nodes >= size_needed){
			printf("\nPut it in section [%i]\n", i);
			use = i;
			break;
		}
	}
	if (free_list[use] == NULL){
		printf("\n--- Splitting! ---\n");
		int split_section = -1;
		for (int i = use + 1; i < free_list_length; i++){
			if (free_list[i] != NULL){
				//printf("Please split section: %p\nPlease go down to this: %p\n", free_list[i], free_list[use]);
				split_section = i;
				break;
			}
		}
		if (use == -1 || split_section == -1){
			//exit(EXIT_FAILURE);
			printf("Error: No space for size of %i!\n", size_needed);
			return 0;
		}

		
		printf("SPLIT SECTION: %i, USE: %i\n", split_section, use);
		for (int i = split_section; i > use; i--){
			if (free_list[i] == NULL){ printf("Section empty"); }
			if (i == 0){ printf("Cannot split further than %i", size_of_nodes); }
			struct Node* current_node = free_list[i];
			free_list[i] = current_node->next;

			printf("\n%i - Location of node: %p, size of node: %i\n", i, current_node, current_node->size);
			
			current_node->size = current_node->size/2;

			printf("Start of memory block: %p, with size of: %i\n", current_node, current_node->size);
			
			//Addr next_node_ptr = (Addr)((unsigned long)current_node ^ (unsigned long)(1 << (unsigned long)(log2(current_node->size))));
		
			// matthew says to do this because current_node->size will XOR the binary representation anyways
			//Addr buddy = (Addr)(current_node ^ (current_node->size));

			Addr next_node_ptr = (Addr)((unsigned long)current_node + (int)(current_node->size));

			//Addr next_node_ptr = (Addr)(

			printf("Current node is at %p. Next node will be put at %p.\n", current_node, next_node_ptr);

			current_node->next = (struct Node*)next_node_ptr;
			current_node->next->size = current_node->size;
			current_node->next->next = NULL;

			//printf("Size of node being stored: %i, %i\n\n", current_node->size, current_node->next->size);
			//printf("\n\nFree list @ %i: %p\n\n", i, free_list[i]);
			
			free_list[i-1] = current_node;
		}
	}
	printf("The address is %p. The next node is %p.\n", free_list[use], free_list[use]->next);
	//printf("These added together is %p.\n", free_list[use]+sizeof(struct Node));
	Addr address = (Addr)((Addr)free_list[use] + sizeof(struct Node));
	printf("Storing the node @ %p\n", address);
	free_list[use]->free = false;
	free_list[use] = free_list[use]->next;
	printf("Storing the node @ %p\n", address);
	return address;
}

extern int my_free(Addr _a) {
	// I think this is where you recursively join up free buddies?
	/* Same here! */
	printf("\n=\t=\t=\t=\t=\t=\t=\nFreeing: %p\n", _a);
	if (_a == NULL){
		return 1;
	}
	
	printf("Finding node..\n");
	_a -= sizeof(struct Node);	
	struct Node* node = (struct Node*)_a;
	printf("Node address: %p\n\n", node);

	printf("Finding buddy..\n");
	unsigned long ptr = (unsigned long)node;
	ptr = ptr - (unsigned long)main_block_start;
	ptr ^= 1 << (int)log2(node->size);
	ptr += (unsigned long)main_block_start;
	printf("Buddy address: %p\n\n", ptr);

	// This is the buddy of the given node....
	// but what now.....
	Addr buddy = (Addr)ptr;

	// find the buddy in free list and combine
	printf("Finding buddy in free_list..\n");
	int i = log2((node->size)/size_of_nodes);

	struct Node* buddy_node = (struct Node*)buddy;
	struct Node* buddy_lookup = (struct Node*)free_list[i];
	printf("step.......\n");
	while (buddy_node->free == true && buddy_lookup != buddy){
		printf("step..\n");
		buddy_lookup = buddy_lookup->next;
	}
	if (buddy_lookup == NULL){
		printf("No buddy node to join to.\n");
		node->free = true;
		node->next = free_list[i];
//		free_list[i]->next = free_list[i];
		free_list[i] = node;
		return 0;
	}
	
	printf("Found node in list %p\n", buddy_lookup);
	printf("Buddy node: %p\n", buddy);

	// Found this ? : stuff on stackoverflow
	Addr left_node = (Addr)((int)node < (int)buddy_lookup ? node : buddy_lookup);

	struct Node* new_node = (struct Node*)left_node;
	new_node->size = node->size + buddy_lookup->size;
	new_node->next = NULL;
	printf("New node: %p, size: %i\n", new_node, new_node->size);
	
	if (free_list[i]->next != NULL){
		free_list[i] = free_list[i]->next;
	} else {
		free_list[i] = NULL;
	}
	
	if (free_list[i+1] == NULL){
		free_list[i+1] = new_node;
	} else {
		new_node->next = free_list[i+1];
		//free_list[i+1] = new_node;
		printf("Will now add %p and %p to free list and try to combine\n", free_list[i+1], free_list[i+1]->next);
		printf("Current node pos: %p\n", new_node);
		printf("size: %i\n", free_list[i+1]->size);
		Addr to_release = (Addr)((Addr)new_node + sizeof(struct Node));
		my_free(to_release);
		//my_free(free_list[i+1]);
	}
	
	//free(_a);
	return 0;
}

void free_list_check(){
	printf("== == == Printing of current free list == == ==\n");
	for (int i = 0; i < free_list_length; i++){
		int num = 0;
		struct Node* node = free_list[i];
		if (node != NULL){
			printf("Address: %p, size: %i (", node, node->size);
		}
		while (node != NULL){
			printf("%p, ", node);
			num++;
			node = node->next;
		}
		printf(") Number of items: %i\n", num);
	}
	printf("== == == == == == == == == == == == == == == == ==\n");
}
