//
// Created by Hongyu Chow on 20/05/20.
//

#include <stdio.h>
#include <stdlib.h>
#include "task.h"
#include "extmem.h"
#include "utils.h"

int RSABLS(int start_addr, int block_num) {
    // 基于线性搜索的关系选择算法
    printf("\n****基于线性搜索的关系选择算法****\n");

    Buffer buf;  // buffer

    if (!initBuffer(520, 64, &buf)) {
        perror("Buffer Initialization Failed!\n");
        return -1;
    }

    unsigned char *blk;  // 内存块

    OutBlk out_blk;  // 输出块

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

        if (readAddress(blk, 7, &addr) == -1) {
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

int TPMMS(int addr, int block_num, int out_start_addr, int set_block_num) {
    // 两阶段多路归并排序算法

    printf("\n****两阶段多路归并排序算法****\n");

    Buffer buf;

    if (!initBuffer(520, 64, &buf)) {
        perror("Buffer Initialization Failed!\n");
        return -1;
    }

    // 子集合的数量
    int set_num = block_num / set_block_num + (block_num % set_block_num == 0 ? 0 : 1);

    // 每个子集合中各个块的地址
    // 本实验中块地址是顺序的所以没啥必要
    int set_addr[set_num][set_block_num];

    int counts[set_num];

    {
        // 内部排序
        unsigned char *blk[set_block_num];

        int count = 0;

        for (int i = 0; i < block_num; i++) {
            // 读取数据块
            if ((blk[i % set_block_num] = readBlockFromDisk(addr, &buf)) == NULL) {
                perror("Read block failed.\n");
                return -1;
            }

            set_addr[i / set_block_num][i % set_block_num] = addr;

            count++;

            // 读取下一个数据块的地址
            if (readAddress(blk[i % set_block_num], 7, &addr) == -1) {
                perror("Read address failed.\n");
                return -1;
            }

            if ((i + 1) % set_block_num == 0 || i == block_num - 1) {
                // 对一个子集合进行排序
                sort(blk, count, 7, 2);

                int next_addr;

                // 写回
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

        // 把每个子集合的第一个块读进内存
        // 并把第 i 个块的第 0 个元组放到 compare 块的第 i 个位置
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

        // 记录 compare 块中最小的值
        int min_val[2];
        // 记录 compare 块中最小的值的位置
        int pos;

        // 封装好的输出缓冲区，可以在满的时候自动输出到磁盘并清空缓冲区
        OutBlk out_blk;
        if (InitOutBlock(&out_blk, &buf, 7, out_start_addr) == -1) {
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

            if (blk_index == set_block_num) {
                // 超过一个子集合的块数，该子集合已经读完
                // 向 compare 块中写入 '\0'
                writeProperty(cmp, pos, 2, NULL);
            } else if (set_index[pos] % 7 == 0) {
                // 需要读取下一块
                freeBlockInBuffer(blk[pos], &buf);

                if ((blk[pos] = readBlockFromDisk(set_addr[pos][blk_index], &buf)) == NULL) {
                    perror("Read block failed.\n");
                    return -1;
                }
            }
                {
                if (next_blk != cur_set_blk_index[pos]) {
                    // 下一个要读的块不在内存中
                    // 释放当前块并读入下一个块
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
        // 输出缓冲区是在读入一个块前判断满没满
        // 所以最后需要把缓冲区中所有数据写到磁盘块
        freeOutBlockInBuffer(&out_blk, &buf);
    }

    return 0;
}
