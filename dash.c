#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>
#include <stdbool.h>
#include <sys/wait.h>
#include "cmdhistory.c"

char path[1024] = "/bin";

void throwError()
{
    char error_message[30] = "An error has occured\n";
    write(STDERR_FILENO, error_message, strlen(error_message));
    exit(1);
}

int countChar(const char *cmd, const char target)
{
    int cmdlen = strlen(cmd);
    int count = 0;
    int i;
    for (i = 0; i < cmdlen; i++)
    {
        if (cmd[i] == target)
        {
            count++;
        }
    }
    return count;
}

int strsplitsize(const char *input, const char delimiter)
{
    if (input == NULL || strcmp(input, "") == 0)
    {
        return 0;
    }
    int output = 1;
    int len = strlen(input);
    int i;
    for (i = 1; i < len; i++)
    {
        if (input[i] == delimiter && input[i - 1] != delimiter)
        {
            output++;
        }
    }
    return output;
}

// needs testing
void trim(char *inputstr)
{
    if (inputstr == NULL)
    {
        return;
    }
    int length = strlen(inputstr);
    if (length > 0)
    {
        int front = 0;
        int back = length - 1;
        while (isspace(inputstr[front]))
        {
            front++;
        }
        while (back >= front && isspace(inputstr[back]))
        {
            back--;
        }
        int i;
        for (i = front; i <= back; i++)
        {
            inputstr[i - front] = inputstr[i];
        }
        // Null-terminate the new string.
        inputstr[1 + (back - front)] = '\0';
    }
}

/*
**returns array of tokens, updates num_tokens to number of tokens in the array. last token in the array is NULL
**allocates memory on heap. caller is responsible for freeing the memory after usage.
*/
char **strsplit(const char *input, const char *delimiter, int *num_tokens)
{
    int numtokens = strsplitsize(input, delimiter[0]);
    char **splitarray = (char **)malloc((numtokens + 1) * sizeof(char *));
    char copy[strlen(input) + 1];
    strcpy(copy, input);
    char *token = strtok(copy, delimiter);
    int i = 0;
    while (token != NULL)
    {
        splitarray[i] = strdup(token);
        token = strtok(NULL, delimiter);
        i++;
    }
    splitarray[i] = NULL;
    (*num_tokens) = i;
    return splitarray;
}

void freetokenlistmemory(char **tokenlist, int numtokens)
{
    int i;
    for (i = 0; i < numtokens; i++)
    {
        free(tokenlist[i]);
    }
    free(tokenlist);
}

char *searchfilepath(const char *name)
{
    int numdirs = 0;
    char delimiter[2] = " ";
    if (countChar(path, ':') > 0)
    {
        delimiter[0] = ':';
    }
    char **dirlist = strsplit(path, delimiter, &numdirs);
    int i;
    for (i = 0; i < numdirs; i++)
    {
        char *filepath = (char *)malloc((strlen(dirlist[i]) + 2 + strlen(name)) * sizeof(char));
        strcat(filepath, dirlist[i]);
        strcat(filepath, "/");
        strcat(filepath, name);
        if (access(filepath, F_OK) == 0)
        {

            freetokenlistmemory(dirlist, numdirs);
            return filepath;
        }
        else
        {
            free(filepath);
        }
    }
    freetokenlistmemory(dirlist, numdirs);
    return NULL;
}

void validateredirectioncmd(char *cmd)
{
    int cmdlen = strlen(cmd);
    int count = countChar(cmd, '>');
    if (count > 1 || (count == 1 && ((cmd[cmdlen - 1] == '>' || cmd[0] == '>'))))
    {
        throwError();
    }
    if (count == 1)
    {
        int numargs = 0;
        char **redirectionsplit = strsplit(cmd, ">", &numargs);
        trim(redirectionsplit[1]);
        int spacecount = countChar(redirectionsplit[1], ' ');
        if (spacecount > 0)
        {
            freetokenlistmemory(redirectionsplit, numargs);
            throwError();
        }
    }
}

void truncateargs(int *argc, char **argv, int tailsize)
{
    int back = (*argc);
    int k = tailsize;
    while (back > -1 && k > -1)
    {
        free(argv[back]);
        k--;
        back--;
    }
    argv[back] = NULL;
    (*argc) = back;
}

void outputredirection(const char *filepath)
{
    int fd = open(filepath, O_CREAT | O_WRONLY | O_TRUNC, 0777);
    if (fd == -1)
    {
        throwError();
    }
    dup2(fd, fileno(stdout));
    dup2(fd, fileno(stderr));
    close(fd);
}

char *strconcat(int start, int end, char **argv, const char delimiter)
{
    int outputlen = 0;
    int i;
    for (i = start; (i <= end) && (argv[i] != NULL); i++)
    {
        outputlen += (strlen(argv[i]) + 1);
    }
    int a = (start <= end);
    outputlen -= a;
    char *output = (char *)malloc(outputlen * sizeof(char));
    int k = 0;
    for (i = start; i < end; i++)
    {
        int ilen = strlen(argv[i]);
        int j;
        for (j = 0; j < ilen; j++)
        {
            output[k] = argv[i][j];
            k++;
        }
        output[k] = delimiter;
        k++;
    }
    if (start <= end)
    {
        int ilen = strlen(argv[end]);
        int j;
        for (j = 0; j < ilen; j++)
        {
            output[k] = argv[end][j];
            k++;
        }
    }
    output[k] = '\0';
    return output;
}

