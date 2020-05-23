//
// Created by Hongyu Chow on 20/05/18.
//

#ifndef DBEXP4_UTILS_H
#define DBEXP4_UTILS_H

#include "extmem.h"

#define FIRST_PROPERTY 0
#define SECOND_PROPERTY 4

typedef struct {
    unsigned char *blk;
    int index;
    int size;
    int out_addr;
}OutBlk;

// 从内存块中读取一个元组
int readProperty(const unsigned char *blk, int pos, int property_num, int *res);

void writeProperty(unsigned char *blk, int pos, int property_num, int *val);

int readAddress(const unsigned char *blk, int tuple_num, int *res);

void writeAddress(unsigned char *blk, int tuple_num, int val);

void clearBlock(unsigned char *blk, int size);

int InitOutBlock(OutBlk *blk, Buffer *buffer, int size, int addr);

int putInOutBlock(OutBlk *blk, Buffer *buffer, int property_num, int *val);

int freeOutBlockInBuffer(OutBlk *blk, Buffer *buffer);

int sort(unsigned char **blk, int block_num, int block_tuple_num, int property_num);

int cmp(const void *a, const void *b);

int cmpInBlock(unsigned char *blk, int tuple_num, int *res);

int findIndex(int addr, int block_num, int value, Buffer *buf);

#endif //DBEXP4_UTILS_H
