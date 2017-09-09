#include <stdbool.h>
#include <stdio.h>
#include <stdio_ext.h>
#include <stdlib.h>
#include <string.h>

int generateString(size_t n, char* string)
{
	if (string == NULL)
		return 1;
	size_t i;
	*string = 'a';
	for (i = 1; i < n; i++)
	{
		strncpy(string + (1 << i), string, (1 << i) - 1);
		*(string + (1 << i) - 1) = 'a' + i;
	}
	*(string + (1 << n) - 1) = '\0';

	return 0;
}

int main()
{
	size_t n;
	int scanf_retval;
	printf("Input n\n");

	scanf_retval = scanf("%lu", &n);

	while (scanf_retval != 1 || n > 26)
	{
		printf("Incorrect input. Please try again.\n");
		if (scanf_retval != 1)
			__fpurge(stdin);
		scanf_retval = scanf("%lu", &n);
	}
	
	char* string = malloc(sizeof(*string) * (1 << n));
	if (string == NULL)
	{
		fprintf(stderr, "Error in memory allocation\n");
		return 1;
	}
	if (generateString(n, string))
	{
		fprintf(stderr, "Error in string generation\n");
		return 1;
	}

	puts(string);
	free(string);
	return 0;
}

