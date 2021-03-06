#include "stdio.h"
#include "stdbool.h"
#include "string.h"
#include "stdlib.h"
#include "pthread.h"
#include <ctype.h>

#define BUFFER_SIZE 1000
#define MAX_LINE_LENGTH 512
#define TEMP_FILE_NAME "temp.txt"

struct thread_args
{
    char *singleCommand;
    char *fileName;
};

pthread_mutex_t mutexLock;

char optionalFileName[50] = {'\0'}; //Initting it with empty so that i can check

//This function trims whitespaces from right and left. i.e. "   java  " --> "java"
char *trim_space(char *str)
{
    char *end;
    /* skip leading whitespace */
    while (isspace(*str))
    {
        str = str + 1;
    }
    /* remove trailing whitespace */
    end = str + strlen(str) - 1;
    while (end > str && isspace(*end))
    {
        end = end - 1;
    }
    /* write null character */
    *(end + 1) = '\0';
    return str;
}
int writingOperation(char *x, int y, int z, FILE *f)
{

    pthread_mutex_lock(&mutexLock);
    //printf("LOCKED BY MUTEX -TM-\n");
    int writtenLenght = fwrite(x, y, z, f);
    pthread_mutex_unlock(&mutexLock);
    //printf("UNLOCKED BY MUTEX -TM-\n");

    return writtenLenght;
}

int splitCommands(const char *input, const char delim, char *Commands[]);

FILE *openFileForReadPlus(char *fileName)
{

    if (fileName[strlen(fileName) - 1] == '\n')
        fileName[strlen(fileName) - 1] = '\0';
    FILE *fp = fopen(fileName, "r+");
    if (fp == NULL)
    {
        printf("Unable to open file named : -%s-\n", fileName);
        perror("fopen");
        return NULL;
    }
    else
    {
        printf("File named %s is opened\n", fileName);
        rewind(fp);
        return fp;
    }
    return NULL;
}

bool writeEntireFile(FILE *in, FILE *out)
{
    rewind(in);
    rewind(out);
    char buffer[BUFFER_SIZE];
    //Write the new file into Input File
    while (fgets(buffer, BUFFER_SIZE, out))
        writingOperation(buffer, 1, strlen(buffer), in);
    return true;
}

int lineCount(FILE *inFile)
{
    int lineCount = 0;
    char buffer[9999];
    while (fgets(buffer, BUFFER_SIZE, inFile))
    {
        lineCount++;
    }
    printf("File has: '%d' lines.\n", lineCount);
    return lineCount;
}

//This search algorithm finds and conuts the occurences of a keyword
//and prints the lines with those keywords.

//If a line has 2 occurences of a keyword, the line is printed twice.
void search(const char *keyword, bool countFlag, FILE *inFile)
{
    char str[BUFFER_SIZE];
    char *pos;
    int index, count = 0;

    if (inFile == NULL)
    {
        printf("File is not open!\n");
        return;
    }
    else if (keyword == NULL)
    {
        printf("What am i searching for. The search word is NULL, sorry man something is wrong.\n");
        return;
    }
    else
    {
        while ((fgets(str, BUFFER_SIZE, inFile)) != NULL)
        {
            index = 0;
            while ((pos = strstr(str + index, keyword)) != NULL)
            {
                // Index of word in str is
                // Memory address of pos - memory
                // address of str.
                index = (pos - str) + 1;
                count++;
                //Print the line that contains searched word.
                printf("\t\t->%s\n", str);
            }
        }

        if (countFlag)
            printf("\nOcurrence of %s is : %d\n", keyword, count);
        fclose(inFile);
        return;
    }
}
//TODO:FIX ALL fwrites with this.

