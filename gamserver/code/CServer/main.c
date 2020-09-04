#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

int main(void) {
    float weight;
    scanf("%f", &weight);

    printf("weight %f\n", weight);
    return 0;
}