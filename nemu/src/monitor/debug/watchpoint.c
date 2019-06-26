#include "monitor/watchpoint.h"
#include "monitor/expr.h"
#include "nemu.h"

#define NR_WP 32

static WP wp_pool[NR_WP];
static WP *head, *free_;

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
	if((free_ + 1) != (wp_pool + NR_WP)){
		free_+=1;
		if(head==NULL)
			head = wp_pool;
		else
			head+=1;
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
		wp->content = wp->next->content;
		wp->point = wp->next->point;
	}
	head-=1;
	if(head<wp_pool)
		head=NULL;
	free_ -=1;
}

bool check_wp_pool(){
	bool result = false;
	WP* point = wp_pool;
	while(head !=NULL &&  point < head){
		if(point->content != swaddr_read((swaddr_t)point->point, 4)){
			printf("the content of addre %08x has changed, the origin vale is: %x08, the new value is: %08x\n",point->point, point->content, swaddr_read((swaddr_t)point->point, 4));
			result = true;
		}
		point = point->next;		
	}
			
	return result;
}

void delete_wp(unsigned addr){
	WP* point = wp_pool;
	while(head!=NULL && point<head){
		if(point->point == addr){
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
	while(tmp<head){
		printf("watchpoint: %08x,  content: %08x,  current_value: %08x\n", tmp->point, tmp->content, swaddr_read((swaddr_t)tmp->point, 4));
	}	
}
