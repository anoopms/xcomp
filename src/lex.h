#ifndef _X_LEX_H
#define _X_LEX_H


enum X_TOKENS_OPERATORS
{
    X_TOKID_ASSIGN,/* = */
    X_TOKID_PLUS,/* + */
    X_TOKID_MINUS,/* - */
    X_TOKID_DIV,/* / */
    X_TOKID_MUL,/* * */

    X_TOKID_MOD,/* % */
    X_TOKID_EQ,/* == */
    X_TOKID_NOT_EQ,/* != */
    X_TOKID_LESS,/* < */
    X_TOKID_LESS_EQ,/* <= */

    X_TOKID_GRT,/* > */
    X_TOKID_GRT_EQ,/* >= */
    X_TOKID_BRAC_OPEN,/* ( */
    X_TOKID_BRAC_CLOSE,/* ) */
    X_TOKID_CBRAC_OPEN,/* { */

    X_TOKID_CBRAC_CLOSE,/* } */
    X_TOKID_FOR,/* for */
    X_TOKID_IF,/* if */
    X_TOKID_ELSEIF,/* elseif*/
    X_TOKID_ELSE,/* else */
   
    X_TOKID_FUN,/* fun */
    X_TOKID_AND,/* and */
    X_TOKID_OR,/* or */
    X_TOKID_NOT,/* not */
    X_TOKID_SYM,/* any symbol */
   
    X_TOKID_NUM,/* any numbers */
    X_TOKID_NEWLINE,/* end of a line */
    X_TOKID_SEMICOL,/* ; */
    X_TOKID_STRING,/* ; */
    X_TOKID_COMMA/* ,*/
};


typedef struct tokendefs{
    int tokId;
    /* This restrict the token value length to max 100. 
    TODO Make it dynamic to take more/variable length values  */
    char tokVal[100];
    int tokLen;
    struct tokendefs *next;
}tokendefs_t;

typedef struct {
    int tokId;
    char symbol[100];
}xtoken_t;

typedef char* (*getLine_t)(void);
typedef int (*getLineNumber_t)();

void xlex_register(getLine_t getLineFnptr, getLineNumber_t getLineNumFnptr);
int xlex_doLex();
tokendefs_t* xlex_getTokenList();
int xlex_getLineNumber();

#endif

