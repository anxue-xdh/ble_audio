/***
 * @Author: YourName
 * @Date: 2025-02-12 16:00:36
 * @LastEditTime: 2025-02-13 14:07:53
 * @LastEditors: YourName
 * @Description:
 * @FilePath: \MDK-ARMd:\Work_YJH\Projection\04_MyPrj\02_BleAudio\03_Software\LocalSound\06_ReadWav_Update\User\Lib\my_list.h
 * @版权声明
 */
/***文件名：list.h ***/
#ifndef _MYLIST_H_
#define _MYLIST_H_

// #ifdef __cplusplus
// extern "C"
// {
// #endif

#include <stdbool.h>

typedef struct node
{ // 节点结构
	void *data;
	struct node *next;
} yNode;

typedef struct
{ // 链表结构
	struct node *head;
	struct node *tail;
	long len;
} yList;

#define list_insert(list, data) list_insert_at_tail(list, data)

extern void list_init(yList *list);
extern bool is_empty(yList *list);
// extern void list_insert(yList *list, void *data);					 // 默认采用尾插法
extern void list_insert_at_head(yList *list, void *data);			 // 头插法
extern void list_insert_at_tail(yList *list, void *data);			 // 尾插法
extern void list_insert_at_index(yList *list, void *data, long idx); // 定插法

extern void *list_delete(yList *list, void *key, int (*compare)(const void *, const void *));
extern void *list_search(yList *list, void *data, int (*compare)(const void *, const void *));
extern void list_sort(yList *list, int (*compare)(const void *, const void *));
extern void list_traverse(yList *list, void (*handle)(void *, int));
extern void list_reverse(yList *list);
extern long list_get_lenth(yList *list);
extern void *list_get_element(yList *list, int idx);
extern void list_destroy(yList *list, void (*destroy)(void *));

#endif

// #ifdef __cplusplus
// }
// #endif
