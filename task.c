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
    // 基于线性搜索的关系选择算法
    printf("\n****基于线性搜索的关系选择算法****\n");

    Buffer buf;  // buffer

    if (!initBuffer(520, 64, &buf)) {
        perror("Buffer Initialization Failed!\n");
        return -1;
    }

    unsigned char *blk;  // 内存块

    OutBlk out_blk;  // 输出块

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
        printf("读入数据块 %d.blk.\n", addr);

        // 按顺序读取7个元组
        for (int j = 0; j < 7; j++) {
            int val[2];

            if (readProperty(blk, j, 2, val) == -1) {
                perror("Read property failed.\n");
                return -1;
            }

            if (val[0] == 30) {
                res_num++;
                printf("(R.A = %d, R.B = %d)\n", val[0], val[1]);
                // 置入输出块
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

    printf("满足条件的元组一共%d个.\n", res_num);

    printf("IO读写一共%d次.\n", io_num);

    return 0;
}

int TPMMS(int addr, int block_num, int set_res_out_addr, int out_addr, int set_block_num) {
    // Two-Phase Multiway Merge-Sort
    // 两阶段多路归并排序算法

    printf("\n****两阶段多路归并排序算法****\n");

    Buffer buf;

    if (!initBuffer(520, 64, &buf)) {
        perror("Buffer Initialization Failed!\n");
        return -1;
    }

    // 子集合的数量
    const int set_num = block_num / set_block_num + (block_num % set_block_num == 0 ? 0 : 1);

    // 每个子集合中各个块的地址
    // 本实验中块地址是顺序的所以没啥必要
//    int set_addr[set_num][set_block_num];

    int counts[set_num];

    {
        // 内部排序
        unsigned char *blk[set_block_num];

        int count = 0;

        int set_addr = set_res_out_addr;

        for (int i = 0; i < block_num; i++) {
            // 读取数据块
            if ((blk[i % set_block_num] = readBlockFromDisk(addr, &buf)) == NULL) {
                perror("Read block failed.\n");
                return -1;
            }

            count++;

            // 读取下一个数据块的地址
            if (readAddress(blk[i % set_block_num], 64, &addr) == -1) {
                perror("Read address failed.\n");
                return -1;
            }

            if ((i + 1) % set_block_num == 0 || i == block_num - 1) {
                counts[i / set_block_num] = count;

                // 对一个子集合进行排序
                sort(blk, count, 7, 2);

                int next_addr;

                // 写回
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
        // 四个块用于二阶段排序的时候依次存四个子集合中的每个块
        unsigned char *blk[set_num];
        // compare内存块
        unsigned char *cmp;

        // 记录每个子集合中读到了哪个元组 0 - (4*7-1)
        // 该值计算应该读当前哪个块 并通过 set_addr 获取块地址
        // 这是考虑了地址不连续的情况 在这个实验里有点多此一举
        int set_index[set_num];

        // 初始化 compare 块
        if ((cmp = getNewBlockInBuffer(&buf)) == NULL) {
            perror("Buffer is full.\n");
            return -1;
        }

        int set_addr = set_res_out_addr;

        // 把每个子集合的第一个块读进内存
        // 并把第 i 个块的第 0 个元组放到 compare 块的第 i 个位置
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

        // 记录 compare 块中最小的值
        int min_val[2];
        // 记录 compare 块中最小的值的位置
        int pos;

        // 封装好的输出缓冲区，可以在满的时候自动输出到磁盘并清空缓冲区
        OutBlk out_blk;
        if (InitOutBlock(&out_blk, &buf, 7, out_addr) == -1) {
            perror("Out block init failed.\n");
            return -1;
        }

        // 归并排序
        // compare 中没有值时结束循环
        while ((pos = cmpInBlock(cmp, set_num, min_val)) != -1) {
            if (putInOutBlock(&out_blk, &buf, 2, min_val) == -1) {
                perror("Put in out block failed.\n");
                return -1;
            }

            // 集合元组索引加 1
            set_index[pos]++;

            int blk_index = set_index[pos] / 7;

            if (blk_index >= counts[pos]) {
                // 超过一个子集合的块数，该子集合已经读完
                // 向 compare 块中写入 '\0'
                writeProperty(cmp, pos, 2, NULL);
            } else {
                if (set_index[pos] % 7 == 0) {
                    // 需要读取下一块
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
        // 输出缓冲区是在读入一个块前判断满没满
        // 所以最后需要把缓冲区中所有数据写到磁盘块
        freeOutBlockInBuffer(&out_blk, &buf);
    }

    printf("IO读写次数一共%lu次.\n", buf.numIO);

    return 0;
}

int IBRS(int addr, int block_num, int index_out_addr, int out_addr, int value) {
    // Index-based relationship selection
    // 基于索引的关系选择算法

    printf("\n****基于索引的关系选择算法****\n");

    // 建立索引
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

    // 选择
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
        // 主索引 索引项数量等于数据块数量
        for (int i = 0; i < block_num; i++) {
            int val[2];

            if (readProperty(blk, i % 7, 2, val) == -1) {
                perror("Read property failed.\n");
                return -1;
            }

            if (val[0] > value && i == 0) {
                // value 小于最小值，没有满足条件的元组
                break;
            } else if (val[0] == value && i == 0) {
                // value 等于最小值，从第一个数据块开始找
                res = val[1];
                break;
            } else if (val[0] >= value) {
                //
                break;
            } else {
                res = val[1];
            }

            if (i % 7 == 0 && i != 0) {
                // 要读下一块
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

        printf("读入数据块 %d.blk\n", res);

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

                printf("读入数据块 %d.blk\n", res + i / 7);
            }
        }

        freeOutBlockInBuffer(&out_blk, &buf);

        printf("满足条件的元组 %d 个.\n", res_count);
        printf("IO读写一共 %lu 次.\n", buf.numIO);
    }

    return 0;
}

int RP(int addr, int block_num, int out_addr) {
    // Relation projection
    // 关系投影算法
    // R.A 的值域为 [1, 40]

    printf("\n****关系投影算法****\n");

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
    printf("读入数据块 %d.blk.\n", addr++);

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
            // 读下一个数据块
            freeBlockInBuffer(blk, &buf);

            if ((blk = readBlockFromDisk(addr, &buf)) == NULL) {
                perror("Read block failed.\n");
                return -1;
            }
            printf("读入数据块 %d.blk.\n", addr++);
        }
    }


    freeOutBlockInBuffer(&out_blk, &buf);

    return 0;
}

int SBCO(int r_addr, int s_addr, int r_block_num, int s_block_num, int out_addr) {
    // Sort-based join operation
    // 基于排序的连接操作算法

    printf("\n****基于排序的连接操作算法****\n");

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

            // R 中连续两个 A 相等的元组，把 S 的指针移回去
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

    printf("总共连接 %d 次.\n", res_count);

    return 0;
}

int SO(int r_addr, int s_addr, int r_block_num, int s_block_num, int out_addr) {
    // Set operation
    // 集合操作算法
    // 交

    printf("\n****集合操作算法：交****\n");

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

            // R 中连续两个 A 相等的元组，把 S 的指针移回去
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
