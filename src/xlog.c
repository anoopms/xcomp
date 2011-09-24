#include <stdio.h>
#include "xlog.h"

void logMsg(char *msg)
{
    printf("%s\n",msg);
}

void logMsg2(char *msg1, char *msg2)
{
    printf("%s%s\n",msg1,msg2);
}

void logLineMsg(int linenu, char *msg)
{
    if (msg == NULL)
    {
        msg = "Unknown reason";
    }
    printf("LINE %d: %s\n",linenu, msg);
}

void logLineMsg2(int linenu, char *msg1, char* msg2)
{
    if (msg1 == NULL)
    {
        msg1 = "Unknown reason";
    }
    if (msg2 == NULL)
    {
        msg2 = "Unknown reason";
    }
    printf("LINE %d: %s %s\n",linenu, msg1, msg2);
}

void printTokenList(tokendefs_t *res)
{
    while(res != NULL)
    {
        printf("[%d(%d) %s ]  ", res->tokId, res->tokLen, res->tokVal);
        res = res->next;
    }
    printf("\n");
}

