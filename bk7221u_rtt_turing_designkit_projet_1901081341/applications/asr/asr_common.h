#ifndef _COMMON_H_
#define _COMMON_H_

#ifdef __cplusplus
extern "C" {
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define init_st(s)			memset(&s,0,sizeof(s))
#define init_pst(s)			memset(s,0,sizeof(*s))
#define mw_st_alloc(t, s)	(t*)sdram_malloc(sizeof(t)*s)
#define mw_ost_alloc(t)		(t*)sdram_malloc(sizeof(t))
#define mw_free_pstr(s)		do{if(s){sdram_free(s);s=NULL;}}while(0)
#define mw_free(s)			if(s)sdram_free(s)
#define mw_check_free(s)	if(s)sdram_free(s)
#define mw_strcmp(s1,s2)	strcmp(s1,s2)
#define mw_strdup(s)		(s ? strdup(s) : NULL)


#ifdef __cplusplus
}
#endif



#endif
