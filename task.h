//
// Created by Hongyu Chow on 20/05/20.
//

#ifndef DBEXP4_TASK_H
#define DBEXP4_TASK_H

int RSABLS(int addr, int out_addr, int block_num);

int TPMMS(int addr, int block_num, int set_res_out_addr, int out_addr, int set_block_num);

int IBRSA(int addr, int block_num, int index_out_addr, int out_addr, int value);

#endif //DBEXP4_TASK_H
