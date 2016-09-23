#include "ackerman.h"
//#include "my_allocator.c"
#include "my_allocator.h"

int main(int argc, char** argv){

	init_allocator(16, 1024);
	
//	my_malloc(16);
//	my_malloc(16);
//	my_malloc(128);
	free_list_check();

	release_allocator();

}
