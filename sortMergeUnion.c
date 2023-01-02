

// #define R_SORT_BLK_BEGIN 100    // 排序后的R的初始块号
// #define S_SORT_BLK_BEGIN 150    // 排序后的S的初始块号
#include "def.h"
int sortMergeUnion(void)
{
    /* Initialize the buffer */
    if (!initBuffer(520, 64, &buf))
    {
        perror("Buffer Initialization Failed!\n");
        return -1;
    }
    printf("\n--------------------------------------\n");
    printf("基于排序的集合的并算法 ");
    printf("\n--------------------------------------\n");

    const int res_addr = 800;               // 结果起始块号
    unsigned char *blk[2];                  // 保存两个关系的当前数据块指针
    blk[0] = readBlockFromDisk(R_SORT_BLK_BEGIN, &buf);
    blk[1] = readBlockFromDisk(S_SORT_BLK_BEGIN, &buf);


    int set_ptr[2];                             // 每组最小值的指针
    int visited_blk[2];                         // 每组已访问块数-1
    int blk_num[2] = {R_BLK_NUM, S_BLK_NUM};    // 每组块数
    memset(set_ptr, 0, sizeof(set_ptr));
    memset(visited_blk, 0, sizeof(visited_blk));

    int A_val,B_val,C_val,D_val;
    int finish_count = 0;

    // 申请一个块用于保存结果
    unsigned char *res_blk = getNewBlockInBuffer(&buf);
    int save_idx = 0;
    int save_addr = 0;
    int res_amount = 0;
    while (true)
    {
        A_val = read4bytes(blk[0] + set_ptr[0]);
        C_val = read4bytes(blk[1] + set_ptr[1]);
        B_val = read4bytes(blk[0] + set_ptr[0]+4);
        D_val = read4bytes(blk[1] + set_ptr[1]+4);
        if (A_val == C_val)
        {
            save_idx = set_ptr[0];
            save_addr = visited_blk[0];
            // 搜索B和D是否相等
            while (A_val == C_val && B_val != D_val)
            {
                // 指针移动
                if (!shiftPointer(&set_ptr[0], &visited_blk[0], blk_num[0], &blk[0]))
                    break;
                A_val = read4bytes(blk[0] + set_ptr[0]);
                B_val = read4bytes(blk[0] + set_ptr[0]+4);
            }
            // 搜索结果
            if (A_val == C_val && B_val!=D_val)     //A_val == C_val && B_val!=D_val 说明关系R已访问完
            {
                writeToBlk(&res_amount, res_addr, &res_blk, blk[0] + set_ptr[0]);
                write4bytes(blk[0] + 48, 999);      // 设为最大
                finish_count++;
            }
            if (A_val != C_val || B_val!=D_val)
            {
                writeToBlk(&res_amount, res_addr, &res_blk, blk[1] + set_ptr[1]);
            }
            // R回退
            traceBack(&visited_blk[0], &blk[0], &set_ptr[0], save_idx, save_addr);
            // S指针移动
            if (!shiftPointer(&set_ptr[1], &visited_blk[1], blk_num[1], &blk[1]))
                break;
        }
        else if(A_val < C_val)
        {
            writeToBlk(&res_amount, res_addr, &res_blk, blk[0] + set_ptr[0]);
            // 指针移动
            if (!shiftPointer(&set_ptr[0], &visited_blk[0], blk_num[0], &blk[0]))
            {
                write4bytes(blk[0] + 48, 999);      // 设为最大
                finish_count++;
            }

        }
        else
        {
            writeToBlk(&res_amount, res_addr, &res_blk, blk[1] + set_ptr[1]);
            // 指针移动
            if (!shiftPointer(&set_ptr[1], &visited_blk[1], blk_num[1], &blk[1]))
            {
                write4bytes(blk[1] + 48, 999);      // 设为最大
                finish_count++;
            }
        }
        if (finish_count==2)
            break;


    }
    writeLastBlk(res_amount, res_addr, res_blk, NUM_PER_BLK);
    printf("\nS和R的并集有%d个元组\n", res_amount);
    // 释放占用的缓存块
    for (int i = 0; i < 2; i++)
        freeBlockInBuffer(blk[i], &buf);
    return 0;
}
