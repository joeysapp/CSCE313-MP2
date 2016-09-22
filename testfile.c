#include "ackerman.h"
#include "my_allocator.h"

int main(int argc, char** argv){
	init_allocator(16, 256);

	my_malloc(10);

	//free_list_check();
	
	release_allocator();

	//free_list_check();
}
