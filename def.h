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

/******************************/
/*工具函数*/
/******************************/
int read4bytes(unsigned char *);
int write4bytes(unsigned char *, int);
void write8bytes(unsigned char *, unsigned char *);
void QuickSort(unsigned char *, int, int);
int findAddr(int, int);

#endif // DEF_H_INCLUDED
