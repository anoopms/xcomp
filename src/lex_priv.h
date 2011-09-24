#ifndef _LEX_PRIV_H
#define _LEX_PRIV_H
#include "lex.h"
typedef struct keyword{
    char *value;
    int id;
}keyword_t;


void freeOldStatements();
int splitLine(char *pchLine);
char* getStatement();
tokendefs_t* addTokenNode(tokendefs_t* current, int tokId, int tokLen, char *tokVal);
void freeTokenNodes(tokendefs_t *root);
tokendefs_t* addTokenNodeEq(char *statement, int index, tokendefs_t* current);
tokendefs_t* addTokenNodeNum(char *statement, int index, tokendefs_t* current);
tokendefs_t* addTokenNodeSymbol(char *statement, int index, tokendefs_t* current);
tokendefs_t* addTokenNodeOper(char *statement, int index, tokendefs_t* current);
tokendefs_t* addTokenNodeQuote(char *statement, int index, tokendefs_t* current);
tokendefs_t* splitToTokens(char *statement);


#endif

