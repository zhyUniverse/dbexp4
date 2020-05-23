//
// Created by Hongyu Chow on 20/05/20.
//

#include <stdio.h>
#include <stdlib.h>
#include "task.h"
#include "extmem.h"
#include "utils.h"

int RSABLS(int addr, int block_num, int out_addr) {
    // �������������Ĺ�ϵѡ���㷨
    printf("\n****�������������Ĺ�ϵѡ���㷨****\n");

    Buffer buf;  // buffer

    if (!initBuffer(520, 64, &buf)) {
        perror("Buffer Initialization Failed!\n");
        return -1;
    }

    unsigned char *blk;  // �ڴ��

    OutBlk out_blk;  // �����

    if (InitOutBlock(&out_blk, &buf, 7, out_addr) == -1) {
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

int TPMMS(int addr, int block_num, int set_res_out_addr, int out_addr, int set_block_num) {
    // ���׶ζ�·�鲢�����㷨
    // TODO: �Ż� �м�����ַ

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
//    int set_addr[set_num][set_block_num];

    int counts[set_num];

    {
        // �ڲ�����
        unsigned char *blk[set_block_num];

        int count = 0;

        int set_addr = set_res_out_addr;

        for (int i = 0; i < block_num; i++) {
            // ��ȡ���ݿ�
            if ((blk[i % set_block_num] = readBlockFromDisk(addr, &buf)) == NULL) {
                perror("Read block failed.\n");
                return -1;
            }

            count++;

            // ��ȡ��һ�����ݿ�ĵ�ַ
            if (readAddress(blk[i % set_block_num], 7, &addr) == -1) {
                perror("Read address failed.\n");
                return -1;
            }

            if ((i + 1) % set_block_num == 0 || i == block_num - 1) {
                counts[i / set_block_num] = count;

                // ��һ���Ӽ��Ͻ�������
                sort(blk, count, 7, 2);

                int next_addr;

                // д��
                for (int j = 0; j < count; j++) {
                    if (readAddress(blk[j], 7, &next_addr) == -1) {
                        perror("Read address failed.\n");
                        return -1;
                    }

                    writeAddress(blk[j], 7, set_addr + 1);

                    if (writeBlockToDisk(blk[j], set_addr++, &buf) == -1) {
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

        int set_addr = set_res_out_addr;

        // ��ÿ���Ӽ��ϵĵ�һ��������ڴ�
        // ���ѵ� i ����ĵ� 0 ��Ԫ��ŵ� compare ��ĵ� i ��λ��
        for (int i = 0; i < set_num; i++) {
            set_index[i] = 0;

            if ((blk[i] = readBlockFromDisk(set_addr + i * set_block_num, &buf)) == NULL) {
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
        if (InitOutBlock(&out_blk, &buf, 7, out_addr) == -1) {
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

            if (blk_index >= counts[pos]) {
                // ����һ���Ӽ��ϵĿ��������Ӽ����Ѿ�����
                // �� compare ����д�� '\0'
                writeProperty(cmp, pos, 2, NULL);
            } else {
                if (set_index[pos] % 7 == 0) {
                    // ��Ҫ��ȡ��һ��
                    freeBlockInBuffer(blk[pos], &buf);

                    if ((blk[pos] = readBlockFromDisk(set_res_out_addr + pos * set_block_num + blk_index, &buf)) == NULL) {
                        perror("Read block failed.\n");
                        return -1;
                    }
                }

                int val[2];

                if (readProperty(blk[pos], set_index[pos] % 7, 2, val) == -1) {
                    writeProperty(cmp, pos, 2, NULL);
                } else {
                    writeProperty(cmp, pos, 2, val);
                }
            }
        }
        // ������������ڶ���һ����ǰ�ж���û��
        // ���������Ҫ�ѻ���������������д�����̿�
        freeOutBlockInBuffer(&out_blk, &buf);
    }

    printf("IO��д����һ��%lu��.\n", buf.numIO);

    return 0;
}

int IBRSA(int addr, int block_num, int index_out_addr, int out_addr, int value) {
    // ���������Ĺ�ϵѡ���㷨

    printf("\n****���������Ĺ�ϵѡ���㷨****\n");

    // ��������
    {
        Buffer buf;

        if (!initBuffer(520, 64, &buf)) {
            perror("Buffer init failed.\n");
            return -1;
        }

        unsigned char *blk;
        OutBlk out_blk;
        if (InitOutBlock(&out_blk, &buf, 7, index_out_addr) == -1) {
            perror("Out block init failed.\n");
            return -1;
        }

        for (int i = 0; i < block_num; i++) {
            if ((blk = readBlockFromDisk(addr, &buf)) == NULL) {
                perror("Read block failed.\n");
                return -1;
            }

            int val[2];

            if (readProperty(blk, 0, 2, val) == -1) {
                perror("Read property failed.\n");
                return -1;
            }

            val[1] = addr;

            if (putInOutBlock(&out_blk, &buf, 2, val) == -1) {
                perror("Put in output block failed.\n");
                return -1;
            }

            if (readAddress(blk, 7, &addr) == -1) {
                perror("Read address failed.\n");
                return -1;
            }

            freeBlockInBuffer(blk, &buf);
        }
        freeOutBlockInBuffer(&out_blk, &buf);
    }

    // ѡ��
    {
        Buffer buf;

        if (!initBuffer(520, 64, &buf)) {
            perror("Buffer init failed.\n");
            return -1;
        }

        unsigned char *blk;

        if ((blk = readBlockFromDisk(index_out_addr, &buf)) == NULL) {
            perror("Read block failed.\n");
            return -1;
        }

        int res = -1;
        // ������ �����������������ݿ�����
        for (int i = 0; i < block_num; i++) {
            if (i % 7 == 0 && i != 0) {
                // Ҫ����һ��
                freeBlockInBuffer(blk, &buf);
                if ((blk = readBlockFromDisk(index_out_addr + i / 7, &buf)) == NULL) {
                    perror("Read block failed.\n");
                    return -1;
                }
            }

            int val[2];

            readProperty(blk, i % 7, 2, val);

            if (val[0] > value && i == 0) {
                // value С����Сֵ��û������������Ԫ��
                break;
            } else if (val[0] == value && i == 0) {
                // value ������Сֵ���ӵ�һ�����ݿ鿪ʼ��
                res = val[1];
                break;
            } else if (val[0] >= value) {
                //
                break;
            } else {
                res = val[1];
            }
        }

        freeBlockInBuffer(blk, &buf);

        int i = 0;
        int res_count = 0;
        int val[2];

        OutBlk out_blk;
        if (InitOutBlock(&out_blk, &buf, 7, out_addr) == -1) {
            perror("Out block init failed.\n");
            return -1;
        }

        if ((blk = readBlockFromDisk(res, &buf)) == NULL) {
            perror("Read block failed.\n");
            return -1;
        }

        printf("�������ݿ� %d.blk\n", res);

        while (readProperty(blk, i % 7, 2, val) != -1) {
            if (val[0] == value) {
                res_count++;
                printf("(R.A = %d, R.B = %d)\n", val[0], val[1]);
                if (putInOutBlock(&out_blk, &buf, 2, val) == -1) {
                    perror("Put in out block failed.\n");
                    return -1;
                }
            }

            if (val[0] > value) {
                break;
            }

            i++;

            if (i % 7 == 0) {
                if ((blk = readBlockFromDisk(res + i / 7, &buf)) == NULL) {
                    perror("Read block failed.\n");
                    return -1;
                }

                printf("�������ݿ� %d.blk\n", res + i / 7);
            }
        }

        freeOutBlockInBuffer(&out_blk, &buf);

        printf("����������Ԫ�� %d ��.\n", res_count);
        printf("IO��дһ�� %lu ��.\n", buf.numIO);
    }

    return 0;
}
