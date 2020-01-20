//COP4610
//Project 1 Starter Code
//example code for initial parsing

//*** if any problems are found with this code,
//*** please report them to the TA


#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef struct
{
	char** tokens;
	int numTokens;
} instruction;

void addToken(instruction* instr_ptr, char* tok);
void printTokens(instruction* instr_ptr);
void clearInstruction(instruction* instr_ptr);
void addNull(instruction* instr_ptr);

void interpret(instruction* instr_ptr);
int inside(char* str, int i);

char** resizeArray(char**, int*);	// returns an array with double the size as was previously passed, doubles passed int
char** createArray(int);		// initiates a new 2d dynamically allocated array
void deallocateArray(char**, int); 	// destructs a dynamically allocated array
char* parse_path(char* str);


int main() {
	char* token = NULL;
	char* temp = NULL;

	instruction instr;
	instr.tokens = NULL;
	instr.numTokens = 0;

	char* user = getenv("USER");
	char* machine = getenv("MACHINE");	
	char * path = "";

	while (1) {
		// printf("Please enter an instruction: ");
		printf("%s%s%s%s%s%s", user, "@", machine, ":~", path,  " > ");
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
		interpret(&instr);
		printTokens(&instr);
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
void interpret(instruction* instr_ptr) {
	if (!(strcmp(instr_ptr->tokens[0], "echo")) ){

		if (!(strcmp(instr_ptr->tokens[1], "$USER")))
			printf("%s\n", getenv("USER"));
		
//		printf("It's working");
	}
	if (!(strcmp(instr_ptr->tokens[0], "cd")) ){
		char* ret = parse_path(instr_ptr->tokens[1]);
		printf("return = %s\n", ret);
	}
	else {
		printf("%d \n", instr_ptr->tokens[0] ); 
		printf("error");
	}
	printf("\n");
}

// gets absolute path returned at string
char* parse_path(char* str){
	char** array;
	int size = 5;

	array = createArray(size);
//	printf("str: ", str);
	char* token = strtok(str, "/");
	int i = 1, j = 0, parsed = 0;

	while(token != NULL){
			
		if(i == size - 1){
//			printf("Resizing\n");
			array = resizeArray(array, &size);
		}
	
		strcpy(array[i], token);
		i++;
		token = strtok(NULL, "/");
	
//		printf("Printing entire array\n");
//		for(j = 0; j < size; j++) {
//			printf("array[%d]%s\n", j, array[j]);
//		}

	}

	char* ret = (char*)malloc(100 * sizeof(char));
//	printf("Attempting to combine strings\n");
//	printf("array[0] = %s\n", array[0]); 
	strcpy(ret, array[0]);

//	printf("ret before entering loop: %s\n", ret);
	for ( j = 1; j < i; j++){
	//	printf("array[%d] = %s\n", j, array[j]);
	//	printf("current token %s\n\n", token);

		if(strcmp(array[j], ".") == 0){
			// printf("Doing nothing");
		}
		else if(strcmp(array[j], "..") == 0){
			int ch = strlen(ret) - 1;
			array[ch] = '\0';
			ch--;
			
			while (ret[ch] != '/' || ch == 0){
				ret[ch] = '\0';
				ch--;
			}
			//ret[strlen(ret) -1] = '\0'; 
			 			
		}
		else {		
			strcat(ret, array[j]);
			strcat(ret, "/");
//			printf("ret %s\n", ret);
		}
	}
	return ret;
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


