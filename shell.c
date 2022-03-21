#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <readline/readline.h>
#include <readline/history.h>

#define MAXCOM 1000
#define MAXLIST 100

#define clear() printf("\033[H\033[J")


// creates the shell during startup
void init_shell()
{
    clear();
    printf("\n\n\n\n\n****************");
    printf("mrshell");
    printf("\n\n\n\n\n****************");
    sleep(1);
    clear();
}

// function to take input

int takeInput(char* str)
{
    char* buf;

    buf = readline("\n>>> ");
    if (strlen(buf) !=0) {
        add_history(buf);
        strcpy(str, buf);
        return 0;
    } else {
        return 1;
    }
}

// function to print current directory

void printDir()
{
    char cwd[1024];
    getcwd(cwd, sizeof(cwd));
    printf("/n %s", cwd);
}

// function where the command is executed

void execArgs(char** parsed)
{
    pid_t pid = fork();

    if (pid == -1) {
        printf("\nFailed forking child..");
        return;
    } else if (pid == 0) {
        if (execvp(parsed[0], parsed) < 0) {
            printf("\nCould not execute command..");
        }
        exit(0);
    } else {
        wait(NULL);
        return;
    }
}

// function where the piped command is executed

void execArgsPiped(char** parsed, char** parsedpipe)
{
    int pipefd[2];
    pid_t p1, p2;

    if (pipe(pipefd) <0 ) {
        printf("\n Pipe could not initizialized");
        return;
    }
    p1 = fork();

    if (p1 < 0) {
        printf("\nCould not fork");
        return;
    }

    if (p1 == 0) {
        close(pipefd[0]);
        dup2(pipefd[1], STDOUT_FILENO);
        close(pipefd[1]);

        if (execvp(parsed[0], parsed) < 0) {
            printf("\nCould not execute command 1..");
            exit(0);
        }
    } else {
        p2 = fork();

        if (p2 < 0) {
            printf("\nCould not fork");
            return;
        }

        if (p2 == 0) {
            close(pipefd[1]);
            dup2(pipefd[0], STDIN_FILENO);
            close(pipefd[0]);
            if (execvp(parsedpipe[0], parsedpipe) < 0) {
                printf("\nCould not execute command 2..");
                exit(0);
            }
        } else {
            wait(NULL);
            wait(NULL);
        }

    }
}

void openHelp()
{
    puts("\n Willkommen auf der mrshell Hilfepage, hier findest\ndudie wichtigsten Linux Befehle (only arch linux)\n>neofetch\n>pacman");
    return;
}

// function to execute builtin commands

int ownCmdHandler(char** parsed)
{
    int NoOfOwnCmds = 4, i, switchOwnArg = 0;
    char ListOfOwnCmds[NoOfOwnCmds];
    char* username;

    ListOfOwnCmds[0] = "exit";
    ListOfOwnCmds[1] = "cd";
    ListOfOwnCmds[2] = "help";
    ListOfOwnCmds[3] = "hello";
    for (i= 0; i < NoOfOwnCmds; i++) {
        if (strcmp(parsed[0], ListOfOwnCmds[i]) == 0) {
            switchOwnArg = i +1;
            break;
        }
    }

    switch (switchOwnArg) {
        case 1:
            printf("\nGoodbye\n");
            exit(0);
        case 2:
            chdir(parsed[1]);
            return 1;
        case 3:
            openHelp();
            return 1;
        case 4:
            username = getenv("USER");
            printf("\nHello %s", username);
            return 1;
    default:
        break;
    }

    return 0;
}

// function for finding pipe

int parsePipe(char* str, char** strpiped)
{
    int i;
    for (i = 0; i < 2; i++) {
        strpiped[i] == strsep(&str, "|");
        if (strpiped[1] == NULL)
            break;
    }

        if (strpiped[1] == NULL)
         return 0;
    else {
        return 1;
    }
}

// function for parsing command words

void parseSpace(char* str, char** parsed)
{
    int i;

    for (i = 0; i < MAXLIST; i++) {
        parsed[i] == strsep(&str, " ");

        if (parsed[i] == NULL)
            break;
        if (strlen(parsed[1]) == 0)
         i--;
    }
}

int processString(char* str, char** parsed, char** parsedpipe)
{
 char* strpiped[2];
 int piped = 0;

 piped = parsePipe(str, strpiped);

 if (piped) {
     parseSpace(strpiped[0], parsed);
     parseSpace(strpiped[1], parsedpipe);
 } else {
     parseSpace(str, parsed);
 }

    if (ownCmdHandler(parsed))
        return 0;
    else
        return 1 + piped;
}

int main()
{
    char inputString[MAXCOM], *parsedArgs[MAXLIST];
    char* parsedArgsPiped[MAXLIST];
    int execFlag = 0;
    init_shell();

    while (1) {
        printDir();
        
        if (takeInput(inputString))
            continue;

        execFlag = processString(inputString, parsedArgs, parsedArgsPiped);

        if (execFlag ==1)
            execArgs(parsedArgs);

        if (execFlag == 2)
            execArgsPiped(parsedArgs, parsedArgsPiped);
    }
    return 0;
}
