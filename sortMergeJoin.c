// #define R_SORT_BLK_BEGIN 100    // 排序后的R的初始块号
// #define S_SORT_BLK_BEGIN 150    // 排序后的S的初始块号
#include "def.h"
int sortMergeJoin()
{
    /* Initialize the buffer */
    if (!initBuffer(520, 64, &buf))
    {
        perror("Buffer Initialization Failed!\n");
        return -1;
    }
    printf("\n--------------------------------------\n");
    printf("基于排序的连接算法: ");
    printf("\n--------------------------------------\n");

    const int res_addr = 400;               // 结果起始块号
    unsigned char *blk[2];                  // 保存两个关系的当前数据块指针
    blk[0] = readBlockFromDisk(R_SORT_BLK_BEGIN, &buf);
    blk[1] = readBlockFromDisk(S_SORT_BLK_BEGIN, &buf);

    int set_ptr[2];                             // 每组最小值的指针
    int visited_blk[2];                         // 每组已访问块数-1
    int blk_num[2] = {R_BLK_NUM, S_BLK_NUM};    // 每组块数
    memset(set_ptr, 0, sizeof(set_ptr));
    memset(visited_blk, 0, sizeof(visited_blk));

    int A_val,C_val;

    // 申请一个块用于保存结果
    unsigned char *res_blk = getNewBlockInBuffer(&buf);
    int save_idx = 0;
    int save_addr = 0;
    int res_idx = 0;
    int res_amount = 0,next;
    while (true)
    {
        A_val = read4bytes(blk[0] + set_ptr[0]);
        C_val = read4bytes(blk[1] + set_ptr[1]);
        if (A_val == C_val)
        {
            save_idx = set_ptr[0];
            save_addr = visited_blk[0];
            while (A_val == C_val)
            {
                res_idx = res_amount % 3;               // 每个块存3条记录
                // 内存块写满
                if (res_amount !=0 && res_idx == 0 )
                {
                    // 下一块号
                    next = res_addr + res_amount / 3;
                    // 保存结果块的下一块号(便于链接)
                    write4bytes(res_blk + 8 * NUM_PER_BLK, next);
                    // 写回磁盘
                    writeBlockToDisk(res_blk, next-1, &buf);
                    printf("注：结果写入磁盘：%d\n", next - 1);
                    // 申请缓冲区
                    res_blk = getNewBlockInBuffer(&buf);
                }
                write8bytes(res_blk+res_idx*16, blk[0] + set_ptr[0]);
                write8bytes(res_blk+res_idx*16+8, blk[1] + set_ptr[1]);
                res_amount++;
                // 指针移动
                if(!shiftPointer(&set_ptr[0], &visited_blk[0], blk_num[0], &blk[0]))
                    break;
                A_val = read4bytes(blk[0] + set_ptr[0]);
                C_val = read4bytes(blk[1] + set_ptr[1]);
            }
            // 回退
            traceBack(&visited_blk[0], &blk[0], &set_ptr[0], save_idx, save_addr);
            // 指针移动
            if(!shiftPointer(&set_ptr[1], &visited_blk[1], blk_num[1], &blk[1]))
                break;
        }
        else if (A_val > C_val)
        {
            // // 指针移动
            if(!shiftPointer(&set_ptr[1], &visited_blk[1], blk_num[1], &blk[1]))
                break;
        }
        else
        {
            // 指针移动
            if(!shiftPointer(&set_ptr[0], &visited_blk[0], blk_num[0], &blk[0]))
                break;
        }
    }
    // 写回最后一个结果块
    writeLastBlk(res_amount, res_addr, res_blk,3);
    printf("\n总共连接%d次\n", res_amount);
    // 释放占用的缓存块
    for (int i = 0; i < 2; i++)
        freeBlockInBuffer(blk[i], &buf);
    return 0;
}
