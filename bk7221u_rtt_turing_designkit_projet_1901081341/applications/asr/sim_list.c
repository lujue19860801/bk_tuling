
#define _GNU_SOURCE 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdarg.h>

//#define TEST_SIM_LIST

#include "sim_list.h"
#include "asr_common.h"
//==================================================
#define SHOW_ELEM(s)		if(s) printf(" %s: %s\n", #s, s)
#define DEBUG_PRINT(...)	//fprintf(stderr, __VA_ARGS__)

#define SIM_LIST_LOCK(L)		pthread_mutex_lock(&(L->mutex))
#define SIM_LIST_UNLOCK(L)		pthread_mutex_unlock(&(L->mutex))
//==================================================

static void free_node_data(void *pv)
{
	if(pv)
		free(pv);
}

static inline void sim_init(sim_list_s *li)
{
	li->num = 0;
	li->pcur = NULL;
	INIT_LIST_HEAD(&li->lst);
}

int sim_list_init(sim_list_s *li, void (*free_func) (void *))
{
	li->num = 0;
	li->pcur = NULL;
	INIT_LIST_HEAD(&li->lst);
	pthread_mutex_init(&li->mutex, NULL);

	if (free_func)
	{
		li->free_func = free_func;
	}
	else
	{
		li->free_func = free_node_data;
	}

	return 0;
}

void sim_list_free(sim_list_s *li)
{
	struct list_head *lst = &li->lst;
	sim_list_node *pnode;
	while(!list_empty(lst))
	{
		pnode = get_first_item(lst, sim_list_node, hdr);
		list_del((struct list_head *)pnode);
		li->free_func(pnode->data);
		free(pnode);
	}

	pthread_mutex_destroy(&li->mutex);
}

void sim_list_special_free (sim_list_s * li, void (*free_func) (void *))
{
	struct list_head *lst = &li->lst;
	sim_list_node *pnode;
	while(!list_empty(lst))
	{
		pnode = get_first_item(lst, sim_list_node, hdr);
		list_del((struct list_head *)pnode);
		if(free_func)
		{
			free_func(pnode->data);
		}
		free(pnode);
	}

	pthread_mutex_destroy(&li->mutex);
}

void sim_list_clear(sim_list_s *li)
{
	struct list_head *lst = &li->lst;
	sim_list_node *pnode;

	SIM_LIST_LOCK(li);
	//free
	while(!list_empty(lst))
	{
		pnode = get_first_item(lst, sim_list_node, hdr);
		list_del((struct list_head *)pnode);
		li->free_func(pnode->data);
		free(pnode);
	}

	//re-init hdr
	li->num = 0;
	li->pcur = NULL;
	INIT_LIST_HEAD(&li->lst);

	SIM_LIST_UNLOCK(li);
}

void * sim_list_pop_data(sim_list_s *li)
{
	struct list_head *lst = &li->lst;
	if (!list_empty(lst))
	{
		sim_list_node *pnode;
		char *pdata;

		SIM_LIST_LOCK(li);

		pnode = get_first_item(lst, sim_list_node, hdr);
		pdata = pnode->data;
		list_del((struct list_head *)pnode);

		li->num--;
		SIM_LIST_UNLOCK(li);

		free(pnode);

		return pdata;
	}

	return NULL;
}

void * sim_list_pop_node(sim_list_s *li)
{
	struct list_head *lst = &li->lst;
	if (!list_empty(lst))
	{
		sim_list_node *pnode;

		SIM_LIST_LOCK(li);

		pnode = get_first_item(lst, sim_list_node, hdr);
		list_del((struct list_head *)pnode);

		li->num--;
		SIM_LIST_UNLOCK(li);

		return pnode;
	}

	return NULL;
}

sim_list_node * sim_list_peek_node(sim_list_s *li)
{
	struct list_head *lst = &li->lst;
	if (!list_empty(lst))
	{
		sim_list_node *pnode;

		SIM_LIST_LOCK(li);
		pnode = get_first_item(lst, sim_list_node, hdr);
		SIM_LIST_UNLOCK(li);

		return pnode;
	}

	return NULL;
}

