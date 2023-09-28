#ifndef CMDHISTORY_H
#define CMDHISTORY_H
typedef struct cmdhistory
{
    char *cmd;
    struct cmdhistory *next;
    struct cmdhistory *prev;
} cmdhistory;
void initcmdhistory(cmdhistory **ch);
void addToHistory(cmdhistory *ch, char *cmd);
void clearhistory(cmdhistory *ch);
char *movenext(cmdhistory **ch);
char *moveprev(cmdhistory **ch);
int checkhistory(cmdhistory **ch);
#endif

