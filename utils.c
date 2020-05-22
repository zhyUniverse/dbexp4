//
// Created by Hongyu Chow on 20/05/18.
//

#include <stdlib.h>
#include <stdio.h>
#include "utils.h"

// 从一个块里读一个元组
// property_num 是一个元组里属性的数量，本实验中都是2
int readProperty(const unsigned char *blk, int pos, int property_num, int *res) {
    // 属性大小均为 4 字节

    if (*(blk + pos * 8) == '\0') {
        // 该位置没有属性值
        return -1;
    }

    for (int i = 0; i < property_num; i++) {
        char tmpStr[5] = {'\0' * 5};

        for (int j = 0; j < 4; j++) {
            tmpStr[j] = *(blk + pos * 8 + i * 4 + j);
        }

        res[i] = strtol(tmpStr, NULL, 10);
    }

    return 0;
}

// 向一个块里写入一个元组
// val == NULL 时写入 '\0'
void writeProperty(unsigned char *blk, int pos, int property_num, int *val) {
    if (val == NULL) {
        for (int i = 0; i < property_num * 4; i++) {
            *(blk + pos * property_num * 4 + i) = '\0';
        }
        return;
    }

    for (int i = 0; i < property_num; i++) {
        char tmpStr[5] = {'\0' * 5};

        snprintf(tmpStr, 5, "%d", val[i]);
        for (int j = 0; j < 4; j++) {
            *(blk + pos * property_num * 4 + i * 4 + j) = tmpStr[j];
        }
    }
}

// 从一个块里读下一块地址
// tuple_num 是一个块里元组数量
int readAddress(const unsigned char *blk, int tuple_num, int *res) {
    if (*(blk + tuple_num * 8) == '\0') {
        return -1;
    }

    char tmpStr[9] = {'\0' * 9};

    for (int i = 0; i < 8; i++) {
        tmpStr[i] = *(blk + tuple_num * 8 + i);
    }

    *res = strtol(tmpStr, NULL, 10);

    return 0;
}

// 向一个块里写入地址
void writeAddress(unsigned char *blk, int tuple_num, int val) {
    char tmpStr[9] = {'\0' * 9};

    snprintf(tmpStr, 9, "%d", val);
    for (int i = 0; i < 8; i++) {
        *(blk + tuple_num * 8 + i) = tmpStr[i];
    }
}

// 清空一个块，全部写为 '\0'
void clearBlock(unsigned char *blk, int size) {
    for (int i = 0; i < size; i++) {
        *(blk + i) = '\0';
    }
}

// 初始化输出缓冲区（实际上是缓冲区里的一个块）
int InitOutBlock(OutBlk *blk, Buffer *buffer, int size, int addr) {
    blk->blk = getNewBlockInBuffer(buffer);
    if (blk->blk == NULL) {
        perror("Buffer is full.\n");
        return -1;
    }
    blk->index = 0;
    blk->size = size;
    blk->out_addr = addr;

    return 0;
}

// 把一个元组置入输出缓冲区
int putInOutBlock(OutBlk *blk, Buffer *buffer, int property_num, int *val) {
    writeProperty(blk->blk, blk->index, property_num, val);
    blk->index++;

    if (blk->index == blk->size) {
        // 输出缓冲区已满
        writeAddress(blk->blk, 7, blk->out_addr + 1);
        if (writeBlockToDisk(blk->blk, blk->out_addr, buffer) == -1) {
            perror("Write block failed.\n");
            return -1;
        }
        printf("写入磁盘 %d.blk\n", blk->out_addr);
        if (InitOutBlock(blk, buffer, blk->size, blk->out_addr + 1) == -1) {
            clearBlock(blk->blk, 64);
            perror("Out block init failed.\n");
            return -1;
        }
    }
    return 0;
}

int freeOutBlockInBuffer(OutBlk *blk, Buffer *buffer) {
    if (blk->index != 0) {
        // 缓冲区有数据
        writeAddress(blk->blk, 7, blk->out_addr + 1);
        if (writeBlockToDisk(blk->blk, blk->out_addr, buffer) == -1) {
            perror("Write block failed.\n");
            return -1;
        }
        printf("写入磁盘 %d.blk\n", blk->out_addr);
        blk->index = 0;
    }
    freeBlockInBuffer(blk->blk, buffer);
    return 0;
}

int sort(unsigned char **blk, int block_num, int block_tuple_num, int property_num) {
    int val[block_num * block_tuple_num][property_num];

    for (int i = 0; i < block_num; i++) {
        for (int j = 0; j < block_tuple_num; j++) {
            if (readProperty(blk[i], j, property_num, val[i * block_tuple_num + j]) == -1) {
                perror("Read property failed.\n");
                return -1;
            }
        }
    }

    qsort(val, block_num * block_tuple_num, sizeof(int) * property_num, cmp);

    for (int i = 0; i < block_num; i++) {
        for (int j = 0; j < block_tuple_num; j++) {
            writeProperty(blk[i], j, property_num, val[i * block_tuple_num + j]);
        }
    }

    return 0;
}

int cmp(const void *a, const void *b) {
    return ((int *)a)[0] - ((int *)b)[0];
}

int cmpInBlock(unsigned char *blk, int tuple_num, int *res) {
    int res_pos = -1;
    int val[2];
    res[0] = INT_MAX;

    for (int i = 0; i < tuple_num; i++) {
        if (readProperty(blk, i, 2, val) == -1) {
            continue;
        }

        if (val[0] < res[0]) {
            res_pos = i;
            res[0] = val[0];
            res[1] = val[1];
        }
    }

    return res_pos;
}
