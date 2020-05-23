#include <stdio.h>
#include <stdlib.h>
#include "extmem.h"
#include "task.h"
#include "utils.h"

int main() {
    if (RSABLS(1, 16, 100) == -1) {
        perror("Task 1 failed.\n");
    }

    if (TPMMS(1, 16, 201, 301, 4) == -1) {
        perror("Task 2 sort R failed.\n");
    }

    if (TPMMS(17, 32, 217, 317, 6) == -1) {
        perror("Task 2 sort S failed.\n");
    }

    if (IBRSA(301, 16, 351, 120, 30) == -1) {
        perror("Task 3 failed.\n");
    }

//    int index;
//    Buffer buf;
//    if (!initBuffer(520, 64, &buf)) {
//        perror("Buffer init failed.\n");
//        return -1;
//    }
//    index = findIndex(351, 16, 30, &buf);
//
//    printf("index: %d\n", index);

    return 0;
}
