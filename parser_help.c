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
#include <fcntl.h>


typedef struct
{
	char** tokens;
	int numTokens;
} instruction;

void addToken(instruction* instr_ptr, char* tok);
void printTokens(instruction* instr_ptr);
void clearInstruction(instruction* instr_ptr);
void addNull(instruction* instr_ptr);

void interpret(instruction* instr_ptr, char* PWD);

int inside(char* str, int i);

char** resizeArray(char**, int*);	// returns an array with double the size as was previously passed, doubles passed int
char** createArray(int);		// initiates a new 2d dynamically allocated array
void deallocateArray(char**, int); 	// destructs a dynamically allocated array
char* parse_path(char* str, const char* PWD);
void double_period(char* ret);		// help parse double period
int path_check(const char* path);	// cheaks if path namne valid
char* commandPath(char* cmd);		//returns command added to correct $PATH location
void my_execute(char** cmd);			//executes commands
char* resizeTeacher(char* his, char* ours);	// resizes origional null terminated array of strings

void output_redirect(char** cmd, int index); 
void input_redirect(char**cmd, int index);
void in_out_redirect(char** cmd, int in, int out);
int redirect_check(char** cmd, int index);

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
						addToken(&instr, temp);
					}

					char specialChar[2];
					specialChar[0] = token[i];
					specialChar[1] = '\0';

					addToken(&instr,specialChar);

					start = i + 1;
				}
			}

			if (start < strlen(token)) {
				memcpy(temp, token + start, strlen(token) - start);
				temp[i-start] = '\0';
				addToken(&instr, temp);
			}
		
			//free and reset variables
			free(token);
			free(temp);

			token = NULL;
			temp = NULL;
		} while ('\n' != getchar());    //until end of line is reached
		
		addNull(&instr);
		interpret(&instr, PWD);
//		printTokens(&instr);
		clearInstruction(&instr);
	}
	
	return 0;
}

//reallocates instruction array to hold another token
//allocates for new token within instruction array
void addToken(instruction* instr_ptr, char* tok)
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


	int i;
	for (i = 0; i < instr_ptr->numTokens; i++)
	{
		if ((instr_ptr->tokens)[i] != NULL)
		{			
			if (strstr(instr_ptr->tokens[i],"/") != NULL || strstr(instr_ptr->tokens[i],".") != NULL || strstr(instr_ptr->tokens[i],"~") != NULL)
			{
				char * temp = (char*)malloc(500 * sizeof(char));
				temp = parse_path(instr_ptr->tokens[i], PWD);  

				if (strlen(temp) > strlen(instr_ptr->tokens[i])){
					instr_ptr->tokens[i] = resizeTeacher(instr_ptr->tokens[i], temp);
					strcpy(instr_ptr->tokens[i], temp);
				}
				else
					strcpy(instr_ptr->tokens[i], temp);
				
			}
		}
	}
	
	i = 0;
	while(instr_ptr->tokens[i] != NULL ) {
		if(strcmp(instr_ptr->tokens[i], "") == 0) {
			printf("Invalid statemnt\n");
			return;
		}
		i++;
	}
	
	int in_redirect = 0, out_redirect = 0, pipe = 0, background = 0, out_index = 0, in_index = 0;

	for (i = 0; i < instr_ptr->numTokens; i++)
	{
		if ((instr_ptr->tokens)[i] != NULL)
		{	
			if( strcmp(instr_ptr->tokens[i], "<") == 0){
				in_redirect = 1;
				in_index = i;
			}
			if (strcmp(instr_ptr->tokens[i], ">") == 0){
				out_redirect = 1;
				out_index = i;
			}
			else if (strcmp(instr_ptr->tokens[i], "|") == 0)
				pipe = 1;
			if (strcmp(instr_ptr->tokens[i], "&") == 0)
				background = 1;
		}

	}
	
	if (in_redirect == 1 && out_redirect == 1){
		in_out_redirect(instr_ptr->tokens, in_index, out_index);
	}
	else if (in_redirect == 1){
		if( redirect_check(instr_ptr->tokens, in_index) == 1)
			input_redirect(instr_ptr->tokens, in_index);
		else 
			printf("Command not found\n");
	}
	else if(out_redirect == 1){
		if( redirect_check(instr_ptr->tokens, out_index) == 1)
			output_redirect(instr_ptr->tokens, out_index);
		else
			printf("Command not found\n");
	}
	else if (pipe == 1){
	}
	else if (background == 1){		//there is an & somewhere in line
	}
	
	else if (strcmp(instr_ptr->tokens[0], "cd") == 0)
	{
		if (strcmp(instr_ptr->tokens[1],".") == 0)
		{}
		else if(strstr(instr_ptr->tokens[1],"/") == NULL && strstr(instr_ptr->tokens[1],"..") == NULL)
		{	//if there is not a / and not a ..
			char * tempPWD = (char*)malloc(500 * sizeof(char)); 
			strcpy(tempPWD,PWD);
			strcat(tempPWD,"/");
			strcat(tempPWD,instr_ptr->tokens[1]);
			if (strlen(tempPWD) > strlen(instr_ptr->tokens[1]))	//if not enough space
                        	instr_ptr->tokens[1] = resizeTeacher(instr_ptr->tokens[1], tempPWD);
			else
				strcpy(instr_ptr->tokens[1],tempPWD);
		}
		else
		{
			//printf("%s\n",instr_ptr->tokens[1]);
			instr_ptr->tokens[1][strlen(instr_ptr->tokens[1])] = '\0';
		}
		
		if (chdir(instr_ptr->tokens[1]) == -1)		//directory does not exist
			printf("No such directory\n");
		else
			setenv("PWD",instr_ptr->tokens[1],1);
	}
	else
		my_execute(instr_ptr->tokens);

	in_redirect = 0;
	out_redirect = 0;
	pipe = 0;
	background = 0;
	out_index = 0;
	in_index = 0;
}

