#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>

#define MAX_LINE 255 // Maximum input line length

//Initial shell path
static char *shell_paths[64] = {"/bin", "/usr/bin", NULL}; // intialize to 64 string buffer.


// returns the number of paths in our shell (buffer of strings)
int sizeof_shell_paths() {
	return sizeof(shell_paths) / sizeof(char*);
}

// Function to read a line of input
char* read_line(void) {
    char* line = NULL;
    size_t bufsize = 0; // getline will allocate a buffer
    getline(&line, &bufsize, stdin);
    return line;
}

// Function to parse the input line into arguments
char** parse_line(char* line) {

    int bufsize = 64, position = 0;
    char **tokens = malloc(bufsize * sizeof(char*)); // intialize tokens to 64 string buffer.
    char *token;

    if (!tokens) { // error allocating tokens
	char error_message[30] = "An error has occurred\n";
	write(STDERR_FILENO, error_message, strlen(error_message));
	fflush(stdout);
        exit(EXIT_FAILURE);
    }
    
    token = strsep(&line, " \t\n"); // seperate arguments by white spaces.
    if(line == NULL) { // check if strsep() couldn't find the deliminator, return the whole string
    	tokens[0] == token;
    	tokens[1] == NULL;
    	return tokens;
    }
    while (line != NULL) { 
    	if (strcmp(token, "")==0) { // look for end of line condition. makes line = NULL if true.
    		token = strsep(&line, " \t\n");
    		continue;
    	}
        tokens[position++] = token; // add token to buffer
        if (position >= bufsize) {
            bufsize += 64;
            tokens = realloc(tokens, bufsize * sizeof(char*)); // if position is past buffer size.
            							// reallocate tokens buffer.
            if (!tokens) { // error allocating tokens
		char error_message[30] = "An error has occurred\n";
		write(STDERR_FILENO, error_message, strlen(error_message));
		fflush(stdout);
                exit(EXIT_FAILURE);
            }
        }
        token = strsep(&line, " \t\n"); // parse line
    }
    tokens[position] = NULL; // add the NULL character bit
    return tokens;
}

// Function to split a line into commands given the parallel operator '&'.
char **split_commands(char* line, int* num_commands) {
    int bufsize = 64, num_c = 0;
    char** commands = malloc(bufsize * sizeof(char*));
    char* command;
    
   // check if only one command
    command = strsep(&line, "&");
    if (line == NULL) {
        *num_commands = 1;
        commands[0] = command;
        commands[1] = NULL;
    	return commands;
    }
    
    // check for parallel commands 
    while (line != NULL) {
    	commands[num_c++] = command;
    	command = strsep(&line, "&");
    	if (line == NULL) { // check if strsep failed
        	*num_commands = num_c+1;
        	commands[num_c] = command;
        	commands[num_c+1] = NULL;
    		return commands;
    	}
    }
}



/// Check if command exists in the path and return the full path if it does
char* find_command_in_path(char* command) {
	static char full_path[255];
	int i = 0;
	// try /bin/command then /usr/bin/command
	while (shell_paths[i] != NULL) {
		strcpy(full_path, shell_paths[i]);
		strcat(full_path, "/");
		strcat(full_path, command);
		// check if command is in shell path
		if (access(full_path, X_OK) == 0) {
			return full_path;
		}
		i++;
	}
	return NULL;	
}

