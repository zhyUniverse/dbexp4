//
// Created by Hongyu Chow on 20/05/20.
//

#include <stdio.h>
#include <stdlib.h>
#include "task.h"
#include "extmem.h"
#include "utils.h"

int RSABLS(int start_addr, int block_num) {
    // �������������Ĺ�ϵѡ���㷨
    printf("\n****�������������Ĺ�ϵѡ���㷨****\n");

    Buffer buf;  // buffer

    if (!initBuffer(520, 64, &buf)) {
        perror("Buffer Initialization Failed!\n");
        return -1;
    }

    unsigned char *blk;  // �ڴ��

    OutBlk out_blk;  // �����

    int addr = start_addr;

    if (InitOutBlock(&out_blk, &buf, 7, 100) == -1) {
        perror("Out block init failed.\n");
        return -1;
    }

    int res_num = 0, io_num = 0;

    for (int i = 0; i < block_num; i++) {
        blk = readBlockFromDisk(addr, &buf);

        if (blk == NULL) {
            perror("Read block failed.\n");
        }

        io_num++;
        printf("�������ݿ� %d.blk.\n", addr);

        // ��˳���ȡ7��Ԫ��
        for (int j = 0; j < 7; j++) {
            int val[2];

            if (readProperty(blk, j, 2, val) == -1) {
                perror("Read property failed.\n");
                return -1;
            }

            if (val[0] == 30) {
                res_num++;
                printf("(R.A = %d, R.B = %d)\n", val[0], val[1]);
                // ���������
                putInOutBlock(&out_blk, &buf, 2, (int *) val);
            }
        }

        freeBlockInBuffer(blk, &buf);

        if (readAddress(blk, 7, &addr) == -1) {
            perror("Read address failed.\n");
            return -1;
        }
    }

    if (freeOutBlockInBuffer(&out_blk, &buf) == -1) {
        perror("Out block free failed.\n");
        return -1;
    }

    printf("����������Ԫ��һ��%d��.\n", res_num);

    printf("IO��дһ��%d��.\n", io_num);

    return 0;
}

