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
	free_list = malloc(free_list_length * sizeof(struct Node*));

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
			struct Node* header = (struct Node*)main_block;
			header->size = current_block_size;
			main_block = (Addr)header + current_block_size;
			free_list[allocate_in] = header;
			to_allocate -= current_block_size;
		}
		allocate_in--;
	}

/*	for (int i = 0; i < free_list_length; i++){
		struct Node* header = (struct Node*)main_block_start;
		int size_for_block = pow(2, i) * size_of_nodes;
		header->size = size_for_block;
		free_list[i] = header;
		main_block_start += size_for_block;
	}*/

	printf("\n== The free list is %i entries.\n", free_list_length);

	return total_size;
}

int release_allocator(){
	printf("\nFreeing main memory block!\n\n");	
	free(main_block);
	return 0;
}

extern Addr my_malloc(unsigned int _length) {
	/* This preliminary implementation simply hands the call over the 
	the C standard library! 
	Of course this needs to be replaced by your implementation.
	*/

	printf("Location of main block: %p\n", main_block);
	printf("Location of main block start: %p\n", main_block_start);

	int use;
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
		int split_section = -1;
		for (int i = use + 1; i < free_list_length; i++){
			if (free_list[i] != NULL){
				printf("Please split this: %i\nPlease use this section: %i\n", i, use);
				split_section = i;
				break;
			}
		}
		if (split_section == -1){ printf("No space"); }
		// wait what, < works and > doesn't??
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

			Addr next_node_ptr_s = (Addr)((Addr)current_node - (int) main_block_start);
			// Bit manipulation here, found on StackOverflow
			Addr next_node_ptr_b = (Addr) ((int)next_node_ptr_s ^ ((int)current_node->size));
			//next_node_ptr_b = (int)next_node_ptr_b >> 8;
			//next_node_ptr_b = (int)next_node_ptr_b >> 4;
			Addr next_node_ptr = (Addr)((int)next_node_ptr_b + main_block_start);
			
			printf("Offset start: %p, Offset buddy: %p, next ptr: %p\n", next_node_ptr_s, next_node_ptr_b, next_node_ptr);

			printf("star: %p\n", main_block_start);
			printf("incr: %p\n", next_node_ptr_b);
			printf("????: %p\n", next_node_ptr);

			//printf("The buddy should go to: %p\n", (Addr)((char)next_node_ptr_b - (int) main_block_start));
			//printf("The buddy should go to: %p\n", (struct Node*)next_node_ptr);
			//struct Node* next_node = next_node_ptr;

			current_node->next = (struct Node*) next_node_ptr;
			printf("%p\n", current_node->next);
			current_node->next->size = current_node->size;
			//current_node->

			
			//current_node->next->size = current_node->size;
			
			free_list[i-1] = current_node;
			
		}
	}
	Addr address = (Addr) free_list[use] + sizeof(struct Node);
	free_list[use]->free = false;
	free_list[use] = free_list[use]->next;
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
	printf("free_list\n");
	for (int i = 0; i < free_list_length; i++){
		int number_of_items = 0;
		printf("%i: ", i);
		struct Node* node = free_list[i];
		while (node != NULL){
			number_of_items++;
			printf("Address: %p, Size: %i ", node, node->size);
			node = node->next;
		}
		printf("Number of Items: %i\n", number_of_items);
	}
}


/*

   
	int use;
	int size_needed = _length + sizeof(struct Node);
	for (int i = 0; i < free_list_length; i++){
		if (size_needed <= pow(2, i) * size_of_nodes){
			use = i;
			break;
		}
	}

	if (free_list[use] == NULL){
		int split_section = NULL;
		for (int i = use + 1; i < free_list_length; i++){
			if (free_list[i] != NULL){
				split_section = i;
				break;
			}
		}
		if (split_section == NULL){ printf("No space"); }
		for (int i = split_section; i < use; i--){
			// SplitSection(i) here
			if (free_list[i] == NULL){
				printf("Section empty");
			}
			if (i == 0){
				printf("Cannot split further than %i", size_of_nodes);
			}
			struct Node* current_node = free_list[i];
			free_list[i] = current_node->next;
			current_node->size = current_node->size/2;

			Addr next_node_ptr = current_node + sizeof(struct Node);

			current_node->next = (struct Node*) next_node_ptr;
			current_node->next->size = current_node->size;

			free_list[i-1] = current_node;

		}
	}


				Addr next_node_ptr_s = (Addr) ((char) main_block_start - (char) current_node );
			// Bit manipulation here, found on StackOverflow
			Addr next_node_ptr_b = (Addr) ((char)next_node_ptr_s ^ (current_node->size));
			Addr next_node_ptr = (Addr) (((char)next_node_ptr_b) + main_block_start);
			
			printf("Offset start: %p, Offset buddy: %p\n", next_node_ptr_s, next_node_ptr_b);

			printf("star: %p\n", (Addr)main_block_start);
			printf("incr: %p\n", (Addr)next_node_ptr_b);
			printf("????: %p\n", (Addr)((char)next_node_ptr_b+main_block_start));

			printf("The buddy should go to: %p\n", (Addr)((char)next_node_ptr_b +main_block_start));
			//printf("The buddy should go to: %p\n", (struct Node*)next_node_ptr);
			//struct Node* next_node = next_node_ptr;

			current_node->next = (struct Node*)next_node_ptr;
			printf("%p\n", current_node->next);
			//current_node->next->size = current_node->size;

			
			//current_node->next->size = current_node->size;
			
			free_list[i-1] = current_node;


	*/
