Bryce Brooks 	bbb14c
Myra Mullis	mlm16e

Division of labor -

Parsing:               Given
Environment Variables: Bryce
Prompt:                Myra
Shortcut Resolution:   Bryce
Path resolution:       Both
Execution:             Myra
I/O redirection:       Bryce
Pipes:                 Bryce
Background processing: Myra
Built-ins:             Both
        Myra: exit, jobs
	Bryce: cd, echo

Contents: README 
          project1.c - main program
          makefile - created executable

-- Type make and it will create an executable called shell.
   make clean will remove .o files and shell

Bugs --


when entering "cd" if you enter cd\n if will work but if you 
enter cd \n with white space it will just wait for more input. 
The same is true for incorrect I/O direction. 
ex: "executable < " with a space at the end will not throw an 
error but "executable <" will.

Outputting to a file:
When outputting to a file that doesn't exist a new file will be 
created but it will be a read only file that is empty. Were not 
sure how this is possible, we used O_RDWR|O_CREAT|O_TRUNC flags
that (as far as we understand) should create a file if it doesn't 
already exits that is read and writable and truncate to it.

Double piping also doesn't work-
No excuse here just couldn't get it to work. It will output the 
execution of the last entered executable but with no input so 
we believe the problem is with the linking of it and the
second executable entered.

Background:
The exact specifics of how the printout should happen aren't 
clear so we did our best to print out correct brackets and 
+ placement. Also sometimes you need to enter a new instruction 
to see of the process has been completed. If typing ls &, it 
will show the process started, prompt the user for next command 
while also outputting the results of ls, and the user must now
type another command to be able to see if the background process
completed.

Extra Credit:
We completed I/O redirection within the same line both in normal
execution and in the background.