void replace(char *keyword, char *sourceKeyword, bool countFlag, FILE *inFile, char *optionalOutputFile)
{
    printf("REPLACING %s with %s \n", sourceKeyword, keyword);
    char buffer[BUFFER_SIZE];

    FILE *Output = fopen(TEMP_FILE_NAME, "w+");

    if (inFile == NULL)
    {
        printf("File is not open!\n");
        return;
    }
    else
    {
        while (NULL != fgets(buffer, BUFFER_SIZE, inFile))
        {
            // For each incidence of "is"
            char *Stop = NULL;    // Where to stop copying (at 'is')
            char *Start = buffer; // Start at the beginning of the line, and after each match

            while (1)
            {
                // Find next match
                Stop = strstr(Start, sourceKeyword);

                if (Stop == NULL)
                {
                    // Print the remaining text in the line
                    writingOperation(Start, 1, strlen(Start), Output);

                    break;
                }

                printf("Match starts at: %s\n", Stop);

                // We have found a match!  Copy everything from [Start, Stop)
                writingOperation(Start, 1, Stop - Start, Output);

                // Write our replacement text
                writingOperation(keyword, 1, strlen(keyword), Output);

                // Next time, we want to start searching after our 'match'
                Start = Stop + strlen(sourceKeyword);

                printf("Search resumes at: %s\n", Start);
            }
        }
        // If desired, rename the Output file to the Input file

        if (optionalOutputFile != NULL && optionalOutputFile[0] != '\0')
        {
            FILE *f = fopen(optionalOutputFile, "r+");
            if (f == NULL)
            {
                writeEntireFile(inFile, Output);
            }
            else
            {
                writeEntireFile(f, Output);
                fclose(f);
            }
        }
        else
            writeEntireFile(inFile, Output);
    }
    // Close our files
    fclose(inFile);
    //Remove the temp file
    remove(TEMP_FILE_NAME);
    fclose(Output);
}

int insert(char keywordToInsert[], bool countFlag, bool afterFlag, char *keywordToInsertAfter, FILE *inFile, char *optionalOutputFile)
{ //The current program inserts after the word.
    //pthread_mutex_lock(&mutexLock);
    char insertKeyword[BUFFER_SIZE];

    printf("Inserting -%s- next to -%s-\n", keywordToInsert, keywordToInsertAfter);

    char buffer[BUFFER_SIZE];
    FILE *Output = fopen(TEMP_FILE_NAME, "w+");

    if (keywordToInsert == NULL || keywordToInsertAfter == NULL || inFile == NULL || Output == NULL)
    {
        printf("insert : Some value is not correct. Returning to main loop\n");
        return -78;
    }
    strcat(insertKeyword, keywordToInsert);

    while (fgets(buffer, BUFFER_SIZE, inFile))
    {
        char *Stop = NULL;
        char *Start = buffer; // Start at the beginning of the line, and after each match

        while (1)
        {
            // Find next match and move the pointer to the end of our target keyword
            Stop = strstr(Start, keywordToInsertAfter);
            if (Stop == NULL)
            {
                // Print the remaining text in the line
                writingOperation(Start, 1, strlen(Start), Output);
                break;
            }


            // We have found a match!  Copy everything from [Start, Stop)
            if (afterFlag)
            {
                writingOperation(Start, 1, Stop - Start + strlen(keywordToInsertAfter), Output);
                writingOperation(keywordToInsert, 1, strlen(keywordToInsert), Output);
            }
            else
            {
                writingOperation(Start, 1, Stop - Start, Output);
                writingOperation(keywordToInsert, 1, strlen(keywordToInsert), Output);
                writingOperation(keywordToInsertAfter, 1, strlen(keywordToInsertAfter), Output);
            }
            // Next time, we want to start searching after our 'match'

            Start = Stop + strlen(keywordToInsertAfter);
        }
    }

    if (optionalOutputFile != NULL)
    {
        FILE *f = fopen(optionalOutputFile, "r+");
        writeEntireFile(f, Output);
        fclose(f);
    }
    else
        writeEntireFile(inFile, Output);

    fclose(inFile);
    remove(TEMP_FILE_NAME); //DONT REMOVE IF ANOTHER THREAD IS USING THIS
    fclose(Output);
    //pthread_mutex_unlock(&mutexLock);
    return 1;
}

int showHeadLines(FILE *inFile, int lineAmountToShow)
{
    int atLine = 1;
    char buffer[9999];

    if (inFile == NULL || lineAmountToShow == NULL || lineAmountToShow < 0)
    {
        printf("Some value is not correct. Returning to main loop\n");
        return -76;
    }

    while (fgets(buffer, BUFFER_SIZE, inFile) && atLine <= lineAmountToShow)
    {
        printf("Line %d: %s\n", atLine, buffer);
        atLine++;
    }

    return -213;
}

