#include <stdlib.h>
#include <stdio.h>
#include "parser.h"
#include "lex.h"
#include "parser_priv.h"
#include "xcommon.h"
#include "xlog.h"

/* The function pointer is used to write the output the intermediate code to the output */
static putCode_t gPutCodeFnPtr = NULL;
/* Counter to create unique temporary variables in the pseudocode */
static int giTempCount = 0;
/*Array to store the temporary variable name */
static char gachTempSym[20];

/*
   This function is used to register the funtion that is used to output the 
   generated intermediate code.

   @param putLineFnPtr The function pointer that is used to output the code.
 */

void xpar_register(putCode_t putLineFnPtr)
{
    gPutCodeFnPtr = putLineFnPtr;
}

/*
   The function initialtes the parsing of the the .xs code. 
   
   @return will return the parse is a success or a failure.
 */
int xpar_doParse()
{
    int res = X_SUCCESS;
    int finalRes = X_SUCCESS;

    tokendefs_t *statement = NULL;
    statement = xlex_getTokenList();
    while(statement != NULL)
    {
        res = parse_statement(statement);
        if(res == X_FAILURE)
        {
            finalRes = X_FAILURE;
            logLineMsg(xlex_getLineNumber(), "Parse Error");
        }
        statement = xlex_getTokenList();
    }

    return finalRes;
}
/*
   This function will initialtes the the parsing of the function block in the file.

   @param statement The statement where the function block starts
   @return will return the parse is a success or a failure.

 */
int parse_statement(tokendefs_t *statement)
{
    int ret = X_FAILURE;
    switch(statement->tokId){
    case X_TOKID_FUN:
        ret = parse_functionDef(statement);
        if (ret == X_FAILURE){
            ret = X_FAILURE;
        }
        break;
    case X_TOKID_NEWLINE:
    case X_TOKID_SEMICOL:
        ret = X_SUCCESS;
        break;
    default:
        ret = X_FAILURE;
        break;

    }
    return ret;
}

/*
   This function will split the function block in to header and body and initiates the 
   parsing of the header and body.
   
   @param statement The statement where the function block starts
   @return will return the parse is a success or a failure.
 */
int  parse_functionDef(tokendefs_t *statement)
{
    int ret;
    ret = parse_functionHdr(statement);
    if (ret == X_FAILURE)
    {
        return X_FAILURE;
    }
    return parseBody();
}
/*
   This function will parse the function Header. It will check the function header 
   for the grammer 
   functionHeader <= <function name>(<arguments>)

   @return will return the parse is a success or a failure.

 */
int parse_functionHdr(tokendefs_t *statement)
{
    tokendefs_t* current = statement;
    tokendefs_t* next = NULL;

    if(current == NULL)
    {
        return X_FAILURE;
    }
    
    next = isTokenValid(current, X_TOKID_FUN);
    if(next == current || next == NULL)
    {
        return X_FAILURE;
    }
    current = next;
    
    next = isTokenValid(current, X_TOKID_SYM);
    if(next == current || next == NULL)
    {
        return X_FAILURE;
    }
    
    gPutCodeFnPtr("\n");
    gPutCodeFnPtr("fun ");
    gPutCodeFnPtr(current->tokVal);
    gPutCodeFnPtr("\n");
    current = next;

    next = isTokenValid(current, X_TOKID_BRAC_OPEN);
    if(next == current || next == NULL)
    {
        return X_FAILURE;
    }
    current = next;

    next = parse_functionParams(current);
    if(next == NULL)
    {
        return X_FAILURE;
    }
    current = next;
    next = isTokenValid(current, X_TOKID_BRAC_CLOSE);
    if(next == current || next == NULL)
    {
        return X_FAILURE;
    }
    current = next;
    next = isTokenValid(current, X_TOKID_NEWLINE);
    if(next == current)
    {
        return X_FAILURE;
    }
 
    return X_SUCCESS;
}


/*
   This function will parse the function body. It will check whether the body will check 
   for the grammer 
   functionBody<={<statements>}

   @return will return the parse is a success or a failure.

 */
