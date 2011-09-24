/*
   A sample code for xcompiler
*/

#include <stdio.h>
#include "xcommon.h"
#include "lex.h"
#include "parser.h"
#include "fileoper.h"
#include "xlog.h"

/*
   The function is the starting point for the compiler
   
   @param argc The number of arguments in the command line.
   @param argv The argument string
   @return On successful compilation, returns 0, on failure returns 1 
*/

static FILE *globalFpSrc;
static FILE *globalFpDest;
static char lineArray[500];
static int gLineNumber=0;

int main(int argc, char **argv)
{

    int ret = X_FAILURE;

    if(argc < 2)
    {
        logMsg("Give the Source file");
        return 1;
    }
    printf("Starting with ..%s\n", argv[1]);

    ret = createFiles(argv[1]);
    if(ret == X_FAILURE)
    {
        printf("yooo\n");
        return 1;
    }
    return processFile(argv[1]);
}

int createFiles(char *filename)
{

    globalFpSrc = fopen(filename,"r");
    if(globalFpSrc == NULL)
    {
        logMsg("Given file dosent exist. Check the path");
        return X_FAILURE;
    }

    globalFpDest = fopen("dest.xc","w");
    if(globalFpDest == NULL)
    {
        logMsg("Given file dosent exist. Check the path");
        return X_FAILURE;
    }
    return X_SUCCESS;
}

/*
   This file starts the processing of the file.
   @param The file name
   @return On successful compilation, returns 0, on failure returns 1 
*/
int processFile()
{
    int ret = X_FAILURE;
    xlex_register(&getLine, &getLineNumber);
    xpar_register(&putLine);
    ret = xpar_doParse();
    if (ret == X_FAILURE){
        logMsg("Parse failed");
        return 1;
    }
    return 0;
}
/*
   The function returns a line from the programm. 
   @return A line from the program
*/

char* getLine()
{
    gLineNumber++;
    return fgets(lineArray,500,globalFpSrc);
}
/*
   The function returns the line number of currenly returned line 
*/

int getLineNumber()
{
    return gLineNumber;
}

int putLine(char* line)
{
    printf("%s", line);
    return 0;
}