// gets absolute path returned at string
char* parse_path(char* path, const char* PWD){
	char** array;	// 2D array
	int size = 5;	// initial size
	array = createArray(size);

	char* token = strtok(path, "/");
	int i = 1, j = 0, parsed = 0;

	while(token != NULL){			// will end with a list of all strings in path seperated by /
		if(i == size - 1){
			array = resizeArray(array, &size);	// cheak for resize
		}
	
		strcpy(array[i], token);
		i++;
		token = strtok(NULL, "/");
	}

	char* ret = (char*)malloc(500 * sizeof(char));		// ret will be the new absolute path

	// Cheaking inly first element
	if (strcmp(array[1], "~") == 0){
		strcpy(ret, getenv("HOME"));
		strcpy(array[1], "");	// make empty so not copied twice
	}
	else if (strcmp(array[1], ".") == 0){
		strcpy(ret, PWD); 
                strcat(ret, "/");
	}
	else if (strcmp(array[1], "..") == 0){
		strcpy(ret, PWD); 
      	        strcat(ret, "/");	
	}
	else{
		//strcpy(ret, PWD); 
		strcpy(ret, getenv("PWD"));
		strcat(ret, "/");
//		strcat(ret, array[1]);		// add slash and path name
	}

	// for all token after the first
	for ( j = 1; j < i; j++){
		if(strcmp(array[j], ".") == 0)	{}		// '.' doesn't change directory
		else if(strcmp(array[j], "~") == 0 ){		// cannot appear in remainder of path
			ret = "";
			break;
		}
		else if(strcmp(array[j], "..") == 0){
			double_period(ret);			// erases chars up to next '/'
 		}
		else {						// just get the next name in the path		
			strcat(ret, array[j]);
			strcat(ret, "/");
		}
	}
	if(ret[strlen(ret) - 1] == '/')		// if the last char in ret is a slash take it off
		ret[strlen(ret) -1 ] = '\0';

	deallocateArray(array, size);

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
	for (i = 0; i < size; i++) 
		free(arr[i]);
	
	free(arr);	
}

// create new dynamic array of size 'size'
char** createArray(int size) {
	char** newarr = (char**)malloc(size * sizeof(char*));	// size of array

	int i;
	for (i = 0; i < size; i++) 			// size of each index
		newarr[i] = (char*)malloc(100 * sizeof(char));
	

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
	for (i = 0; i < *sizeofarray; i++) 
		new[i] = (char*)malloc(100 * sizeof(char));
	

	// copy contents from old to new
	for (i = 0; i < oldsize; i++) 
		strcpy(new[i], array[i]);


	// free old array
	for (i = 0; i < oldsize; i++) 
		free(array[i]);
	
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
		if (path_check(temp) == 1)		//path exists 
			break;
		path = strtok(NULL, ":");
	}	

	if(strstr(temp, "./")) 
		strcpy(temp, cmd);


	return temp;			
}

