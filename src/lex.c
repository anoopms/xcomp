#include<string.h>
#include <stdlib.h>
#include "lex.h"
#include "xlog.h"
#include "xcommon.h"
#include "lex_priv.h"

/* Globals */

/* Function pointer to get a line from the file */
static getLine_t gGetLinePtr;
/* Function pointer to get a line number of the line obtained from gGetLinePtr() */
static getLineNumber_t gGetLineNumPtr;

/* The keywords used by the language */
static keyword_t gXKeywords[] = {
                            {"for", X_TOKID_FOR},
                            {"if", X_TOKID_IF},
                            {"elseif", X_TOKID_ELSEIF},
                            {"else", X_TOKID_ELSE},
                            {"fun", X_TOKID_FUN},
                            {"and", X_TOKID_AND},
                            {"or", X_TOKID_OR},
                            {"not", X_TOKID_NOT},
                        };
static int giXKeywordsLen = sizeof(gXKeywords)/sizeof(keyword_t);
/*
   This is the array that holds the statements in a line. 
   This has a limitation of having maximum of 100 statements in a line.
   TODO Need to chaged to dynamic allocation to take any number of statements.
 */
static char* gpchStatements[100];
/* Number of statement in the given line */
static int giStatementCount = 0;
/*The currently processed statement */
static int giStatementIndex = 0;

/* The current token list processed by the parser. It is used to clear the previous token list */
static tokendefs_t *gpstCurrentTokList = NULL;


/*
   This function registers the function for the lex to get the line and line number from the input (Eg File)

   @param getLineFnPtr Function pointer to get a line from the file
   @param getLineNumFnPtr Function pointer to get the corresponding line number from the file
*/

void xlex_register(getLine_t getLineFnPtr, getLineNumber_t getLineNumFnPtr)
{
    gGetLinePtr = getLineFnPtr;
    gGetLineNumPtr = getLineNumFnPtr;
}

/*
   This function returns the line number of the currently processed statement.

   @return Line number of the currently processed statement.
 */
int xlex_getLineNumber()
{
    return gGetLineNumPtr();
}

/*
   This function will read a statement, cuts in to token and returns the token as a list.
   This function is used by parser. 
   
   @return The list of tokens
 */
tokendefs_t* xlex_getTokenList()
{
    char* val = NULL;
    
    freeTokenNodes(gpstCurrentTokList);
    gpstCurrentTokList = NULL;

    val = getStatement();
    if(val!=NULL)
    {
        gpstCurrentTokList = splitToTokens(val);
    }
    if(gpstCurrentTokList == NULL)
    {
        return NULL;
    }
    /* Ignore emplty lines */
    while(gpstCurrentTokList->tokId == X_TOKID_NEWLINE || gpstCurrentTokList->tokId == X_TOKID_SEMICOL)
    {
        freeTokenNodes(gpstCurrentTokList);
        gpstCurrentTokList = NULL;
        val = getStatement();
        if(val!=NULL)
        {
            gpstCurrentTokList = splitToTokens(val);
        }
        if(gpstCurrentTokList == NULL)
        {
            return NULL;
        }

    }
//    printTokenList(gpstCurrentTokList);
    return gpstCurrentTokList;
}

/*
   This funtion will split the statement in to token linked list.
    we are not assuming a NULL or an empty string here, there should be atleast a newline charecter in the string.

    @param statement The input statement

    @return If the token is valid it returns the address of the list of tokens, else returns NULL
*/
tokendefs_t* splitToTokens(char *statement)
{
    int index = 0;
    char ch = statement[index];
    char errStr[2];
    int temp;

    tokendefs_t* root = NULL;
    tokendefs_t* current = NULL;

    while(statement[index] != '\0')
    {
        ch = statement[index];
        if (ch == ' ' || ch == 13)
        {
            index ++;
            continue;
        }
        if ((ch == '_') || 
                (ch >= 'A' && ch <= 'Z')||
                (ch >= 'a' && ch <= 'z'))
        {
            current = addTokenNodeSymbol(statement, index, current);
        }
        else if (ch >= '0' && ch <= '9')
        {
            current = addTokenNodeNum(statement, index, current);
        }
        else if(ch == '\"')
        {
            current = addTokenNodeQuote(statement, index, current);

        }
        else
        {
            current = addTokenNodeOper(statement, index, current);
        }
        if(current == NULL)
        {
            errStr[0]=ch;
            errStr[1]='\0';
            temp = gGetLineNumPtr();
            logLineMsg2(temp, "Invalid token ", errStr);
            freeTokenNodes(root);
            return NULL;
        }
        if(root == NULL)
        {
            root = current;
        }
        index+=current->tokLen;
    }
    return root;
}
/*
   This function will parse the statement to get the String Token.

   @param statement The program statement from which the token is identified
   @param index The index where the token begins
   @param current The last item in the token list

   @return A token node in the list for the correponding token
 */
