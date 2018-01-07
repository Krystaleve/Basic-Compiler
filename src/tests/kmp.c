#include <stdio.h>
#include <string.h>

char s[1010];
char t[1010];
int next[1010];

int main(int argc, char *argv[]) {
	int i, j, k; 
	scanf("%s", s);
	scanf("%s", t);
	int sLen = strlen(s);  
	int tLen = strlen(t);

	next[0] = -1;
	
	for (i = 1; i < tLen; i++)
	{
		j = next[i - 1];
		while( j != -1 && t[j + 1] != t[i])
			j=next[j];
		if(t[j + 1] == t[i])
			next[i] = j+1;
		else next[i] = -1;
	}
	
	
	
	
	i = 0;  // S 的下标
	j = 0;  // t 的下标

	while (i < sLen)
	{
		if (s[i] == t[j])  // P 的第一个字符不匹配或 S[i] == P[j]
		{
			i++;
			j++;
			if(j == tLen){
				printf("%d ", i - tLen);
				j = next[j - 1] + 1;
			}
		}
		else if(j == 0){
			i++;
		}
		else
			j = next[j - 1] + 1;
	}
	
	return 0;
	
}
