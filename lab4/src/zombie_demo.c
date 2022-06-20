#include <unistd.h>
#include <stdlib.h>

int main() {
	pid_t pid = fork();
	if (pid == 0) {
		return 0;
	}
	else {
		sleep(30);
	}
	return 0;
}