// Function to launch shell commands
int rush_shell(char **args) {
    pid_t pid;
    int file_out;
    char* output_filename = NULL;
    char* cmd_path;
    
    // check for redirection 
    if (strcmp(args[0], ">")==0) { // if nothings is before the > symbol, declare error.
        		char error_message[30] = "An error has occurred\n";
			write(STDERR_FILENO, error_message, strlen(error_message));
			fflush(stdout);
			return 1;
    }
    for (int i = 0; args[i] != NULL; i++) {
    	if (strcmp(args[i], ">")==0) {
    		if (args[i+1] == NULL || args[i+2] != NULL) { // error: incorrect number of arguments
    			char error_message[30] = "An error has occurred\n";
			write(STDERR_FILENO, error_message, strlen(error_message));
			fflush(stdout);
			return 1;	
    		}
    		output_filename = args[i+1];
    		args[i] = NULL; // remove redirection character '>' and filename from args
    		break;
    	}
    }
    
    // Check if the command exists in any of our shell paths
    cmd_path = find_command_in_path(args[0]);
    if (cmd_path == NULL) { // command not found
	char error_message[30] = "An error has occurred\n";
	write(STDERR_FILENO, error_message, strlen(error_message));
	fflush(stdout);
    	return 1;
    }


    // fork() and execv() to execute shell commands
    pid = fork();
    if (pid == 0) {  // Child process
        
        // check for redirection
        if (output_filename != NULL) {
		// open file for writing and executing. 
		file_out = open(output_filename, O_CREAT | O_WRONLY | O_TRUNC, S_IRWXU);
		if (file_out == -1) { // an error has occured
			char error_message[30] = "An error has occurred\n";
			write(STDERR_FILENO, error_message, strlen(error_message));
			fflush(stdout);
		}
		// redirect stdout to the file
		dup2(file_out, STDOUT_FILENO);
		close(file_out); // don't need this file descriptor any more
    	}		
    	// execute shell commands
        if (execv(cmd_path, args) == -1) {
        	char error_message[30] = "An error has occurred\n";
		write(STDERR_FILENO, error_message, strlen(error_message));
		fflush(stdout);
        }
        exit(EXIT_FAILURE); // Error has occured
    } if (pid == -1) {  // Error forking
        char error_message[30] = "An error has occurred\n";
	write(STDERR_FILENO, error_message, strlen(error_message));
	fflush(stdout);
    }
    return 0;
}


// Function to execute built-in commands. returns 1 if command is executed, 0 otherwise
int rush_builtins(char **args) {

    if (args[0] == NULL) {  // An empty command was entered.
        return 1;
    }

    // exit command
    if (strcmp(args[0], "exit")==0) {
    	if (args[1] != NULL) {
    		char error_message[30] = "An error has occurred\n";
		write(STDERR_FILENO, error_message, strlen(error_message));
		fflush(stdout);
		return 1;
	}
        exit(0);
    }

    // change directory (cd) command
    if (strcmp(args[0], "cd")==0) {
	if(args[1] == NULL || args[2] != NULL) {  // the wrong number of arguments was given
		char error_message[30] = "An error has occurred\n";
		write(STDERR_FILENO, error_message, strlen(error_message));
		fflush(stdout);
		return 1;
	}
	if (chdir(args[1]) != 0) {  // if chdir returns there was an error
		char error_message[30] = "An error has occurred\n";
		write(STDERR_FILENO, error_message, strlen(error_message));
		fflush(stdout);
	}
    	return 1;
    }
    
    // path command
    if (strcmp(args[0], "path")==0) {
    	// free existing shell_paths by making everything in the array NULL
    	for (int i = 0; i < 64; i++) {
    		shell_paths[i] = NULL;
    	}
	// duplicate the specified path(s) and put it in global shell_paths
	for (int i = 0; args[i+1] != NULL; i++) {
		shell_paths[i] = strdup(args[i+1]); // strcpy causes segmentaion fault because shell_path is not allocated with malloc().
	}
    	return 1;
    }
    
    return 0;
}



// Main loop of the shell
void rush_loop(void) {
    char *line;
    char **commands;
    char **args;
    int num_coms;
    int status = 1;

    do {
        printf("rush> ");
        fflush(stdout);
        line = read_line(); // read line from user input
        commands = split_commands(line, &num_coms); // look for parallel commands denoted by '&'
        
        for (int i=0; i<num_coms; i++) { // iterate and run each command
        	args = parse_line(commands[i]); // tokenize command
        	status = rush_builtins(args); // check for builtin command
        	if(!status) { // handle shell commands if command isn't found in built-in commands
        		rush_shell(args); // execute shell commands
        	}
        	free(args);
        }
        
        // wait for all child processes to finish
        while (num_coms > 0) {
        	wait(NULL);
        	num_coms--;
        }
        free(line);
        free(commands);
    } while (1);
}

int main(int argc, char **argv) {
    if (argc > 1) { // wrong number of arguments
	char error_message[30] = "An error has occurred\n";
	write(STDERR_FILENO, error_message, strlen(error_message));
	fflush(stdout);
	return EXIT_FAILURE;
    }


    // Run command loop.
    rush_loop();

    return EXIT_SUCCESS;
}