sim_list_node * sim_list_peek_node_index(sim_list_s *li,int index)
{
	struct list_head *lst = &li->lst;
	struct list_head *tmp;
	SIM_LIST_LOCK(li);
	
	list_for_each(tmp, lst)
	{
		if(index-->0)
		{
			continue;
		}else{
			break;
		}
	}
	//
	SIM_LIST_UNLOCK(li);
	return (sim_list_node*)tmp;
}


/*! 
 *data 即数据由函数分配，外部pop使用以后需要释放pop的数据
 * 注意：
 *  此函数适用于临时变量入队列的情况,
 *  如果传入的data是动态分配的，可以用另外一个不拷贝的函数
 */
int sim_list_add_node_cpy(sim_list_s *li, char *data, int len)
{
	sim_list_node *pnew = mw_st_alloc(sim_list_node, 1);
	if (pnew)
	{
		pnew->data = (char*)malloc(len);
		if (pnew->data)
		{
			memcpy(pnew->data, data, len);
			pnew->dlen = len;

			SIM_LIST_LOCK(li);
			list_add_tail((struct list_head *)pnew, &li->lst);
			li->num ++;
			SIM_LIST_UNLOCK(li);

			return 0;
		}

		free(pnew);
	}

	return ENOMEM;
}

//data 即数据由外部分配\释放，外部自己管理
int sim_list_add_node(sim_list_s *li, void *data, int len)
{
	sim_list_node *pnew = mw_st_alloc(sim_list_node, 1);
	if (pnew)
	{
		pnew->data = data;
		pnew->dlen = len;

		SIM_LIST_LOCK(li);
		list_add_tail((struct list_head *)pnew, &li->lst);
		li->num ++;
		SIM_LIST_UNLOCK(li);

		return 0;
	}

	return ENOMEM;
}


//psrc的节点转移给pdst
static inline void list_trans_nodes(struct list_head *psrc, struct list_head *pdst) 
{
	if (!list_empty(psrc))
	{
		if (list_empty(pdst))
		{
			pdst->next = psrc->next;
			pdst->prev = psrc->prev;

			psrc->next->prev = pdst;
			psrc->prev->next = pdst;
		}
		else
		{
			pdst->prev->next = psrc->next;
			psrc->next->prev = pdst->prev;

			pdst->prev = psrc->prev;
			psrc->prev->next = pdst;
		}
	}
}

//psrc的节点转给pdst
void sim_list_trans(sim_list_s *psrc, sim_list_s *pdst)
{
	pdst->num = psrc->num;
	pdst->pcur = psrc->pcur;

	list_trans_nodes(&psrc->lst, &pdst->lst);

	sim_init(psrc);
}

sim_list_node *sim_list_next_node(sim_list_s *psal, int *is_loop)
{
	//if it's empty
	if (list_empty(&psal->lst))
	{
		return NULL;
	}

	if (is_loop)
	{
		*is_loop = 0;
	}

	if (psal->pcur == NULL)
	{
		psal->pcur = (sim_list_node *)psal->lst.next;
		//	return psal->pcur;
	}
	else if (psal->pcur->hdr.next == &psal->lst)
	{
		psal->pcur = (sim_list_node *)psal->lst.next;
		if (is_loop)
		{
			*is_loop = 1;
		}
	}
	else
	{
		psal->pcur = (sim_list_node *)psal->pcur->hdr.next;
	}

	return psal->pcur;
}

int sim_list_remove_node(sim_list_s *li, sim_list_node *node)
{
	SIM_LIST_LOCK(li);
	list_del((struct list_head *)node);
	li->num--;
	SIM_LIST_UNLOCK(li);
	return 0;
}

int sim_list_remove_free_node(sim_list_s *li, sim_list_node *node)
{
	sim_list_remove_node(li, node);

	li->free_func(node->data);
	free(node);
	return 0;
}

