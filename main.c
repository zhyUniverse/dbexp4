#include <stdio.h>
#include <stdlib.h>
#include "task.h"

int main() {
    int choice;

    while (1) {
        printf("1. 基于线性搜索的关系选择算法\n"
               "2. 两阶段多路归并排序算法\n"
               "3. 基于索引的关系选择算法\n"
               "4. 关系投影算法\n"
               "5. 基于排序的连接操作算法\n"
               "6. 集合操作算法\n"
               "0. 退出\n"
               "选择: ");

        choice = getchar();

        switch (choice) {
            case '0':
                return 0;
            case '1':
                if (RSBLS(1, 16, 100) == -1) {
                    perror("Task 1 failed.\n");
                }
                break;
            case '2':
                if (TPMMS(1, 16, 201, 301, 4) == -1) {
                    perror("Task 2 sort R failed.\n");
                }

                if (TPMMS(17, 32, 217, 317, 6) == -1) {
                    perror("Task 2 sort S failed.\n");
                }

                break;
            case '3':
                if (IBRS(301, 16, 351, 120, 30) == -1) {
                    perror("Task 3 failed.\n");
                }
                break;
            case '4':
                if (RP(301, 16, 130) == -1) {
                    perror("Task 4 failed.\n");
                }
                break;
            case '5':
                if (SBCO(301, 317, 16, 32, 401) == -1) {
                    perror("Task 5 failed.\n");
                }
                break;
            case '6':
                if (SO(301, 317, 16, 32, 140) == -1) {
                    perror("Task 6 failed.\n");
                }
                break;
            default:
                printf("No such function.\n");
                break;
        }

        fflush(stdin);

        system("Pause");

        system("cls");
    }

    return 0;
}