int showMidLines(FILE *inFile, int startLine, int endLine)
{

    int linesShown = 0;
    int atLine = 0;
    char buffer[9999];

    if (inFile == NULL || startLine == NULL || endLine == NULL || startLine < 0 || endLine < 0)
    {
        printf("midLines : Some value is not correct. Returning to main loop\n");
        return -74;
    }

    //Extreme case handling. If starting line is smaller than end. Swap the values.
    if (endLine < startLine)
    {
        int temp = endLine;
        endLine = startLine;
        startLine = temp;
    }

    int lineCountOfFile = lineCount(inFile);
    if (lineCountOfFile < endLine - startLine + 1)
    {
        printf("WARNING : given input is larger than the line count. Showing entire file\n");
        endLine = lineCountOfFile;
        startLine = 0;
    }

    rewind(inFile);

    while (fgets(buffer, BUFFER_SIZE, inFile))
    {
        atLine++;
        if (linesShown > endLine - startLine + 1) //Enough lines have been shown. Break out of the loop
            return 659;
        else if (atLine >= startLine && atLine <= endLine)
        {
            printf("Line %d: %s\n", atLine, buffer);
            linesShown++;
        }
        else
        {
            //Empty for now, maybe has a use, I dunno.
        }
    }

    return 211;
}

int showTailLines(FILE *inFile, int lineAmountToShow)
{

    if (inFile == NULL || lineAmountToShow == NULL || lineAmountToShow < 0)
    {
        printf("showTailLines : Some value is not correct. Returning to main loop\n");
        return -72;
    }

    int linesShown = 0;
    int atLine = 0;
    char buffer[9999];
    int lineCountOfFile = lineCount(inFile);
    rewind(inFile);
    printf("\nSHOWING LAST %d LINES\n", lineAmountToShow);
    while (fgets(buffer, BUFFER_SIZE, inFile))
    {
        atLine++;
        if (linesShown >= lineAmountToShow)
        {
            //Code should never reach here.
            printf("ERROR : showTailLines method reached beyond the file but tried showing lines. Something is wrong.");
            return -156;
        }
        else if (atLine >= lineCountOfFile - lineAmountToShow + 1)
        {
            printf("Line %d: %s\n", atLine, buffer);
            linesShown++;
        }
        else
        {
            //Empty for now, maybe has a use, I dunno.
        }
    }
}

int split(int charCount, FILE *inFile, char *optionalOutputFile)
{

    FILE *out = fopen(TEMP_FILE_NAME, "w+");
    char buffer[BUFFER_SIZE];
    char *lineEndingChar = "\n";

    if (out == NULL || charCount == NULL || charCount < 1 || inFile == NULL)
    {
        printf("Split method recieved something null, returning to main loop\n");
        return -89;
    }

    charCount++; //So that termination char is added too

    while (fgets(buffer, charCount, inFile))
    {
        if (buffer[strlen(buffer) - 1] != '\n') //This is necessary. If the last char is a new line, it adds a second unnecessary line to the file without this check
            strcat(buffer, lineEndingChar);
        writingOperation(buffer, 1, charCount, out);
    }

    if (optionalOutputFile != NULL)
    {
        FILE *f = fopen(optionalOutputFile, "r+");
        writeEntireFile(f, out);
    }
    else
        writeEntireFile(inFile, out);

    return 123;
}

bool checkInputLengthValidity(char line[])
{
    for (int i = 0; i < (MAX_LINE_LENGTH + 1); i++)
    {
        if (line[i] == EOF || line[i] == '\0')
            return true;
    }
    return false;
}

char **getKeywordFromQuotationedLine(char line[], char *words[])
{
    int wordCount = 0;
    int Start = -1;
    int Stop = -1;
    char *ptr = &line;
    for (int i = 0; i < strlen(line); i++)
    {
        if (line[i] == '\"')
        {
            ptr = &line[i + 1];
            Start = i;
            i++;
            for (; i < strlen(line); i++)
            {
                if (line[i] == '\"')
                {
                    Stop = i;

                    words[wordCount] = malloc(sizeof(char) * (Stop - Start));
                    strncpy(words[wordCount], ptr, Stop - Start - 1);
                    wordCount++;
                    //printf("Copied %s\n", words[wordCount-1]);
                    break;
                }
            }
        }
    }
    return words;
}

