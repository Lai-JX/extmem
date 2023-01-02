#include "def.h"

int tpmms(void)
{
    /* Initialize the buffer */
    if (!initBuffer(520, 64, &buf))
    {
        perror("Buffer Initialization Failed!\n");
        return -1;
    }
    printf("\n--------------------------------------\n");
    printf("两阶段多路归并排序算法: ");
    printf("\n--------------------------------------\n");

    // 对R进行排序
    printf("对R进行排序\n");
    tpmms_(R_BLK_BEGIN, R_BLK_NUM, BLKS_PER_SET, R_SORT_BLK_BEGIN);    // 初始块号为R_BLK_BEGIN，块数为R_BLK_NUM，每个子集8块，结果块初始块号为100
    printf("如上，关系R排序后输出到文件%d.blk到%d.blk。\n\n",R_SORT_BLK_BEGIN,R_SORT_BLK_BEGIN+R_BLK_NUM-1);

    // 对S进行排序
    printf("对S进行排序\n");
    tpmms_(S_BLK_BEGIN, S_BLK_NUM, BLKS_PER_SET, S_SORT_BLK_BEGIN);    // 每个子集8块，结果块初始块号为150
    printf("如上，关系S排序后输出到文件%d.blk到%d.blk。\n\n",S_SORT_BLK_BEGIN,S_SORT_BLK_BEGIN+S_BLK_NUM-1);

    freeBuffer(&buf);
    return 0;
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
    // 2. 归并
    int tol_set_num = BLK_NUM / step;     // 子集个数
    tpmms_step2(BLK_BEGIN, step, tol_set_num, res_addr_begin);
}


// 将addr后面链接的set_num个块进行排序
int tpmms_step1(int addr, int set_num)
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
        addr = read4bytes(blk[count]+ NUM_PER_BLK * 8);
        count++;
    }
    // （2）排序
    BubbleSort(buf.data, 65 * set_num - 16);
    // （3）写回
    count = 0;
    addr = addr_begin;
    while (count < set_num)
    {
        writeBlockToDisk(blk[count],addr, &buf);
        addr = read4bytes(blk[count]+ NUM_PER_BLK * 8);
        count++;
    }
    return 0;
}



// 归并。addr:起始块号；set_num:组内块数；tol_set_num:组数；res_addr_begin:保存结果块的起始块号
void tpmms_step2(int addr, int set_num, int tol_set_num, int res_addr_begin)
{
    unsigned char *blk[tol_set_num];        // 保存各子集块指针
    for (int i = 0; i < tol_set_num; i++)
        blk[i] = readBlockFromDisk(findAddr(addr,i*set_num), &buf);

    int set_ptr[tol_set_num];           // 每个组最小值的指针
    int visited_blk[tol_set_num];       // 每组已访问块数-1
    int cur_val = 0;
    int min_val = 999;              // 当前最小值
    int min_idx = 0;                // 最小值所在的组
    memset(set_ptr, 0, sizeof(set_ptr));
    memset(visited_blk, 0, sizeof(visited_blk));
    // 申请一个块用于保存结果
    unsigned char *res_blk = getNewBlockInBuffer(&buf);
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
        writeToBlk(&res_amount, res_addr_begin, &res_blk, blk[min_idx] + set_ptr[min_idx]);

        // 最小值所在块的指针进行偏移
        if(!shiftPointer(&set_ptr[min_idx], &visited_blk[min_idx], set_num, &blk[min_idx]))
            write4bytes(blk[min_idx] + 48, 999);        // 设为最大，避免被选中

        if (res_amount == set_num*NUM_PER_BLK*tol_set_num)
            break;
    }
    // 写回最后一个结果块
    writeLastBlk(res_amount, res_addr_begin, res_blk,NUM_PER_BLK);

    for (int i = 0; i < tol_set_num; i++)
        freeBlockInBuffer(blk[i], &buf);
}