void my_execute(char** cmd)
{	
	char* path = (char*)malloc(500 * sizeof(char));				
	strcpy(path, commandPath(cmd[0]));
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

void output_redirect(char** cmd, int index){
	char* file = cmd[index + 1]; // grab the name of the file
	int fd = open(file, O_RDWR|O_CREAT|O_TRUNC);
	int status, i = 0;

	
	instruction Myinstr;		// create our own null terminated list
	Myinstr.tokens = NULL;
	Myinstr.numTokens = 0;
	
	for (i = 0; i < index; i++){	// copy up in 'index' where the > is located
		char * temp = (char*)malloc((strlen(cmd[i]) + 1) * sizeof(char));
		strcpy(temp, cmd[i]);
		addToken(&Myinstr, temp);
	}
	addNull(&Myinstr);	
	
	pid_t pid = fork();		// begin fork
	if (pid == -1){
		printf("Error opening file\n");
	}
	else if(pid == 0){		// child
		close(1);
		dup(fd);
		close(fd);
		
		char * path = commandPath(cmd[0]);

		execv(path, Myinstr.tokens);

		printf("Problem executing\n");
	}
	else {				// parent
		waitpid(pid, &status, 0);
		close(fd);
	}
}

void input_redirect(char** cmd, int index){
	char* file = cmd[index + 1]; // grab the name of the file
	int fd = open(file, O_RDONLY);
	int status, i = 0;


	instruction Myinstr;		// create our own null terminated list
	Myinstr.tokens = NULL;
	Myinstr.numTokens = 0;
	
	for (i = 0; i < index; i++){	// copy up in 'index' where the > is located
		char * temp = (char*)malloc((strlen(cmd[i]) + 1) * sizeof(char));
		strcpy(temp, cmd[i]);
		addToken(&Myinstr, temp);
	}
	addNull(&Myinstr);	
	
	pid_t pid = fork();		// begin fork
	if (pid == -1){
		printf("Error opening file\n");
	}
	else if(pid == 0){		// child
		close(0);
		dup(fd);
		close(fd);
		
		char * path = commandPath(cmd[0]);
	
		execv(path, Myinstr.tokens);

		printf("Problem executing\n");
	}
	else {				// parent
		waitpid(pid, &status, 0);
		close(fd);
	}

}

void in_out_redirect(char** cmd, int in, int out){
	char* Input_file = cmd[in + 1]; 	// input file
	int Input_fd = open(Input_file, O_RDONLY);

	char* Output_file = cmd[out + 1];	// output file
	int Output_fd = open(Output_file, O_RDWR|O_CREAT|O_TRUNC);
	int status, i = 0;

	if(in < out)
		out = in;

	instruction Myinstr;		// create our own null terminated list
	Myinstr.tokens = NULL;
	Myinstr.numTokens = 0;
	
	for (i = 0; i < out; i++){	// copy up in the lowest command
		char * temp = (char*)malloc((strlen(cmd[i]) + 1) * sizeof(char));
		strcpy(temp, cmd[i]);
		addToken(&Myinstr, temp);
	}
	
	addNull(&Myinstr);	
	
	
	pid_t pid = fork();		// begin fork
	printf("pid %d\n", pid);
	if (pid == -1){
		printf("Error opening file\n");
	}
	else if(pid == 0){		// child
		close(0);		// input first
		dup(Input_fd);
		close(Input_fd);
		
		close(1);		// output
		dup(Output_fd);
		close(Output_fd);
		printf("In here\n");
		char * path = commandPath(cmd[0]);

		execv(path, Myinstr.tokens);

		printf("Problem executing\n");
	}
	else {				// parent
		waitpid(pid, &status, 0);
		close(Input_fd);
		close(Output_fd);
	}
}

// returns 1 if good redirect call 0 otherwise
int redirect_check(char** cmd, int index){
	if(path_check(cmd[0]) == 1) { // if index 1 is a command
		if(strcmp(cmd[index], ">") && cmd[index +1] != NULL && path_check(cmd[index +1]) == 1){
			return 1;
		}
		else if (strcmp(cmd[index], "<") && cmd[index + 1] != NULL){
			return 1;
		} 
	}
	return 0;
}
char* resizeTeacher(char* his, char* ours) // will resize the curent string in the instruction struct
{
	free(his);
        his = (char*)malloc(strlen(ours) * sizeof(char));
        his = ours;
        ours = NULL;
	return his;
}