tokendefs_t* addTokenNodeQuote(char *statement, int index, tokendefs_t* current)
{
    char tokVal[100];
    int i = index;
    tokVal[i - index] = statement[i];
    i++;
    while(statement[i] != '\"')
    {
        if(statement[i] == 10)
        {
            return NULL;
        }
        tokVal[i-index] = statement[i];
        i++;
    }
    tokVal[i-index] = '\"';
    tokVal[i-index+1] = '\0';

    return addTokenNode(current, X_TOKID_STRING, i - index+1, tokVal);    

}
/*
   This function will parse the statement to get the Number Token.

   @param statement The program statement from which the token is identified
   @param index The index where the token begins
   @param current The last item in the token list

   @return A token node in the list for the correponding token
 */
tokendefs_t* addTokenNodeNum(char *statement, int index, tokendefs_t* current)
{
    char tokVal[100];
    int i = index;

    while(statement[i] >= '0' && statement[i] <= '9')
    {
        tokVal[i-index] = statement[i];
        i++;
    }
    tokVal[i-index] = '\0';
    return addTokenNode(current, X_TOKID_NUM, i - index, tokVal);    
}

/*
   This function will parse the statement to get the Symbol Token, like variable name.

   @param statement The program statement from which the token is identified
   @param index The index where the token begins
   @param current The last item in the token list

   @return A token node in the list for the correponding token
 */
tokendefs_t* addTokenNodeSymbol(char *statement, int index, tokendefs_t* current){
    char tokVal[100];
    int i = index;
    tokendefs_t* ret = NULL;
    int j;
    while ((statement[i]== '_') || 
                (statement[i] >= 'A' && statement[i] <= 'Z')||
                (statement[i] >= 'a' && statement[i] <= 'z') ||
                (statement[i] >= '0' && statement[i] <= '9'))
    {
        tokVal[i-index] = statement[i];
        i++;
    }
    tokVal[i-index] = '\0';
    
    for (j = 0; j < giXKeywordsLen; j++)
    {
        if(strcmp(gXKeywords[j].value, tokVal) == 0)
        {
            ret = addTokenNode(current, gXKeywords[j].id, i - index, tokVal);
            break;
        }
    }
    if (ret == NULL)
    {
        ret = addTokenNode(current, X_TOKID_SYM, i - index, tokVal);
    }
    return ret;
}


/*
   This function will parse the statement to get the Operator Token.

   @param statement The program statement from which the token is identified
   @param index The index where the token begins
   @param current The last item in the token list

   @return A token node in the list for the correponding token
 */
tokendefs_t* addTokenNodeOper(char *statement, int index, tokendefs_t* current)
{
    switch (statement[index]){
        case '+':
            current = addTokenNode(current, X_TOKID_PLUS, 1, "+");
            break;
        case ',':
            current = addTokenNode(current, X_TOKID_COMMA, 1, "+");
            break;
        case '-':
            current = addTokenNode(current, X_TOKID_MINUS, 1, "-");
            break;
        case 10:
            current = addTokenNode(current, X_TOKID_NEWLINE, 1, "newLine");
            break;
        case '(':
            current = addTokenNode(current, X_TOKID_BRAC_OPEN, 1, "(");
            break;
        case ')':
            current = addTokenNode(current, X_TOKID_BRAC_CLOSE, 1, ")");
            break;
        case '{':
            current = addTokenNode(current, X_TOKID_CBRAC_OPEN, 1, "{");
            break;
        case '}':
            current = addTokenNode(current, X_TOKID_CBRAC_CLOSE, 1, "}");
            break;
        case ';':
            current = addTokenNode(current, X_TOKID_SEMICOL, 1, ";");
            break;
        case '=':
            current = addTokenNodeEq(statement, index, current);
            break;
        default:
            return NULL;
    }
    return current;
}
/*
   This function is used to free the memory allocated for the token nodes

   @param The root node of the token
 */
