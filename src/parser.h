#ifndef _X_PARSER_H
#define _X_PARSER_H

typedef int (*putCode_t)(char *);
void xpar_register(putCode_t putLineFnPtr);
int xpar_doParse();

#endif