void sim_list_traverse(sim_list_s *li, int (*on_node_func) (void *user_data, void *pdata), void *user_data)
{
	struct list_head *lst = &li->lst;
	struct list_head *tmp;
	SIM_LIST_LOCK(li);
	
	list_for_each(tmp, lst)
	{
		if( SL_TRAVE_CONTINUE != on_node_func(user_data, ((sim_list_node*)tmp)->data) )
		{
			break;
		}
	}
	//
	SIM_LIST_UNLOCK(li);
}

void* sim_list_cmp_remove(sim_list_s *li, int (*cmp_func) (void *user_data, void *pdata), void *user_data)
{
	struct list_head *lst = &li->lst;
	struct list_head *cur, *next;
	int rv;
	void *pv_ret = NULL;
	SIM_LIST_LOCK(li);

	list_for_each_safe(cur, next, lst)
	{
		rv = cmp_func(user_data, ((sim_list_node*)cur)->data);
		if (0 != rv)
		{
			if (SL_TRAVE_REMOVE_RETURN == rv)
			{
				sim_list_node *pnode = (sim_list_node *)cur;
				pv_ret = pnode->data;
				list_del(cur);
				//	li->free_func(pnode->data);
				free(pnode);

				if (li->num > 0)
					li->num--;
			}

			break;
		}
	}
	//
	SIM_LIST_UNLOCK(li);

	return pv_ret;
}

void sim_list_cmp_remove_free(sim_list_s *li, int(*cmp_func) (void *user_data, void *pdata), void *user_data)
{
	struct list_head *lst = &li->lst;
	struct list_head *cur, *next;
	int rv;
	//void *pv_ret = NULL;
	SIM_LIST_LOCK(li);

	list_for_each_safe(cur, next, lst)
	{
		rv = cmp_func(user_data, ((sim_list_node*)cur)->data);
		if (0 != rv)
		{
			sim_list_node *pnode = (sim_list_node *)cur;
			
			if (SL_TRAVE_REMOVE_CONTINUE == rv)
			{
				//释放，继续遍历
				list_del(cur);
				li->free_func(pnode->data);
				free(pnode);

				if (li->num > 0)
					li->num--;

				continue;
			}

			if (SL_TRAVE_REMOVE_RETURN == rv)
			{
				//释放
				list_del(cur);
				li->free_func(pnode->data);
				free(pnode);

				if (li->num > 0)
					li->num--;
			}

			//终止遍历
			break;
		}
	}
	//
	SIM_LIST_UNLOCK(li);
}

void* sim_list_cmp_peek_data(sim_list_s *li,int (*cmp_func) (void *user_data, void *pdata), void *user_data)
{
	struct list_head *lst = &li->lst;
	struct list_head *cur, *next;
	int rv;
	void *pv_ret = NULL;
	SIM_LIST_LOCK(li);

	list_for_each_safe(cur, next, lst)
	{
		rv = cmp_func(user_data, ((sim_list_node*)cur)->data);
		if (0 != rv)
		{
			if (SL_TRAVE_REMOVE_RETURN == rv)
			{
				sim_list_node *pnode = (sim_list_node *)cur;
				pv_ret = pnode->data;
			}

			break;
		}
	}
	//
	SIM_LIST_UNLOCK(li);

	return pv_ret;
}

void sim_list_traverse_nolck(sim_list_s *li, int(*on_node_func) (void *user_data, void *pdata), void *user_data)
{
	struct list_head *lst = &li->lst;
	struct list_head *tmp;

	list_for_each(tmp, lst)
	{
		if (SL_TRAVE_CONTINUE != on_node_func(user_data, ((sim_list_node*)tmp)->data))
		{
			break;
		}
	}
}

