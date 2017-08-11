// This is what is going to be klsh 
// (pronounced clash, stands for Kyle Lanmon SHell)
/**
 *  TODO:
 *      - .klsh_history to support up/down commands
 *      - piping
 *      - redirecting
 *      - background with &
 *      - coloring (in ls)
 *      - tab to complete
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pwd.h>
#include <sys/wait.h>

typedef struct commands {
    int argc;
    char **argv;
} command;

void log_cmd(command cmd);
void klsh_loop();
void klsh_split(command *cmd, char *args[], char *line, int size, char *delimiter);
int klsh_exec(command cmd);
int klsh_launch(command cmd);
int klsh_cd(command cmd);
int num_builtins();
FILE *historyFile();

char *builtin_str[] = {
    "cd"
};

int main (int argc, char **argv) {
    //config
    //loop
    klsh_loop();
    //shutdown and cleanup
    return 0;
} 

FILE *historyFile() {
    char history[80];
    sprintf(history, "%s/.klsh_history", getenv("HOME"));
    return fopen(history, "a+");
}

void log_cmd(command cmd) {
    FILE *file = historyFile();
    char format[] = "%s ";
    for (int i = 0; i < cmd.argc; i++) {
        if (i == cmd.argc - 1) {
            format[strlen(format)-1] = '\n';
        }
        fprintf(file, format, cmd.argv[i]);
    }
    fclose(file);
}

void klsh_loop() {
    char *line;
    ssize_t capacity = 0;
    char *args[256];
    int status;
    char delimiter[2] = " ";
    do {
        command cmd;
        printf(/*prompt*/"klsh> ");
        getline(&line, &capacity, stdin); 
        klsh_split(&cmd, args, line, strlen(line), delimiter); // change later to handle quotes being one arg
        log_cmd(cmd);
        status = klsh_exec(cmd);
    } while (status);
}

void klsh_split(command *cmd, char *args[], char *line, int size, char *delimiter) {
    int index = 0;
    args[index] = strtok(line, delimiter);
    while(args[index]) {
        args[++index] = strtok(NULL, delimiter);
    }
    args[index-1][strlen(args[index-1])-1] = 0; // remove newline
    cmd->argc = index;
    cmd->argv = args;
}

int klsh_exec(command cmd) {
    if (cmd.argv[0] == NULL) {
        return 1;
    }
    for (int i = 0; i < cmd.argc; i++) {
        char *temp = cmd.argv[i];
        temp++;
        char new[80];
        strcpy(new, "");

        if (strstr(cmd.argv[i], "~/") == cmd.argv[i] || strcmp(cmd.argv[i], "~") == 0) { // starts with ~/ or is only ~
            strcat(new, getenv("HOME"));
            strcat(new, temp);
            cmd.argv[i] = new;
        } else if (strstr(cmd.argv[i], "~") == cmd.argv[i]) {
            char user[80];
            char *start = temp;
            if (strstr(temp, "/")) {
                int i = 0;
                while (temp[0] !=  '/') {
                    i++;
                    temp++;
                }
                strcpy(user, "");
                strncpy(user, start, i);
                strcat(new, getpwnam(user)->pw_dir);
                strcat(new, temp);
            } else {
                strcat(new, getpwnam(temp)->pw_dir);
            }
            cmd.argv[i] = new;
        }
    }

    if (strcmp(cmd.argv[0], "cd") == 0) {
        return klsh_cd(cmd);
    }

    return klsh_launch(cmd);
}

int klsh_launch(command cmd) {
    pid_t pid, wpid;
    int status;

    pid = fork();
    if (pid == 0) { // Child
        if (execvp(cmd.argv[0], cmd.argv) == -1) {
            perror("klsh");
        }
        exit(1);
    } else if (pid < 0) { // Error
        perror("klsh could not fork");
    } else { // Parent
        do {
            wpid = waitpid(pid, &status, WUNTRACED);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
        return 1;
    }
}

int num_builtins() {
    return sizeof(builtin_str) / sizeof(char*);
}

int klsh_cd(command cmd) {
    if (cmd.argv[1] == NULL) {
        chdir(getenv("HOME"));
    } else {
        if (chdir(cmd.argv[1]) != 0) {
            perror("klsh cd");
        }
    }
    return 1;
}
