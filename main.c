#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "calc.h"

#define MAX_BUFFERSIZE 1024
<<<<<<< Updated upstream

int main(void) {
    char *input_buff;
    int result;
    int len;
    
=======
#define HISTORY_SIZE 3

static Value history[HISTORY_SIZE];
static int   count = 0;

static void print_val(Value v) {
    if (v.type == VAL_INT) {
        printf("%ld", v.as.ival);
    } else if (v.type == VAL_FLOAT) {
        printf("%g", v.as.fval);
    }
}

static void print_history(void) {
    int i;
    int total;

    total = count < HISTORY_SIZE ? count : HISTORY_SIZE;
    if (total == 0) {
        printf("No history yet.\n");
        return;
    }

    for (i = total - 1; i >= 0; i--) {
        int pos;
        pos = (count - 1 - i) % HISTORY_SIZE;
        printf("  { %d } = ", total - i);
        print_val(history[pos]);
        printf("\n");
    }
}

static void push_history(Value val) {
    history[count % HISTORY_SIZE] = val;
    count++;
}

int main(void) {
    char *input_buff;
    Value result;
    int   len;

>>>>>>> Stashed changes
    input_buff = malloc(MAX_BUFFERSIZE);
    if (input_buff == NULL) {
        fprintf(stderr, "error: out of memory\n");
        return 1;
    }
<<<<<<< Updated upstream
    
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

=======

    printf(">> ");

    while (fgets(input_buff, MAX_BUFFERSIZE, stdin) != NULL) {
        len = strlen(input_buff);
        if (len > 0 && input_buff[len - 1] == '\n')
            input_buff[len - 1] = '\0';

        if (input_buff[0] == '\0') {
            printf(">> ");
            continue;
        }

        /* history command */
        if (strcmp(input_buff, "history") == 0) {
            print_history();
            printf(">> ");
            continue;
        }

        /* insert ANS as a variable  so it is stored in the var_list*/

        if (eval(input_buff, &result) == 0) {
            print_val(result);
            printf("\n");
            push_history(result);
            public_set_var("ANS", 3, result);
        }

        printf(">> ");
    }

>>>>>>> Stashed changes
    free(input_buff);
    return 0;
}