#include "def.h"

int tpmms(void)
{
    /* Initialize the buffer */
    if (!initBuffer(520, 64, &buf))
    {
        perror("Buffer Initialization Failed!\n");
        return -1;
    }
    // 对R进行排序
    tpmms_R(4, 100);    // 每个子集4块，结果块初始块号为100
    // 对S进行排序
    tpmms_S(4, 150);    // 每个子集4块，结果块初始块号为150

    freeBuffer(&buf);
}
// 对R进行排序
void tpmms_R(step, res_addr_begin)
{
    int addr = R_BLK_BEGIN;
    // 1. 分组排序
    for (int i = R_BLK_BEGIN; i < R_BLK_BEGIN + R_BLK_NUM; i+=step)
    {
        tpmms_step1(addr, step);
        addr = findAddr(addr, step);
    }
    printf1(R_BLK_BEGIN, R_BLK_NUM);
    // 2. 归并
    int tol_set_num = R_BLK_NUM / step;     // 子集个数
    tpmms_step2(R_BLK_BEGIN, step, tol_set_num, res_addr_begin);

    printf1(res_addr_begin, R_BLK_NUM);
    printf("buf free num:%d\n", buf.numFreeBlk);
}
// 对S进行排序
void tpmms_S(step, res_addr_begin)
{
    int addr = S_BLK_BEGIN;
    // 1. 分组排序
    for (int i = S_BLK_BEGIN; i < S_BLK_BEGIN + S_BLK_NUM; i+=step)
    {
        tpmms_step1(addr, step);
        addr = findAddr(addr, step);
        printf("buf free num:%d\n", buf.numFreeBlk);
    }
    printf1(S_BLK_BEGIN, S_BLK_NUM);
    printf("buf free num:%d\n", buf.numFreeBlk);
    // 2. 归并
    int tol_set_num = S_BLK_NUM / step;     // 子集个数
    tpmms_step2(S_BLK_BEGIN, step, tol_set_num, res_addr_begin);

    printf1(res_addr_begin, S_BLK_NUM);
}


// 将addr后面链接的set_num个块进行排序
void tpmms_step1(int addr, int set_num)
{
    if (set_num >= buf.numAllBlk)
    {
        perror("set_num is too overflow!\n");
        return -1;
    }
    int addr_end;
    int addr_all[set_num];
    int count = 0;
    unsigned char *blk[set_num];
    // （1）读取set_num个块到缓冲区,并在块内排序
    while (count < set_num)
    {
        addr_all[count] = addr;
        if ((blk[count] = readBlockFromDisk(addr, &buf)) == NULL)
        {
            perror("Reading Block Failed!\n");
            return -1;
        }
        // 对块进行排序
        QuickSort(blk[count], 0, 48);
        /*下一块的块号*/
        addr = read4bytes(blk[count] + 7 * 8);
        count++;
    }
    addr_end = addr;
    // （2）set_num个块进行排序，并输出到原磁盘
    int set_ptr[set_num];           // 每个组最小值的指针
    int cur_val = 0;
    int min_val = 999;          // 当前最小值
    int min_idx = 0;                // 最小值所在的组
    memset(set_ptr, 0, sizeof(set_ptr));
    // 申请一个块用于保存结果
    unsigned char *res_blk = getNewBlockInBuffer(&buf);
    int res_idx = 0;
    int res_amount = 0;
    while (true)
    {
        min_val = 999;
        // 遍历各块，获取最小值
        for (int i = 0; i < set_num; i++)
        {
            cur_val = read4bytes(blk[i] + set_ptr[i]);
            if (cur_val < min_val)
            {
                min_val = cur_val;
                min_idx = i;
            }
        }
        // 写到内存，满时写回磁盘
        res_idx = res_amount % NUM_PER_BLK;
        // 内存块写满
        if (res_amount !=0 && res_idx == 0 )
        {
            // 保存结果块的下一块号
            write4bytes(res_blk + 8 * NUM_PER_BLK, addr_all[res_amount / NUM_PER_BLK]);
            // 写回磁盘
            writeBlockToDisk(res_blk, addr_all[res_amount / NUM_PER_BLK - 1], &buf);
            // 申请缓冲区
            res_blk = getNewBlockInBuffer(&buf);
        }
        write8bytes(res_blk + 8 * res_idx, blk[min_idx] + set_ptr[min_idx]);

        res_amount++;

        // 最小值所在块的指针进行偏移
        if (set_ptr[min_idx] < 48)
            set_ptr[min_idx] += 8;
        else
            write4bytes(blk[min_idx] + 48, 999);        // 设为最大，避免被选中
        
        if (res_amount == set_num*NUM_PER_BLK)
            break;
    }
    
    // 保存最后一个结果块的下一块号
    write4bytes(res_blk + 8 * NUM_PER_BLK, addr_end);
    if (res_amount !=0)     // 写回最后一个结果块
    {
        if (res_amount % NUM_PER_BLK == 0)
            writeBlockToDisk(res_blk, addr_all[res_amount / NUM_PER_BLK-1], &buf);
        else
            writeBlockToDisk(res_blk, addr_all[res_amount / NUM_PER_BLK], &buf);
    }
    
    // 释放各缓存块
    for (int i = 0; i < set_num; i++)
    {
        freeBlockInBuffer(blk[i], &buf);
    }
}



