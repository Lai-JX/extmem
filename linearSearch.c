
#include "def.h"

int linearSearch(void)
{
    /* Initialize the buffer */
    if (!initBuffer(520, 64, &buf))
    {
        perror("Buffer Initialization Failed!\n");
        return -1;
    }
    printf("\n--------------------------------------\n");
    printf("基于线性搜索的关系选择算法 S.C=128:");
    printf("\n--------------------------------------\n");
    const int result_addr = 60;         // 写入磁盘块的块号
    int i = 0, C, D;
    unsigned char *blk, *result_blk;     /* A pointer to a block */
    // 在缓冲区中申请新快用于存放结果
    result_blk = getNewBlockInBuffer(&buf);

    int addr = S_BLK_BEGIN;
    int count = 0;          // 已访问的块数
    int amount = 0;         // 满足条件的元组数量

    // 遍历存放关系S的所有块
    while (count < S_BLK_NUM)
    {
        printf("读入数据块%d\n", addr);
        if ((blk = readBlockFromDisk(addr, &buf)) == NULL)
        {
            perror("Reading Block Failed!\n");
            return -1;
        }
        // 遍历7个元组
        for (i = 0; i < NUM_PER_BLK; i++) //一个blk存7个元组加一个地址
        {
            // 读取前4个字节
            C = read4bytes(blk + i * 8);
            // 读取后4个字节
            D = read4bytes(blk + i * 8 + 4);
            // 是否为指定元组
            if (C == 128)
            {
                writeToBlk(&amount, result_addr, &result_blk, blk + i * 8);
                printf("(C=%d, D=%d) \n", C, D);

            }

        }
        // 读取下一块号
        addr = read4bytes(blk + i * 8);
        // 解除当前块对缓冲区的占用
        freeBlockInBuffer(blk, &buf);
        // 已访问块数加1
        count++;
    }
    // 写回最后一个结果块
    writeLastBlk(amount, result_addr, result_blk,NUM_PER_BLK);

    printf("\n满足选择条件的元组一共有%d个\n\n", amount);
    printf("IO读写一共%d次\n\n",buf.numIO);
    freeBuffer(&buf);
    return 0;
}
