#include <stdio.h>  
#include <stdlib.h> 
 
#define MAX_LEN 80  

void convert2postfix(char *src, char *dst) {
	char *psrc, *pdst;
	char stack[MAX_LEN];
	int top;
 
	top = -1;
	psrc = src;
	pdst = dst;
	while (*psrc != '\0') {
		if (*psrc >= '0' && *psrc <= '9') {
			*pdst = *psrc;
			pdst++;
			//加入分隔空格  
			if (!(*(psrc + 1) >= '0' && *(psrc + 1) <= '9') && *(psrc + 1) != '.') {
				*pdst = ' ';
				pdst++;
			}
		}
		if (*psrc == '.') {
			*pdst = *psrc;
			pdst++;
		}
		if (*psrc == '(') {
			stack[++top] = *psrc;
		}
		if (*psrc == ')') {
			while (stack[top] != '(') {
				*pdst = stack[top--];
				pdst++;
				//加入分隔空格  
				*pdst = ' ';
				pdst++;
			}
			//弹出'('  
			top--;
		}
		if (*psrc == '*' || *psrc == '/') {
			if (stack[top] == '*' || stack[top] == '/') {
				*pdst = stack[top--];
				pdst++;
				//加入分隔空格  
				*pdst = ' ';
				pdst++;
			}
			stack[++top] = *psrc;
		}
		if (*psrc == '+' || *psrc == '-') {
			while (stack[top] == '*'
				|| stack[top] == '/'
				|| stack[top] == '+'
				|| stack[top] == '-') {
				*pdst = stack[top--];
				pdst++;
				//加入分隔空格  
				*pdst = ' ';
				pdst++;
			}
			stack[++top] = *psrc;
		}
		psrc++;
	}
	//扫描完成后，取出栈中所有运算符，写入后缀表达式数组。  
	while (top != -1) {
		*pdst = stack[top--];
		*pdst++;
		*pdst = ' ';
		pdst++;
	}
	*pdst = '\0';
}


int cal(char *src) {
	int stack[MAX_LEN];
	int opd1, opd2;
	int top;
	char *p, *pre;
	top = -1;

	p = src;
	while (*p != '\0') {
		if (*p >= '0' && *p <= '9') {
			pre = p;
			while ((*p >= '0' && *p <= '9') || *p == '.') {
				p++;
			}
			*p = '\0';
			stack[++top] = atof(pre);
		}
		if (*p == '+' || *p == '-' || *p == '*' || *p == '/') {
			opd2 = stack[top--];
			opd1 = stack[top--];
			switch (*p) {
			case '+':
				stack[++top] = opd1 + opd2;
				break;
			case '-':
				stack[++top] = opd1 - opd2;
				break;
			case '*':
				stack[++top] = opd1*opd2;
				break;
			case '/':
				//更严格一点，应该处理除数为0的情况  
				stack[++top] = opd1 / opd2;
				break;
			}
		}
		p++;
	}
	return stack[top--];
}

int main() {
	char str1[MAX_LEN], str2[MAX_LEN];
	int res;

	gets(str1);
	convert2postfix(str1, str2);
	
	res = cal(str2);
	printf("%d\n", res);
	return 0;
}