void* sim_list_cmp_remove_nolck(sim_list_s *li, int(*cmp_func) (void *user_data, void *pdata), void *user_data)
{
	struct list_head *lst = &li->lst;
	struct list_head *cur, *next;
	int rv;
	void *pv_ret = NULL;

	list_for_each_safe(cur, next, lst)
	{
		rv = cmp_func(user_data, ((sim_list_node*)cur)->data);
		if (0 != rv)
		{
			if (SL_TRAVE_REMOVE_RETURN == rv)
			{
				sim_list_node *pnode = (sim_list_node *)cur;
				pv_ret = pnode->data;
				list_del(cur);
				//	li->free_func(pnode->data);
				free(pnode);

				if (li->num > 0)
					li->num--;
			}

			break;
		}
	}

	return pv_ret;
}

void sim_list_loop_init(sim_list_s *li)
{
	SIM_LIST_LOCK(li);
	li->pcur = NULL;
}
void sim_list_loop_deinit(sim_list_s *li)
{
	li->pcur = NULL;
	SIM_LIST_UNLOCK(li);
}
void *sim_list_loop_get_data(sim_list_s *li)
{
	//if it's empty
	if (list_empty(&li->lst))
	{
		return NULL;
	}


	if (li->pcur == NULL)
	{
		li->pcur = (sim_list_node *)li->lst.next;
	}
	else
	{
		if (li->pcur->hdr.next == &li->lst)
		{
//			printf("loop\n");
			return NULL;
		}
		
		li->pcur = (sim_list_node *)li->pcur->hdr.next;
	}
	

	return li->pcur->data;
}

//================== TEST ===================
#if defined(TEST_SIM_LIST) || defined(AVS_TEST_ALL)
typedef struct _sample_node
{
	int value;
	char *pstr;

	//
	void *pv;
}sample_node;

void free_sample_node(void *pv)
{
	sample_node *pn = (sample_node*)pv;
	if (pn->pstr)
	{
		free(pn->pstr);
	}

	free(pn);
}

sample_node *new_sample_node(int value, char *str, void *pv)
{
	sample_node *pn = (sample_node*)malloc(sizeof(sample_node));
	if (pn)
	{
		pn->value = value;
		pn->pstr = strdup(str);
		pn->pv = pv;
	}

	return pn;
}

static int cb_show_sample_node(void *user_data, void *pnode_data)
{
	sample_node *pn = (sample_node *)pnode_data;
	printf("node v=%d, str=%s\n", pn->value, pn->pstr);
	return 0;
}

static int cmp_remove_by_key(void *user_data, void *pdata)
{
	char * key = (char*)user_data;
	sample_node *node = (sample_node *)pdata;

	if (node)
	{
		if (!strcmp(key, node->pstr))
		{
			printf("cmp_remove_active_token Found\n");
			return SL_TRAVE_REMOVE_RETURN;
		}
		printf("cmp_remove_by_key un match %s,%s\n", key, node->pstr);
	}
	
	return  0;
}

void test_sim_list()
{
	int i;
	char buff[128];
	sim_list_s lst;
	void *usr_data = NULL;
	sample_node *pn;

	sim_list_init(&lst, free_sample_node);

	//add
	for (i = 0; i < 10; i++)
	{
		snprintf(buff, 32, "node_%02d", i);
		sample_node * node = new_sample_node(i, buff, usr_data);
		sim_list_add_node(&lst, node, sizeof(*node));
	}

	//遍历1
	sim_list_traverse(&lst, cb_show_sample_node, usr_data);

	//遍历2
	printf("traverse 2\n");
	printf("num node=%d\n", lst.num);
	sim_list_loop_init(&lst);
	while ( (pn = (sample_node *)sim_list_loop_get_data(&lst)) != NULL)
	{
		printf("node v=%d, str=%s\n", pn->value, pn->pstr);
	}

	sim_list_loop_deinit(&lst);

	//删除
	printf("try remove\n");
	sim_list_cmp_remove(&lst, cmp_remove_by_key, (void*)"node_05");

	printf("num node=%d\n", lst.num);
	sim_list_traverse(&lst, cb_show_sample_node, usr_data);

	//
	sim_list_free(&lst);
}
#endif
