#include "stdio.h"
#include "stdbool.h"
#include "string.h"
#include "stdlib.h"
#include "pthread.h"

#define BUFFER_SIZE 1000
#define MAX_LINE_LENGTH 512
#define TEMP_FILE_NAME "temp.txt"

struct thread_args
{
    char *singleCommand;
    char *fileName;
};

int splitCommands(const char *input, const char delim, char* Commands[]);

FILE* openFileForReadPlus(char *fileName)
{

    if(fileName[strlen(fileName)-1] == '\n')
        fileName[strlen(fileName)-1] = '\0';
    FILE *fp = fopen(fileName, "r+");
    if(fp == NULL)
    {
        printf("Unable to open file named : -%s-\n", fileName);
        perror("fopen");
        return NULL;
    }
    else{
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
    while(fgets(buffer, BUFFER_SIZE, out))
        fwrite(buffer, 1, strlen(buffer), in);
    return true;
}

int lineCount(FILE *inFile){
    int lineCount = 0;
    char buffer[9999];
    while(fgets(buffer, BUFFER_SIZE, inFile)){
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

    if(inFile == NULL)
    {
        printf("File is not open!\n");
        return;
    }else if(keyword == NULL){
        printf("What am i searching for. The search word is NULL, sorry man something is wrong.\n");
        return;
    }
    else
    {
            printf("\nSTARTING SEARCH FOR %s\n", keyword);
        while((fgets(str, BUFFER_SIZE, inFile)) != NULL)
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
    if(countFlag)
        printf("\nOcurrence of %s is : %d\n", keyword, count);
    fclose(inFile);
    return;
    }
}

void replace(const char *keyword, const char *sourceKeyword, bool countFlag, FILE *inFile)
{
    char buffer[BUFFER_SIZE];

    FILE *Output = fopen(TEMP_FILE_NAME, "w+");

    if(inFile == NULL)
    {
        printf("File is not open!\n");

        return;
    }
    else
    {
         while(NULL != fgets(buffer, BUFFER_SIZE, inFile))
    {
        // For each incidence of "is"
        char *Stop = NULL;    // Where to stop copying (at 'is')
        char *Start = buffer; // Start at the beginning of the line, and after each match

        while(1)
        {
            // Find next match
            Stop = strstr(Start, sourceKeyword);

            if(Stop == NULL)
            {
                // Print the remaining text in the line
                fwrite(Start, 1, strlen(Start), Output);
                break;
            }

            // Write out everything between the end of the previous match, and the
            // beginning of the current match.
            //
            // For example:
            //
            // "Jack is a pirate who is cool"
            //
            // Has two instances to replace.  In order, we'd find them as such:
            //
            // "Jack is a pirate who is cool"
            //       ^
            //                        ^
            // What we want to do is write:
            // - "Jack "
            // - "was"
            // - "a pirate who "
            // - "was"
            // - "cool"
            printf("Match starts at: %s\n", Stop);

            // We have found a match!  Copy everything from [Start, Stop)
            fwrite(Start, 1, Stop - Start, Output);

            // Write our replacement text
            fwrite(keyword, 1, strlen(sourceKeyword), Output);

            // Next time, we want to start searching after our 'match'
            Start = Stop + strlen(sourceKeyword);

            printf("Search resumes at: %s\n", Start);
        }
    }
    // If desired, rename the Output file to the Input file

    writeEntireFile(inFile,Output);
    }
    // Close our files
    fclose(inFile);
        //Remove the temp file
    remove(TEMP_FILE_NAME);
    fclose(Output);

}

void insert(char keywordToInsert[], bool countFlag, bool afterFlag, char *keywordToInsertAfter, FILE *inFile)
{ //The current program inserts after the word.
    char insertKeyword[BUFFER_SIZE];
    char buffer[BUFFER_SIZE];
    FILE *Output = fopen(TEMP_FILE_NAME, "w+");
    strcat(insertKeyword,keywordToInsert);

    while(fgets(buffer, BUFFER_SIZE, inFile)){
        char *Stop = NULL;
        char *Start = buffer; // Start at the beginning of the line, and after each match

        while(1)
        {
            // Find next match and move the pointer to the end of our target keyword
            Stop = strstr(Start, keywordToInsertAfter);

            if(Stop == NULL)
            {
                // Print the remaining text in the line
                fwrite(Start, 1, strlen(Start), Output);
                break;
            }

            printf("Match starts at: %s\n", Stop);

            // We have found a match!  Copy everything from [Start, Stop)
            fwrite(Start, 1, Stop - Start + strlen(keywordToInsertAfter), Output);

            fwrite(keywordToInsert, 1, strlen(keywordToInsert), Output);
            // Next time, we want to start searching after our 'match'

            Start = Stop + strlen(keywordToInsertAfter);

            printf("Search resumes at: %s\n", Start);

        }
    }

    writeEntireFile(inFile, Output);

    fclose(inFile);
    //remove(TEMP_FILE_NAME);//DONT REMOVE IF ANOTHER THREAD IS USING THIS
    //fclose(Output);

    return;
}

void showHeadLines(FILE *inFile, int lineAmountToShow){
    int atLine = 1;
    char buffer[9999];
    printf("\n---SHOWING FIRST %d LINES\n",lineAmountToShow);
    while(fgets(buffer, BUFFER_SIZE, inFile) && atLine <= lineAmountToShow){
        printf("Line %d: %s\n", atLine, buffer);
        atLine++;
    }
}

void showMidLines(FILE *inFile, int startLine, int endLine){
    int linesShown = 0;
    int atLine = 0;
    char buffer[9999];

    //Extreme case handling. If starting line is smaller than end. Swap the values.
    if(endLine < startLine){
        int temp = endLine;
        endLine = startLine;
        startLine = temp;
    }

    int lineCountOfFile = lineCount(inFile);
    rewind(inFile);
    printf("\nSHOWIN LINES BETWEEN %d-%d\n", startLine, endLine);
    while(fgets(buffer, BUFFER_SIZE, inFile)){
        atLine++;
        if(linesShown > endLine - startLine + 1) //Enough lines have been shown. Break out of the loop
            return;
        else if(atLine >= startLine && atLine <= endLine){
            printf("Line %d: %s\n", atLine, buffer);
            linesShown++;
        }else{
            //Empty for now, maybe has a use, I dunno.
        }
    }
}

void showTailLines(FILE *inFile, int lineAmountToShow){
    int linesShown = 0;
    int atLine = 0;
    char buffer[9999];
    int lineCountOfFile = lineCount(inFile);
    rewind(inFile);
    printf("\nSHOWING LAST %d LINES\n", lineAmountToShow);
    while(fgets(buffer, BUFFER_SIZE, inFile)){
        atLine++;
        if(linesShown >= lineAmountToShow){
            //Code should never reach here.
            printf("ERROR : showTailLines method reached beyond the file but tried showing lines. Something is wrong.");
            return;
        }
        else if(atLine >= lineCountOfFile - lineAmountToShow + 1){
            printf("Line %d: %s\n", atLine, buffer);
            linesShown++;
        }else{
            //Empty for now, maybe has a use, I dunno.
        }
    }
}

int split(int charCount, FILE *inFile){
    FILE *out = fopen("temp.txt", "w+");
    char buffer[BUFFER_SIZE];
    char *lineEndingChar = "\n";
    charCount++;//So that termination char is added too
    while(fgets(buffer, charCount, inFile)){
        printf("writeee\n");
        if(buffer[strlen(buffer)-1] != '\n')//This is necessary. If the last char is a new line, it adds a second unnecessary line to the file without this check
            strcat(buffer, lineEndingChar);
        fwrite(buffer, 1, charCount, out);
    }
    writeEntireFile(inFile, out);
}


bool checkInputLengthValidity(char line[])
{
        for(int i = 0 ; i < (MAX_LINE_LENGTH+1); i++){
            if(line[i] == EOF || line[i] == '\0') return true;
        }
        return false;
}

void *parseSingleCommand(void *ptr){
    struct thread_args *args = (struct thread_args *)ptr;
    char *token = args->singleCommand;
    printf("token is : %s\n", token);
    char *fileName = args->fileName;
    //Method gets a single char array containing one command as token, and the fileName it should be writing into
    char *mainCommand = strtok(token, " ");
    if(strcmp(mainCommand, "search") == 0){
        //The current code only checks for first 3 keywords. If there are, say 15; no error is shown.
        char *keyword = strtok(NULL, " ");
        bool countFlag = false;

        if(strtok(NULL, " ") == "-c")
            countFlag = true;

        if(keyword == NULL)
            printf("Error keyword is null in search command\n, its token is %s\n", token);

        search(keyword, countFlag, openFileForReadPlus(fileName));

    }else if(strcmp(mainCommand, "replace")==0){
        char *targetKeyword = strtok(NULL, " ");
        char *sourceKeyword = strtok(NULL, " ");
        bool countFlag = false;

        if(strtok(NULL, " ") == "-c")
            countFlag = true;

         if(targetKeyword == NULL || sourceKeyword == NULL)
            perror("Error. A necessary command is missing in replace command");

        replace(targetKeyword, sourceKeyword, countFlag, openFileForReadPlus(fileName));

    }else if(strcmp(mainCommand, "insert") == 0){
        char *insertedKeyword  = strtok(NULL, " ");
        bool countFlag = false;

        if(strtok(NULL, " ") == "-c")
            countFlag = true;

    }else if(strcmp(mainCommand, "lineCount") == 0){

    }else if(strcmp(mainCommand, "split") == 0){

    }else if(strcmp(mainCommand, "head") == 0){

    }else if(strcmp(mainCommand, "tail") == 0){

    }else if(strcmp(mainCommand, "mid") == 0){

    }else{
        printf("Unknown main command entered. Please try again man");
        return NULL;
    }
}

char* getFileNameFromInputLine(char *l){
 char line[512];
    strcpy(line, l);
    char fileName[512];
    char *p = strrchr(line, ' ');
    if(p == NULL && strlen(l) != 0){
        return l; //If there is no space but there are chars, then there is only one word
    }else if (p && *(p + 1)){
        printf("cis");
        return p+1;
    }
    else{
        printf("Unknown error. File name is not acquired\n");
        return NULL;
    }
}

//search hakan hehe.txt : search ahmet hehe.txt ; search hakan hehe.txt


int inputLoop(void)
{
    char line[MAX_LINE_LENGTH];
    bool whileFlag = true;
    char *commandsSequential[512] = {NULL};
    char *commandsThreaded[512] = {NULL};
    char *linePtr;
    char fileName[512] = {'\0'};
    do
    {
        printf("command > ");
        if (fgets(line, 515, stdin))
        {
            linePtr = line;
            linePtr[strlen(linePtr)-1] = '\0';
            char *tempFileName = getFileNameFromInputLine(linePtr);
            if(tempFileName != NULL)
            strcpy(fileName, tempFileName);
            if(strlen(linePtr) > MAX_LINE_LENGTH){
                perror("Error. This line is too long. Max length is 512");
                continue;
            }else{
                int sequentialCount = splitCommands(linePtr, ':', commandsSequential);
                if(sequentialCount > 0){ //Then there is atleast one command
                    for(int i = 0; i < sequentialCount; i++){
                        if(commandsSequential[i] == NULL){
                            perror("Error, a command string is NULL when reading sequential commands.");
                            continue;
                        }else{
                            int commandsThreadedLen = splitCommands(commandsSequential[i], ';', commandsThreaded);

                            pthread_t threads[commandsThreadedLen];
                            for(int j = 0; j<commandsThreadedLen; j++){
                                printf("\n>-%s-\n", commandsThreaded[j]);
                            }
                            int threadsLen = 0;

                            for(int j = 0; j<commandsThreadedLen; j++){
                                if(strstr(commandsThreaded[j], "exit") != NULL)
                                    whileFlag = false;
                                struct thread_args args;
                                args.fileName = fileName;
                                args.singleCommand = commandsThreaded[j];
                                int rc = pthread_create(&threads[threadsLen++], NULL, parseSingleCommand, (void *)&args);
                                if(rc){
                                    perror("Error when opening thread.");
                                }
                            }

                            for(int j = 0; j< threadsLen; j++){
                                pthread_join(threads[i], NULL);
                            }
                            printf("\nJOINED ALL THREADS\n");
                        }
                    }
                }else{
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

    } while (whileFlag);
    return 0;
}

int methodTests()
{

    printf("\nStarting by 117");
    search("hakan", true, openFileForReadPlus("hehe.txt"));
    replace("murat", "hakan tekin", true, openFileForReadPlus("hehe.txt"));
    exit(117);
    return 132;
}

int main()
{
    printf("CMPE 382 - Project #1\nAuthor : Hakan Ahmet Tekin\n----------\n");
    inputLoop();
    return 0;
}


//This method splits a line according to the delimeter and puts each substring in an array
int splitCommands(const char *input, char delim, char *Commands[])
{

        char *tofree = malloc(sizeof(char) * strlen(input));

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
            Commands[0] = malloc(sizeof(char) * strlen(token));
            strcpy(Commands[0], token);
            int i = 1;
            while (token != NULL)
            {
                token = strtok(NULL, &delim);
                if (token == NULL){
                    continue;
                }else{
                Commands[i] = malloc(sizeof(char) * strlen(token));

                strcpy(Commands[i], token);
                i++;
                }
            }
            return i;
        }else{
            printf("NULL input.\n");
            return -1;
        }
}
