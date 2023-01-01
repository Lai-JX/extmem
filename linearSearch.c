
#include "def.h"

int lab5_all(void)
{

    printf("ljx\n");
    /* Initialize the buffer */
    if (!initBuffer(520, 64, &buf))
    {
        perror("Buffer Initialization Failed!\n");
        return -1;
    }
    printf("data_begin:%p\n", buf.data);

    linearSearch();
    return 0;
}

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
    int idx, next;
    int i = 0, C, D;
    unsigned char *blk, *result_blk;     /* A pointer to a block */
    // 在缓冲区中申请新快用于存放结果
    result_blk = getNewBlockInBuffer(&buf);

    int addr = S_BLK_BEGIN;
    char str[5];
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
        for (i = 0; i < 7; i++) //一个blk存7个元组加一个地址
        {
            // 读取前4个字节
            C = read4bytes(blk + i * 8);
            // 读取后4个字节
            D = read4bytes(blk + i * 8 + 4);
            // 是否为指定元组
            if (C == 128)
            {
                printf("(C=%d, D=%d) \n", C, D);
                idx = amount % NUM_PER_BLK;
                // 内存块写满
                if (amount !=0 && idx == 0 )
                {
                    // 下一块号
                    next = result_addr + amount / NUM_PER_BLK;
                    // 保存结果块的下一块号(便于链接)
                    write4bytes(result_blk + 8 * NUM_PER_BLK, next);
                    // 写回磁盘
                    writeBlockToDisk(result_blk, next-1, &buf);
                    // 申请缓冲区
                    result_blk = getNewBlockInBuffer(&buf);
                }
                write8bytes(result_blk + 8 * idx, blk + i * 8);
                amount++;
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
    if (amount !=0)
    {
        if(amount % NUM_PER_BLK == 0)
            writeBlockToDisk(result_blk, result_addr + amount / NUM_PER_BLK-1, &buf);
        else
            writeBlockToDisk(result_blk, result_addr + amount / NUM_PER_BLK, &buf);
    }
        


    printf("注：结果写入磁盘：%d\n\n", result_addr);
    printf("满足选择条件的元组一共有%d个\n\n", amount);
    printf("IO读写一共%d次\n\n",buf.numIO);
    // printf1(60, 2);
    freeBuffer(&buf);
}
