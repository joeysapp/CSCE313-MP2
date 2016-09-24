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


#include <signal.h>

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

	// Main block (where we will be storing blocks)
	main_block = (Addr)malloc((total_size * sizeof(char)));
	main_block_start = main_block;
	
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
		}
		allocate_in--;
	}
	return total_size;
}

int release_allocator(){
	free(main_block_start);
	return 1;
}

extern Addr my_malloc(unsigned int _length) {
	int use = -1;
	int size_needed = _length + sizeof(struct Node*);
	for (int i = 0; i < free_list_length; i++){
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
				split_section = i;
				break;
			}
		}
		if (use == -1 || split_section == -1){
			//exit(EXIT_FAILURE);
			printf("Error: No space for size of %i!\n", size_needed);
			return 0;
		}
		
		for (int i = split_section; i > use; i--){
			if (free_list[i] == NULL){ printf("Section empty"); }
			if (i == 0){ printf("Cannot split further than %i", size_of_nodes); }
			struct Node* current_node = free_list[i];
			free_list[i] = current_node->next;	
			current_node->size = current_node->size/2;
		
			// matthew says to do this because current_node->size will XOR the binary representation anyways
			//Addr buddy = (Addr)(current_node ^ (current_node->size));

			Addr next_node_ptr = (Addr)((unsigned long)current_node + (int)(current_node->size));

			current_node->next = (struct Node*)next_node_ptr;
			current_node->next->size = current_node->size;
			current_node->next->next = NULL;

			free_list[i-1] = current_node;
		}
	}
	Addr address = (Addr)((unsigned long)free_list[use] + (int)sizeof(struct Node));
	free_list[use]->free = false;
	free_list[use] = free_list[use]->next;
	return address;
}

extern int my_free(Addr _a) {
	if (_a == NULL){ return 1; }
	
	_a = (Addr)((unsigned long)_a - sizeof(struct Node));
	struct Node* node = (struct Node*)_a;

	unsigned long ptr = (unsigned long)node;
	ptr = ptr - (unsigned long)main_block_start;
	ptr ^= 1 << (int)log2(node->size);
	ptr += (unsigned long)main_block_start;

	Addr buddy = (Addr)ptr;

	int i = log2((node->size)/size_of_nodes);

	struct Node* buddy_node = (struct Node*)buddy;
	struct Node* buddy_lookup = (struct Node*)free_list[i];
	
	//printf("%i: This node is of size %i and is looking at a node of size %i.\n", i, node->size, buddy_node->size);

	//printf("Node %p has buddy %p and is at %p in freelist\n", node, buddy_node, buddy_lookup);

	if (node->size != buddy_node->size){
		if (free_list[i] == NULL){
			node->free = true;
			node->next = NULL;
			free_list[i] = node;
			return 1;
		} else {
			node->free = true;
			node->next = NULL;
			free_list[i]->next = node;
			return 1;
		}
	}

	if (free_list[i] == NULL || buddy_lookup == NULL){
		node->free = true;
		node->next = NULL;
		free_list[i] = node;
		return 1;
	}

	while (buddy_lookup != buddy){
		printf("step");
		if (buddy_lookup->next != NULL){
			buddy_lookup = buddy_lookup->next;
		} else {
			//return 1; 
		}
	}
	// Found this ? : stuff on stackoverflow
	Addr left_node = (Addr)((unsigned long)node < (unsigned long)buddy_lookup ? node : buddy_lookup);

	struct Node* new_node = (struct Node*)left_node;
	new_node->size = node->size + buddy_lookup->size;
	new_node->next = NULL;
	
	if (free_list[i]->next != NULL){
		free_list[i] = free_list[i]->next;
	} else {
		free_list[i] = NULL;
	}
	if (free_list[i+1] == NULL){
		free_list[i+1] = new_node;
	} else {
		printf("cascade");
		free_list[i+1]->next = new_node;
		//new_node->next = free_list[i+1];
		Addr to_release = (Addr)((unsigned long)new_node + sizeof(struct Node));
		my_free(to_release);
	}
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

/*
   extern int my_free(Addr _a) {
	if (_a == NULL){ return 1; }
	
	_a = (Addr)((unsigned long)_a - sizeof(struct Node));
	struct Node* node = (struct Node*)_a;

	unsigned long ptr = (unsigned long)node;
	ptr = ptr - (unsigned long)main_block_start;
	ptr ^= (1 << (int)log2(node->size));
	ptr += (unsigned long)main_block_start;

	Addr buddy = (Addr)ptr;

	int i = log2((node->size)/size_of_nodes);

	struct Node* buddy_node = (struct Node*)buddy;
	struct Node* buddy_lookup = (struct Node*)free_list[i];
	
	printf("%i: This node is of size %i and is looking at a node of size %i.\n", i, node->size, buddy_node->size);

	printf("Node %p has buddy %p and is at %p in freelist\n", node, buddy_node, buddy_lookup);
	
	if (node->size != buddy_node->size){
		if (free_list[i] == NULL){
			node->free = true;
			node->next = NULL;
			free_list[i] = node;
			return 1;
		} else {
			node->free = true;
			node->next = NULL;
			free_list[i]->next = node;
			return 1;
		}
	}
	if (free_list[i] == NULL || buddy_lookup == NULL){
		node->free = true;
		node->next = NULL;
		free_list[i] = node;
		return 1;
	}

	while (buddy_lookup != NULL && buddy_lookup != buddy){
		buddy_lookup = buddy_lookup->next;
		printf("step\n");
	}

	// Found this ? : stuff on stackoverflow
	Addr left_node = (Addr)((unsigned long)node < (unsigned long)buddy_lookup ? node : buddy_lookup);

	struct Node* new_node = (struct Node*)left_node;
	new_node->size = node->size + buddy_lookup->size;
	new_node->next = NULL;
	
	if (free_list[i]->next != NULL){
		free_list[i] = free_list[i]->next;
	} else {
		free_list[i] = NULL;
	}
	if (free_list[i+1] == NULL){
		free_list[i+1] = new_node;
	} else {
		free_list[i+1]->next = new_node;
		//new_node->next = free_list[i+1];
		Addr to_release = (Addr)((unsigned long)new_node + sizeof(struct Node));
		my_free(to_release);
	}
	return 0;
}

*/
