#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <stdio.h>

#define TAB_SIZE 500
int main()
{
	char* mem[TAB_SIZE] = {0};
  int loop = 0;
	int i;
	int j;
	srand(time(NULL));
	do
	{
		i = 0;
		while (i < 1000)
		{
			j = rand() % TAB_SIZE;
			if (mem[j] == NULL)
			{
				size_t size = rand() % 1024;
				mem[j] = malloc(size);
        if (mem[j] == NULL)
        {
          fprintf(stderr, "malloc size %zu failed\n", size);
          return 1;
        }
        size_t x;
        for (x = 0; x < size; x++)
          mem[j][x] = 'a';
			}
			j = rand() % TAB_SIZE;
			if (mem[j] != NULL)
			{
				free(mem[j]);
				mem[j] = NULL;
			}
			++i;
		}
    usleep(2000);
	} while (++loop < 10000);
	return 0;
}
