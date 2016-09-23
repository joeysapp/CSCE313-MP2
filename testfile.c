#include "ackerman.h"
//#include "my_allocator.c"
#include "my_allocator.h"

int main(int argc, char** argv){
	init_allocator(16, 256);

	//Addr add = my_malloc(128);
	
	printf("\n@\t@\t@\t@\t@\t INSERTING 16 \t@\t@\t@\t@\t@\t\n");
	my_malloc(16);

	printf("\n@\t@\t@\t@\t@\t LIST CHECK \t@\t@\t@\t@\t@\t\n");
	free_list_check();
	
	printf("\n@\t@\t@\t@\t@\t INSERTING 16 \t@\t@\t@\t@\t@\t\n");
	my_malloc(16);

	//printf("omg: %p, %i", add, (struct Node*)add);

	printf("\n");
	//my_malloc(12);

	free_list_check();
	
	release_allocator();

	//free_list_check();
}
