#include "monitor/monitor.h"
#include "monitor/expr.h"
#include "monitor/watchpoint.h"
#include "nemu.h"

#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>

void cpu_exec(uint32_t);

/* We use the ``readline'' library to provide more flexibility to read from stdin. */
char* rl_gets() {
	static char *line_read = NULL;

	if (line_read) {
		free(line_read);
		line_read = NULL;
	}

	line_read = readline("(nemu) ");

	if (line_read && *line_read) {
		add_history(line_read);
	}

	return line_read;
}

static int cmd_c(char *args) {
	cpu_exec(-1);
	return 0;
}

static int cmd_q(char *args) {
	return -1;
}

static int cmd_si(char *args){
	int execute_num = atoi(args);
	execute_num = execute_num<=0 ? 1 : execute_num;
	cpu_exec(execute_num);
	return 0;
}

static int cmd_info(char *args){
	if(*args == 'r')
	{
		printf("eax: 0x%08x \n", cpu.eax);
		printf("ebp: 0x%08x \n", cpu.ebp);
		printf("ebx: 0x%08x \n", cpu.ebx);
		printf("ecx: 0x%08x \n", cpu.ecx);
		printf("edi: 0x%08x \n", cpu.edi);
		printf("edx: 0x%08x \n", cpu.edx);
		printf("eip: 0x%08x \n", cpu.eip);
		printf("esi: 0x%08x \n", cpu.esi);
		printf("esp: 0x%08x \n", cpu.esp);
	}
	if(*args == 'w'){
		
	}
	return 0;
}

static int cmd_x(char *args){
	char *arg1 = strtok(args, " ");
	bool result=false;
	if(arg1 ==NULL){
		Assert(0, "N can not be NULL");
	}
	char *arg2 = args+strlen(arg1)+1;
	if(arg2 ==  NULL){
		Assert(0, "Address can not be NULL");
	}
	int n = atoi(arg1);
	int addr =0 ;
	//addr = (int)strtol(arg2,NULL,16);
	addr = expr(arg2, &result);
	int i=0;
	for(i=0; i<n;i++){
		printf("address 0x%08x : 0x%08x \n",(swaddr_t)(addr+i*4), swaddr_read((swaddr_t)(addr+i*4), 4));
	}
	return 0;
}

static int cmd_p(char *args){
	bool result = false;
	if(args == NULL)
	{
		Assert(0, "expr is null");
	}
	int num = expr(args, &result);
	printf("result: 0x%08x\n", num);
	return 0;
}

static int cmd_w(char *args)
{
	bool result = false;
	unsigned int addr = expr(args, &result);
	printf("%d\n", addr);
	WP *wp = new_wp();
	printf("%d",(int) wp);
	//wp->point = addr;
	//wp->content = swaddr_read((swaddr_t)addr, 4);
	return 0;	
}

static int cmd_d(char* args){
	bool result = false;
	unsigned int addr = expr(args, &result);
	delete_wp(addr);
	return 0;
}

static int cmd_help(char *args);

static struct {
	char *name;
	char *description;
	int (*handler) (char *);
} cmd_table [] = {
	{ "help", "Display informations about all supported commands", cmd_help },
	{ "c", "Continue the execution of the program", cmd_c },
	{ "q", "Exit NEMU", cmd_q },
	{"si", "Executing next instruction", cmd_si},
	{"info", "show executing statue", cmd_info},
	{"x", "scan memory", cmd_x},
	{"p", "print expr", cmd_p},
	{"w", "set watchpoint", cmd_w},
	{"d", "delete watchpoint", cmd_d}
	/* TODO: Add more commands */

};

#define NR_CMD (sizeof(cmd_table) / sizeof(cmd_table[0]))

static int cmd_help(char *args) {
	/* extract the first argument */
	char *arg = strtok(NULL, " ");
	int i;

	if(arg == NULL) {
		/* no argument given */
		for(i = 0; i < NR_CMD; i ++) {
			printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
		}
	}
	else {
		for(i = 0; i < NR_CMD; i ++) {
			if(strcmp(arg, cmd_table[i].name) == 0) {
				printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
				return 0;
			}
		}
		printf("Unknown command '%s'\n", arg);
	}
	return 0;
}

void ui_mainloop() {
	while(1) {
		char *str = rl_gets();
		char *str_end = str + strlen(str);

		/* extract the first token as the command */
		char *cmd = strtok(str, " ");
		if(cmd == NULL) { continue; }

		/* treat the remaining string as the arguments,
		 * which may need further parsing
		 */
		char *args = cmd + strlen(cmd) + 1;
		if(args >= str_end) {
			args = NULL;
		}

#ifdef HAS_DEVICE
		extern void sdl_clear_event_queue(void);
		sdl_clear_event_queue();
#endif

		int i;
		for(i = 0; i < NR_CMD; i ++) {
			if(strcmp(cmd, cmd_table[i].name) == 0) {
				if(cmd_table[i].handler(args) < 0) { return; }
				break;
			}
		}

		if(i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }
	}
}