void *multiWordParseSingleCommand(void *ptr)
{

    struct thread_args *args = (struct thread_args *)ptr;

    if (args == NULL && args->fileName && NULL && args->singleCommand == NULL && strlen(args->singleCommand) < 4)
    {
        printf("Wrong or Missing Command skipping this one\n");
        return;
    }

    char token[512];
    strcpy(token, args->singleCommand);

    char *fileName = args->fileName;

    //NEW PART
    char *keywords[512] = {NULL};
    char **keywordsptr = getKeywordFromQuotationedLine(args->singleCommand, keywords);
    //NEW PART



    //Method gets a single char array containing one command as token, and the fileName it should be writing into
    char *mainCommand = strtok(token, " ");
    if (mainCommand == NULL)
    {
        printf("Main Command is null. Dont know what to do. Ignoring this command subset.\n");
        return;
    }

    if (strcmp(mainCommand, "search") == 0)
    { ///SEARCH
        //The current code only checks for first 3 keywords. If there are, say 15; no error is shown.
        char *keyword = strtok(NULL, " ");
        bool countFlag = false;

        if (strstr(args->singleCommand, "-c") != NULL)
        {
            countFlag = true;
        }

        if (keyword == NULL)
            printf("Error keyword is null in search command\n, its token is %s\n", token);

        char newK[512] = {'\0'};

        if (strlen(keywordsptr) > 0 && keywordsptr[0] == NULL)
        {
            printf("NULL WORD");
        }
        else
        {
            if(keywordsptr[0] != NULL){
                strcpy(newK, keywordsptr[0]);
                search(newK, countFlag, openFileForReadPlus(fileName));
            }
        }
    }
    else if (strcmp(mainCommand, "replace") == 0)
    { ///REPLACE
        printf("REPLACE COMMAND RECIEVED \n");
        char *targetKeyword = strtok(NULL, " ");
        char *sourceKeyword = strtok(NULL, " ");
        bool countFlag = false;

        if (strstr(args->singleCommand, "-c") != NULL)
        {
            countFlag = true;

        }

        if (targetKeyword == NULL || sourceKeyword == NULL)
            perror("Error. A necessary command is missing in replace command");
        char *name = NULL;

        if (strstr(args->singleCommand, ">") != NULL)
        {
            printf("Alternate output file recognized. Results will be written to that.\n");

            name = strstr(args->singleCommand, ">");
            name = name + 2;
            char *h = name;
            char temp[50] = {'\0'};
            char *h2 = strstr(name, " ");
            strncpy(temp, name, h2 - h);

            strcpy(optionalFileName, temp);

        }

        if (strlen(keywordsptr) != 2)
        {
            printf("Wrong number of keywords found exiting command\n");
        }
        else
        {
            char newK[512];
            strcpy(newK, keywordsptr[0]);
            char newK2[512];
            strcpy(newK2, keywordsptr[1]);
            replace(newK, newK2, countFlag, openFileForReadPlus(fileName), name);
        }
    }
    else if (strcmp(mainCommand, "insert") == 0)
    {

        char *insertedKeyword = strtok(NULL, " ");
        bool countFlag = false;
        bool afterFlag = true;
        char *keywordToInsertAfter = strtok(NULL, " ");

        if (strstr(args->singleCommand, "-c") != NULL)
        {
            countFlag = true;

        }

        if (strstr(args->singleCommand, "-a") != NULL)
            afterFlag = true;

        else if (strstr(args->singleCommand, "-b") != NULL)
        {

            afterFlag = false;
        }
        if (keywordToInsertAfter == NULL || insertedKeyword == NULL)
            perror("Error. A necessary command is missing in replace command");
        char *name = NULL;
        if (strstr(args->singleCommand, ">") != NULL)
        {
            printf("Alternate output file recognized. Results will be written.\n");
            name = (strstr(args->singleCommand, ">"));
            name = name + 2;
            char *h = name;
            char temp[50] = {'\0'};
            char *h2 = strstr(name, " ");
            strncpy(temp, name, h2 - h);
        }
        if (strlen(keywordsptr) != 2)
        {
            printf("Wrong number of keywords found exiting command\n");
        }
        else
        {
            char newK[512];
            strcpy(newK, keywordsptr[0]);
            char newK2[512];
            strcpy(newK2, keywordsptr[1]);
            printf("THE FILE BEING TREATED AS inFile : %s\n", args->fileName);
            insert(newK, countFlag, afterFlag, newK2, openFileForReadPlus(args->fileName), name);
        }
    }
    else if (strcmp(mainCommand, "lineCount") == 0)
    {
        lineCount(openFileForReadPlus(args->fileName));
    }
    else if (strcmp(mainCommand, "split") == 0)
    {
        char *p = strtok(NULL, " ");
        char temp[50];
        strcpy(temp, p);
        char *tempP = trim_space(temp);
        int charCount = atoi(tempP);

        char *name = NULL;

        if (strstr(args->singleCommand, ">") != NULL)
        {
            printf("Alternate output file recognized. Results will be written.\n");

            name = strstr(args->singleCommand, ">");
            name = name + 2;
            char *h = name;
            char temp[50] = {'\0'};
            char *h2 = strstr(name, " ");
            strncpy(temp, name, h2 - h);

            strcpy(optionalFileName, temp);

        }

        split(charCount, openFileForReadPlus(args->fileName), name);
    }
    else if (strcmp(mainCommand, "head") == 0)
    {

        char *p = strtok(NULL, " ");
        char temp[50];
        strcpy(temp, p);
        char *tempP = trim_space(temp);
        int lineCount = atoi(tempP);

        showHeadLines(openFileForReadPlus(args->fileName), lineCount);
    }
    else if (strcmp(mainCommand, "tail") == 0)
    {

        char *p = strtok(NULL, " ");
        char temp[50];
        strcpy(temp, p);
        char *tempP = trim_space(temp);
        int lineCount = atoi(tempP);

        showTailLines(openFileForReadPlus(args->fileName), lineCount);
    }
    else if (strcmp(mainCommand, "mid") == 0)
    {

        char *p = strtok(NULL, " ");
        char temp[50];
        strcpy(temp, p);
        char *tempP = trim_space(temp);
        int startLine = atoi(tempP);

        p = strtok(NULL, " ");
        strcpy(temp, p);
        tempP = trim_space(temp);
        int endLine = atoi(tempP);

        showMidLines(openFileForReadPlus(args->fileName), startLine, endLine);
    }
    else if (strcmp(mainCommand, "exit") == 0)
    {
        printf("Recieved exit command. Execution will be stopped after this iteration.\n");
    }
    else
    {
        printf("Unknown main command entered. Please try again man");
        return NULL;
    }
    //pthread_exit(NULL);
}

