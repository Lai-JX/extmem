
#include "def.h"

int test()
{
    Buffer buf; /* A buffer */
    unsigned char *blk; /* A pointer to a block */
    int i = 0;

    /* Initialize the buffer */
    if (!initBuffer(520, 64, &buf))
    {
        perror("Buffer Initialization Failed!\n");
        return -1;
    }

    /* Get a new block in the buffer */
    blk = getNewBlockInBuffer(&buf);

    /* Fill data into the block */
    for (i = 0; i < 8; i++)
        *(blk + i) = 'a' + i;

    /* Write the block to the hard disk */
    if (writeBlockToDisk(blk, 8888, &buf) != 0)
    {
        perror("Writing Block Failed!\n");
        return -1;
    }

    /* Read the block from the hard disk */
    if ((blk = readBlockFromDisk(1, &buf)) == NULL)
    {
        perror("Reading Block Failed!\n");
        return -1;
    }

    /* Process the data in the block */
    int X = -1;
    int Y = -1;
    int addr = -1;

    char str[5];
    printf("block 1:\n");
    // 遍历7个元组
    for (i = 0; i < 7; i++) //一个blk存7个元组加一个地址
    {
        // 读取前4个字节
        for (int k = 0; k < 4; k++)
        {
            str[k] = *(blk + i*8 + k);
        }
        X = atoi(str);
        // 读取后4个字节
        for (int k = 0; k < 4; k++)
        {
            str[k] = *(blk + i*8 + 4 + k);
        }
        Y = atoi(str);
        printf("(%d, %d) ", X, Y);
    }
    // 读取下一块号
    for (int k = 0; k < 4; k++)
    {
        str[k] = *(blk + i*8 + k);
    }
    addr = atoi(str);
    printf("\nnext address = %d \n", addr);


    printf("\n");
    printf("IO's is %d\n", buf.numIO); /* Check the number of IO's */


    return 0;
}

