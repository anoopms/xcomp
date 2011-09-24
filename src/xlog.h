#ifndef _X_LOG_H
#define _X_LOG_H
#include "lex.h"
void logMsg(char *msg);
void logMsg2(char *msg1, char *msg2);

void logLineMsg(int linenu, char *msg);
void logLineMsg2(int linenu, char *msg1, char *msg2);
void printTokenList(tokendefs_t *res);

#endif


