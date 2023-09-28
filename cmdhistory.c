#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <curses.h>
#include "cmdhistory.h"

void initcmdhistory(cmdhistory **ch)
{
    *ch = (cmdhistory *)malloc(sizeof(cmdhistory));
    (*ch)->cmd = NULL;
    (*ch)->next = *ch;
    (*ch)->prev = *ch;
}

void addToHistory(cmdhistory *ch, char *cmd)
{
    if (ch->cmd == NULL)
    {
        char *new_cmd = (char *)malloc((strlen(cmd) + 1) * sizeof(char));
        strcpy(new_cmd, cmd);
        ch->cmd = new_cmd;
    }
    else if (strcmp(ch->cmd, cmd) != 0)
    {
        cmdhistory *new_entry = (cmdhistory *)malloc(sizeof(cmdhistory));
        new_entry->prev = ch;
        ch->next->prev = new_entry;
        new_entry->next = ch->next;
        ch->next = new_entry;
        char *new_cmd = (char *)malloc((strlen(cmd) + 1) * sizeof(char));
        strcpy(new_cmd, cmd);
        new_entry->cmd = new_cmd;
    }
}

void clearhistory(cmdhistory *ch)
{
    cmdhistory *next = ch->next;
    while (ch)
    {
        next = ch->next;
        free(ch->cmd);
        free(ch);
        ch = next;
    }
}

char *movenext(cmdhistory **ch)
{
    *ch = (*ch)->next;
    return (*ch)->cmd;
}
char *moveprev(cmdhistory **ch)
{
    *ch = (*ch)->prev;
    return (*ch)->cmd;
}
void clearline()
{
    printf("\033[K\r");
}

void emptyInputBuffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF) {
    }
    c = getchar();
}
void simulateInput(const char* inputString) {
    size_t inputLength = strlen(inputString);
    fwrite(inputString, 1, inputLength, stdin);
    rewind(stdin);
}

int checkhistory(cmdhistory **ch) {
    char key = '|';
    emptyInputBuffer();
    if ((*ch)->cmd != NULL) {
        // Set to unbuffered mode
        setvbuf(stdin, NULL, _IONBF, BUFSIZ);
        while (key == '|') {
            key = getchar();
            if (key == '|') {
                setvbuf(stdin, NULL, _IOFBF, BUFSIZ);
                printf("%s", movenext(ch));
                simulateInput((*ch)->cmd);
            }else{
                char input[2];
                input[0] = key;
                input[1] = '\0';
                simulateInput(input);
            }
        }
        // Set back to fully buffered mode
        setvbuf(stdin, NULL, _IOFBF, BUFSIZ);
    }
    return 1;
}