int TPMMS(int addr, int block_num, int out_start_addr, int set_block_num) {
    // ���׶ζ�·�鲢�����㷨

    printf("\n****���׶ζ�·�鲢�����㷨****\n");

    Buffer buf;

    if (!initBuffer(520, 64, &buf)) {
        perror("Buffer Initialization Failed!\n");
        return -1;
    }

    // �Ӽ��ϵ�����
    int set_num = block_num / set_block_num + (block_num % set_block_num == 0 ? 0 : 1);

    // ÿ���Ӽ����и�����ĵ�ַ
    // ��ʵ���п��ַ��˳�������ûɶ��Ҫ
    int set_addr[set_num][set_block_num];

    int counts[set_num];

    {
        // �ڲ�����
        unsigned char *blk[set_block_num];

        int count = 0;

        for (int i = 0; i < block_num; i++) {
            // ��ȡ���ݿ�
            if ((blk[i % set_block_num] = readBlockFromDisk(addr, &buf)) == NULL) {
                perror("Read block failed.\n");
                return -1;
            }

            set_addr[i / set_block_num][i % set_block_num] = addr;

            count++;

            // ��ȡ��һ�����ݿ�ĵ�ַ
            if (readAddress(blk[i % set_block_num], 7, &addr) == -1) {
                perror("Read address failed.\n");
                return -1;
            }

            if ((i + 1) % set_block_num == 0 || i == block_num - 1) {
                // ��һ���Ӽ��Ͻ�������
                sort(blk, count, 7, 2);

                int next_addr;

                // д��
                for (int j = 0; j < count; j++) {
                    if (readAddress(blk[j], 7, &next_addr) == -1) {
                        perror("Read address failed.\n");
                        return -1;
                    }

                    writeAddress(blk[j], 7, next_addr + 100);

                    if (writeBlockToDisk(blk[j], set_addr[i / set_block_num][j] + 100, &buf) == -1) {
                        perror("Write to block failed.\n");
                        return -1;
                    }
                }

                count = 0;
            }
        }
    }

    {
        // �ĸ������ڶ��׶������ʱ�����δ��ĸ��Ӽ����е�ÿ����
        unsigned char *blk[set_num];
        // compare�ڴ��
        unsigned char *cmp;

        // ��¼ÿ���Ӽ����ж������ĸ�Ԫ�� 0 - (4*7-1)
        // ��ֵ����Ӧ�ö���ǰ�ĸ��� ��ͨ�� set_addr ��ȡ���ַ
        // ���ǿ����˵�ַ����������� �����ʵ�����е���һ��
        int set_index[set_num];

        // ��ʼ�� compare ��
        if ((cmp = getNewBlockInBuffer(&buf)) == NULL) {
            perror("Buffer is full.\n");
            return -1;
        }

        // ��ÿ���Ӽ��ϵĵ�һ��������ڴ�
        // ���ѵ� i ����ĵ� 0 ��Ԫ��ŵ� compare ��ĵ� i ��λ��
        for (int i = 0; i < set_num; i++) {
            set_index[i] = 0;

            if ((blk[i] = readBlockFromDisk(set_addr[i][0], &buf)) == NULL) {
                perror("Read block failed.\n");
                return -1;
            }

            int val[2];

            if (readProperty(blk[i], 0, 2, val) == -1) {
                perror("Read property failed.\n");
                return -1;
            }

            writeProperty(cmp, i, 2, val);
        }

        // ��¼ compare ������С��ֵ
        int min_val[2];
        // ��¼ compare ������С��ֵ��λ��
        int pos;

        // ��װ�õ����������������������ʱ���Զ���������̲���ջ�����
        OutBlk out_blk;
        if (InitOutBlock(&out_blk, &buf, 7, out_start_addr) == -1) {
            perror("Out block init failed.\n");
            return -1;
        }

        // �鲢����
        // compare ��û��ֵʱ����ѭ��
        while ((pos = cmpInBlock(cmp, set_num, min_val)) != -1) {
            if (putInOutBlock(&out_blk, &buf, 2, min_val) == -1) {
                perror("Put in out block failed.\n");
                return -1;
            }

            // ����Ԫ�������� 1
            set_index[pos]++;

            int blk_index = set_index[pos] / 7;

            if (blk_index == set_block_num) {
                // ����һ���Ӽ��ϵĿ��������Ӽ����Ѿ�����
                // �� compare ����д�� '\0'
                writeProperty(cmp, pos, 2, NULL);
            } else if (set_index[pos] % 7 == 0) {
                // ��Ҫ��ȡ��һ��
                freeBlockInBuffer(blk[pos], &buf);

                if ((blk[pos] = readBlockFromDisk(set_addr[pos][blk_index], &buf)) == NULL) {
                    perror("Read block failed.\n");
                    return -1;
                }
            }
                {
                if (next_blk != cur_set_blk_index[pos]) {
                    // ��һ��Ҫ���Ŀ鲻���ڴ���
                    // �ͷŵ�ǰ�鲢������һ����
                    freeBlockInBuffer(blk[pos], &buf);
                    if ((blk[pos] = readBlockFromDisk(set_addr[pos][next_blk], &buf)) == NULL) {
                        perror("Read block failed.\n");
                        return -1;
                    }
                    cur_set_blk_index[pos] = next_blk;
                }

                int val[2];

                if (readProperty(blk[pos], set_index[pos] % 7, 2, val) == -1) {
                    perror("Read property failed.\n");
                    return -1;
                }

                writeProperty(cmp, pos, 2, val);
            }
        }
        // ������������ڶ���һ����ǰ�ж���û��
        // ���������Ҫ�ѻ���������������д�����̿�
        freeOutBlockInBuffer(&out_blk, &buf);
    }

    return 0;
}
