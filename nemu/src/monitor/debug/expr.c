#include "nemu.h"

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>
#include <stdlib.h>

#define MAX_LEVEL 32;

enum {
	NOTYPE = 256, EQ, NUM,HEXNUM, MINUS, OR, AND, NEQ, NOT, POINT, EAX, ECX, EDX,EBX,ESP,EBP,ESI,EDI,EIP

	/* TODO: Add more token types */

};

static struct rule {
	char *regex;
	int token_type;
} rules[] = {

	/* TODO: Add more rules.
	 * Pay attention to the precedence level of different rules.
	 */

	{" +",	NOTYPE},				// spaces
	{"\\+", '+'},					// plus
	{"==", EQ},						// equal
	{"-", '-'},
	{"\\*", '*'},
	{"\\(", '('},
	{"\\)", ')'},
	{"0x[0-9a-fA-F]+", HEXNUM},
	{"\\$eax", EAX},
	{"\\$ecx", ECX},
	{"\\$edx", EDX},
	{"\\$ebx", EBX},
	{"\\$esp", ESP},
	{"\\$ebp", EBP},
	{"\\$esi", ESI},
	{"\\$edi", EDI},
	{"\\$eip", EIP},
	{"[0-9]+", NUM},
	{"\\|\\|", OR},
	{"&&", AND},
	{"!=", NEQ},
	{"!", NOT},
};

#define NR_REGEX (sizeof(rules) / sizeof(rules[0]) )

static regex_t re[NR_REGEX];

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
	int i;
	char error_msg[128];
	int ret;

	for(i = 0; i < NR_REGEX; i ++) {
		ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
		if(ret != 0) {
			regerror(ret, &re[i], error_msg, 128);
			Assert(ret == 0, "regex compilation failed: %s\n%s", error_msg, rules[i].regex);
		}
	}
}

typedef struct token {
	int type;
	char str[32];
} Token;

Token tokens[32];
int nr_token;

int check_parentheses(int p, int q);

int getDomainPositon(int p, int q);

static bool make_token(char *e) {
	int position = 0;
	int i;
	regmatch_t pmatch;
	
	nr_token = 0;

	while(e[position] != '\0') {
		/* Try all rules one by one. */
		for(i = 0; i < NR_REGEX; i ++) {
			if(regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
				char *substr_start = e + position;
				int substr_len = pmatch.rm_eo;

				//Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s", i, rules[i].regex, position, substr_len, substr_len, substr_start);
				position += substr_len;

				/* TODO: Now a new token is recognized with rules[i]. Add codes
				 * to record the token in the array ``tokens''. For certain 
				 * types of tokens, some extra actions should be performed.
				 */

				switch(rules[i].token_type) {
					case '+':
						tokens[nr_token++].type = '+';
						break;
					case '-':
						if(nr_token==0 ||(tokens[nr_token-1].type!=NUM && tokens[nr_token-1].type!=')'))
						{
							tokens[nr_token++].type=MINUS;
							break;
						}	
						//break;						
						tokens[nr_token++].type='-';
						break;
					case '*':
						if(nr_token==0 || (tokens[nr_token-1].type!=NUM && tokens[nr_token-1].type!=')'))
						{
							tokens[nr_token++].type=POINT;
							break;
						}
						tokens[nr_token++].type='*';
						break;
					case '/':
						tokens[nr_token++].type='/';
					case NOTYPE:
						break;
					case EAX:
					case ECX:
					case EDX:
					case EBX:
					case ESP:
					case EBP:
					case ESI:
					case EDI:
					case EIP:
						tokens[nr_token++].type=rules[i].token_type;
						break;
					case NEQ:
					case AND:
					case OR:
					case NOT:
					case EQ:
						tokens[nr_token++].type=rules[i].token_type;
						break;
					case HEXNUM:
						tokens[nr_token].type=HEXNUM;
						if(substr_len<32){
							strncpy(tokens[nr_token].str, substr_start, substr_len);
							tokens[nr_token].str[substr_len]='\0';	
						}else{
							Assert(0, "the len of HEXNUM exceed 32");
						}
						nr_token++;
						break;
					case NUM:
						tokens[nr_token].type=NUM;
						if(substr_len<32 )
						{
							strncpy(tokens[nr_token].str, substr_start, substr_len);
							tokens[nr_token].str[substr_len]='\0';
						}
						else{
							Assert(0, "the len of NUM exceed 32");
						}
						nr_token++;
						break;
					default: panic("please implement me");
				}

				break;
			}
		}

		if(i == NR_REGEX) {
			printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
			return false;
		}
	}

	return true; 
}


