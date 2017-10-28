#include <cstdio>
#include <cstdlib>

#include "Klient.h"

int main() {
	printf("Starting...\n");
	Klient_t Klient;
	if (!Klient.Start()) {
		printf("Error - Start Klient\n");
		return false;
	}
	return true;
}