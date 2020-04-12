#include "stdio.h"
#include "stdbool.h"
#include "string.h"
#include "stdlib.h"

#define BUFFER_SIZE 1000
#define MAX_LINE_LENGTH 512
#define TEMP_FILE_NAME "temp.txt"

FILE* openFileForReadPlus(char *fileName)
{
    FILE *fp = fopen(fileName, "r+");
    if(fp == NULL)
    {
        printf("Unable to open file.\n");
        exit(-10);
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
        exit(-13);
        return;
    }
    else
    {
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
            printf("->%s\n", str);
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
        exit(-11);
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

int methodTests(){
    
    printf("\nStarting by 117");
    
    replace("mjolnir", "hakan", true, openFileForReadPlus("hehe.txt"));
    split(5, openFileForReadPlus("hehe.txt"));
    printf("\nExiting by 117");
    exit(117);
    return 132;
}

int simultaneousSeperator(char *line){
    //; is threaded    
    char *partOne[MAX_LINE_LENGTH];
    char *partTwo[MAX_LINE_LENGTH];

}

int normalModeSeperator(char *line){ //: is normal
    strtok(line, ":")
}

int inputLoop(){
    char line[MAX_LINE_LENGTH];
    bool whileFlag = true;
    do{
        if(gets(line)){
            simultaneousSeperator(line);
        }else{
            printf("Error ocurred when reading from the Input line\n");
            perror("Gets returned NULL. Either nothing has been entered or something went wrong.");
        }
    }while(whileFlag);
}

int main()
{
    printf("Hello World");
    bool whileFlag = true;
    printf("CMPE 382 - Project #1\nAuthor : Hakan Ahmet Tekin\n----------\n");
    char line[MAX_LINE_LENGTH+1];
    do
    {
        printf("inupt > ");
        gets(line);
       
        if(!checkInputLengthValidity(line)) //if input size is bigger than 512 characters give error
        {
            printf("This input line is too big ( < %s characters ). Try entering a shorter version\n", MAX_LINE_LENGTH);
        }
        else
        {
            char *token = strtok(line, " ");
            if(strcmp(token, "search") == 0){

                token = strtok(NULL, " "); 
                if(token != NULL){
                    char *keyword = token;
                    FILE *fp = openFileForReadPlus("hehe.txt");
                    search(keyword,false, fp);

                }else{
                    printf("Not enough arguments");
                }
            }else if(token == "replace"){
                
            }else{
                printf("Very unknown first argument entered, please enter the command correctly.\nEntered command is: -%s-\n", token);
            }
        }

    } while (whileFlag);
    return 0;
}