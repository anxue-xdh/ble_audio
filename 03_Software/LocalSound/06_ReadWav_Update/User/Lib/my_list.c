/***文件名：list.c ***/
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "my_list.h"

static struct node *make_node(void *data);
static struct node *find_min_node(yList *list, int (*compare)(const void *, const void *));
static void delete_node(yList *list, struct node *node);
static void insert_node(yList *list, struct node *node); // 尾插法

static struct node *make_node(void *data) // 把用户传递过来的数据打包为一个链表节点
{
	struct node *n;

	n = malloc(sizeof(struct node));
	assert(n != NULL);

	n->next = NULL;
	n->data = data;

	return n;
}

/**
 * @brief 在链表中查找最小元素对应的节点
 *
 * 在给定的链表中，通过比较函数 `compare` 查找最小元素对应的节点。
 *
 * @param list 链表指针
 * @param compare 比较函数，用于比较两个元素的大小
 *
 * @return 返回链表中最小元素对应的节点指针，若链表为空则返回 nullptr
 */
static struct node *find_min_node(yList *list, int (*compare)(const void *, const void *))
{
	struct node *min, *n;

	n = list->head;
	min = list->head;

	while (n)
	{
		if (compare(min->data, n->data) > 0)
		{
			min = n;
		}
		n = n->next;
	}

	return min;
}

/**
 * @brief 从链表中删除指定节点
 *
 * 从给定的链表中删除与给定节点指针相同的节点。
 *
 * @param list 链表
 * @param node 要删除的节点
 */
static void delete_node(yList *list, struct node *node)
{
	struct node *n;

	n = list->head;

	if (n == node)
	{
		list->head = n->next;
		return;
	}

	while (n->next)
	{
		if (n->next == node)
		{
			if (node == list->tail)
			{
				list->tail = n;
			}
			n->next = n->next->next;
			return;
		}
		n = n->next;
	}
}

static void insert_node(yList *list, struct node *node)
{
	if (list->head == NULL)
	{
		list->head = node;
		list->tail = node;
	}
	else
	{
		list->tail->next = node;
		list->tail = node;
	}
}

bool is_empty(yList *list)
{
	return (list->head == NULL);
}

long list_get_lenth(yList *list)
{
	return (list->len);
}

void list_init(yList *list)
{
	list->head = NULL;
	list->tail = NULL;
	list->len = 0;
}

/**
 * @brief 销毁链表
 *
 * 销毁链表，并可选地调用用户提供的函数来释放链表节点中的数据。
 * 如果销毁数据前，可执行一些清理工作，则可以传递一个自定义的函数指针。
 * destroy可以用来free掉Node节点中的data数据。
 *
 * @param list 待销毁的链表指针
 * @param destroy 用户自定义的数据处理函数指针，为NULL时不执行
 */
void list_destroy(yList *list, void (*destroy)(void *))
{
	list->len = 0;
	struct node *n, *t;
	n = list->head;

	while (n)
	{
		t = n->next; // t只起一个记录n->next的功能，否则后面把n free掉之后，就找不到n->next了。
		if (destroy)
		{					  // 传递用户自定义的数据处理函数，为0时不执行
			destroy(n->data); // 使用用户提供的destroy函数来释放用户传递过来的数据。
		}
		free(n);
		n = t; // 把n free掉之后，再把t给n，相当于把n->next给n,如此循环遍历链表，挨个删除，
	}
}

void list_insert_at_head(yList *list, void *data) // 头插法
{
	struct node *n;
	n = make_node(data);

	if (list->head == NULL)
	{ // 如果是空链表
		list->head = n;
		list->tail = n;
	}
	else
	{ // 如果不是非空链表
		n->next = list->head;
		list->head = n;
	}
	list->len++;
}

void list_insert_at_index(yList *list, void *data, long index) // 定插法
{
	long i = 1; // 从1开始算
	struct node *p, *n;

	p = list->head;

	while (p && i < index)
	{
		p = p->next;
		i++;
	}

	if (p)
	{ // 如果链表遍历完了，计数i还没到index，说明第index个节点不存在。
		n = make_node(data);
		n->next = p->next;
		p->next = n;
		list->len++;
	}
}

void list_insert_at_tail(yList *list, void *data) // 尾插法
{
	struct node *n;
	n = make_node(data);

	if (is_empty(list))
	{ // 如果是空链表
		list->head = n;
		list->tail = n;
	}
	else
	{ // 如果不是非空链表
		list->tail->next = n;
		list->tail = n;
	}
	list->len++;
}