void *parseSingleCommand(void *ptr)
{
    struct thread_args *args = (struct thread_args *)ptr;

    char token[512];
    strcpy(token, args->singleCommand);

    char *fileName = args->fileName;

    //Method gets a single char array containing one command as token, and the fileName it should be writing into
    char *mainCommand = strtok(token, " ");
    if (mainCommand == NULL)
        printf("Main Command is null\n");

    if (strcmp(mainCommand, "search") == 0)
    { ///SEARCH
        //The current code only checks for first 3 keywords. If there are, say 15; no error is shown.
        char *keyword = strtok(NULL, " ");
        bool countFlag = false;

        if (strstr(args->singleCommand, "-c") != NULL)
        {
            countFlag = true;

        }

        if (keyword == NULL)
            printf("Error keyword is null in search command\n, its token is %s\n", token);

        search(keyword, countFlag, openFileForReadPlus(fileName));

    }
    else if (strcmp(mainCommand, "replace") == 0)
    { ///REPLACE

        char *targetKeyword = strtok(NULL, " ");
        char *sourceKeyword = strtok(NULL, " ");
        bool countFlag = false;

        if (strstr(args->singleCommand, "-c") != NULL)
        {
            countFlag = true;

        }

        if (targetKeyword == NULL || sourceKeyword == NULL)
            perror("Error. A necessary command is missing in replace command");
        char *name = NULL;

        if (strstr(args->singleCommand, ">") != NULL)
        {
            printf("Alternate output file recognized. Results will be written.\n");

            name = strstr(args->singleCommand, ">");
            name = name + 2;
            char *h = name;
            char temp[50] = {'\0'};
            char *h2 = strstr(name, " ");
            strncpy(temp, name, h2 - h);
            printf("WRITING NOW\n");
            strcpy(optionalFileName, temp);
            printf("WRITING END : -%s-\n", optionalFileName);
        }

        replace(targetKeyword, sourceKeyword, countFlag, openFileForReadPlus(fileName), name);
    }
    else if (strcmp(mainCommand, "insert") == 0)
    {

        char *insertedKeyword = strtok(NULL, " ");
        bool countFlag = false;
        bool afterFlag = true;
        char *keywordToInsertAfter = strtok(NULL, " ");

        if (strstr(args->singleCommand, "-c") != NULL)
        {
            countFlag = true;
            printf("Flag is now true\n");
        }

        if (strstr(args->singleCommand, "-a") != NULL)
            afterFlag = true;
        else if (strstr(args->singleCommand, "-b") != NULL)
        {
            printf("OH WOULD YOU LOOK AT THAT SOMEONE IS TRYING TO INSERT BEFORE THE WORD\n");
            afterFlag = false;
        }

        if (keywordToInsertAfter == NULL || insertedKeyword == NULL)
            perror("Error. A necessary command is missing in replace command");
        char *name = NULL;
        if (strstr(args->singleCommand, ">") != NULL)
        {
            printf("Alternate output file recognized. Results will be written.\n");

            name = strstr(args->singleCommand, ">");
            name = name + 2;
            char *h = name;
            char temp[50] = {'\0'};
            char *h2 = strstr(name, " ");
            strncpy(temp, name, h2 - h);
            printf("WRITING NOW\n");
            strcpy(optionalFileName, temp);
            printf("WRITING END : -%s-\n", optionalFileName);
        }

        insert(insertedKeyword, countFlag, afterFlag, keywordToInsertAfter, openFileForReadPlus(args->fileName), name);
    }
    else if (strcmp(mainCommand, "lineCount") == 0)
    {
        lineCount(openFileForReadPlus(args->fileName));
    }
    else if (strcmp(mainCommand, "split") == 0)
    {
        char *p = strtok(NULL, " ");
        char temp[50];
        strcpy(temp, p);
        char *tempP = trim_space(temp);
        int charCount = atoi(tempP);

        char *name = NULL;

        if (strstr(args->singleCommand, ">") != NULL)
        {
            printf("Alternate output file recognized. Results will be written.\n");

            name = strstr(args->singleCommand, ">");
            name = name + 2;
            char *h = name;
            char temp[50] = {'\0'};
            char *h2 = strstr(name, " ");
            strncpy(temp, name, h2 - h);
            printf("WRITING NOW\n");
            strcpy(optionalFileName, temp);
            printf("WRITING END : -%s-\n", optionalFileName);
        }

        split(charCount, openFileForReadPlus(args->fileName), name);
    }
    else if (strcmp(mainCommand, "head") == 0)
    {

        char *p = strtok(NULL, " ");
        char temp[50];
        strcpy(temp, p);
        char *tempP = trim_space(temp);
        int lineCount = atoi(tempP);

        showHeadLines(openFileForReadPlus(args->fileName), lineCount);
    }
    else if (strcmp(mainCommand, "tail") == 0)
    {

        char *p = strtok(NULL, " ");
        char temp[50];
        strcpy(temp, p);
        char *tempP = trim_space(temp);
        int lineCount = atoi(tempP);

        showTailLines(openFileForReadPlus(args->fileName), lineCount);
    }
    else if (strcmp(mainCommand, "mid") == 0)
    {

        char *p = strtok(NULL, " ");
        char temp[50];
        strcpy(temp, p);
        char *tempP = trim_space(temp);
        int startLine = atoi(tempP);
        p = strtok(NULL, " ");
        strcpy(temp, p);
        tempP = trim_space(temp);
        int endLine = atoi(tempP);

        showMidLines(openFileForReadPlus(args->fileName), startLine, endLine);
    }
    else
    {
        printf("Unknown main command entered. Please try again man");
        return NULL;
    }
    //pthread_exit(NULL);
}

