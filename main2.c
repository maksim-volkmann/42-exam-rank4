#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

int	main(int ac, char **av, char **envp)
{
	int	i = 1;
	int j;
	int status = 0;

	while(i < ac)
	{
		j = i;
		while(j < ac && strcmp(av[j], "|") && strcmp(av[j], ";"))
			j++;
		if(j > i)
			status = execute(av + i, j - i, envp);
		i = j + 1;
	}

	return status;
}
