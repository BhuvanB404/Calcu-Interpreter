#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int eval(const char* input, int* out_result);

int main(void) {
    char *ip_buf;
    int len;
    int result;

    ip_buf = (char *)malloc(1024);
    if (ip_buf == NULL) {
        fprintf(stderr, "Out of memory\n");
        return 1;
    }

    printf("Enter expression: \n");
    printf(">>");

    while (fgets(ip_buf, 1024, stdin) != NULL) {
        len = strlen(ip_buf);
        if (len != 0 && ip_buf[len - 1] == '\n') {
            ip_buf[len - 1] = '\0';
        }

        if (ip_buf[0] == '\0') {
         printf(">> ");
            continue;
        }

        if (eval(ip_buf, &result) == 0) {
            printf("%d\n", result);
        }

     
        printf(">> ");
    }

    free(ip_buf);
    return 0;
}