char *getFileNameFromInputLine(char *l)
{
    char line[512];
    strcpy(line, l);
    char fileName[512];
    char *p = strrchr(line, ' ');

    if (p == NULL && strlen(l) != 0)
    {
        return l; //If there is no space but there are chars, then there is only one word
    }
    else if (p && *(p + 1))
    {
        return p + 1;
    }
    else
    {
        printf("Unknown error. File name is not acquired\n");
        return NULL;
    }
}

//search hakan hehe.txt : search ahmet hehe.txt ; search hakan hehe.txt

int inputLoop(void)
{
    printf("INPUT LOOP STARTS\n");
    char line[MAX_LINE_LENGTH];
    bool whileFlag = true;
    char *commandsSequential[512] = {NULL};
    char *commandsThreaded[512] = {NULL};
    char *linePtr;
    char fileName[512] = {'\0'};
    do
    {
        printf("\n");
        printf("command > ");
        if (fgets(line, 515, stdin))
        {
            linePtr = line;
            linePtr[strlen(linePtr) - 1] = '\0';
            char *tempFileName = getFileNameFromInputLine(linePtr);
            if (tempFileName != NULL)
                strcpy(fileName, tempFileName);
            if (strlen(linePtr) > MAX_LINE_LENGTH)
            {
                perror("Error. This line is too long. Max length is 512");
                continue;
            }
            else
            {
                int sequentialCount = splitCommands(linePtr, ':', commandsSequential);
                // printf("SEQUENTIAL COUNT IS : %d\n" , sequentialCount);
                if (sequentialCount > 0)
                { //Then there is atleast one command
                    for (int i = 0; i < sequentialCount; i++)
                    {
                        if (commandsSequential[i] == NULL)
                        {
                            perror("Error, a command string is NULL when reading sequential commands.");
                            continue;
                        }
                        else
                        {
                            int commandsThreadedLen = splitCommands(commandsSequential[i], ';', commandsThreaded);

                            pthread_t threads[commandsThreadedLen];
                            printf("threaded COUNT IS : %d\n", commandsThreadedLen);
                            for (int j = 0; j < commandsThreadedLen; j++)
                            {
                                printf("\n>-%s-\n", commandsThreaded[j]);
                            }
                            int threadsLen = 0;

                            for (int j = 0; j < commandsThreadedLen; j++)
                            {
                                if (commandsThreaded[j] != NULL && strstr(commandsThreaded[j], "exit") != NULL)
                                    whileFlag = false;
                                struct thread_args args;
                                args.fileName = fileName;
                                args.singleCommand = commandsThreaded[j];
                                printf("\n");
                                int rc = -1;
                                rc = pthread_create(&threads[threadsLen++], NULL, multiWordParseSingleCommand, (void *)&args);
                                //parseSingleCommand((void *) &args);
                                if (rc != 0)
                                {
                                    perror("Error when opening thread.");
                                }
                                for (int j = 0; j < threadsLen; j++)
                                {
                                    pthread_join(threads[i], NULL);
                                    printf("I wait is end for thread\n");
                                }
                            }
                        }
                    }
                }
                else
                {
                    //gets(line);
                    //continue;
                }
            }
        }
        else
        {
            printf("Error ocurred when reading from the Input line\n");
            perror("Gets error");
        }
        printf("\n");
    } while (whileFlag);

    return 0;
}

