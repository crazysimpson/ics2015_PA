#include "monitor/watchpoint.h"
#include "monitor/expr.h"
#include "nemu.h"

#define NR_WP 32

static WP wp_pool[NR_WP];
static WP *head, *free_;
static bool expr_result;
void init_wp_pool() {
	int i;
	for(i = 0; i < NR_WP; i ++) {
		wp_pool[i].NO = i;
		wp_pool[i].next = &wp_pool[i + 1];
	}
	wp_pool[NR_WP - 1].next = NULL;

	head = NULL;
	free_ = wp_pool;
}

/* TODO: Implement the functionality of watchpoint */

WP* new_wp(){
	if((free_) != (wp_pool + NR_WP)){
			head = free_;
		
			free_+=1;
		return head;
	}else{
		Assert(0, "WP_POOL is not enough ");
	}
}

void free_wp(WP* wp){
	if(wp<wp_pool || wp>head){
		Assert(0,"the wp in note in WP_POOL");
	}
	while(head !=NULL && wp!=head){
		wp->NO = wp->next->NO;
		strcpy(wp->expr, wp->next->expr);
		wp->content = wp->next->content;
	}
	head-=1;
	if(head<wp_pool)
		head=NULL;
	free_ -=1;
}

bool check_wp_pool(){
	bool result = false;
	WP* point = wp_pool;
	while(head !=NULL &&  point <= head){
		if(point->content != expr(point->expr, &expr_result)){
			printf("the content of expr %s has changed, the origin vale is: %08x, the new value is: %08x\n",point->expr, point->content, expr(point->expr, &expr_result));
			point->content  = expr(point->expr, &expr_result);
			result = true;
		}
		point = point->next;		
	}
			
	return result;
}

void delete_wp(char *args){
	WP* point = wp_pool;
	while(head!=NULL && point<=head){
		if(strcmp(args, point->expr)==0){
			free_wp(point);
			continue;
		}
		else{
			point = point->next;
		}
	}
	return;
}

void print_wp(){
	WP *tmp = wp_pool;
	while(tmp<=head){
		printf("watchpoint: %s,  content: %08x,  current_value: %08x\n", tmp->expr, tmp->content, expr(tmp->expr, &expr_result));
		tmp = tmp->next;
	}	
}
