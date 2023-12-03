#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#define delimiters " \t\f\r\v\n"

char *read_line(char *buffer, int size);
int tokenize(char *input, int *token_count, char ***token_vector);
void execute_command(char **tokens);
void wait_for_child(int pid);
void free_tokens(char **tokens, int token_count);

int main () {
	char line[1024];
	char **tokens = NULL;
	int token_count = 0;
	
	printf("minishell> "); //prompt
	
	if (!read_line(line, sizeof(line))) {
		if (feof(stdin)) {
			printf("No input provided. Exiting shell.\n");
			return 0;
		} else {
			perror("fgets");
			return EXIT_FAILURE;
		}
	}
	
	if (tokenize(line, &token_count, &tokens) == 0) {
		// No command entered
		printf("No command entered. Exiting shell.\n");
		return 0;
	}
	
	int pid;
	pid = fork();
	if (pid == -1) {
		perror("fork");
		exit(EXIT_FAILURE);
	} else if (pid == 0) {
		// In child process
		execute_command(tokens);
		// If execvp fails, free memory before exiting
		free_tokens(tokens, token_count);
		exit(EXIT_FAILURE);
	} else {
		// In the parent process, wait for the child and free memory
		wait_for_child(pid);
		free_tokens(tokens, token_count);
		tokens = NULL;
		token_count = 0;
	}
	
	return(0);
}


char *read_line(char *buffer, int size) {
	return fgets(buffer, size, stdin);
}

int tokenize(char *input, int *token_count, char ***token_vector) {
	int capacity = 10; // initial token array capacity
	char **tokens = malloc(capacity * sizeof(char *));
	
	if (tokens == NULL) {
		exit(1);
	}
	
	*token_count = 0; // Initialize token count
	
	char *token = strtok(input, delimiters); // Tokenize the input string
	
	while (token != NULL) { // Check if the token count exceeds the current capacity
		if (*token_count >= capacity) {
			capacity *= 2; // Double the capacity 
			tokens = realloc(tokens, capacity * sizeof(char *)); //reallocate memory for tokens
			if (tokens == NULL) { // Check if reallocation was successful
				exit(1);
			}
		}
		tokens[*token_count] = strdup(token); // Duplicate the token and store it in the tokens array
		if (tokens[*token_count] == NULL) {
			exit(1);
		}
		(*token_count)++; // Increment the token count
		token = strtok(NULL, delimiters);
	}
	
	*token_vector = tokens; // Set the token_vector pointer to the tokens array
	return *token_count; // Return number of tokens
}

void execute_command(char **tokens) {
	execvp(tokens[0], tokens);
}

void wait_for_child(int pid) {
	int status;
	if (waitpid(pid, &status, 0) == -1) {
		perror("waitpid");
	}
}

void free_tokens(char **tokens, int token_count) {
	for (int i = 0; i < token_count; ++i) {
		free(tokens[i]);
	}
	free(tokens);
}