int batchedInputLoop(FILE *f)
{
    printf("BATCH LOOP STARTS\n");
    char line[MAX_LINE_LENGTH];
    bool whileFlag = true;
    char *commandsSequential[9999] = {NULL};
    char *commandsThreaded[9999] = {NULL};
    char *linePtr;
    char fileName[512] = {'\0'};

    if (f == NULL)
    {
        perror("File NULL : ");
        printf("FILE IS NULL, switching to input mode\n");
        return inputLoop();
    }
    do
    {
        printf("\n");
        if (fgets(line, 515, f))
        {
            if (feof(f))
            {
                printf("END of FILE\n");
                exit(78);
            }
            if (line == NULL)
                exit(-99);
            optionalFileName[0] = '\0';
            linePtr = line;
            linePtr[strlen(linePtr) - 1] = '\0';
            //printf("Line is %s\n", linePtr);
            char *tempFileName = getFileNameFromInputLine(linePtr);
            if (tempFileName != NULL)
                strcpy(fileName, tempFileName);
            if (strlen(linePtr) > MAX_LINE_LENGTH)
            {
                perror("Error. This line is too long. Max length is 512");
                continue;
            }
            else
            {
                int sequentialCount = splitCommands(linePtr, ':', commandsSequential);
                // printf("SEQUENTIAL COUNT IS : %d\n" , sequentialCount);
                if (sequentialCount > 0)
                { //Then there is atleast one command
                    for (int i = 0; i < sequentialCount; i++)
                    {
                        if (commandsSequential[i] == NULL)
                        {
                            perror("Error, a command string is NULL when reading sequential commands.");
                            continue;
                        }
                        else
                        {
                            int commandsThreadedLen = splitCommands(commandsSequential[i], ';', commandsThreaded);

                            pthread_t threads[commandsThreadedLen + 1];
                            //printf("threaded COUNT IS : %d\n" , commandsThreadedLen);
                            int threadsLen = 0;
                            int rc[50] = {-1};
                            struct thread_args args[50];
                            int argsCount = 0;

                            if (optionalFileName != NULL && strlen(optionalFileName) > 0 && strcmp(optionalFileName, "\0") != 0)
                            {
                                strcpy(args[argsCount].fileName, optionalFileName);
                                printf("Optional file entry found. The file is being treated as inputFile : -%s-\n", args[argsCount].fileName);
                            }

                            for (int j = 0; j < commandsThreadedLen; j++)
                            {
                                if (commandsThreaded[j] != NULL && strstr(commandsThreaded[j], "exit") != NULL)
                                    whileFlag = false;
                                args[argsCount].fileName = fileName;
                                args[argsCount].singleCommand = commandsThreaded[j];
                                rc[threadsLen] = pthread_create(&threads[threadsLen], NULL, multiWordParseSingleCommand, (void *)&args[argsCount++]);
                                if (rc[threadsLen] != 0)
                                {
                                    perror("Error when opening thread.");
                                }
                                threadsLen++;
                            }
                            for (int j = 0; j < threadsLen; j++)
                            {
                                if (rc[j] == 0 && threads[j] != NULL)
                                {
                                    pthread_join(threads[j], NULL);
                                }
                                else
                                {
                                }
                            }
                        }
                    }
                }
                else
                {
                }
            }
        }
        else
        {
            exit(-510);
        }
        printf("\n");
    } while (whileFlag);
}