int executecmd(char *cmd)
{
    trim(cmd);
    if (cmd == NULL || strlen(cmd) == 0)
    {
        return 0;
    }
    int argc = 0;
    char **argv = strsplit(cmd, " ", &argc);
    if (strcmp(argv[0], "exit") == 0)
    {
        if (argc > 1)
        {
            throwError();
        }
        exit(0);
    }
    else if (strcmp(argv[0], "cd") == 0)
    {
        if (argc == 1 || argc > 2)
        {
            throwError();
        }
        if (chdir(argv[1]) != 0)
        {
            throwError();
        }
    }
    else if (strcmp(argv[0], "path") == 0)
    {
        if (argc == 1)
        {
            strcpy(path, "");
        }
        else
        {
            char pathvalue[4 + strlen(cmd)];
            strcpy(pathvalue, cmd);
            int i;
            for (i = 0; i < 4; i++)
            {
                pathvalue[i] = ' ';
            }
            trim(pathvalue);
            strcpy(path, pathvalue);
        }
    }
    else
    {
        int status;
        int execreturn = 0;
        char *programpath = searchfilepath(argv[0]);
        pid_t pid = fork();
        if (pid < 0)
        {
            throwError();
        }
        else if (pid == 0)
        {
            validateredirectioncmd(cmd);
            if (countChar(cmd, '>'))
            {
                int numargs = 0;
                char **redirectionsplit = strsplit(cmd, ">", &numargs);
                trim(redirectionsplit[1]);
                outputredirection(redirectionsplit[1]);
                truncateargs(&argc, argv, argc);
                argv = strsplit(redirectionsplit[0], " ", &argc);
                freetokenlistmemory(redirectionsplit, numargs);
            }
            execreturn = execv(programpath, argv);
            if (execreturn == -1)
            {
                throwError();
            }
        }
        else
        {
            waitpid(pid, &status, 0);
        }
        free(programpath);
    }
    freetokenlistmemory(argv, argc);
    return 1;
}

void validateparallelcmd(const char *cmd)
{
    if (countChar(cmd, '&') > 0)
    {
        int len = strlen(cmd);
        int i;
        int nonspacenonamp = 0;
        for (i = 0; i < len; i++)
        {
            if (cmd[i] != ' ' && cmd[i] != '&')
            {
                nonspacenonamp++;
            }
        }
        if (nonspacenonamp == 0)
        {
            throwError();
        }
    }
}

int processcmd(const char *cmd)
{
    int cmdexecuted = 0;
    int num_cmds = 0;
    validateparallelcmd(cmd);
    char **cmds = strsplit(cmd, "&", &num_cmds);
    int i;
    for (i = 0; i < num_cmds; i++)
    {
        if (executecmd(cmds[i]) == 1)
        {
            cmdexecuted = 1;
        }
    }
    freetokenlistmemory(cmds, num_cmds);
    return cmdexecuted;
}

int readInput(char **cmd, FILE *fp)
{
    size_t buffersize = 1024;
    ssize_t bytesRead;
    static int memoryallocated = 0;
    if (!memoryallocated)
    {
        (*cmd) = (char *)malloc(buffersize * sizeof(char));
        memoryallocated = 1;
    }
    bytesRead = getline(cmd, &buffersize, fp);
    if (bytesRead > 0 && (*cmd)[bytesRead - 1] == '\n')
    {
        (*cmd)[bytesRead - 1] = '\0';
    }
    return bytesRead;
}

int runBatchMode(char *filename)
{
    FILE *fp = fopen(filename, "r");
    if (fp == NULL)
    {
        throwError();
    }
    char *cmd;
    bool cmdexec = false;
    while (readInput(&cmd, fp) != -1)
    {
        trim(cmd);
        if (processcmd(cmd) == 1)
        {
            cmdexec = true;
        }
    }
    if (!cmdexec)
    {
        throwError();
    }
    free(cmd);
    fclose(fp);
    return 0;
}

int runInteractiveMode()
{
    // cmdhistory *ch = NULL;
    // initcmdhistory(&ch);
    char *cmd;
    printf("dash> ");
    while (1)
    {
        // checkhistory(&ch);
        readInput(&cmd, stdin);
        trim(cmd);
        processcmd(cmd);
        // addToHistory(ch, cmd);
        printf("dash> ");
    }
    free(cmd);
    return 0;
}

int main(int argc, char *argv[])
{
    if (argc == 1)
    {
        runInteractiveMode();
    }
    else if (argc == 2)
    {
        runBatchMode(argv[1]);
    }
    else
    {
        throwError();
    }
    return 0;
}