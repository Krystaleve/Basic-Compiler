#include <stdio.h>
#include <string.h>

char s[1010];

int main(int argc, char *argv[]) {
	int i; 
	int flag = 0;
	scanf("%s",s);
	int L = strlen(s);
	for(i = 0; i < L - i - 1; i++) {
		if(s[i] != s[L - i - 1]) {
			flag = 1;
			break;
		}
	}
	if(flag) {
		printf("False");
	}
	else {
		printf("True");
	}
	return 0;
}
