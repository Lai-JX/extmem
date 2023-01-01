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
        addr = read4bytes(blk + 7 * 8);
        freeBlockInBuffer(blk, &buf);
    }
    return addr;
}

int Partition1(unsigned char* A,int low,int high)
{
    int pivotpos1 = read4bytes(A + low);
    int pivotpos2 = read4bytes(A + low + 4);
    // printf("pivotpos:%d\n", pivotpos);
    while(low<high)
    {
        // printf("low:%d\n", low);
        while(low<high && read4bytes(A+high)>=pivotpos1)high -= 8;
        write8bytes(A+low, A+high);
        while(low<high && read4bytes(A+low)<=pivotpos1)low += 8;
        write8bytes(A+high, A+low);
    }
    write4bytes(A+low, pivotpos1);
    write4bytes(A + low + 4, pivotpos2);
    return low;
}
void QuickSort(unsigned char *A,int low,int high)
{
    if(low<high)
    {
        int pivotpos = Partition1(A,low,high);
        // printf("index:%d", pivotpos);
        QuickSort(A,low,pivotpos-8);
        QuickSort(A,pivotpos+8,high);
    }
}