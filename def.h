#ifndef DEF_H_INCLUDED
#define DEF_H_INCLUDED

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
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
/* 函数声明 */
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
void BubbleSort(unsigned char *, int);
int findAddr(int, int);
void printf1(int, int);

#endif // DEF_H_INCLUDED
