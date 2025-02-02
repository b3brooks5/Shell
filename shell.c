//Bryce Brooks and Myra Mullis
//COP4610
//Project 1 Starter Code
//given example code for initial parsing

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

void interpret(instruction* instr_ptr, char* PWD, char** backProc, pid_t** id, int size,
    int* current, int count);	// main function called once every instruction
// echo
char* echo(instruction* cmd );

// path finding
char* parse_path(char* str, const char* PWD);	// gets absolute path of one entered
void double_period(char* ret);		// help parse double period
int path_check(const char* path);	// checks if path name valid

// execute
char* commandPath(char* cmd);		//returns command added to correct $PATH location
void my_execute(char** cmd);			//executes commands
void check_instruction_paths(instruction* instr,
    const char* PWD);	// makes sure instructions are valid
void execute_found_instruction(instruction* instr, int * inDirect, int * outDirect, int * pipe,
	int* background, int* outIndex, int* inIndex, char** PWD);// decides what instruction to execute

// redirection
int output_redirect(char** cmd, int index, int bg);
int input_redirect(char**cmd, int index, int bg);
int in_out_redirect(char** cmd, int in, int out, int bg);
int redirect_check(char** cmd, int index);		// makes sure file paths are correct

// Piping
int piping(char** cmd, int index, int bg);
int pipe_check(char** cmd, int index, int index2);	// cheaks that pipe tokens and paths are valid
int double_pipe(char** cmd, int index1, int index2, int bg);	// for two pipes, doesn't work

// backgound processing
void is_background(instruction* instr_ptr, char** backProc,
    pid_t** id, int size, int* current);	//determine if background or foreground process
void addProcess(char** backProc, pid_t ** id, int size, int current);
void deleteProcess(char** backProc, pid_t ** id, char* procToRem, int size, int* current);
void processBegin(char** cmd,char** backProc, pid_t ** id, int size, int current);
int processEnd(char** backProc, pid_t ** id, int size, int* current);
char* commandLine(instruction* instr_ptr);	//returns command line as one string
char** makeCopy(instruction* instr_ptr);	//makes copy without the &
pid_t** idArray(int size);	                //creates id array
pid_t** resizeId(pid_t** array, int* sizeofarray);

// other functions
char* resizeTeacher(char* his, char* ours);	// resize original null terminated array of strings
char** resizeArray(char**, int*);	// returns an array with double the size, doubles passed int
char** createArray(int);		// initiates a new 2d dynamically allocated array
void deallocateArray(char**, int); 	// destructs a dynamically allocated array

