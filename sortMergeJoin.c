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
    printf("基于排序的连接算法 ");
    printf("\n--------------------------------------\n");
    // printf1(R_SORT_BLK_BEGIN, R_BLK_NUM);
    // printf1(S_SORT_BLK_BEGIN, S_BLK_NUM);
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
        // printf("A:%d,C:%d\n", A_val, C_val);
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
                    write4bytes(res_blk + 8 * 7, next);
                    // 写回磁盘
                    writeBlockToDisk(res_blk, next-1, &buf);
                    printf("注：结果写入磁盘：%d\n", next - 1);
                    // 申请缓冲区
                    res_blk = getNewBlockInBuffer(&buf);
                }
                write8bytes(res_blk+res_idx*16, blk[0] + set_ptr[0]);
                write8bytes(res_blk+res_idx*16+8, blk[1] + set_ptr[1]);
                /*debug*/
                // printf("(%d,%d),(%d,%d)\n", read4bytes(blk[0] + set_ptr[0]), read4bytes(blk[0] + set_ptr[0]+ 4), read4bytes(blk[1] + set_ptr[1]), read4bytes(blk[1] + set_ptr[1] + 4));

                res_amount++;
                // 指针移动
                if (set_ptr[0] < 48)
                    set_ptr[0] += 8;
                else
                {
                    if (visited_blk[0] < blk_num[0]-1)           // 读取下一块
                    {
                        int next = read4bytes(blk[0] + 7 * 8);

                        freeBlockInBuffer(blk[0], &buf);
                        blk[0] = readBlockFromDisk(next, &buf);
                        visited_blk[0]++;
                        set_ptr[0] = 0;
                    }
                    else
                        break;        
                }
                A_val = read4bytes(blk[0] + set_ptr[0]);
                C_val = read4bytes(blk[1] + set_ptr[1]);
            }
            // 回退
            if (visited_blk[0] != save_addr)  // 若读取到别的块
            {
                freeBlockInBuffer(blk[0], &buf);
                // printf("findaddr:%d\n", findAddr(R_BLK_BEGIN, save_addr));
                blk[0] = readBlockFromDisk(findAddr(R_SORT_BLK_BEGIN,save_addr), &buf);
                visited_blk[0] = save_addr;
            }
            set_ptr[0] = save_idx;
            // 指针移动
            if (set_ptr[1] < 48)
                set_ptr[1] += 8;
            else
            {
                if (visited_blk[1] < blk_num[1]-1)           // 读取下一块
                {
                    int next = read4bytes(blk[1] + 7 * 8);

                    freeBlockInBuffer(blk[1], &buf);
                    blk[1] = readBlockFromDisk(next, &buf);
                    visited_blk[1]++;
                    set_ptr[1] = 0;
                }
                else
                    break;        
            }
        }
        else if (A_val > C_val)
        {
            // 指针移动
            if (set_ptr[1] < 48)
                set_ptr[1] += 8;
            else
            {
                if (visited_blk[1] < blk_num[1]-1)           // 读取下一块
                {
                    int next = read4bytes(blk[1] + 7 * 8);

                    freeBlockInBuffer(blk[1], &buf);
                    blk[1] = readBlockFromDisk(next, &buf);
                    visited_blk[1]++;
                    set_ptr[1] = 0;
                }
                else
                    break;       
            }
        }
        else
        {
            // 指针移动
            if (set_ptr[0] < 48)
                set_ptr[0] += 8;
            else
            {
                if (visited_blk[0] < blk_num[0]-1)           // 读取下一块
                {
                    int next = read4bytes(blk[0] + 7 * 8);

                    freeBlockInBuffer(blk[0], &buf);
                    blk[0] = readBlockFromDisk(next, &buf);
                    visited_blk[0]++;
                    set_ptr[0] = 0;
                }
                else
                    break;        
            }
        }
    }
    // 写回最后一个结果块
    if (res_amount !=0)
    {
        next = res_addr + res_amount / 3;
        if (res_amount % 3 == 0)
            next--;
        writeBlockToDisk(res_blk, next, &buf);
        printf("注：结果写入磁盘：%d\n\n", next);
    }
    printf("总共连接%d次\n", res_amount);
    // 释放占用的缓存块
    for (int i = 0; i < 2; i++)
        freeBlockInBuffer(blk[i], &buf);
}