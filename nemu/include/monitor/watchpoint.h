#ifndef __WATCHPOINT_H__
#define __WATCHPOINT_H__

#include "common.h"

typedef struct watchpoint {
	int NO;
	unsigned int content;
	unsigned int point;
	struct watchpoint *next;
	
	/* TODO: Add more members if necessary */


} WP;

void free_wp(WP *wp);

WP* new_wp();

bool check_wp_pool();

void delete_wp(unsigned addr);
#endif
