#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

int err(char *str)
{
	while (*str)
		write(2, str++, 1);
	return 1;
}

int cd(char **argv, int i)
{
	if (i != 2)
		return err("error: cd: bad arguments\n");
	if (chdir(argv[1]) == -1)
		return err("error: cd: cannot change directory to "), err(argv[1]), err("\n");
	return 0;
}

int execute(char **argv, int i, char **envp)
{
	int	fd[2];
	int	status;
	int	has_pipe = argv[i] && !strcmp(argv[i], "|");

	if (!has_pipe && !strcmp(*argv, "cd"))
		return cd(argv, i);

	if (has_pipe && pipe(fd) == -1)
		return err("error: fatal\n");

	int pid = fork();
	if (!pid)
	{
		argv[i] = 0;
		if (has_pipe && (dup2(fd[1], 1) == -1 || close(fd[0]) == -1 || close(fd[1]) == -1))
			return err("error: fatal\n");
		if (!strcmp(*argv, "cd"))
			return cd(argv, i);
		execve(*argv, argv, envp);
		return err("error: cannot execute "), err(*argv), err("\n");
	}

	waitpid(pid, &status, 0);
	if (has_pipe && (dup2(fd[0], 0) == -1 || close(fd[0]) == -1 || close(fd[1]) == -1))
		return err("error: fatal\n");
	return WIFEXITED(status) && WEXITSTATUS(status);
}

int main(int ac, char **av, char **envp)
{
	int	i = 1;
	int	status = 0;

	while (i < ac)
	{
		int j = i;
		while (j < ac && strcmp(av[j], "|") && strcmp(av[j], ";"))
			j++;
		if (j > i)
			status = execute(av + i, j - i, envp);
		i = j + 1;
	}
	return status;
}



// int main(int argc, char **argv, char **envp)
// {
// 	int i = 0;
// 	int status = 0;

// 	if (argc > 1)
// 	{
// 		while (argv[i] && argv[++i])
// 		{
// 			argv += i;
// 			i = 0;
// 			while (argv[i] && strcmp(argv[i], "|") && strcmp(argv[i], ";"))
// 				i++;
// 			if (i)
// 				status = exec(argv, i, envp);
// 		}
// 	}
// 	return status;
// }
