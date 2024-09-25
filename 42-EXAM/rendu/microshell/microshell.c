#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

// Function to print an error message to STDERR
int print_error(char *msg)
{
    while (*msg)
        write(2, msg++, 1);
    return 1;
}

// Function to handle the 'cd' command
int handle_cd(char **args, int argc)
{
    if (argc != 2)
        return print_error("error: cd: bad arguments\n");
    if (chdir(args[1]) == -1)
    {
        write(2, "error: cd: cannot change directory to ", 38);
        write(2, args[1], strlen(args[1]));
        write(2, "\n", 1);
        return 1;
    }
    return 0;
}

// Function to execute a single command, possibly setting up a pipe
int execute_command(char **args, int argc, char **env)
{
    int pipe_fd[2];
    pid_t pid;
    int status;

    // Check if the next argument is a pipe
    int is_pipe = (args[argc] && strcmp(args[argc], "|") == 0);

    // Handle 'cd' if it's a standalone command
    if (!is_pipe && strcmp(args[0], "cd") == 0)
        return handle_cd(args, argc);

    // If there's a pipe, create it
    if (is_pipe && pipe(pipe_fd) == -1)
        return print_error("error: fatal\n");

    // Fork the process
    pid = fork();
    if (pid == -1)
        return print_error("error: fatal\n");

    if (pid == 0)
    {
        // Child process

        // If there's a pipe, redirect stdout to the pipe's write end
        if (is_pipe)
        {
            if (dup2(pipe_fd[1], STDOUT_FILENO) == -1)
                return print_error("error: fatal\n");
            close(pipe_fd[0]);
            close(pipe_fd[1]);
        }

        // Execute the command
        execve(args[0], args, env);

        // If execve fails, print an error and exit
        write(2, "error: cannot execute ", 21);
        write(2, args[0], strlen(args[0]));
        write(2, "\n", 1);
        _exit(1);
    }

    // Parent process waits for the child to finish
    waitpid(pid, &status, 0);

    // If there was a pipe, set up the read end for the next command
    if (is_pipe)
    {
        if (dup2(pipe_fd[0], STDIN_FILENO) == -1)
            return print_error("error: fatal\n");
        close(pipe_fd[0]);
        close(pipe_fd[1]);
    }

    // Return the child's exit status
    if (WIFEXITED(status))
        return WEXITSTATUS(status);
    return 1;
}

int main(int argc, char **argv, char **env)
{
    int i = 1;          // Start from 1 to skip the program name
    int status = 0;     // To store the status of the last executed command

    while (i < argc)
    {
        int j = i;

        // Find the next separator ('|' or ';') or the end of arguments
        while (j < argc && strcmp(argv[j], "|") && strcmp(argv[j], ";"))
            j++;

        // Execute the command from argv[i] to argv[j-1] if there are arguments
        if (j > i)
            status = execute_command(&argv[i], j - i, env);

        // Move to the next argument after the separator
        i = j + 1;
    }

    return status;  // Return the exit status of the last command
}
