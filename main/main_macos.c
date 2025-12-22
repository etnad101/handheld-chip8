#include "../app/app.h"
#include "stdio.h"

int main() {
    printf("Starting CHIP-8\n");
    int status = (int)app_run();
    if (status < 0) {
        printf("ERROR: %d\n", status);
    }
    printf("Exiting\n");
    return 0;
}