int parseBody()
{
    tokendefs_t *statement;
    int ret;

    statement = xlex_getTokenList();
    if(statement == NULL)
    {
        return X_FAILURE;
    }
    
    if(statement->tokId != X_TOKID_CBRAC_OPEN)
    {
        return X_FAILURE;
    }
    
    statement = xlex_getTokenList();
    if(statement == NULL)
    {
        return X_FAILURE;
    }
    while(statement->tokId != X_TOKID_CBRAC_CLOSE)
    {
        ret = processStatement(statement);
    
        if (ret == X_FAILURE)
        {
            return X_FAILURE;
        }
        statement = xlex_getTokenList();
        if(statement == NULL)
        {
            return X_FAILURE;
        }
    }
    return X_SUCCESS;    
}
/*
   
   This function will parse the statements. The statements currently checked are operation statement 
   and function call statement.
   It will check whether the body will check 
   for the grammer 
   <result> = <OperationStatement>; (; is optional)
   <result> = <FunctionCall>; (; is optional)
   <FunctionCall>; (; is optional)

   @param statement Start of the statement

   @return will return the parse is a success or a failure.

 */
int processStatement(tokendefs_t *statement)
{
    tokendefs_t *current = statement;
    tokendefs_t *next;
    tokendefs_t *head;
    char *val;

    next = isTokenValid(current, X_TOKID_SYM);
    if(next == current || next == NULL)
    {
        return X_FAILURE;
    }
    head = current;
    current = next;

    val = NULL;
    if(current->tokId == X_TOKID_ASSIGN){
        current = current->next;
        if(current == NULL)
        {
            return X_FAILURE;
        }
        val = parseOperation(current);
        if(val == NULL)
        {
            val = parseCall(current);
        } 
        if (val == NULL)
        {
            return X_FAILURE;
        }

        gPutCodeFnPtr("asn ");
        gPutCodeFnPtr(head->tokVal);
        gPutCodeFnPtr(" ");
        gPutCodeFnPtr(val);
        gPutCodeFnPtr("\n");

    }
    else
    {
        val = parseCall(head);
        if (val == NULL)
        {
            return X_FAILURE;
        }
    }

    return X_SUCCESS;
}
/*
   This fuunction will check for the arithmetic and logic operators. Currently it is done
   for only the following grammer
   <operand1>
   <operand1><operator><operand2>
   
   operators <= + - / *

   This function may need to do the infix to post or prefix conversion to handle complex operational statement.

    @param current The token node where the operation statement starts

    @return If there is only one operand, the operand name is returned. If the operation is complex, it
    is split in to mutilple statements and the final result is stored in a temporary statement. The name 
    of the temporary statement is returned.
*/
char*  parseOperation(tokendefs_t *current)
{
    char *oper1;
    char *oper2;
    if(current->tokId != X_TOKID_SYM && current->tokId != X_TOKID_STRING)
    {
        return NULL;
    }
    oper1 = current->tokVal;

    current = current->next;

    /* Only one value after '='*/
    if (current == NULL)
    {
        return oper1;
    }
    /* Only one value after '='*/
    if(current->tokId == X_TOKID_NEWLINE || current->tokId == X_TOKID_SEMICOL)
    {
        return oper1;
    }
    switch(current->tokId)
    {
        case X_TOKID_PLUS:
            gPutCodeFnPtr("plus ");
            break;
        case X_TOKID_MINUS:
            gPutCodeFnPtr("minus ");
            break;
        case X_TOKID_DIV:
            gPutCodeFnPtr("div ");
            break;
        case X_TOKID_MUL:
            gPutCodeFnPtr("mul ");
            break;
        default:
            return NULL;
    }
    current = current->next;
    
    if(current == NULL)
    {
        return NULL;
    }
    if(current->tokId != X_TOKID_SYM && current->tokId != X_TOKID_STRING)
    {
        return NULL;
    }
    oper2 = current->tokVal;


    sprintf(gachTempSym, "@T_%d", giTempCount++);

    gPutCodeFnPtr(gachTempSym);
    gPutCodeFnPtr(" ");
    gPutCodeFnPtr(oper1);
    gPutCodeFnPtr(" ");
    gPutCodeFnPtr(oper2);
    gPutCodeFnPtr("\n");

    current = current-> next;

    if(current == NULL)
    {
        return gachTempSym;
    }
    if(current->tokId == X_TOKID_SEMICOL || current->tokId == X_TOKID_NEWLINE)
    {
        return gachTempSym;
    }
    return NULL;
}
/*

   This fuunction will check for the function call operation. It will check for the following grammer. 
   Before the function call, the arguments need to be pushed to the stack in the reverse order. 
   The code to do that is added here.
   <functionName>(<arguments>);


   @param current The token node where the function call statement starts

   @return After a function call the return value will be pushed to the stack. The 
    statement to pop this value to a temporary variable is added after the function call.
    The name of the temporay variable is returned. 
 */

