#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>

#define BUFFER_SIZE 1024
#define TOKEN_SIZE 64

// ANSI Color codes for Fallout terminal aesthetic
#define GREEN "\033[32m"
#define BRIGHT_GREEN "\033[1;32m"
#define YELLOW "\033[33m"
#define CYAN "\033[36m"
#define RESET "\033[0m"
#define BOLD "\033[1m"

// Function prototypes
char *read_line(void);
char **split_line(char *line);
int execute_command(char **args);
int launch_process(char **args);
int builtin_cd(char **args);
int builtin_help(char **args);
int builtin_exit(char **args);
int builtin_clear(char **args);
int builtin_status(char **args);
void print_boot_sequence(void);
void print_prompt(void);
int pipe_cmd(char **args, int start, int end, int pipe_pos);

// Built-in command names
char *builtin_names[] = {
    "cd",
    "help",
    "exit",
    "clear",
    "status"
};

// Built-in command functions
int (*builtin_functions[]) (char **) = {
    &builtin_cd,
    &builtin_help,
    &builtin_exit,
    &builtin_clear,
    &builtin_status
};

int num_builtins() {
    return sizeof(builtin_names) / sizeof(char *);
}

// Boot sequence for that authentic Fallout feel
void print_boot_sequence(void) {
    printf(GREEN);
    printf("███████╗ ██████╗ ██╗      █████╗ ██████╗ ██╗████████╗\n");
    printf("██╔════╝██╔═══██╗██║     ██╔══██╗██╔══██╗██║██╔═════╝\n");
    printf("███████╗██║   ██║██║     ███████║██████╔╝██║████████╗\n");
    printf("╚════██║██║   ██║██║     ██╔══██║██╔══██╗██║╚═════██║\n");
    printf("███████║╚██████╔╝███████╗██║  ██║██║  ██║██║████████║\n");
    printf("╚══════╝ ╚═════╝ ╚══════╝╚═╝  ╚═╝╚═╝  ╚═╝╚═╝╚══════╝\n\n");
    printf("N3WL1N3 SHELL\n");
    printf("ENTERING SHELL\n");
    printf("INITIALIZING COMMAND INTERFACE...\n");
    printf("LOADING SYSTEM DIAGNOSTICS...\n");
    usleep(500000); // 0.5 second delay
    printf("WELCOME TO TERMINAL\n");
    printf("COPYRIGHT 2025 SOLARIS\n");
    printf(RESET);
    printf("\n");
}

// Fallout-style prompt
void print_prompt(void) {
    time_t rawtime;
    struct tm *timeinfo;
    char time_buffer[20];

    time(&rawtime);
    timeinfo = localtime(&rawtime);
    strftime(time_buffer, sizeof(time_buffer), "%H:%M:%S", timeinfo);

    printf(BRIGHT_GREEN "[%s]" RESET " " CYAN "TERMINAL>" RESET " ", time_buffer);
    fflush(stdout);
}

// Read line from user with proper memory management
char *read_line(void) {
    int buffer_size = BUFFER_SIZE;
    int position = 0;
    char *buffer = malloc(sizeof(char) * buffer_size);
    int c;

    if (!buffer) {
        fprintf(stderr, "TERMINAL: MEMORY ALLOCATION FAILED\n");
        exit(EXIT_FAILURE);
    }

    while (1) {
        c = getchar();

        if (c == EOF || c == '\n') {
            buffer[position] = '\0';
            return buffer;
        } else {
            buffer[position] = c;
        }

        position++;

        if (position >= buffer_size) {
            buffer_size += BUFFER_SIZE;
            buffer = realloc(buffer, buffer_size);
            if (!buffer) {
                fprintf(stderr, "TERMINAL: MEMORY REALLOCATION FAILED\n");
                exit(EXIT_FAILURE);
            }
        }
    }
}

// Split line into tokens
char **split_line(char *line) {
    int buffer_size = TOKEN_SIZE;
    int position = 0;
    char **tokens = malloc(buffer_size * sizeof(char*));
    char *token;

    if (!tokens) {
        fprintf(stderr, "TERMINAL: TOKEN ALLOCATION FAILED\n");
        exit(EXIT_FAILURE);
    }

    token = strtok(line, " \t\r\n\a");
    while (token != NULL) {
        tokens[position] = token;
        position++;

        if (position >= buffer_size) {
            buffer_size += TOKEN_SIZE;
            tokens = realloc(tokens, buffer_size * sizeof(char*));
            if (!tokens) {
                fprintf(stderr, "TERMINAL: TOKEN REALLOCATION FAILED\n");
                exit(EXIT_FAILURE);
            }
        }

        token = strtok(NULL, " \t\r\n\a");
    }
    tokens[position] = NULL;
    return tokens;
}