void freeTokenNodes(tokendefs_t *root)
{
    tokendefs_t* current = root;
    while(current != NULL)
    {
        tokendefs_t* temp = current->next;
        free(current);
        current = temp;
    }
}

/*
   This function will parse the statement to get the tokens '=' or '=='.

   @param statement The program statement from which the token is identified
   @param index The index where the token begins
   @param current The last item in the token list

   @return A token node in the list for the correponding token

 */
tokendefs_t* addTokenNodeEq(char *statement, int index, tokendefs_t* current)
{
    tokendefs_t* node = NULL;
    index++;
    switch (statement[index])
    {
        case '=':
            node = addTokenNode(current, X_TOKID_EQ, 2, "==");
            break;
        default:
            node = addTokenNode(current, X_TOKID_ASSIGN, 1,"=");
    }
    return node;
}

/*
   This function will add the a new token node to the existing token list.

   @param current The last token in the token list
   @param tokId The token ID
   @param tokLen The string length of the token
   @param tokVal The token value

   @return New token node.
 */
tokendefs_t* addTokenNode(tokendefs_t* current, int tokId, int tokLen, char* tokVal){
    
    tokendefs_t* node = (tokendefs_t*)malloc(sizeof(tokendefs_t));
    node->next = NULL;
    if (tokVal != NULL)
    {
        strcpy(node->tokVal, tokVal);
    }
    node->tokLen = tokLen;
    node->tokId = tokId;
    if(current != NULL)
    {
        current->next = node;
    }
    return node;
}

/*
   This function gets each statement from the global array gpchStatements. This internally uses 
   funciton splitLine() to fill the global array gpchStatements
*/
char* getStatement()
{
    char* pchLine;
    
    if(giStatementIndex >= giStatementCount)
    {
        freeOldStatements();
        pchLine = gGetLinePtr();
        if(pchLine != NULL)
        {
            splitLine(pchLine);
        }
        else
        {
            return NULL;
        }
    }
    return gpchStatements[giStatementIndex++];

}

/*
   Frees the memory allocated in the function splitLine
*/
void freeOldStatements()
{
    int i = 0;
    for(i = 0; i < giStatementCount; i++)
    {
        if(gpchStatements[i] != NULL)
        {
            free(gpchStatements[i]);
        }
    }
}

/*
   This function takes a line from the file and split it in to the statements seperated by ';'.
   This will fill the global array gpchStatement and sets the variables giStatementCount and giStatementIndex.
   giStatementCount is the number of statement obtained from the file. 
   giStatementIndex is the current index processed by the lex, initialy set to 0.

   @param pchLine The input line from the file
   @return the number of statement obtained from the file.
*/
int splitLine(char *pchLine)
{
    giStatementCount = 0;
    giStatementIndex = 0;
    char *val = pchLine;
    char *ret = NULL;
    int len = 0;
    char *temp = NULL;

    while(*val != '\0')
    {
        ret = strchr(val, ';');
        /* If there is no ';' in the string, copy entire string  */
        if(ret == NULL)
        {
            len = strlen(val);
            temp = (char*)malloc(len+1);
            strcpy(temp, val);
            gpchStatements[giStatementCount] = temp;
            giStatementCount++;
            break;
        }
        /* If there is a ';' in the string, copy till the string  */
        len = ret-val+1;
        temp = (char*)malloc(len+1);
        memcpy(temp, val, len);
        temp[len] = '\0';
        gpchStatements[giStatementCount] = temp;
        giStatementCount++;
        val = ret+1;
    }
    return giStatementCount;
}

