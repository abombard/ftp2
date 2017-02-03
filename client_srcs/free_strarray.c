#include <stdlib.h>

void	free_strarray(char **array)
{
	int	i;

	i = 0;
	while (array[i])
	{
		free(array[i++]);
	}
	free(array);
}