// Built-in command implementations
int builtin_cd(char **args) {
    if (args[1] == NULL) {
        if (chdir(getenv("HOME")) != 0) {
            perror("TERMINAL: CD");
        }
    } else {
        if (chdir(args[1]) != 0) {
            perror("TERMINAL: CD");
        }
    }
    return 1;
}

int builtin_help(char **args) {
    printf(YELLOW "NEO TERMINAL HELP\n" RESET);
    printf("=============================\n");
    printf("Built-in commands:\n");
    for (int i = 0; i < num_builtins(); i++) {
        printf("  %s\n", builtin_names[i]);
    }
    printf("\nI/O Redirection:\n");
    printf("  > file    - Redirect output to file\n");
    printf("  >> file   - Append output to file\n");
    printf("  < file    - Redirect input from file\n");
    printf("  cmd1 | cmd2 - Pipe output of cmd1 to cmd2\n");
    printf("\nUse 'man command' for more information on system commands.\n");
    return 1;
}

int builtin_exit(char **args) {
    printf(GREEN "TERMINAL SESSION TERMINATED\n" RESET);
    printf("we do it because we are driven.\n");
    return 0;
}

int builtin_clear(char **args) {
    printf("\033[2J\033[H"); // Clear screen and move cursor to top
    print_boot_sequence();
    return 1;
}

int builtin_status(char **args) {
    printf(CYAN "SYSTEM STATUS: OPERATIONAL\n" RESET);
    printf("NEO TERMINAL VERSION: 1.0\n");
    printf("ACCESS: USER\n");
    printf("CURRENT DIR: %s\n", getcwd(NULL, 0));
    return 1;
}

// Fixed pipe command function
int pipe_cmd(char **args, int start, int end, int pipe_pos) {
    // Validate input parameters
    if (pipe_pos <= start || pipe_pos >= end - 1) {
        fprintf(stderr, "TERMINAL: INVALID PIPE POSITION\n");
        return 1;
    }
    
    // Calculate proper sizes
    int cmd1_size = pipe_pos - start + 1; // +1 for NULL terminator
    int cmd2_size = end - pipe_pos; // This already includes space for NULL terminator
    
    if (cmd1_size <= 0 || cmd2_size <= 0) {
        fprintf(stderr, "TERMINAL: INVALID COMMAND SIZE\n");
        return 1;
    }
    
    char **cmd1 = malloc(sizeof(char*) * cmd1_size);
    char **cmd2 = malloc(sizeof(char*) * cmd2_size);
    
    if (!cmd1 || !cmd2) {
        fprintf(stderr, "TERMINAL: MEMORY ALLOCATION FAILED\n");
        if (cmd1) free(cmd1);
        if (cmd2) free(cmd2);
        return 1;
    }

    // Build cmd1 array
    int i = 0;
    for (int idx = start; idx < pipe_pos; idx++) {
        cmd1[i] = args[idx];
        i++;
    }
    cmd1[i] = NULL;

    // Build cmd2 array (skip the pipe symbol)
    int j = 0;
    for (int idx = pipe_pos + 1; idx < end; idx++) {
        cmd2[j] = args[idx];
        j++;
    }
    cmd2[j] = NULL;

    int pipefd[2];
    if (pipe(pipefd) == -1) {
        perror("TERMINAL: PIPE");
        free(cmd1);
        free(cmd2);
        return 1;
    }

    pid_t pid1 = fork();
    if (pid1 == 0) {
        // Child process 1 - write to pipe
        close(pipefd[0]);
        if (dup2(pipefd[1], STDOUT_FILENO) == -1) {
            perror("TERMINAL: DUP2");
            exit(EXIT_FAILURE);
        }
        close(pipefd[1]);
        
        if (execvp(cmd1[0], cmd1) == -1) {
            perror("TERMINAL: COMMAND 1 NOT FOUND");
            exit(EXIT_FAILURE);
        }
    } else if (pid1 < 0) {
        perror("TERMINAL: FORK 1");
        close(pipefd[0]);
        close(pipefd[1]);
        free(cmd1);
        free(cmd2);
        return 1;
    }

    pid_t pid2 = fork();
    if (pid2 == 0) {
        // Child process 2 - read from pipe
        close(pipefd[1]);
        if (dup2(pipefd[0], STDIN_FILENO) == -1) {
            perror("TERMINAL: DUP2");
            exit(EXIT_FAILURE);
        }
        close(pipefd[0]);
        
        if (execvp(cmd2[0], cmd2) == -1) {
            perror("TERMINAL: COMMAND 2 NOT FOUND");
            exit(EXIT_FAILURE);
        }
    } else if (pid2 < 0) {
        perror("TERMINAL: FORK 2");
        close(pipefd[0]);
        close(pipefd[1]);
        free(cmd1);
        free(cmd2);
        return 1;
    }

    // Parent process - close both ends of pipe and wait for children
    close(pipefd[0]);
    close(pipefd[1]);

    int status1, status2;
    waitpid(pid1, &status1, 0);
    waitpid(pid2, &status2, 0);

    free(cmd1);
    free(cmd2);

    return 1;
}

