//COP4610
//Project 1 Starter Code
//example code for initial parsing

//*** if any problems are found with this code,
//*** please report them to the TA


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>



typedef struct
{
	char** tokens;
	int numTokens;
} instruction;

void addToken(instruction* instr_ptr, char* tok, char* PWD);
void printTokens(instruction* instr_ptr);
void clearInstruction(instruction* instr_ptr);
void addNull(instruction* instr_ptr);

void interpret(instruction* instr_ptr, char* PWD);

int inside(char* str, int i);

char** resizeArray(char**, int*);	// returns an array with double the size as was previously passed, doubles passed int
char** createArray(int);		// initiates a new 2d dynamically allocated array
void deallocateArray(char**, int); 	// destructs a dynamically allocated array
char* parse_path(char* str, char* PWD);
void double_period(char* ret);		// help parse double period
int path_check(const char* path);	// cheaks if path namne valid
char* commandPath(char* cmd);		//returns command added to correct $PATH location
void my_execute(char** cmd);			//executes commands

int main() {
	char* token = NULL;
	char* temp = NULL;

	instruction instr;
	instr.tokens = NULL;
	instr.numTokens = 0;

	char* user = getenv("USER");
	char* machine = getenv("MACHINE");	
	while (1) {
		char * PWD = getenv("PWD");
		// printf("Please enter an instruction: ");
		printf("%s%s%s%s%s%s", user, "@", machine, ":", PWD, "> ");
		// loop reads character sequences separated by whitespace
		do {
			//scans for next token and allocates token var to size of scanned token
			scanf("%ms", &token);
			temp = (char*)malloc((strlen(token) + 1) * sizeof(char));

			int i;
			int start = 0;
			for (i = 0; i < strlen(token); i++) {
				//pull out special characters and make them into a separate token in the instruction
				if (token[i] == '|' || token[i] == '>' || token[i] == '<' || token[i] == '&') {
					if (i-start > 0) {
						memcpy(temp, token + start, i - start);
						temp[i-start] = '\0';
						addToken(&instr, temp, PWD);
					}

					char specialChar[2];
					specialChar[0] = token[i];
					specialChar[1] = '\0';

					addToken(&instr,specialChar, PWD);

					start = i + 1;
				}
			}

			if (start < strlen(token)) {
				memcpy(temp, token + start, strlen(token) - start);
				temp[i-start] = '\0';
				addToken(&instr, temp, PWD);
			}
		
			//free and reset variables
			free(token);
			free(temp);

			token = NULL;
			temp = NULL;
		} while ('\n' != getchar());    //until end of line is reached
		
		addNull(&instr);
		interpret(&instr, PWD);
		printTokens(&instr);
		clearInstruction(&instr);
	}
	
	return 0;
}

//reallocates instruction array to hold another token
//allocates for new token within instruction array
void addToken(instruction* instr_ptr, char* tok, char* PWD)
{
	//extend token array to accomodate an additional token
	if (instr_ptr->numTokens == 0)
		instr_ptr->tokens = (char**) malloc(sizeof(char*));
	else
		instr_ptr->tokens = (char**) realloc(instr_ptr->tokens, (instr_ptr->numTokens+1) * sizeof(char*));

	//allocate char array for new token in new slot
	instr_ptr->tokens[instr_ptr->numTokens] = (char *)malloc((strlen(tok)+1) * sizeof(char));
	strcpy(instr_ptr->tokens[instr_ptr->numTokens], tok);

	instr_ptr->numTokens++;
}

void addNull(instruction* instr_ptr)
{
	//extend token array to accomodate an additional token
	if (instr_ptr->numTokens == 0)
		instr_ptr->tokens = (char**)malloc(sizeof(char*));
	else
		instr_ptr->tokens = (char**)realloc(instr_ptr->tokens, (instr_ptr->numTokens+1) * sizeof(char*));

	instr_ptr->tokens[instr_ptr->numTokens] = (char*) NULL;
	instr_ptr->numTokens++;
}

void printTokens(instruction* instr_ptr)
{
	int i;
	printf("Tokens:\n");
	for (i = 0; i < instr_ptr->numTokens; i++) {
		if ((instr_ptr->tokens)[i] != NULL)
			printf("%s\n", (instr_ptr->tokens)[i]);
	}
}

void clearInstruction(instruction* instr_ptr)
{
	int i;
	for (i = 0; i < instr_ptr->numTokens; i++)
		free(instr_ptr->tokens[i]);

	free(instr_ptr->tokens);

	instr_ptr->tokens = NULL;
	instr_ptr->numTokens = 0;
}

// read through tokens
void interpret(instruction* instr_ptr, char* PWD) {
	if (!(strcmp(instr_ptr->tokens[0], "echo")) && instr_ptr->tokens[1][0] == '$'){          // if statement for all eachos
		char * dest = (char*)malloc(50 * sizeof(char));  
		strcpy(dest, &(instr_ptr->tokens[1][1]));
		printf("%s\n", getenv(dest));			
	}
	if (strcmp(instr_ptr->tokens[0], "cd") == 0)
	{
		if(strstr(instr_ptr->tokens[1],"/") == NULL)
		{
			char * tempPWD = (char*)malloc(500 * sizeof(char)); 
			strcpy(tempPWD,PWD);
			strcat(tempPWD,"/");
			strcat(tempPWD,instr_ptr->tokens[1]);
			strcpy(instr_ptr->tokens[1],tempPWD);
		}
		if (path_check(instr_ptr->tokens[1]) == 1)
		{
			chdir(instr_ptr->tokens[1]);
			setenv("PWD",instr_ptr->tokens[1],1);
		}
		else
			printf("No such file or directory\n");
		
	}
	else	
		my_execute(instr_ptr->tokens);
}


