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
	free_list = (struct Node**)malloc(free_list_length * sizeof(struct Node*));

	//printf("%p\n%p\n\n", main_block, free_list);
	//printf("%i", free_list_length * sizeof(struct Node*));

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
			main_block = (Addr)(header + current_block_size);
			free_list[allocate_in] = header;
			printf("Free space of size %i going into position %p. Beginning of list is %p.\n",
			       current_block_size, free_list[allocate_in], main_block_start);
		}
		allocate_in--;
	}

	printf("\n== The free list is %i entries.\n", free_list_length);
	for (int i = 0; i < free_list_length; i++){
		printf("== free_list[%i] - %p ==\n", i, free_list[i]);
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

	printf("Location of main block: %p\n", main_block);
	printf("Location of main block start: %p\n", main_block_start);

	int use = -1;
	int size_needed = _length + sizeof(struct Node);
	printf("* We need a space of %i\n", size_needed);
	for (int i = 0; i < free_list_length; i++){
		//if (size_needed < pow(2, i) * size_of_nodes){
		if (pow(2, i)*size_of_nodes >= size_needed){
			use = i;
			break;
		}
	}
	if (free_list[use] == NULL){
		printf("\n--- Splitting! ---\n");
		int split_section = -1;
		for (int i = use + 1; i < free_list_length; i++){
			if (free_list[i] != NULL){
				printf("Please split this: %i\nPlease use this section: %i\n", i, use);
				split_section = i;
				break;
			}
		}
		if (split_section == -1){ printf("No space"); }
		
		printf("SPLIT SECTION: %i, USE: %i\n", split_section, use);
		for (int i = split_section; i > use; i--){
			if (free_list[i] == NULL){
				printf("Section empty");
			}
			if (i == 0){
				printf("Cannot split further than %i", size_of_nodes);
			}
			
			struct Node* current_node = free_list[i];
			free_list[i] = current_node->next;

			printf("\n%i - Location of node: %p, size of node: %i\n", i, current_node, current_node->size);
			
			current_node->size = current_node->size/2;

			printf("Start of memory block: %p, with size of: %i\n", current_node, current_node->size);

			Addr next_node_ptr = (Addr)((unsigned long)current_node ^ (1 << (int)(log2(current_node->size))));

			//Addr next_node_ptr_s = (Addr)((Addr)current_node - main_block_start);
			// Bit manipulation here, found on StackOverflow
			//Addr next_node_ptr_b = (Addr) ((unsigned long)next_node_ptr_s ^ ((unsigned long)current_node->size));
			//Addr next_node_ptr = (Addr)((unsigned long)next_node_ptr_b + (unsigned long) main_block_start);

			printf("Current node is at %p. Next node will be put at %p.\n", current_node, next_node_ptr);

			current_node->next = (struct Node*)next_node_ptr;
			current_node->next->size = current_node->size;
			printf("Size of node being stored: %i, %i\n\n", current_node->size, current_node->next->size);
			printf("\n\nFree list @ %i: %p\n\n", i, free_list[i]);
			//current_node->next->size = current_node->size;
			
			//current_node->
			//current_node->next->size = current_node->size;
			
			free_list[i-1] = current_node;
			
		}
	}
	//printf("The address is %p. The size of node is %i.\n", free_list[use], sizeof(struct Node));
	//printf("These added together is %p.\n", free_list[use]+sizeof(struct Node));
	Addr address = (Addr)(free_list[use] + sizeof(struct Node*));
	free_list[use]->free = false;
	free_list[use] = free_list[use]->next;
	//printf("Storing the node @ %p\n", address);
	return address;
}

extern int my_free(Addr _a) {
	// I think this is where you recursively join up free buddies?
	/* Same here! */
	printf("\nFreeing: %p\n", _a);
	free(_a);
	return 0;
}

void free_list_check(){
	for (int i = 0; i < free_list_length; i++){
		struct Node* node = free_list[i];
		if (node != NULL){
			printf("== free_list[%i], address: %p, size: %i\n", i, node, node->size);
		}
	}

/*	printf("free_list\n");
	for (int i = 0; i < free_list_length; i++){
		int number_of_items = 0;
		printf("%i: ", i);
		struct Node* node = free_list[i];
		while (node != NULL){
			number_of_items++;
			printf("Address: %p, Size: %i ", node, node->size);
			node = node->next;
		}
		printf("node %p\n", node);
		printf("Number of Items: %i\n", number_of_items);
	}*/
}

