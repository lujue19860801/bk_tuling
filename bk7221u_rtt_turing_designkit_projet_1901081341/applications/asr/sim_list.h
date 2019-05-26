#ifndef _MW_SIM_LIST_H_
#define _MW_SIM_LIST_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdarg.h>
#include <pthread.h>
#include "linux_list.h"


typedef void (*sim_list_free_func)(void *data);

//节点
typedef struct sim_list_node
{
	struct list_head hdr;

	char *data;
	int dlen;
}sim_list_node;

//
typedef struct
{
	int num;
	sim_list_node *pcur;	//当前的节点，主要用于遍历，注意刚开始为NULL

	struct list_head lst;

	sim_list_free_func free_func;
	pthread_mutex_t mutex;
}sim_list_s;

//初始化
int sim_list_init(sim_list_s *li, void (*free_func) (void *));

//完全释放，不能再操作
void sim_list_free(sim_list_s *li);

//只清空节点，可以继续操作
void sim_list_clear(sim_list_s *li);

//添加节点
int sim_list_add_node(sim_list_s *p, void *data, int len);

int sim_list_add_node_cpy(sim_list_s *p, char *data, int len);

//取出数据
void* sim_list_pop_data(sim_list_s *li);
//取出node，用完需要释放node，node->data则根据添加节点的操作处理
void * sim_list_pop_node(sim_list_s *li);

sim_list_node * sim_list_peek_node(sim_list_s *li);

sim_list_node * sim_list_peek_node_index(sim_list_s *li,int index);


//从链表移除，函数不做释放处理，需要外部释放
int sim_list_remove_node(sim_list_s *p, sim_list_node *node);


int sim_list_remove_free_node(sim_list_s *li, sim_list_node *node);

//psrc的节点转给pdst
void sim_list_trans(sim_list_s *psrc, sim_list_s *pdst);

//循环获取下一个节点的指针
sim_list_node *sim_list_next_node(sim_list_s *psal, int *is_loop);

/*
 *遍历方式1
 *调用顺序 :
 *	sim_list_loop_init
 *	sim_list_loop_get_data * n 循环获取节点，返回NULL说明已获取所有节点
 *	sim_list_loop_deinit
 *
 *sim_list_loop_init 跟 sim_list_loop_deinit分别使用了加锁/解锁, 必须调用
 */
void sim_list_loop_init(sim_list_s *li);
void sim_list_loop_deinit(sim_list_s *li);
void *sim_list_loop_get_data(sim_list_s *li);


/*
 *遍历方式2
 *回调方式遍历链表，每个节点调用回调函数，如果回调函数返回非0，终止遍历
 *注意在回调中不要调用sim_list的操作函数
 */
enum
{
	SL_TRAVE_CONTINUE = 0,	//继续遍历
	SL_TRAVE_BREAK//终止遍历
};
void sim_list_traverse(sim_list_s *li, int (*on_node_func) (void *user_data, void *pdata), void *user_data);
void sim_list_traverse_nolck(sim_list_s *li, int(*on_node_func) (void *user_data, void *pdata), void *user_data);

enum
{
	SL_TRAVE_CONTINUE_TRV = 0,	//继续遍历
	SL_TRAVE_BREAK_TRV,		//终止遍历
	SL_TRAVE_REMOVE_RETURN,	//删除当前节点,并终止遍历
	SL_TRAVE_REMOVE_CONTINUE//删除当前节点,继续遍历
};
//遍历链表，调用回调函数cmp_func，根据cmp_func返回值进行移除节点(由于要返回用户数据,移除只处理SL_TRAVE_REMOVE_RETURN)，并返回用户数据
void* sim_list_cmp_remove(sim_list_s *li, int (*cmp_func) (void *user_data, void *pdata), void *user_data);
void* sim_list_cmp_remove_nolck(sim_list_s *li, int(*cmp_func) (void *user_data, void *pdata), void *user_data);

void* sim_list_cmp_peek_data(sim_list_s *li,int (*cmp_func) (void *user_data, void *pdata), void *user_data);
////遍历链表，调用回调函数cmp_func，根据cmp_func返回值进行移除节点，并释放用户数据
void sim_list_cmp_remove_free(sim_list_s *li, int(*cmp_func) (void *user_data, void *pdata), void *user_data);


static int sim_list_empty(sim_list_s *p)
{
	struct list_head *head = &p->lst;
	return head->next == head;
}

static int sim_list_count(sim_list_s *p)
{
	int cnt = 0;
	struct list_head *head = &p->lst;
	struct list_head *pos = head->next;
	while(pos != head){
		cnt+=1;
		pos = pos->next;
	}
	return cnt;
}


#ifdef __cplusplus
}
#endif

#endif