// gets absolute path returned at string
char* parse_path(char* str, char* PWD){
	char** array;	// 2D array
	int size = 5;	// initial size

	array = createArray(size);	// create array

	char* token = strtok(str, "/");		// start parsing path
	int i = 1, j = 0, parsed = 0;

	while(token != NULL){			// will end with a list of all names in path seperated by /
		if(i == size - 1){
			array = resizeArray(array, &size);
		}
	
		strcpy(array[i], token);
		i++;
		token = strtok(NULL, "/");
	}

	char* ret = (char*)malloc(500 * sizeof(char));		// ret will be the new absolute path

	if (strcmp(array[1], "~") == 0){	// if we need to start form home just get home, igore PWD
		strcpy(ret, getenv("HOME"));
	}
	else{
		strcpy(ret, PWD);		// use PWD to keep track of our own path 
		strcat(ret, "/");
		strcat(ret, array[0]);		// add slash and path name
	}

	for ( j = 1; j < i; j++){		// loop through the rest of the names that we seperated by /
		if(strcmp(array[j], ".") == 0)		{}	// '.' doesn't change directory
		else if(strcmp(array[j], "~") == 0 )	{	// already started at HOME so just add slash
			strcat(ret, "/");
		}
		else if(strcmp(array[j], "..") == 0){
			double_period(ret);	// erases chars up to next '/' or until string is empty
 		}
		else {		// in base case just get then next name in the path		
			strcat(ret, array[j]);
			strcat(ret, "/");
		}
	}
	if(ret[strlen(ret) - 1] == '/')		// if the last char in ret is a slash take it off
		ret[strlen(ret) -1 ] = '\0';
	
	return ret;
}

// return 1 if path name is valid 0 otherwise
int path_check(const char* path){
 	struct stat buffer;
 	int status;
	if(stat(path, &buffer) == 0){
		return 1;
	}
	else {
		return 0;
 	}
}

// returns string from thring passed minus all chars after last '/'
void double_period(char* ret){
	int ch = strlen(ret) - 1;
	ch--;
			
	while (ret[ch] != '/' || ch == 0){
		ret[ch] = '\0';
		ch--;
	}
}

// releases memory of dynamically allocated array 
void deallocateArray(char** arr, int size) {
	int i;
	for (i = 0; i < size; i++) {
		free(arr[i]);
	}
	free(arr);	// releasing memory
}

// create new dynamic array of size 'size'
char** createArray(int size) {
	char** newarr = (char**)malloc(size * sizeof(char*));

	int i;
	for (i = 0; i < size; i++) {	// allocates each index in the array
		newarr[i] = (char*)malloc(100 * sizeof(char));
	}

	return newarr;
}

// resizes an array and changes the size to double the passed size
char** resizeArray(char** array, int* sizeofarray) {
		
	// keep origional size for dealocation, double it for new array
	int oldsize = *sizeofarray;
	*sizeofarray = oldsize * 2;

	// create new array
	char ** new = (char**)malloc(*sizeofarray * sizeof(char*));
	
	// allocate each index of new array
	int i;	
	for (i = 0; i < *sizeofarray; i++) {
		new[i] = (char*)malloc(100 * sizeof(char));
	}

	// copy contents from old to new
	for (i = 0; i < oldsize; i++) {
		strcpy(new[i], array[i]);
		//new[i] = array[i];
	}

	// free old array
	for (i = 0; i < oldsize; i++) {
		free(array[i]);
	}
	free(array);

	return new;
}

char* commandPath(char* cmd)
{
	char * fullPath = getenv("PATH");	
	char * copyFull = malloc(strlen(fullPath) + 1);
	strcpy(copyFull,fullPath);
	char * path = strtok(copyFull,":");
	char * temp = (char*)malloc(500 * sizeof(char));
	while (path != NULL)
	{ 
		strcpy(temp,path);			
		strcat(temp,"/");		
		strcat(temp,cmd);	
//		printf("%s\n",temp);
		if (path_check(temp) == 1)		//path exists 
			break;
		path = strtok(NULL, ":");
	}	
	return temp;			
}

void my_execute(char** cmd)
{	
	char* path = commandPath(cmd[0]);				
	int status;
	pid_t pid = fork();
	if (pid == -1)		//error
	{
		printf("Error");
		exit(1);		
	}
	else if (pid == 0)	//child
	{
		execv(path, cmd);
	//	fprintf("Problem executing %s\n", cmd[0]);	//not sure why it doesn't like this line
		printf("Problem executing %s\n", cmd[0]);
		exit(1);	
	}
	else			//parent
	{
		waitpid(pid, &status, 0);
	}
}


