//
// Created by Hongyu Chow on 20/05/20.
//

#include <stdio.h>
#include <stdlib.h>
#include "task.h"
#include "extmem.h"
#include "utils.h"

int RSBLS(int addr, int block_num, int out_addr) {
    // Relation selection based on linear search
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
        if ((blk = readBlockFromDisk(addr, &buf)) == NULL) {
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

        if (readAddress(blk, 64, &addr) == -1) {
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
    // Two-Phase Multiway Merge-Sort
    // ���׶ζ�·�鲢�����㷨

    printf("\n****���׶ζ�·�鲢�����㷨****\n");

    Buffer buf;

    if (!initBuffer(520, 64, &buf)) {
        perror("Buffer Initialization Failed!\n");
        return -1;
    }

    // �Ӽ��ϵ�����
    const int set_num = block_num / set_block_num + (block_num % set_block_num == 0 ? 0 : 1);

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
            if (readAddress(blk[i % set_block_num], 64, &addr) == -1) {
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
                    if (readAddress(blk[j], 64, &next_addr) == -1) {
                        perror("Read address failed.\n");
                        return -1;
                    }

                    writeAddress(blk[j], 64, set_addr + 1);

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

                    if ((blk[pos] = readBlockFromDisk(set_res_out_addr + pos * set_block_num + blk_index, &buf)) ==
                        NULL) {
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

int IBRS(int addr, int block_num, int index_out_addr, int out_addr, int value) {
    // Index-based relationship selection
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

            if (readAddress(blk, 64, &addr) == -1) {
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
            int val[2];

            if (readProperty(blk, i % 7, 2, val) == -1) {
                perror("Read property failed.\n");
                return -1;
            }

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

            if (i % 7 == 0 && i != 0) {
                // Ҫ����һ��
                freeBlockInBuffer(blk, &buf);
                if ((blk = readBlockFromDisk(index_out_addr + i / 7, &buf)) == NULL) {
                    perror("Read block failed.\n");
                    return -1;
                }
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

int RP(int addr, int block_num, int out_addr) {
    // Relation projection
    // ��ϵͶӰ�㷨
    // R.A ��ֵ��Ϊ [1, 40]

    printf("\n****��ϵͶӰ�㷨****\n");

    char set[40] = {0 * 40};

    Buffer buf;

    if (!initBuffer(520, 64, &buf)) {
        perror("Buffer init failed.\n");
        return -1;
    }

    unsigned char *blk;

    if ((blk = readBlockFromDisk(addr, &buf)) == NULL) {
        perror("Read block failed.\n");
        return -1;
    }
    printf("�������ݿ� %d.blk.\n", addr++);

    int i = 0;
    int val[2];

    OutBlk out_blk;
    if (InitOutBlock(&out_blk, &buf, 14, out_addr) == -1) {
        perror("Out block init failed.\n");
        return -1;
    }

    int p_val;
    int res_count = 0;

    while (readProperty(blk, i % 7, 2, val) != -1) {
        if (set[val[0] - 1] == 0) {
            set[val[0] - 1] = 1;
            printf("(R.A = %d)\n", val[0]);
            putInOutBlock(&out_blk, &buf, 1, &val[0]);
        }

        i++;

        if (i / 7 >= block_num) {
            break;
        }

        if (i % 7 == 0 && i != 0) {
            // ����һ�����ݿ�
            freeBlockInBuffer(blk, &buf);

            if ((blk = readBlockFromDisk(addr, &buf)) == NULL) {
                perror("Read block failed.\n");
                return -1;
            }
            printf("�������ݿ� %d.blk.\n", addr++);
        }
    }


    freeOutBlockInBuffer(&out_blk, &buf);

    return 0;
}

int SBCO(int r_addr, int s_addr, int r_block_num, int s_block_num, int out_addr) {
    // Sort-based join operation
    // ������������Ӳ����㷨

    printf("\n****������������Ӳ����㷨****\n");

    Buffer buf;

    if (!initBuffer(520, 64, &buf)) {
        perror("Buffer init failed.\n");
        return -1;
    }

    OutBlk out_blk;
    if (InitOutBlock(&out_blk, &buf, 7, out_addr) == -1) {
        perror("Out block init failed.\n");
        return -1;
    }

    int i = 0, j = 0;

    int s_start = -1;
    int s_start_addr = -1;

    unsigned char *r_blk, *s_blk;

    if ((r_blk = readBlockFromDisk(r_addr++, &buf)) == NULL) {
        perror("Read block failed.\n");
        return -1;
    }

    if ((s_blk = readBlockFromDisk(s_addr++, &buf)) == NULL) {
        perror("Read block failed.\n");
        return -1;
    }

    int r_val[2], s_val[2];
    int pre_r_val = -1;
    int r_cur_blk = 0, s_cur_blk = 0;
    int con_1 = 0, con_2 = 0;

    int res_count = 0;

    while (readProperty(r_blk, i % 7, 2, r_val) != -1 && readProperty(s_blk, j % 7, 2, s_val) != -1) {
        if (pre_r_val == r_val[0] && con_1 == 1 && con_2 == 1) {
            j = s_start;
            s_addr = s_start_addr;

            // R ���������� A ��ȵ�Ԫ�飬�� S ��ָ���ƻ�ȥ
            freeBlockInBuffer(s_blk, &buf);

            if ((s_blk = readBlockFromDisk(s_addr++, &buf)) == NULL) {
                perror("Read block failed.\n");
                return -1;
            }

            s_cur_blk = j / 7;

            con_1 = 0;
            con_2 = 0;

            continue;
        }

        if (r_val[0] < s_val[0]) {
            pre_r_val = r_val[0];
            con_2 = 1;
            i++;
        } else if (r_val[0] == s_val[0]) {
            con_1 = 1;
            if (s_start == -1) {
                s_start = j;
                s_start_addr = s_addr - 1;
            }
            putInOutBlock(&out_blk, &buf, 2, r_val);
            putInOutBlock(&out_blk, &buf, 2, s_val);
            res_count++;
            j++;
        } else {
            con_1 = 0;
            s_start = -1;
            j++;
        }



        if (i / 7 >= r_block_num || j / 7 >= s_block_num) {
            break;
        }

        if (i % 7 == 0 && r_cur_blk < i / 7) {
            freeBlockInBuffer(r_blk, &buf);

            if ((r_blk = readBlockFromDisk(r_addr++, &buf)) == NULL) {
                perror("Read block failed.\n");
                return -1;
            }

            r_cur_blk = i / 7;
        }

        if (j % 7 == 0 && s_cur_blk < j / 7) {
            freeBlockInBuffer(s_blk, &buf);

            if ((s_blk = readBlockFromDisk(s_addr++, &buf)) == NULL) {
                perror("Read block failed.\n");
                return -1;
            }

            s_cur_blk = j / 7;
        }
    }

    freeOutBlockInBuffer(&out_blk, &buf);

    printf("�ܹ����� %d ��.\n", res_count);

    return 0;
}

int SO(int r_addr, int s_addr, int r_block_num, int s_block_num, int out_addr) {
    // Set operation
    // ���ϲ����㷨
    // ��

    printf("\n****���ϲ����㷨����****\n");

    Buffer buf;

    if (!initBuffer(520, 64, &buf)) {
        perror("Buffer init failed.\n");
        return -1;
    }

    OutBlk out_blk;
    if (InitOutBlock(&out_blk, &buf, 7, out_addr) == -1) {
        perror("Out block init failed.\n");
        return -1;
    }

    int i = 0, j = 0;

    int s_start = -1;
    int s_start_addr = -1;

    unsigned char *r_blk, *s_blk;

    if ((r_blk = readBlockFromDisk(r_addr++, &buf)) == NULL) {
        perror("Read block failed.\n");
        return -1;
    }

    if ((s_blk = readBlockFromDisk(s_addr++, &buf)) == NULL) {
        perror("Read block failed.\n");
        return -1;
    }

    int r_val[2], s_val[2];
    int pre_r_val = -1;
    int r_cur_blk = 0, s_cur_blk = 0;
    int con_1 = 0, con_2 = 0;

    int res_count = 0;

    while (readProperty(r_blk, i % 7, 2, r_val) != -1 && readProperty(s_blk, j % 7, 2, s_val) != -1) {
        if (pre_r_val == r_val[0] && con_1 == 1 && con_2 == 1) {
            j = s_start;
            s_addr = s_start_addr;

            // R ���������� A ��ȵ�Ԫ�飬�� S ��ָ���ƻ�ȥ
            freeBlockInBuffer(s_blk, &buf);

            if ((s_blk = readBlockFromDisk(s_addr++, &buf)) == NULL) {
                perror("Read block failed.\n");
                return -1;
            }

            s_cur_blk = j / 7;

            con_1 = 0;
            con_2 = 0;

            continue;
        }

        if (r_val[0] < s_val[0]) {
            pre_r_val = r_val[0];
            con_2 = 1;
            i++;
        } else if (r_val[0] == s_val[0]) {
            con_1 = 1;
            if (s_start == -1) {
                s_start = j;
                s_start_addr = s_addr - 1;
            }
            if (r_val[1] == s_val[1]) {
                printf("(X = %d, Y = %d)\n", r_val[0], r_val[1]);
                putInOutBlock(&out_blk, &buf, 2, r_val);
                res_count++;
            }
            j++;
        } else {
            con_1 = 0;
            s_start = -1;
            j++;
        }



        if (i / 7 >= r_block_num || j / 7 >= s_block_num) {
            break;
        }

        if (i % 7 == 0 && r_cur_blk < i / 7) {
            freeBlockInBuffer(r_blk, &buf);

            if ((r_blk = readBlockFromDisk(r_addr++, &buf)) == NULL) {
                perror("Read block failed.\n");
                return -1;
            }

            r_cur_blk = i / 7;
        }

        if (j % 7 == 0 && s_cur_blk < j / 7) {
            freeBlockInBuffer(s_blk, &buf);

            if ((s_blk = readBlockFromDisk(s_addr++, &buf)) == NULL) {
                perror("Read block failed.\n");
                return -1;
            }

            s_cur_blk = j / 7;
        }
    }

    freeOutBlockInBuffer(&out_blk, &buf);

    return 0;
}
