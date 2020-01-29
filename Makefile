shell: project1.o
	gcc project1.o -o shell

project.o: project.c
	gcc -c project1.c

clean:
	rm *.o shell