// Launch external process
int launch_process(char **args) {
    pid_t pid;
    int status;
    int redirect = -1;
    char *file = NULL;

    // Check for redirection operators
    for (int i = 0; args[i] != NULL; i++) {
        if (strcmp(args[i], ">") == 0) {
            redirect = 0;
            file = args[i + 1];
            args[i] = NULL;
            break;
        }
        if (strcmp(args[i], ">>") == 0) {
            redirect = 1;
            file = args[i + 1];
            args[i] = NULL;
            break;
        }
        if (strcmp(args[i], "<") == 0) {
            redirect = 2;
            file = args[i + 1];
            args[i] = NULL;
            break;
        }
    }

    pid = fork();
    if (pid == 0) {
        // Child process - handle redirection
        if (redirect == 0) {
            int fd = open(file, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (fd == -1) {
                perror("TERMINAL: OPEN");
                exit(EXIT_FAILURE);
            }
            dup2(fd, STDOUT_FILENO);
            close(fd);
        }
        if (redirect == 1) {
            int fd = open(file, O_WRONLY | O_CREAT | O_APPEND, 0644);
            if (fd == -1) {
                perror("TERMINAL: OPEN");
                exit(EXIT_FAILURE);
            }
            dup2(fd, STDOUT_FILENO);
            close(fd);
        }
        if (redirect == 2) {
            int fd = open(file, O_RDONLY);
            if (fd == -1) {
                perror("TERMINAL: OPEN");
                exit(EXIT_FAILURE);
            }
            dup2(fd, STDIN_FILENO);
            close(fd);
        }
        
        // Execute command
        if (execvp(args[0], args) == -1) {
            perror("TERMINAL: COMMAND NOT FOUND");
            exit(EXIT_FAILURE);
        }
    } else if (pid < 0) {
        perror("TERMINAL: FORK");
    } else {
        // Parent process
        waitpid(pid, &status, WUNTRACED);
    }

    return 1;
}

// Execute command (check built-ins first, then external)
int execute_command(char **args) {
    int start = 0;
    int pipe_pos = -1;  // Initialize to -1
    int end = 0;

    if (args[0] == NULL) {
        return 1; // Empty command
    }

    // Find pipe position and end
    for (int i = 0; args[i] != NULL; i++) {
        if (strcmp(args[i], "|") == 0) {
            pipe_pos = i;
        }
        end = i + 1;
    }

    // If we found a pipe, handle it
    if (pipe_pos != -1) {
        return pipe_cmd(args, start, end, pipe_pos);
    }

    // Check for built-in commands
    for (int i = 0; i < num_builtins(); i++) {
        if (strcmp(args[0], builtin_names[i]) == 0) {
            return (*builtin_functions[i])(args);
        }
    }

    // Execute external command
    return launch_process(args);
}

// Main shell loop
int main(void) {
    char *line;
    char **args;
    int status;

    print_boot_sequence();

    do {
        print_prompt();
        line = read_line();
        args = split_line(line);
        status = execute_command(args);

        free(line);
        free(args);
    } while (status);

    return EXIT_SUCCESS;
}
