#include <stdio.h>

int main()
{
	char text[1024];
	printf("Enter your name: ");
	scanf("%s", text);
	printf("Hello, %s!\n", text);
	return 0;
}
