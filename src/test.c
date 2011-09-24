#include <stdio.h>
#include "lex.h"
#include "lex_priv.h"
int getTestLineNumber(){
    return 0;
}

char * getTestLine(){
    return "Hello";
}

int main()
{
    test1();
}

int test1()
{
    char* statement = "\"funthis\"";
    xlex_register(&getTestLine, &getTestLineNumber);
    tokendefs_t *res = NULL;
    res = splitToTokens(statement);
    printTokenList(res);
}


