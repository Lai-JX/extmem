#ifndef DEF_H_INCLUDED
#define DEF_H_INCLUDED

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include "extmem.h"

/******************************/
/* 宏定义 */
/******************************/
#define R_BLK_BEGIN 1
#define S_BLK_BEGIN 17
#define R_BLK_NUM 16
#define S_BLK_NUM 32
#define NUM_PER_BLK 7       // 一块有7个元组
#define R_SORT_BLK_BEGIN 100    // 排序后的R的初始块号
#define S_SORT_BLK_BEGIN 150    // 排序后的S的初始块号
#define BLKS_PER_SET 8          // 每个子集的块数

/******************************/
/* 全局变量 */
/******************************/
Buffer buf; /* A buffer */

/******************************/
/* 主要功能函数 */
/******************************/
// int lab5_all(void);
int test(void);
int linearSearch(void);
int tpmms(void);
int indexSearch(void);
int sortMergeJoin(void);
int sortMergeIntersection(void);
int sortMergeUnion(void);
int sortMergeDifference(void);

/******************************/
/*工具函数*/
/******************************/
int read4bytes(unsigned char *);
int write4bytes(unsigned char *, int);
void write8bytes(unsigned char *, unsigned char *);
void BubbleSort(unsigned char *, int);                          // 冒泡排序
int findAddr(int, int);                                         // 连接式查找块
void printf1(int, int);                                         // 输出块中内容（debug用）
void Swap(unsigned char *, unsigned char *);                    // 用于冒泡排序
void writeToBlk(int *, int, unsigned char **, unsigned char *); // 写入块，若块满，则保存旧块创建新快
int shiftPointer(int *, int *, int, unsigned char **);          // 指针移动
void writeLastBlk(int, int, unsigned char *, int);              // 将最后一块写回磁盘
void traceBack(int *, unsigned char **, int *, int, int);       // 指针回退

/**排序辅助函数**/
void tpmms_(int, int, int, int);                // 用于排序
int tpmms_step1(int, int);                     // 排序步骤1
void tpmms_step2(int, int, int, int);           // 排序步骤2

/**索引搜索辅助函数**/
int createIndexBlk(int, int, int);              // 创建索引文件


#endif // DEF_H_INCLUDED
