Bryce Brooks 	bbb14c
Myra Mullis

Division of labor -

Parsigng:              Given
Environment Variables: Bryce
Prompt:                Myra
Shortcut Resolution:   Bryce
Path resolution:       Bryce
Execution:             Myra
I/O redirection:       Bryce
Pipes:                 Bryce
Background processing: Myra
Builts ins:            Both
        Myra: exit, jobs
	Bryce: cd, echo

Contents: README 
          shell.c - main program
          makefile - created executable

-- Type make and it will create and executable called shell.
   make clean will remove .o files and shell

Bugs --


when entering "cd" if you enter c,s,\n if will work but if you 
enter c,d, ,\n with white space it will just wait for more input. 
The same is true for incorrect I/O direction. 
ex: "executable < " with a space at the end will not throw an 
error but "executable <" will.

Outputting to a file:
When outputing to a file that doesn't exist a new file will be 
created but it will be a read only file that is empty. Were not 
sure how this is poiisble, we used O_RDWR|O_CREAT|O_TRUNC flags
that (as far as we understand) should create a file if it doesn't 
already exits that is read and writable and tuncate to it.

Double piping also doesn't work-
No excuse here just couldn't get it to work. It will output the 
execution of the last entered executable but with no input so 
we believe the problem is with the linking of it and the
second excutable entered.

Background:
The exact specifics of how the printout should happen aren't 
clear so we did our best to print out correct brackets and 
+ placement. Also sometimes you need to enter a new instruction 
to see of the process has been completed. 

Extra Cereit:
We completed I/O redirection witin the same line both in normal
execution and in the background.
