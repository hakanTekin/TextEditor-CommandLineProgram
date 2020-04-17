# TextEditor-CommandLineProgram
A command line text editor supporting multiple functionalities. Made for Operating Systems course CMPE-382.

## Functions and Descriptions


## Running the program

The program expects one command. A file name (not longer than 512) indicating the file containing commands to run. If no file name is given. Program will try to find a file named 'batch.txt'. If that fails too, it will switch to manual input mode. Which reads from stdin.

##

## Input Parsing
1. The main input file is always at the end of line.

2. If an Output file is specified (e.g. > Output.txt), each sequential command after it will use this file as the main input file. This is reset after a new line of commands.

3. The program always rewrites input files from scratch (if no specific output file name is given).

4. Keyword inputs have to be in double-quotes ("house")

5. Keywords can be multiple words but not multiple lines.

6. Command optional arguments (e.g. -c for search) can be written in any part of the command subset. So search -c hakan text.txt is valid.

7. Program is case sensitive. Every command should be written accordingly.

## Outputs
1. Insert and replace commands use the keywords as they are. No extra whitespaces are placed before or after.

2. Messages from methods are not written to an output file. 

3. When search, tail, mid and head is being run. It will only print results to stdin. 