#include <cstdio>
#include <cstdlib>

#include "Client.h"

int main() {
	printf("Starting...\n");
	Client_t Client;
	if (!Client.Start()) {
		printf("Error - Start Сlient\n");
		return false;
	}
	return true;