int methodTests()
{
    printf("\nStarting by 117");
    struct thread_args args;
    args.fileName = "hehe.txt";
    args.singleCommand = "insert muro hakan -b hehe.txt";
    parseSingleCommand(&args);
    //exit(117);
    return 132;
}

int main(int argc, char *argv[])
{
    printf("CMPE 382 - Project #1\nAuthor : Hakan Ahmet Tekin\n----------\n");
    //methodTests();
    if(argc == 1){
            FILE *f = fopen(argv[0], "r+");
    if(f == NULL){
        printf("Main function could not open a specified input file. Trying open default file batch.txt\n");
        f = fopen("batch.txt", "r+");
        if(f == NULL){
            printf("Main function could not open the default input file. Program Exits\n");
            exit(-111);
        }else{
            printf("batch.txt is open beginning program\n");
            batchedInputLoop(f);
        }
    }else{
        printf("->%s is open beginning program\n", argv[0]);
        batchedInputLoop(f);
    }
    }else{
        printf("Something is Wrong\n");
        FILE *f = fopen("batch.txt", "r+");
        batchedInputLoop(f);
    }
    //inputLoop();
    return 0;
}

//This method splits a line according to the delimeter and puts each substring in an array
int splitCommands(const char *input, char delim, char *Commands[])
{
    //printf("splitCommands : %s\n", input);
    char *tofree = malloc(sizeof(char) * strlen(input) * 10);

    if (input != NULL)
    {
        char *token = NULL;
        strcpy(tofree, input);
        token = strtok(tofree, &delim);
        if (token == NULL)
        {
            printf("%s is not a valid command. Try again\n", input);
            return 0;
        }
        if (token[0] == ' ')
            token = token + 1;
        Commands[0] = malloc(sizeof(char) * strlen(token) - 1);
        strcpy(Commands[0], token);
        int i = 1;
        while (token != NULL)
        {
            //printf("hEYYYY\n");
            token = strtok(NULL, &delim);
            if (token == NULL)
            {
                continue;
            }
            else
            {
                Commands[i] = malloc(sizeof(char) * strlen(token) - 1);
                if (token[0] == ' ')
                    token = token + 1;
                strcpy(Commands[i], token);
                i++;
            }
        }
        return i;
    }
    else
    {
        printf("NULL input.\n");
        return -1;
    }
}
