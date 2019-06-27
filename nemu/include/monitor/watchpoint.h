#ifndef __WATCHPOINT_H__
#define __WATCHPOINT_H__

#include "common.h"

typedef struct watchpoint {
	int NO;
	unsigned int content;
	unsigned int point;
	char expr[32];
	struct watchpoint *next;
	
	/* TODO: Add more members if necessary */


} WP;

void free_wp(WP *wp);

WP* new_wp();

bool check_wp_pool();
void print_wp();
void delete_wp(char *args);
#endif
