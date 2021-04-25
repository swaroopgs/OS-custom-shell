#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

//limits
#define MAX_TOKENS 100
#define MAX_STRING_LEN 100
#define INPUT 1
#define OUTPUT 2
#define INPUTANDOUTPUT 3
#define FILTER 4

size_t MAX_LINE_LEN = 10000;
// builtin commands
#define EXIT_STR "exit"
#define EXIT_CMD 0
#define UNKNOWN_CMD 99

FILE *fp; // file struct for stdin
char *line;
char input[64], output[64];
int in, out;
int operation_type = 0;

void execute_command();
int execute_pipes();

void initialize()
{

	// allocate space for the whole line
	assert((line = malloc(sizeof(char) * MAX_STRING_LEN)) != NULL);

	// open stdin as a file pointer
	assert((fp = fdopen(STDIN_FILENO, "r")) != NULL);
}

char **tokenize(char *string)
{
    int token_count = 0;
	int size = MAX_TOKENS;
    char **tokens = malloc(sizeof(char *) * MAX_TOKENS);
	char *this_token;

	while ((this_token = strsep(&string, " \t\v\f\n\r")) != NULL)
	{

		if (*this_token == '\0')
			continue;

		tokens[token_count] = this_token;

		printf("Token %d: %s\n", token_count, tokens[token_count]);

		token_count++;

		// if there are more tokens than space ,reallocate more space
		if (token_count >= size)
		{
			size *= 2;
			assert((tokens = realloc(tokens, sizeof(char *) * size)) != NULL);
		}
	}

    return tokens;
}

char **tokenize1(char *string)
{
    int token_count1 = 0;
	int size = MAX_TOKENS;
    char **tokens1 = malloc(sizeof(char *) * MAX_TOKENS);
	char *this_token;

	while ((this_token = strsep(&string, "|")) != NULL)
	{

		if (*this_token == '\0')
			continue;

		tokens1[token_count1] = this_token;

		printf("Token1 %d: %s\n", token_count1, tokens1[token_count1]);

		token_count1++;

		// if there are more tokens than space ,reallocate more space
		if (token_count1 >= size)
		{
			size *= 2;

			assert((tokens1 = realloc(tokens1, sizeof(char *) * size)) != NULL);
		}
	}
    return tokens1;
}

void tokenizepipes() {
	int status;
    char *line_copy;
    char **tokens;
	char **tokens1;
    pid_t pid;
    int in, fd[2];
    in = 0;
    int count = 0;
    int seperatedCount = 0;
    // int input, output = 0;

    line_copy = malloc(sizeof(char) * MAX_STRING_LEN);
    strcpy(line_copy, line);
    printf("Shell read this line: %s\n", line_copy);

    tokens = tokenize1(line_copy);

    while(tokens[count]!=NULL)
    {
        count++;
    }
    for (int i = 0; i < count - 1; i++)
    {
        pipe(fd);
        execute_pipes(in, fd[1], tokens[i]);
        close(fd[1]);
        in = fd[0];
    }

    if (in != 0) {
        dup2(in, 0);
    }

	printf("%s last command\n", tokens[count - 1]);
	tokens1 = tokenize(tokens[count - 1]);

    while(tokens1[seperatedCount]!=NULL)
    {
        seperatedCount++;
    }

    if(seperatedCount > 1) {
        if(strcmp(tokens1[1], "<") == 0) {
            printf("*** ERROR: input redirection not allowed");
            exit(1);
        }

        if (strcmp(tokens1[1], ">") == 0) {
            tokens1[1] = NULL;
            strcpy(output, tokens1[2]);
            out = creat(output, 0644);
            if (dup2(out, STDOUT_FILENO) == -1)
            {
                perror(" ");
            }
            close(out);
            close(fd[0]);
        }
    }

	if ((pid = fork()) < 0)
	{ /* fork a child process           */
		printf("*** ERROR: forking child process failed\n");
		exit(1);
	}
	else if (pid == 0)
	{ 
		if (execvp(tokens1[0], tokens1) < 0) {     /* execute the command  */
            printf("*** ERROR: exec failed\n");
            exit(1);
        }
	} else {
		if (waitpid(pid, &status, 0) > 0) { 
    		if (WIFEXITED(status) && !WEXITSTATUS(status))  
              	printf("program execution successfulll\n");
		}
	}
}

void read_command()
{

	// getline will reallocate if input exceeds max length
	assert(getline(&line, &MAX_LINE_LEN, fp) > -1);

	printf("Shell read this line: %s\n", line);

}

void checkforOperators(char **tokens)
{

	int input = 0;
	int output = 0;
	int inputandOutput = 0;
    int count = 0;
    while(tokens[count]!=NULL)
    {
        count++;
    } 

	for (int i = 0; i < count -1; i++)
	{
        if (strcmp(tokens[i], "|") == 0){
            operation_type = FILTER;
            break;
		} else if (strcmp(tokens[i], "<") == 0) {
			input++;
			operation_type = INPUT;
            if (input && output) {
                inputandOutput = 1;
			    operation_type = INPUTANDOUTPUT;
            }
		} else if (strcmp(tokens[i], ">") == 0) {
			output++;
			operation_type = OUTPUT;
            if (input && output) {
                inputandOutput = 1;
			    operation_type = INPUTANDOUTPUT;
            }
		}
	}
}

