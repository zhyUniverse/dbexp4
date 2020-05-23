#include <stdio.h>
#include <stdlib.h>
#include "task.h"

int main() {
    int choice;

    while (1) {
        printf("1. �������������Ĺ�ϵѡ���㷨\n"
               "2. ���׶ζ�·�鲢�����㷨\n"
               "3. ���������Ĺ�ϵѡ���㷨\n"
               "4. ��ϵͶӰ�㷨\n"
               "5. ������������Ӳ����㷨\n"
               "6. ���ϲ����㷨\n"
               "0. �˳�\n"
               "ѡ��: ");

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
