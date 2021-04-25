A C program that:

Writes a prompt to standard output of the form mysh>
Reads a command from standard input, terminated by a newline (enter on the keyboard)
Execute the command it just read.
Compile and Run instructions:

Compile: Go to the source directory containing the project and run 'make' command. This compiles the source code in ./src directory and creates an executable mysh in ./build directory.

Run: Run './build/mysh' from the source directory.

Notes:

For all redirections and pipes, make sure to add white space before and after '<' or '>' or '|' symbols.
Does not handle * in the command line.
No command completion or tab controlled suggestions supported.