// 归并。addr:起始块号；set_num:组内块数；tol_set_num:组数；res_addr_begin:保存结果块的起始块号
void tpmms_step2(int addr, int set_num, int tol_set_num, int res_addr_begin)
{
    unsigned char *blk[tol_set_num];        // 保存各子集块指针
    for (int i = 0; i < tol_set_num; i++)
        blk[i] = readBlockFromDisk(findAddr(addr,i*set_num), &buf);

    printf("set_num:%d\n", set_num);
    printf("tol_set_num:%d\n", tol_set_num);

    int set_ptr[tol_set_num];           // 每个组最小值的指针
    int visited_blk[tol_set_num];       // 每组已访问块数
    int cur_val = 0;
    int min_val = 999;              // 当前最小值
    int min_idx = 0;                // 最小值所在的组
    memset(set_ptr, 0, sizeof(set_ptr));
    memset(visited_blk, 0, sizeof(visited_blk));
    // 申请一个块用于保存结果
    unsigned char *res_blk = getNewBlockInBuffer(&buf);
    int res_idx = 0;
    int res_amount = 0;
    while (true)
    {
        // printf("res_amount:%d\n", res_amount);
        min_val = 999;
        // 遍历各组，获取最小值
        for (int i = 0; i < tol_set_num; i++)
        {
            cur_val = read4bytes(blk[i] + set_ptr[i]);
            if (cur_val < min_val)
            {
                min_val = cur_val;
                min_idx = i;
            }
        }
        // 写到内存，满时写回磁盘
        res_idx = res_amount % NUM_PER_BLK;
        // printf("min_val:%d\n", min_val);
        // printf("min_idx:%d\n", min_idx);
        
        // 内存块写满
        if (res_amount !=0 && res_idx == 0 )
        {
            // printf("cur:%d\n", res_addr_begin + res_amount / NUM_PER_BLK - 1);
            // printf("next:%d\n", res_addr_begin + res_amount / NUM_PER_BLK);
            // printf("write back:%d\n", addr_all[res_amount / NUM_PER_BLK]);
            // 保存结果块的下一块号
            write4bytes(res_blk + 8 * NUM_PER_BLK, res_addr_begin + res_amount / NUM_PER_BLK);
            // 写回磁盘
            writeBlockToDisk(res_blk, res_addr_begin + res_amount / NUM_PER_BLK - 1, &buf);
            // 申请缓冲区
            res_blk = getNewBlockInBuffer(&buf);
        }
        // printf("当前最小：%d\n\n",read4bytes(blk[min_idx] + set_ptr[min_idx]));
        write8bytes(res_blk + 8 * res_idx, blk[min_idx] + set_ptr[min_idx]);

        res_amount++;

        // 最小值所在块的指针进行偏移
        if (set_ptr[min_idx] < 48)
            set_ptr[min_idx] += 8;
        else
        {
            if (visited_blk[min_idx] < set_num-1)
            {
                int next = read4bytes(blk[min_idx] + 7 * 8);
                // printf("data next:%d\n", next);
                freeBlockInBuffer(blk[min_idx], &buf);
                blk[min_idx] = readBlockFromDisk(next, &buf);
                visited_blk[min_idx]++;
                set_ptr[min_idx] = 0;
            }
            else
                write4bytes(blk[min_idx] + 48, 999);        // 设为最大，避免被选中
        }
            
        // printf("222\n");
        // printf("res_amount:%d\n\n", res_amount);
        if (res_amount == set_num*NUM_PER_BLK*tol_set_num)
            break;
        // printf("333\n");
        
        
        
    
    }
    // printf("1111112222\n");
    // 写回最后一个结果块
    if (res_amount !=0)
    {
        if (res_amount % NUM_PER_BLK == 0)
            writeBlockToDisk(res_blk, res_addr_begin + res_amount / NUM_PER_BLK - 1, &buf);
        else
            writeBlockToDisk(res_blk, res_addr_begin + res_amount / NUM_PER_BLK, &buf);
    }

    // printf("111111\n");
    // 1号块的排序结果 debug
    // blk[0] = readBlockFromDisk(2, &buf);
    // for (int k = 0; k < 64; k+=8)
    // {
    //     // blk[k] = readBlockFromDisk(k, &buf);
    //     printf("%d\n",read4bytes(blk[0]+k));
    // }
    
    for (int i = 0; i < tol_set_num; i++)
        freeBlockInBuffer(blk[i], &buf);
    // printf("11132331\n");

}

void printf1(int addr, int n)
{
    unsigned char *blk;
    for (int i = 0; i < n; i++)
    {
        printf("block %d\n", findAddr(addr, i));
        blk = readBlockFromDisk(findAddr(addr,i), &buf);
        for (int k = 0; k < 64; k+=8)
        {
            printf("%d\n",read4bytes(blk+k));
        }

        freeBlockInBuffer(blk, &buf);
    }
    
}