int main() {
	char* token = NULL;
	char* temp = NULL;

	instruction instr;
	instr.tokens = NULL;
	instr.numTokens = 0;

	char** process = NULL;			//background process commands
	pid_t** id = NULL;			//process id's
	int size = 5, instrCount = 0;		//capacity of arrays (both the same)
 	int current = 0;			//current size of arrays
	process = createArray(size);		//create arrays of size 5
	id = idArray(size);

	char* user = getenv("USER");
	char* machine = getenv("MACHINE");
	while (1) {
		char * PWD = getenv("PWD");

		if (processEnd(process, id, size, &current))
			continue;
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

		if(instr.numTokens == 1 && strcmp(instr.tokens[0], "cd") == 0)
			addToken(&instr, "~");	// just cd as ~ to do the work for us

		instrCount++;
		addNull(&instr);
		interpret(&instr, PWD, process, id, size, &current,instrCount);

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

// get the path variables through the entire instruction
void check_instruction_paths(instruction* instr, const char* PWD){
	int i = 0;
	for (i = 0; i < instr->numTokens; i++)
	{	// i for counter, null for the end
		if ((instr->tokens)[i] != NULL)
		{
			if (strstr(instr->tokens[i],"/") != NULL || strstr(instr->tokens[i],".") != NULL ||
			    strstr(instr->tokens[i],"~") != NULL)
			{
				char * temp = (char*)malloc(500 * sizeof(char));
				temp = parse_path(instr->tokens[i], PWD);	// parse path on individual token

				// resize if necessary
				if (strlen(temp) > strlen(instr->tokens[i])){
					instr->tokens[i] = resizeTeacher(instr->tokens[i], temp);
					strcpy(instr->tokens[i], temp);
				}
				else

					strcpy(instr->tokens[i], temp);
			}
		}
	}
}

// return value of 1 for each instruction identified
int check_instruction_type(instruction* instr, int * inDirect,
    int * outDirect, int * pipe, int * background, int * outIndex, int * inIndex){
	int i = 0;
	for (i = 0; i < instr->numTokens; i++)
	{
		if ((instr->tokens)[i] != NULL)
		{
			if( strcmp(instr->tokens[i], "<") == 0 ){
				if( *inDirect == 1)	// only 1 input redirect per instruction
					return 1;
				*inDirect = 1;
				*inIndex = i;
			}
			if (strcmp(instr->tokens[i], ">") == 0 ){
				if( *outDirect == 1)	// only 1 output redirect per instruction
					return 1;
				*outDirect = 1;
				*outIndex = i;
			}
			if (strcmp(instr->tokens[i], "|") == 0 ) {
				if( *pipe == 1)	{	// extra cheak for double pipe
					*outIndex = i;
					*pipe = *pipe + 1;
				}
				else {
					*pipe = 1;
					*inIndex = i;
				}
			}

			if (strcmp(instr->tokens[i], "&") == 0)
				*background = 1;
		}
	}
	if ((*inDirect == 1 || *outDirect == 1) && *pipe == 1)
		return 1;	// we don't do redirection and piping in one instruction


	return 0;	// instruciton is correct
}


// read through tokens
void interpret(instruction* instr_ptr, char* PWD, char** backProc,
    pid_t** id, int size, int* current, int count) {
	if(strcmp(instr_ptr->tokens[0], "echo") == 0) {		// get custom environment variabe string
		printf("%s\n", echo(instr_ptr));
		return;
	}

	check_instruction_paths(instr_ptr, PWD);

	int i = 0;
	while(instr_ptr->tokens[i] != NULL ) {			// cheak any tokens are empty strings
		if(strcmp(instr_ptr->tokens[i], "") == 0) {	// if so their was an invalid path
			printf("Invalid statemnt\n");
			return;
		}
		i++;
	}

	int in_redirect = 0, out_redirect = 0, pipe = 0, background = 0, out_index = 0, in_index = 0;

	if (check_instruction_type(instr_ptr, &in_redirect, &out_redirect,
	    &pipe, &background, &out_index, &in_index) == 1){
		printf("Unknown instruction type\n"); // if tokens not in a valid order
		return;
	}

	if (background == 1){              //there is an & somewhere in line
                is_background(instr_ptr, backProc, id, size, current);
        }
	else if (in_redirect == 1 && out_redirect == 1){
		if( redirect_check(instr_ptr->tokens, in_index) == 1 &&
		    redirect_check(instr_ptr->tokens, out_index) == 1)
			in_out_redirect(instr_ptr->tokens, in_index, out_index, 0);
		else
			printf("Command not found\n");
	}
	else if (in_redirect == 1){
		if( redirect_check(instr_ptr->tokens, in_index) == 1)
			input_redirect(instr_ptr->tokens, in_index, 0);
		else if (redirect_check(instr_ptr->tokens, in_index) == -1)	// incorrect input file
			printf("No such file or directory\n");
		else
			printf("Command not found\n");	// missing a file or execution
	}
	else if(out_redirect == 1) {
		if( redirect_check(instr_ptr->tokens, out_index) == 1)
			output_redirect(instr_ptr->tokens, out_index, 0);
		else
			printf("Command not found\n");
	}
	else if (pipe >= 1){
		printf("%d\n", pipe_check(instr_ptr->tokens, in_index, out_index));
		if(pipe_check(instr_ptr->tokens, in_index, out_index) == 1)
			piping(instr_ptr->tokens, in_index, 0);
		else if (pipe_check(instr_ptr->tokens, in_index, out_index) == 2)
			double_pipe(instr_ptr->tokens, in_index, out_index, 0);
		else
			printf("Command not found\n");
	}

	else if (strcmp(instr_ptr->tokens[0], "cd") == 0)
	{
		if (strcmp(instr_ptr->tokens[1],".") == 0)
		{}
		else if(strstr(instr_ptr->tokens[1],"/") == NULL &&
		    strstr(instr_ptr->tokens[1],"..") == NULL)
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
			instr_ptr->tokens[1][strlen(instr_ptr->tokens[1])] = '\0';
		}
		while(instr_ptr->tokens[i] != NULL){
			i++;
		}
		if ( i > 2)
			printf("cd: to many arguments\n");

		else {
			if (chdir(instr_ptr->tokens[1]) == -1)		//directory does not exist
				printf("No such directory\n");
			else
				setenv("PWD",instr_ptr->tokens[1],1);
		}

	}
	else if (strcmp(instr_ptr->tokens[0], "exit") == 0)
	{
		int status;
		waitpid(-1, &status, 0);				//waits for all processes to finish
		printf("Exiting now!\n");
		printf("\tCommands executed: %d\n", count);
		exit(1);
	}
	else if (strcmp(instr_ptr->tokens[0], "jobs") == 0)
	{
		int i;
		for (i = 0; i < *current; i++)
			printf("[%d]+ [%d] [%s]\n",i+1,*id[i],backProc[i]);
	}
	else
		my_execute(instr_ptr->tokens);

}

            /* echo */
char* echo(instruction* cmd){
	int i = 1;
	char* ret = (char*)malloc(500 * sizeof(char));
	if(strcmp(cmd->tokens[0], "echo") == 0){
		while(cmd->tokens[i] != NULL){
			if(cmd->tokens[i][0] == '$' && isalpha(cmd->tokens[i][1]) != 0 ){
				// see if it is an environment variable
				char* env = getenv(&(cmd->tokens[i][1]));
				if(env != NULL){	// if env var found
					strcat(ret, env);
					strcat(ret, " ");
				}
				else {			// else error
					strcpy(ret, cmd->tokens[i]);
					strcat(ret, ": Undefined variable");
					break;
				}
			}
			else {				// if not env just add to ret
				strcat(ret, cmd->tokens[i]);
				strcat(ret, " ");
			}
		i++;
		}
	}
	return ret;	// return entire string
}

            /* path finding */
// gets absolute path returned at string
char* parse_path(char* path, const char* PWD){
	char** array;	// 2D array
	int size = 5;	// initial size
	int absolute = 0;

	if(path[0] == '/')	// flag for absolute path
		absolute = 1;

	array = createArray(size);

	char* token = strtok(path, "/");
	int i = 1, j = 0;

	while(token != NULL){			// will end with a list of all strings in path seperated by /
		if(i == size - 1){
			array = resizeArray(array, &size);	// cheak for resize
		}

		strcpy(array[i], token);
		i++;
		token = strtok(NULL, "/");
	}

	char* ret = (char*)malloc(500 * sizeof(char));		// ret will be the new absolute path

	// Cheaking first element of path
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
	else if ( absolute == 1){
		strcpy(ret, "/");
	}
	else {
		strcpy(ret, PWD); //getenv("PWD"));
		strcat(ret, "/");		// add slash and path name
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
	if(stat(path, &buffer) == 0){	// is path
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
	if(strcmp(ret, "/home/") != 0){
		while (ret[ch] != '/' || ch == 0){
			ret[ch] = '\0';
			ch--;
		}
	}
}

            /* execution */
void my_execute(char** cmd)
{	char* path = (char*)malloc(500 * sizeof(char));
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
		printf("Problem executing %s\n", cmd[0]);
		exit(1);
	}
	else			//parent
	{
		waitpid(pid, &status, 0);
	}
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

            /* I/O redirection */
int input_redirect(char** cmd, int index, int bg){
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
	addNull(&Myinstr);		// amke null terminated

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
	else if (bg == 1)		//if background
	{
		waitpid(pid, &status, WNOHANG);
                close(fd);
	}
	else {				// parent
		waitpid(pid, &status, 0);
		close(fd);
	}
	clearInstruction(&Myinstr);
	return pid;
}

int output_redirect(char** cmd, int index, int bg){
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
	addNull(&Myinstr);		// make null terminated

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
	else if (bg == 1)		//background processing
	{
		waitpid(pid, &status, WNOHANG);
                close(fd);
	}
	else {				// parent
		waitpid(pid, &status, 0);
		close(fd);
	}
	clearInstruction(&Myinstr);
	return pid;
}

int in_out_redirect(char** cmd, int in, int out, int bg){
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
	addNull(&Myinstr);		// make null terminated


	pid_t pid = fork();		// begin fork
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

		char * path = commandPath(cmd[0]);

		execv(path, Myinstr.tokens);

		printf("Problem executing\n");
	}
	else if (bg == 1)
	{
		waitpid(pid, &status, WNOHANG);
                close(Input_fd);
                close(Output_fd);
	}
	else {				// parent
		waitpid(pid, &status, 0);
		close(Input_fd);
		close(Output_fd);
	}
	clearInstruction(&Myinstr);
	return pid;
}

// returns 1 if good redirect call 0 otherwise
int redirect_check(char** cmd, int index){
	if(path_check(cmd[0]) == 1) { // if index 1 is a command and destinatioin isn't null
		if(strcmp(cmd[index], ">") == 0 && cmd[index + 1] != NULL ){
			return 1;
		}
		else if (strcmp(cmd[index], "<") == 0 && cmd[index + 1] != NULL ) {
			if(path_check(cmd[index + 1]) == 1)	// if destination is valid
				return 1;
			else					// if destination isn't valid
				return -1;
		}
		else if(strcmp(cmd[index], "|") && cmd[index + 1] != NULL && path_check(cmd[index]) == 1)
			return 1;
	}

	return 0;
}

            /* Piping functions */
int piping(char** cmd, int index, int bg) {
	instruction command1;		// create our own null terminated list
	command1.tokens = NULL;
	command1.numTokens = 0;
	int status, i = 0;

	for (i = 0; i < index; i++){	// copy up in the lowest command
		char * temp = (char*)malloc((strlen(cmd[i]) + 1) * sizeof(char));
		strcpy(temp, cmd[i]);
		addToken(&command1, temp);
	}
	addNull(&command1);		// make null terminated

	instruction command2;		// create our own null terminated list
	command2.tokens = NULL;
	command2.numTokens = 0;

	i = index + 1;

	while ( cmd[i] != NULL){
		char * temp = (char*)malloc((strlen(cmd[i]) + 1) * sizeof(char));
		strcpy(temp, cmd[i]);
		addToken(&command2, temp);
		i++;
	}
	addNull(&command2);		// make null terminated

	int FDcmd1 = open(command1.tokens[0], O_RDONLY);

	int FDcmd2 = open(command2.tokens[0], O_RDONLY);


	pid_t pid = fork();
	int fd[2] = { FDcmd1, FDcmd2 };
	if (pid == 0) {		// child
		pipe(fd);
		if(fork() == 0){	// grand child
			close(1);
			dup(fd[1]);
			close(fd[0]);
			close(fd[1]);
			char* path = commandPath(command1.tokens[0]);
			execv(path, command1.tokens);

		}
		else {		// child
			waitpid(pid, &status, 0);
			close(0);
			dup(fd[0]);
			close(fd[0]);
			close(fd[1]);
			char* path = commandPath(command2.tokens[0]);
			execv(path, command2.tokens);
		}
	}
	else if (bg == 1)
	{
		waitpid(pid, &status, WNOHANG);
                waitpid(pid, &status, WNOHANG);
	}
	else {
		waitpid(pid, &status, 0);
		waitpid(pid, &status, 0);
	}
	clearInstruction(&command1);
	clearInstruction(&command2);
	return pid;
}

int double_pipe(char** cmd, int index1, int index2, int bg){
	instruction command0;		// create our own null terminated list
	command0.tokens = NULL;
	command0.numTokens = 0;
	int status, i = 0;

	for (i = 0; i < index1; i++){	// copy up in the lowest command
		char * temp = (char*)malloc((strlen(cmd[i]) + 1) * sizeof(char));
		strcpy(temp, cmd[i]);
		addToken(&command0, temp);
	}
	addNull(&command0);		// make null terminated

	instruction command1;		// create our own null terminated list
	command1.tokens = NULL;
	command1.numTokens = 0;

	i = index1 + 1;

	while ( i < index2){
		char * temp = (char*)malloc((strlen(cmd[i]) + 1) * sizeof(char));
		strcpy(temp, cmd[i]);
		addToken(&command1, temp);
		i++;
	}
	addNull(&command1);		// make null terminated

	instruction command2;		// create our own null terminated list
	command2.tokens = NULL;
	command2.numTokens = 0;

	i = index2 + 1;

	while ( cmd[i] != NULL){
		char * temp = (char*)malloc((strlen(cmd[i]) + 1) * sizeof(char));
		strcpy(temp, cmd[i]);
		addToken(&command2, temp);
		i++;
	}
	addNull(&command2);		// make null terminated


	int FDcmd0 = open(command0.tokens[0], O_RDONLY);

	int FDcmd1 = open(command1.tokens[0], O_RDONLY);

	int FDcmd2 = open(command2.tokens[0], O_RDONLY);
	pid_t pid = fork();

	int fd[3] = { FDcmd0, FDcmd1, FDcmd2 };

	if (pid == 0) {
		pipe(fd);
		printf("1\n");
		if(fork() == 0){
			printf("2\n");
			pipe(fd);
			if(fork() == 0){
				pipe(fd);
				printf("3\n");
				close(1);
				dup(fd[1]);
				close(fd[0]);
				close(fd[1]);
				close(fd[2]);
				char* path = commandPath(command0.tokens[0]);
				execv(path, command0.tokens);
			}
			else {
				printf("4\n");
				close(0);
				dup(fd[0]);
				close(1);
				dup(fd[2]);

				close(fd[0]);
				close(fd[1]);
				close(fd[2]);
				char* path = commandPath(command1.tokens[0]);
				execv(path, command1.tokens);
			}
		}
		else {
			printf("5\n");
			close(0);
			dup(fd[1]);
			close(fd[0]);
			close(fd[1]);
			close(fd[2]);
			char* path = commandPath(command2.tokens[0]);
			execv(path, command2.tokens);
		}
	}
	else if (bg == 1)
        {
                waitpid(pid, &status, WNOHANG);
                waitpid(pid, &status, WNOHANG);
        }
	else {
		printf("6\n");
		waitpid(pid, &status, 0);
		printf("leaving\n");
	}

	clearInstruction(&command0);
	clearInstruction(&command1);
	clearInstruction(&command2);
	return pid;
}

int pipe_check(char** cmd, int index, int index2){
//	if(path_check(cmd[0]) == 1){
		if(index2 != 0){	// more then one pipe
			if(path_check(cmd[index + 1]) == 1 && path_check(cmd[index2 +1]) == 1)
				return 2;	// valid call for two pipes
		}
		else {
			if(path_check(cmd[index +1]) == 1){
				return 1;
			}
		}
//	}
	return 0;
}


            /* Background Processing */
void is_background(instruction* instr_ptr, char** backProc, pid_t** id,
    int size, int* current) //determine if background or foreground process
{
	if (strcmp(instr_ptr->tokens[0],"&") == 0)					//if & CMD
	{
	    //& CMD &, behave same as CMD &
		if (strcmp(instr_ptr->tokens[instr_ptr->numTokens - 2],"&") == 0)
		{
			char * line = commandLine(instr_ptr);
                        if(*current == size - 1)                //make more space if no more room
                        {
                                backProc = resizeArray(backProc, &size);
                                id = resizeId(id, &size);
                        }
                        backProc[*current] = line;      	//stores the command line
                        char ** command = makeCopy(instr_ptr);
                        processBegin(command,backProc,id,size,*current);
                        *current = *current + 1;          	//first process starts at 1
		}
		else						//executes in foreground
		{
			char ** command = makeCopy(instr_ptr);
			my_execute(command);
		}
	}
	else if (strcmp(instr_ptr->tokens[instr_ptr->numTokens - 2],"&") == 0)		//CMD &
	{										//need to execute in background
		int i;
		int count = 0;		//keep track of number of &'s for error checking
		int in_redirect = 0, out_redirect = 0, pipe = 0;
		int in_index = 0, out_index = 0, p_index;
		for (i = 0; i < instr_ptr->numTokens; i++)
        	{
                	if ((instr_ptr->tokens)[i] != NULL)
               		{
				if (strcmp(instr_ptr->tokens[i], "|") == 0)		//there is a pipe
				{
					p_index = i;
					pipe = 1;
				}
				else if (strcmp(instr_ptr->tokens[i], "<") == 0)
				{
					in_index = i;
					in_redirect = 1;
				}
				else if (strcmp(instr_ptr->tokens[i], ">") == 0)
				{
					out_index = i;
					out_redirect = 1;
				}
				else if (strcmp(instr_ptr->tokens[i], "&") == 0)
					count++;
			}
		}
		if (count > 1)
			printf("Error: Invalid Syntax\n");
		else if (count <= 1)
		{
			char * line = commandLine(instr_ptr);
                        if(*current == size - 1)                //make more space if no more room
                        {
                                backProc = resizeArray(backProc, &size);
                                id = resizeId(id, &size);
                        }
                        backProc[*current] = line; 		//stores the command line
                        char ** command = makeCopy(instr_ptr);
			if (pipe >= 1)
			{
				if(pipe_check(instr_ptr->tokens, p_index, out_index) == 1)
                        	{
					*id[*current] = piping(instr_ptr->tokens, p_index, 1);
                			printf("[%d] [%d]\n",*current+1, *id[*current]);    //prints message
                                        *current = *current + 1;
				}
				else if (pipe_check(instr_ptr->tokens, p_index, out_index) == 2)
                        	{
					*id[*current] = double_pipe(instr_ptr->tokens, p_index, out_index, 1);
                			printf("[%d] [%d]\n",*current+1, *id[*current]);    //prints message
                                        *current = *current + 1;
				}
				else
                        		printf("Command not found\n");
			}
			else if (in_redirect == 1 && out_redirect == 1)
			{
                		if( redirect_check(instr_ptr->tokens, in_index) == 1 &&
                		    redirect_check(instr_ptr->tokens, out_index) == 1)
                        	{
					*id[*current] = in_out_redirect(instr_ptr->tokens, in_index, out_index,1);
                			printf("[%d] [%d]\n",*current+1, *id[*current]);    //prints message
                                        *current = *current + 1;
				}
				else
                        		printf("Command not found\n");
        		}
			else if (in_redirect == 1)
			{
				if( redirect_check(command, in_index) == 1)
				{
					*id[*current] = input_redirect(command, in_index, 1);
		                	printf("[%d] [%d]\n",*current+1, *id[*current]);    //prints message
					*current = *current + 1;
				}
				else
                        		printf("Command not found\n");
			}
			else if (out_redirect == 1)
			{
				if( redirect_check(command, out_index) == 1)
                                {
                                        *id[*current] = output_redirect(command, out_index, 1);
                                 	printf("[%d] [%d]\n",*current+1, *id[*current]);//prints message
					*current = *current + 1;
                                }
                                else
                                        printf("Command not found\n");
			}
			else		//CMD &
			{
				processBegin(command,backProc,id,size,*current);
				*current = *current + 1;          	//first process starts at 1
			}
		}
	}
	else
		printf("Error: Invalid Syntax\n");
}

pid_t** idArray(int size)                      //creates id array
{
	pid_t** newarr = (pid_t**)malloc(size * sizeof(pid_t*));
        int i;
        for (i = 0; i < size; i++) {           //allocates each index in the array
                newarr[i] = (pid_t*)malloc(100 * sizeof(pid_t));
        }
        return newarr;
}

pid_t** resizeId(pid_t** array, int* sizeofarray)
{
	//keep origional size for dealocation, double it for new array
	int oldsize = *sizeofarray;
        *sizeofarray = oldsize * 2;
	//create new array
	pid_t ** new = (pid_t**)malloc(*sizeofarray * sizeof(pid_t*));
	//allocate each index of new array
	int i;
        for (i = 0; i < *sizeofarray; i++) {
                new[i] = (pid_t*)malloc(100 * sizeof(pid_t));
        }
	//copy contents from old to new
	for (i = 0; i < oldsize; i++) {
        //        strcpy(new[i], array[i]);
        	new[i] = array[i];
	}
	//free old array
	for (i = 0; i < oldsize; i++) {
                free(array[i]);
        }
        free(array);

        return new;
}

char* commandLine(instruction* instr_ptr)	// returns all the tokens for that lines as one string
{
	char * line = (char*)malloc(500 * sizeof(char));
 	int i;
	for (i = 0; i < instr_ptr->numTokens; i++)
	{
		if ((instr_ptr->tokens)[i] != NULL)
		{
			strcat(line,instr_ptr->tokens[i]);
			if (i != instr_ptr->numTokens - 2)
				strcat(line, " " );
		}
	}
	return line;
}

//prints message and begins execution
void processBegin(char** cmd, char** backProc, pid_t** id, int size, int current)
{
	char* path = commandPath(cmd[0]);
        int status;
        pid_t pid = fork();
	*id[current] = pid;					  //add pid to pid array
	if (pid != 0)
		printf("[%d] [%d]\n",current+1, *id[current]);    //prints message
	if (pid == -1)          //error
        {
                printf("Error");
                exit(1);
        }
        else if (pid == 0)      //child
        {
	        execv(path, cmd);
        	printf("Problem executing %s\n", cmd[0]);
   	    	exit(1);
        }
	else
	{
		waitpid(-1, &status, WNOHANG);
	}
}

char** makeCopy(instruction* instr_ptr)     // makes copy of instr_ptr without the & and adds null
{
	int size = instr_ptr->numTokens - 1;
	instruction instr;
	instr.tokens = NULL;
	instr.numTokens = 0;
	int i;
	for (i = 0; i < size; i++)
	{
		if (strcmp(instr_ptr->tokens[i],"&") == 0)
			continue;
		else if ((instr_ptr->tokens)[i] != NULL)
			addToken(&instr, instr_ptr->tokens[i]);
	}
	addNull(&instr);
	return instr.tokens;
}

void deleteProcess(char** backProc, pid_t ** id, char* procToRem, int size, int* current)
{
	int i;
	for (i = 0; i < *current; i++)
	{
		if (strcmp(backProc[i],procToRem) == 0 &&
		    strcmp(backProc[i],"NULL") != 0)//found the process to remove
		{
			backProc[i] = backProc[i+1];	//remove command and id from both arrays by shifting
			id[i] = id[i+1];
			*current = *current - 1;	//update current
			break;
		}
	}
}

// determine if process done, then will remove
int processEnd(char** backProc, pid_t** id, int size, int* current)
{
	int status;
	int i;
	int retVal = 0;
	for (i = 0; i < *current; i++)
	{
		if (backProc[i] != NULL){
		if(waitpid(*id[i], &status, WNOHANG) != 0) 		//done running
		{
			printf("[%d]+ [%s]\n",i+1, backProc[i]);	//execution complete message
			deleteProcess(backProc,id,backProc[i],size,current);
			retVal = 1;
		}}
	}
	return  retVal;
}

    /* extra functions */
char* resizeTeacher(char* his, char* ours) // will resize the current in instruction struct
{
	free(his);
        his = (char*)malloc(strlen(ours) * sizeof(char));
        his = ours;
        ours = NULL;
	return his;
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
