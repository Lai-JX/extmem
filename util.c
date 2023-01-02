# include "def.h"
// 从base处读取4个字节
int read4bytes(unsigned char * base)
{

    char str[5];        // 用于保存4字节
    for (int k = 0; k < 4; k++)
        str[k] = *(base + k);
    return atoi(str);
}

// 将x写入base处
int write4bytes(unsigned char * base, int x)
{

    unsigned char str_X[5];
    sprintf(str_X, "%d", x);
    for (int i = 0; i < 4; i++)
        *(base + i) = str_X[i];
    return 0;
}



// /* 将(X,Y)写入blk中第off条元组(off下标从0开始) */
// void write8bytes(unsigned char*base, int X, int Y)
// {

//     unsigned char str_X[4];
//     unsigned char str_Y[4];
//     // unsigned char *base = blk + 8 * off;

//     sprintf(str_X, "%d", X);
//     for (int i = 0; i < 4; i++)
//         *(base + i) = str_X[i];

    
//     sprintf(str_Y, "%d", Y);
//     for (int i = 0; i < 4; i++)
//         *(base + 4 + i) = str_Y[i];
    
// }
void write8bytes(unsigned char*base, unsigned char * src)
{

    for (int i = 0; i < 8; i++)
        base[i] = src[i];
}

// 寻找addr后面n块的块号
int findAddr(int addr, int n)
{
    if (n ==0)
        return addr;
    // printf("n:%d\n", n);
    unsigned char* blk;
    for (int i = 0; i < n; i++)
    {   // 读取下一块号
        blk = readBlockFromDisk(addr, &buf);
        addr = read4bytes(blk + NUM_PER_BLK * 8);
        freeBlockInBuffer(blk, &buf);
    }
    return addr;
}

// int Partition1(unsigned char* A,int low,int high)
// {
//     int pivotpos1 = read4bytes(A + low);
//     int pivotpos2 = read4bytes(A + low + 4);
//     // printf("pivotpos:%d\n", pivotpos);
//     while(low<high)
//     {
//         // printf("low:%d\n", low);
//         while(low<high && read4bytes(A+high)>=pivotpos1)high -= 8;
//         write8bytes(A+low, A+high);
//         while(low<high && read4bytes(A+low)<=pivotpos1)low += 8;
//         write8bytes(A+high, A+low);
//     }
//     write4bytes(A+low, pivotpos1);
//     write4bytes(A + low + 4, pivotpos2);
//     return low;
// }
// void QuickSort(unsigned char *A,int low,int high)
// {
//     if(low<high)
//     {
//         int pivotpos = Partition1(A,low,high);
//         // printf("index:%d", pivotpos);
//         QuickSort(A,low,pivotpos-8);
//         QuickSort(A,pivotpos+8,high);
//     }
// }
void Swap(unsigned char *a,unsigned char *b)
{
    unsigned char temp[8];
    write8bytes(temp, b);
    write8bytes(b, a);
    write8bytes(a, temp);
    return;
}
void BubbleSort(unsigned char *A,int n) // n=504
{
    int flag=1,i,j,idx;
    for (i = 0; i < NUM_PER_BLK * 8 - 1; i++) // 56个元组，需要循环55次
    {
        flag = 1;
        for (j = n; j > i*8; j -= 8)
        {
            if ((j-1)%65 == 0)  // 边界情况
                idx = j - 8 - 9;
            else
                idx = j - 8;


            if(read4bytes(&A[idx]) > read4bytes(&A[j]))
            {
                Swap(&A[idx],&A[j]);
                flag=0;
            }
            if ((j-1)%65 == 0)  // 边界情况
                j -= 9;
        }
            
        if(flag)
            return;
    }
}
// 输出块号为addr后的n个块（方便debug）
void printf1(int addr, int n)
{
    unsigned char *blk;
    for (int i = 0; i < n; i++)
    {
        printf("block %d\n", findAddr(addr, i));
        blk = readBlockFromDisk(findAddr(addr,i), &buf);
        for (int k = 0; k < 64; k+=8)
        {
            printf("(%d,%d)\n",read4bytes(blk+k),read4bytes(blk+4+k));
        }

        freeBlockInBuffer(blk, &buf);
    }
    
}
// 写入内存，满时保存旧块创建新快
void writeToBlk(int *res_amount, int res_addr, unsigned char**res_blk, unsigned char* src)
{
    int res_idx = *res_amount % NUM_PER_BLK;               // 每个块存7条记录
    // 内存块写满
    if (*res_amount !=0 && res_idx == 0 )
    {
        // 下一块号
        int next = res_addr + *res_amount / NUM_PER_BLK;
        // 保存结果块的下一块号(便于链接)
        write4bytes((*res_blk) + 8 * NUM_PER_BLK, next);
        // 写回磁盘
        writeBlockToDisk(*res_blk, next-1, &buf);
        printf("注：结果写入磁盘：%d\n", next - 1);
        // 申请缓冲区
        *res_blk = getNewBlockInBuffer(&buf);
    }
    // printf("(C=%d, D=%d) \n", C_val, D_val);
    write8bytes(*res_blk+res_idx*8, src);
    (*res_amount)++;
}
// 指针移动
int shiftPointer(int* set_ptr, int* visited_blk, int blk_num, unsigned char** blk)
{
    // 指针移动
    if (*set_ptr < 48)
        *set_ptr += 8;
    else
    {
        if (*visited_blk < blk_num-1)           // 读取下一块
        {
            int next = read4bytes((*blk) + NUM_PER_BLK * 8);

            freeBlockInBuffer(*blk, &buf);
            *blk = readBlockFromDisk(next, &buf);
            (*visited_blk)++;
            *set_ptr = 0;
        }
        else
        {
            return 0;
        }       
    }
    return 1;
}
// 将最后一块写回磁盘
void writeLastBlk(int res_amount, int res_addr, unsigned char * res_blk, int round)
{
    if (res_amount !=0)
    {
        int next = res_addr + res_amount / round;
        if (res_amount % round == 0)
            next--;
        writeBlockToDisk(res_blk, next, &buf);
        printf("注：结果写入磁盘：%d\n", next);
    }
}
// 指针回退
void traceBack(int* visited_blk, unsigned char** blk, int* set_ptr, int save_idx, int save_addr)
{
    if (*visited_blk != save_addr)  // 若读取到别的块
    {
        freeBlockInBuffer(*blk, &buf);
        // printf("findaddr:%d\n", findAddr(R_BLK_BEGIN, save_addr));
        *blk = readBlockFromDisk(findAddr(R_SORT_BLK_BEGIN,save_addr), &buf);
        *visited_blk = save_addr;
    }
    *set_ptr = save_idx;
}