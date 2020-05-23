//
// Created by Hongyu Chow on 20/05/20.
//

#ifndef DBEXP4_TASK_H
#define DBEXP4_TASK_H

int RSBLS(int addr, int out_addr, int block_num);

int TPMMS(int addr, int block_num, int set_res_out_addr, int out_addr, int set_block_num);

int IBRS(int addr, int block_num, int index_out_addr, int out_addr, int value);

int RP(int addr, int block_num, int out_addr);

int SBCO(int r_addr, int s_addr, int r_block_num, int s_block_num, int out_addr);

int SO(int r_addr, int s_addr, int r_block_num, int s_block_num, int out_addr);

#endif //DBEXP4_TASK_H
