#ifndef _X_PARSER_PRIV_H
#define _X_PARSER_PRIV_H
#include "lex.h"

int parse_statement(tokendefs_t *statement);
int parse_functionDef(tokendefs_t *statement);
int parseBody();
int processStatement(tokendefs_t *statement);
int parse_functionHdr(tokendefs_t *statement);
tokendefs_t* isTokenValid(tokendefs_t * current, int tokId);
tokendefs_t* parse_functionParams(tokendefs_t *current);
char* parseOperation(tokendefs_t *current);
char* parseCall(tokendefs_t *current);

#endif