uint32_t eval(int p, int q)
{
	if(p>q)
	{
		Assert(0, "q exceed p");
	}
	else if(p==q)
	{
		switch(tokens[p].type){
			case NUM:
				return atoi(tokens[p].str);
			case HEXNUM:
				return strtol(tokens[p].str, NULL, 16);
			case EAX:
				return cpu.eax;
			case ECX:
				return cpu.ecx;
			case EDX:
				return cpu.edx;
			case EBX:
				return cpu.ebx;
			case ESP:
				return cpu.esp;
			case EBP:
				return cpu.ebp;
			case ESI:
				return cpu.esi;
			case EDI:
				return cpu.edi;
			case EIP:
				return cpu.eip;				
		}
		//ireturn atoi(tokens[p].str);
	}else if(check_parentheses(p,q)==true){
		return eval(p+1, q-1);
	}else
	{
		int val_l=0, val_r;
		int pos = getDomainPositon(p,q);
		if(pos!=p)
			val_l = eval(p, pos-1);
		val_r = eval(pos+1, q);
		switch(tokens[pos].type)
		{
			case '-':
				return val_l - val_r;
			case '+':
				return val_l + val_r;
			case '*':
				return val_l * val_r;
			case '/':
				return val_l / val_r;
			case MINUS:
				return -val_r;
			case NOT:
				return !val_r;
			case EQ:
				return val_l==val_r;
			case NEQ:
				return val_l!=val_r;
			case AND:
				return val_l && val_r;
			case OR:
				return val_l || val_r;
			case POINT:
				return swaddr_read(val_r, 4);
			default:
				Assert(0, "unknow operator");
		}		
	}
	return -1;
}

int check_parentheses(int p, int q)
{
	int stack=0;
	if(tokens[p].type!='(' || tokens[q].type!=')')
	{
		return false;
	}
	for(; p<=q; p++)
	{
		if(tokens[p].type=='(')
		{
			stack++;
		}
		if(tokens[p].type==')')
		{
			stack--;
		}
	}
	if(stack!=0)
	{
		Assert(0, "check_parentheses fail");
	}
	return true;
}

int getDomainPositon(int p, int q)
{
	int pos = -1;
	int level = MAX_LEVEL;
	for(;p<=q; p++)
	{
		if(tokens[p].type=='(')
		{
			while(tokens[p].type!=')')
			{
				p++;
			}
		}else{
			if(tokens[p].type==OR){
				if(level>=1){
					pos=p;
					level=1;
				}
			}
			if(tokens[p].type==AND){
				if(level>=2)
				{
					pos=p;
					level = 2;
				}
			}
			if(tokens[p].type==NEQ){
				if(level>=3)
				{
					pos=p;
					level = 3;
				}
			}
			if(tokens[p].type==EQ)
			{
				if(level>=4)
				{
					pos = p;
					level=4;
				}
			}
			if(tokens[p].type=='+' || tokens[p].type=='-')
			{
				if(level >=5){
					pos = p;
					level = 5;
				}		
			}
			if(tokens[p].type=='*' || tokens[p].type=='/')
			{
				if(level>=6)
				{
					pos = p;
					level=6;	
				}			
			}
			if(tokens[p].type==MINUS || tokens[p].type==NOT || tokens[p].type==POINT)
			{
				if(level>=7){
					pos = p;
					level = 7;
				}				
			}
		}
	}
	return pos;
}

uint32_t expr(char *e, bool *success) {
	if(!make_token(e)) {
		*success = false;
		return 0;
	}

	/* TODO: Insert codes to evaluate the expression. */
	//panic("please implement me");
	*success = true;
	return eval(0, nr_token-1);
	
}

