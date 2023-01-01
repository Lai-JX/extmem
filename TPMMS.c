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
    tpmms_(R_BLK_BEGIN, R_BLK_NUM, BLKS_PER_SET, R_SORT_BLK_BEGIN);    // 初始块号为R_BLK_BEGIN，块数为R_BLK_NUM，每个子集8块，结果块初始块号为100
    // 对S进行排序
    tpmms_(S_BLK_BEGIN, S_BLK_NUM, BLKS_PER_SET, S_SORT_BLK_BEGIN);    // 每个子集8块，结果块初始块号为150

    freeBuffer(&buf);
}
// 进行排序
void tpmms_(int BLK_BEGIN, int BLK_NUM, int step, int res_addr_begin)
{
    int addr = BLK_BEGIN;
    // 1. 分组排序
    for (int i = 0; i < BLK_NUM; i+=step)
    {
        tpmms_step1(addr, step);
        addr = findAddr(addr, step);
    }
    // printf1(BLK_BEGIN, BLK_NUM);
    // 2. 归并
    int tol_set_num = BLK_NUM / step;     // 子集个数
    tpmms_step2(BLK_BEGIN, step, tol_set_num, res_addr_begin);

    // printf1(res_addr_begin, BLK_NUM);
}


// 将addr后面链接的set_num个块进行排序
void tpmms_step1(int addr, int set_num)
{
    if (set_num > buf.numAllBlk)
    {
        perror("set_num overflow!\n");
        return -1;
    }
    int count = 0;
    int addr_begin = addr;
    unsigned char *blk[set_num];
    // （1）读取set_num个块到缓冲区
    while (count < set_num)
    {
        if ((blk[count]=readBlockFromDisk(addr, &buf)) == NULL)
        {
            perror("Reading Block Failed!\n");
            return -1;
        }
        /*下一块的块号*/
        addr = read4bytes(blk[count]+ 7 * 8);
        count++;
    }
    // （2）排序
    BubbleSort(buf.data, 65 * set_num - 16);
    // （3）写回
    count = 0;
    addr = addr_begin;
    while (count < set_num)
    {
        // printf("addr:%d\n", addr);
        writeBlockToDisk(blk[count],addr, &buf);
        addr = read4bytes(blk[count]+ 7 * 8);
        count++;
    }
}



// 归并。addr:起始块号；set_num:组内块数；tol_set_num:组数；res_addr_begin:保存结果块的起始块号
void tpmms_step2(int addr, int set_num, int tol_set_num, int res_addr_begin)
{
    unsigned char *blk[tol_set_num];        // 保存各子集块指针
    for (int i = 0; i < tol_set_num; i++)
        blk[i] = readBlockFromDisk(findAddr(addr,i*set_num), &buf);

    // printf("set_num:%d\n", set_num);
    // printf("tol_set_num:%d\n", tol_set_num);

    int set_ptr[tol_set_num];           // 每个组最小值的指针
    int visited_blk[tol_set_num];       // 每组已访问块数-1
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
        
        // 内存块写满
        if (res_amount !=0 && res_idx == 0 )
        {
            write4bytes(res_blk + 8 * NUM_PER_BLK, res_addr_begin + res_amount / NUM_PER_BLK);
            // 写回磁盘
            writeBlockToDisk(res_blk, res_addr_begin + res_amount / NUM_PER_BLK - 1, &buf);
            // 申请缓冲区
            res_blk = getNewBlockInBuffer(&buf);
        }
        write8bytes(res_blk + 8 * res_idx, blk[min_idx] + set_ptr[min_idx]);

        res_amount++;

        // 最小值所在块的指针进行偏移
        if (set_ptr[min_idx] < 48)
            set_ptr[min_idx] += 8;
        else
        {
            if (visited_blk[min_idx] < set_num-1)           // 读取下一块
            {
                int next = read4bytes(blk[min_idx] + 7 * 8);

                freeBlockInBuffer(blk[min_idx], &buf);
                blk[min_idx] = readBlockFromDisk(next, &buf);
                visited_blk[min_idx]++;
                set_ptr[min_idx] = 0;
            }
            else
                write4bytes(blk[min_idx] + 48, 999);        // 设为最大，避免被选中
        }
            
        if (res_amount == set_num*NUM_PER_BLK*tol_set_num)
            break;
    }
    // 写回最后一个结果块
    if (res_amount !=0)
    {
        if (res_amount % NUM_PER_BLK == 0)
            writeBlockToDisk(res_blk, res_addr_begin + res_amount / NUM_PER_BLK - 1, &buf);
        else
            writeBlockToDisk(res_blk, res_addr_begin + res_amount / NUM_PER_BLK, &buf);
    }
    
    for (int i = 0; i < tol_set_num; i++)
        freeBlockInBuffer(blk[i], &buf);
}