int run_command(char **tokens)
{

	if (strcmp(tokens[0], EXIT_STR) == 0)
	{
		return EXIT_CMD;
	}
	else
	{
        if(operation_type == FILTER) {
            tokenizepipes();
        } else {
		    execute_command(tokens);
        }
	}

	return UNKNOWN_CMD;
}

int execute_pipes(int in, int out, char *command) {
    pid_t pid;
	int status;
    char *com;
    char ** tokens1;
    int count = 0;
    
    if ((pid = fork()) < 0)
	{ /* fork a child process           */
		printf("*** ERROR: forking child process failed\n");
		exit(1);
	}
	else if (pid == 0)
	{   /* for the child process:         */
		printf("here %s", command);
		
        tokens1 = tokenize(command);

        while(tokens1[count]!=NULL)
        {
            count++;
        }

        if(count > 1) {
            if(strcmp(tokens1[1], ">") == 0) {
                printf("*** ERROR: output redirection not allowed");
                exit(1);
            }

            if (strcmp(tokens1[1], "<") == 0 && in == 0) {
                tokens1[1] = NULL;
                strcpy(input, tokens1[2]);

                in = open(input, O_RDONLY);
                if (dup2(in, STDIN_FILENO) == -1)
                {
                    perror(" ");
                }
                close(in);
            } 
        }

        if (in != 0) {
            printf("whery\n");
            dup2(in, 0);
            close(in);
        }

        if (out != 1) {
            printf("no here\n");
            dup2(out, 1);
            close(out);
        }
		
        if (execvp(tokens1[0], tokens1) < 0) {     /* execute the command  */
            printf("*** ERROR: exec failed\n");
            exit(1);
        }
	}
	else
	{ /* for the parent:      */
		if (waitpid(pid, &status, 0) > 0) { 
    		if (WIFEXITED(status) && !WEXITSTATUS(status))  
              	printf("program execution successful\n");
		}
		// while (wait(&status) != pid);
	}

    return 0;
}

void execute_command(char **tokens)
{
	pid_t pid;
	int status;
	char string[100];

	switch (operation_type)
	{
	case INPUT:
		if (strcmp(tokens[1], "<") == 0)
		{
			tokens[1] = NULL;
			strcpy(input, tokens[2]);

			in = open(input, O_RDONLY);
			if (dup2(in, STDIN_FILENO) == -1)
			{
				perror(" ");
			}
			close(in);
		}

		break;
	case OUTPUT:
		if (strcmp(tokens[1], ">") == 0)
		{
			tokens[1] = NULL;
			strcpy(output, tokens[2]);

			out = creat(output, 0644);
			if (dup2(out, STDOUT_FILENO) == -1)
			{
				perror(" ");
			}
			close(out);
		}
		break;
	case INPUTANDOUTPUT:
        printf("i/o %s", tokens[1]);

		if(strcmp(tokens[1],"<") == 0) {
			in = open(tokens[2], O_RDONLY);
			if (dup2(in, STDIN_FILENO) == -1)
			{
				perror(" ");
			}

			out = open(tokens[4],O_WRONLY|O_CREAT,0666);

			if (dup2(out, STDOUT_FILENO) == -1)
			{
				perror(" ");
			}

			close(out);
			
		} else {
			in = open(tokens[4], O_RDONLY);
			if (dup2(in, STDIN_FILENO) == -1)
			{
				perror(" ");
			}

			out = open(tokens[2],O_WRONLY|O_CREAT,0666);
			if (dup2(out, STDOUT_FILENO) == -1)
			{
				perror(" ");
			}
			close(out);
		}
		break;

	default:
		break;
	}

	if ((pid = fork()) < 0)
	{ /* fork a child process           */
		printf("*** ERROR: forking child process failed\n");
		exit(1);
	}
	else if (pid == 0)
	{   /* for the child process:         */
		   printf("here");
		if(operation_type == INPUTANDOUTPUT) {
            printf("%s", tokens[0]);
			if(execlp(tokens[0],tokens[0],NULL) < 0) {
                perror(tokens[0]);
                printf("*** ERROR: exec failed\n");
                exit(1);
            }
		} else {
            printf("single");
            printf("%s %s %s", tokens[0], tokens[1], tokens[2]);
            if (execvp(tokens[0], tokens) < 0) {     /* execute the command  */
			printf("*** ERROR: exec failed\n");
			exit(1);
		    }
        }
	}
	else
	{ /* for the parent:      */
		while (wait(&status) != pid);
	}
}

int main()
{
    char **tokens;
    char *line_copy;

	initialize();
    line_copy = malloc(sizeof(char) * MAX_STRING_LEN);
    
	do
	{
		printf("sh550> ");
		read_command();
        strcpy(line_copy, line);
        tokens = tokenize(line_copy);
		checkforOperators(tokens);

	} while (run_command(tokens) != EXIT_CMD);

	return 0;
}
