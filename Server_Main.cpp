#include <cstdio>
#include <cstdlib>

#include "Server.h"

int main() {
    printf("Starting...\n");
    Server_t Server;
    if (!Server.Start()) {
        printf("Error - Start Server\n");
        getchar();
        return false;
    }
    getchar();
    return true;
}