char*  parseCall(tokendefs_t *current)
{
    char *fnName;
    tokendefs_t *next;
    if(current->tokId != X_TOKID_SYM)
    {
        return NULL;
    }

    next = isTokenValid(current, X_TOKID_SYM);
    if(current == next || next == NULL)
    {
        return NULL;
    }
    fnName = current->tokVal;
    current = next;
    

    next = isTokenValid(current, X_TOKID_BRAC_OPEN);
    if(current == next || next == NULL)
    {
        return NULL;
    }
    current = next;

    while(current->tokId != X_TOKID_BRAC_CLOSE)
    {
        if(!(current->tokId == X_TOKID_SYM || current->tokId == X_TOKID_STRING))
        {
            return NULL;
        }
        /* Code to puch the argument list */
        gPutCodeFnPtr("push ");
        gPutCodeFnPtr(current->tokVal);
        gPutCodeFnPtr("\n");
        
        current = current->next;
        if(current == NULL)
        {
            return NULL;
        }
        if(current->tokId == X_TOKID_COMMA)
        {
            current = current->next;
            if(current == NULL)
            {
                return NULL;
            }
            continue;
        }
      
    }
    /* Code to call the function */
    gPutCodeFnPtr("call ");
    gPutCodeFnPtr(fnName);
    gPutCodeFnPtr("\n");

    /* Code to pop up the return value */
    sprintf(gachTempSym, "@T_%d", giTempCount++);
    gPutCodeFnPtr("pop ");
    gPutCodeFnPtr(gachTempSym);
    gPutCodeFnPtr("\n");

    current = current -> next;
    if(current == NULL)
    {
        return gachTempSym;
    }

    if(current->tokId == X_TOKID_NEWLINE || current->tokId == X_TOKID_SEMICOL)
    {
        return gachTempSym;
    }

    return NULL;
}




/*
   This function checks if the tokId of the current token matches with given tokId.

   @param current The current token id
   @param tokId The token id to be compared with

   @return if the token id matces the next token node is returned, else the current token node is returned 
*/



tokendefs_t* isTokenValid(tokendefs_t * current, int tokId)
{
    if (current->tokId == tokId)
    {
        return current->next;
    }
    return current;

}

/*
   The function parses the arguments for a function. 
   The arguments are comma seperated values of symbols or any basic types like X_TOKID_STRING

   @param current The token node where the argument list begins

   @return On success returns the token node for ')'. Else NULL
 */
tokendefs_t* parse_functionParams(tokendefs_t *current)
{
    while(current->tokId != X_TOKID_BRAC_CLOSE){
        if(current->tokId != X_TOKID_SYM && current->tokId != X_TOKID_STRING)
        {
            return NULL;
        }
        gPutCodeFnPtr("pop ");
        gPutCodeFnPtr(current->tokVal);
        gPutCodeFnPtr("\n");
        current = current->next;
        if(current == NULL)
        {
            return NULL;
        }
        if(current->tokId == X_TOKID_COMMA)
        {
            current = current->next;
            if(current == NULL)
            {
                return NULL;
            }
        }
    }
    return current;
}