/**
 * @brief 从链表中删除具有指定键值的节点
 *
 * 根据给定的比较函数和键值，从链表中删除第一个匹配的节点，并返回该节点的数据。
 * 如果未找到匹配的节点，则返回NULL。
 *
 * @param list 链表指针
 * @param key 需要删除的节点的键值
 * @param compare 比较函数，用于比较两个键值是否相等
 * @return 被删除节点的数据指针，如果未找到匹配的节点则返回NULL
 */
void *list_delete(yList *list, void *key, int (*compare)(const void *, const void *)) // 以key为删除关键词，compare为节点数据比较机制，由用户自己编写
{
	void *data;
	struct node *n, *t;
	n = list->head;

	if (!compare(n->data, key))
	{ // 如果要删除的节点为首节点
		printf("list_delete\n");
		t = n;
		data = n->data;
		list->head = n->next;
		free(t);
		list->len--;
		return data;
	}

	while (n->next != NULL)
	{ // 遍历查找符合条件的节点，删除之
		if (compare(n->next->data, key) == 0)
		{ // 只删除第一个符合条件的节点。
			t = n->next;
			if (n->next == list->tail)
			{
				list->tail = n;
			}
			n->next = n->next->next;
			data = t->data;
			free(t);
			list->len--;
			return data; // 把删除的数据返回给用户，供用户后续的处理使用。
		}
		n = n->next;
	}
	return NULL; // 没找到匹配的节点，返回NULL
}

/**
 * @brief 在链表中搜索指定元素
 *
 * 在给定的链表中搜索与给定键相等的元素。如果找到，返回该元素；否则返回NULL。
 *
 * @param list 链表头指针
 * @param key 要搜索的键
 * @param compare 比较函数，用于比较链表中的元素和键是否相等
 *
 * @return 如果找到匹配的元素，则返回指向该元素的指针；否则返回NULL
 */
void *list_search(yList *list, void *data, int (*compare)(const void *, const void *))
{
	struct node *n;
	n = list->head;

	while (n)
	{
		if (!compare(n->data, data))
		{ // 找到了，返回找到的数据
			return n->data;
		}
		n = n->next;
	}

	return NULL; // 找不到，返回NULL
}

/**
 * @brief 从链表中获取指定索引位置的元素
 *
 * 从给定的链表中获取指定索引位置的元素，并返回该元素的值。
 *
 * @param list 链表对象指针
 * @param idx 要获取的元素索引，从1开始计数
 *
 * @return 如果索引有效，返回指定索引位置的元素值；否则返回NULL
 */
void *list_get_element(yList *list, int idx)
{
	int i = 1;
	struct node *n;
	n = list->head;
	if (idx > list->len)
		return NULL;

	while (n && i < idx)
	{
		i++;
		n = n->next;
	}

	if (n)
	{
		return n->data;
	}

	return NULL;
}

/**
 * @brief 对链表进行排序
 *
 * 使用给定的比较函数对链表中的元素进行排序。
 *
 * @param list 需要排序的链表
 * @param compare 用于比较链表元素大小的函数指针
 */
void list_sort(yList *list,
			   int (*compare)(const void *, const void *))
{
	yList tmp;
	struct node *n;

	list_init(&tmp);

	while (!is_empty(list))
	{
		n = find_min_node(list, compare);
		delete_node(list, n);
		n->next = NULL;
		insert_node(&tmp, n);
	}
	list->head = tmp.head;
	list->tail = tmp.tail;
}

/**
 * @brief 遍历链表并处理每个节点
 *
 * 遍历传入的链表，并对链表中的每个节点调用传入的回调函数进行处理。
 *
 * @param list 指向链表的指针
 * @param handle 指向回调函数的指针，该回调函数接受一个void*类型的参数
 */
void list_traverse(yList *list, void (*handle)(void *))
{
	struct node *p;
	p = list->head;

	while (p)
	{
		handle(p->data);
		p = p->next;
	}
}

/**
 * @brief 反转链表
 *
 * 该函数将传入的链表进行反转，使得链表的头尾节点互换。
 *
 * @param list 需要反转的链表
 */
void list_reverse(yList *list)
{
	struct node *pre = NULL, *next, *p = list->head;

	list->tail = list->head; // tail指向head；
	while (p)
	{
		next = p->next;
		if (!next)
		{ // 当p->next为最后一个节点时，让head指向p->next
			list->head = p;
		}
		// 记录当前节点为pre，作为下一个节点的next.第一个节点为NULL，初始化时已定义。
		p->next = pre;
		pre = p;
		p = next;
	}
}
