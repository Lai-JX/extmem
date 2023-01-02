#include "def.h"

int indexSearch(void)
{


    printf("\n--------------------------------------\n");
    printf("基于索引的关系选择算法 S.C=128:");
    printf("\n--------------------------------------\n");

    int search_val = 128;
    int res_addr = 250;
    // 创建索引文件
    int index_blk_num = createIndexBlk(S_SORT_BLK_BEGIN, S_BLK_NUM, 200);
    /* Initialize the buffer */
    if (!initBuffer(520, 64, &buf))
    {
        perror("Buffer Initialization Failed!\n");
        return -1;
    }
    // printf1(S_SORT_BLK_BEGIN, S_BLK_NUM);


    unsigned char *index_blk, *blk, *res_blk;
    int addr = 200,idx,val,C,j;
    int data_addr=0, data_addr_pre=0;
    int count = 0;
    // 结果块
    res_blk = getNewBlockInBuffer(&buf);
    // 根据索引进行搜索
    for (int i = 0; i < S_BLK_NUM; i++)
    {
        idx = i % NUM_PER_BLK;
        // 读取索引块
        if (idx==0)
        {
            index_blk = readBlockFromDisk(addr, &buf);
            printf("读入索引块%d\n", addr);
            addr = read4bytes(index_blk + 7 * 8);
        }

        // 索引字段
        val = read4bytes(index_blk + idx * 8);

        data_addr_pre = data_addr;
        data_addr = read4bytes(index_blk + idx * 8 + 4);

        // 第一种情况：val小于要查找的值，继续向下搜索
        if (val < search_val)
        {
            if ((i+1)%NUM_PER_BLK==0)
                printf("没有满足条件的元组\n");
        }
        // 第二种情况：val大于要查找的值，说明需要查找的元组均在blk_pre块中
        else if(val > search_val)
        {
            blk = readBlockFromDisk(data_addr_pre, &buf);
            for (j = 0; j < NUM_PER_BLK; j++)
            {
                C = read4bytes(blk + j * 8);
                if (C==search_val){
                    printf("(C=%d, D=%d) \n", C, read4bytes(blk + j * 8 + 4));
                    write8bytes(res_blk+count*8,blk+j*8);
                    count++;
                }
                if (count)
                    writeBlockToDisk(res_blk, res_addr, &buf);
                else
                    freeBlockInBuffer(res_blk, &buf);
            }
            freeBlockInBuffer(blk, &buf);
            break;
        }
        // 第三种情况：val等于要查找的值，说明需要查找的元组可能在blk_pre块中也存在
        else
        {
            // 先搜索blk_pre
            blk = readBlockFromDisk(data_addr_pre, &buf);
            printf("读入数据块%d\n", data_addr_pre);
            for (j = 0; j < NUM_PER_BLK; j++)
            {
                C = read4bytes(blk + j * 8);
                if (C==search_val){
                    printf("(C=%d, D=%d) \n", C, read4bytes(blk + j * 8 + 4));
                    write8bytes(res_blk + count * 8, blk + j * 8);
                    count++;
                }

            }
            freeBlockInBuffer(blk, &buf);
            // 再搜索当前blk
            blk = readBlockFromDisk(data_addr, &buf);
            printf("读入数据块%d\n", data_addr);
            while (j==7)        // 满足要求的元组可能占多块
            {
                for (j = 0; j < NUM_PER_BLK; j++)
                {


                    C = read4bytes(blk + j * 8);
                    if (C==search_val){
                        // idx = count % NUM_PER_BLK;
                        // if (count!=0 && idx==0)
                        // {
                        //     // 下一块号
                        //     next = res_addr + count / NUM_PER_BLK;
                        //     // 保存结果块的下一块号(便于链接)
                        //     write4bytes(res_blk + 8 * NUM_PER_BLK, next);
                        //     // 写回磁盘
                        //     writeBlockToDisk(res_blk, next-1, &buf);
                        //     printf("注：结果写入磁盘：%d\n", next-1);
                        //     // 申请缓冲区
                        //     res_blk = getNewBlockInBuffer(&buf);
                        // }
                        // write8bytes(res_blk + idx * 8, blk + j * 8);
                        // count++;
                        writeToBlk(&count, res_addr, &res_blk, blk + j * 8);
                        printf("(C=%d, D=%d) \n", C, read4bytes(blk + j * 8 + 4));
                    }
                    else
                        break;
                }
                if (j==7)   // 整个块均为满足要求的元组，则获取下一块
                {
                    freeBlockInBuffer(blk, &buf);
                    printf("读入数据块%d\n", read4bytes(blk + j * 8));
                    blk = readBlockFromDisk(read4bytes(blk + j * 8), &buf);
                }
            }
            // 写回最后一个结果块
            // if (count !=0)
            // {
            //     next = res_addr + count / NUM_PER_BLK;
            //     if (count % 7 == 0)
            //         next--;
            //     writeBlockToDisk(res_blk, next, &buf);
            //     printf("注：结果写入磁盘：%d\n\n", next);
            // }
            writeLastBlk(count, res_addr, res_blk,7);
            freeBlockInBuffer(blk, &buf);
            break;
        }
    }
    printf("\n满足选择条件的元组一共有%d个\n\n", count);
    printf("IO读写一共%d次\n\n",buf.numIO);
    // printf1(250, 2);
    freeBuffer(&buf);
    return 0;
}

// 创建索引文件, 返回创建的索引文件个数
int createIndexBlk(int addr, int blk_num, int index_addr_begin)
{
    /* Initialize the buffer */
    if (!initBuffer(520, 64, &buf))
    {
        perror("Buffer Initialization Failed!\n");
        return -1;
    }
    int idx,i;
    unsigned char *index_blk,*blk;
    index_blk = getNewBlockInBuffer(&buf);
    int count = 1;
    for (i = 0; i < blk_num; i++)
    {
        idx = i % 7;
        // 索引块写满
        if (i!=0 && idx==0)
        {
            // 保存下一块号
            write4bytes(index_blk + 7 * 8, index_addr_begin + i / 7);
            // 写回索引块
            writeBlockToDisk(index_blk, index_addr_begin + i / 7 - 1, &buf);
            // 创建新的索引块
            index_blk = getNewBlockInBuffer(&buf);
            count++;
        }
        blk = readBlockFromDisk(addr, &buf);

        // 保存索引字段和块指针
        write4bytes(index_blk + idx * 8, read4bytes(blk));
        write4bytes(index_blk + idx * 8 + 4, addr);
        addr = read4bytes(blk + 7 * 8);
        // 释放当前数据块
        freeBlockInBuffer(blk, &buf);
    }
    // 写回最后一个索引块
    if (i !=0)
    {
        int next = index_addr_begin + i / 7;
        if (i % 7 == 0)
            next--;
        writeBlockToDisk(index_blk, next, &buf);
        // printf("注：结果写入磁盘：%d\n\n", next);
    }
    return count;
}
