#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "calc.h"

#define MAX_BUFFERSIZE 1024

int main(void) {
    char *input_buff;
    int result;
    int len;
    
    input_buff = malloc(MAX_BUFFERSIZE);
    if (input_buff == NULL) {
        fprintf(stderr, "error: out of memory\n");
        return 1;
    }
    
    printf(">> ");
    
    while (fgets(input_buff, MAX_BUFFERSIZE, stdin) != NULL) {
        len = strlen(input_buff);
        if (len > 0 && input_buff[len - 1] == '\n')
            input_buff[len - 1] = '\0';
        
        if (input_buff[0] == '\0') {
            printf(">> ");
            continue;
        }
        
        if (eval(input_buff, &result) == 0)
            printf("%d\n", result);
        printf(">> ");
    }

    free(input_buff);
    return 0;
}