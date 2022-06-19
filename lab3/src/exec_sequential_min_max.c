#include <unistd.h>
#include <stdio.h>

int main(int argc, char** argv) {
	if (execv("./sequential_min_max", argv) == -1) {
		printf("Program not started\n");
	}
	return 0;
}