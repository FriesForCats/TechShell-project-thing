// Name(s): Kyrylo Prkyhodchenko
// Description: Tech Shell project

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>


typedef struct {
    char *argv[100];
    char *inputFile;
    char *outputFile;
} command;
/* 
    A function that causes the prompt to 
    display in the terminal
*/
int displayPrompt(){
    char cwd[1024];

    if (getcwd(cwd, sizeof(cwd)) != NULL){
        printf("%s", cwd);
        return 0;
    } else{
        perror("cwd error");
        return 1;
    }

}

/*
    A function that takes input from the user.
*/ 
char* getInput(){
    char* text = malloc(100);

    if (text == NULL){
        return NULL;
    }

    printf("$ ");
    fgets(text, 100, stdin);

    return text;
}

/*
    A function that parses through the user input.
*/
void parseInput(char *input, command *cmd){
    
    // sets all struct values back to zero
    int argCount = 0;
    cmd->inputFile = NULL;
    cmd->inputFile = NULL;

    input[strcspn(input, "\n")] = '\0';
    char *src = input;
    char *dst = input;
    char *tokens[100];
    int numTokens = 0;

    while (*src != '\0'){
        // skips any beginning whitespace
        while (*src == ' ' || *src == '\t') {
            src++;
        }

        // stops at the end
        if (*src == '\0') {
            break;
        }
        
        tokens[numTokens] = dst;
        // for qoutes
        int quotes = 0;

        while (*src != '\0') {
            // looks for qoutes
            if (*src == '"') {
                quotes = !quotes;
                src++; 

            // looks for space outside of the quotes
            } else if (!quotes && (*src == ' ' || *src == '\t')) {
                src++;
                break;

            //checks for "\"
            } else if (*src == '\\') {
                src++;
                if (*src != '\0') {
                    *dst++ = *src++;
                }

            } else {
                *dst++ = *src++;
            }
        }
        *dst++ = '\0';
        numTokens++;
    }

    // looks for redirection
    for (int i = 0; i < numTokens; i++) {
        if (strcmp(tokens[i], "<") == 0) {
            if (i + 1 < numTokens) {
                cmd->inputFile = tokens[i + 1];
                i++; 
            }
        } else if (strcmp(tokens[i], ">") == 0) {
            if (i + 1 < numTokens) {
                cmd->outputFile = tokens[i + 1];
                i++;
            }
        } else {
            cmd->argv[argCount++] = tokens[i];
        }
    }

    // adds null terminator at the end of the array
    cmd->argv[argCount] = NULL;
}



/*
    A function that executes the command. 
*/
void executeCommand(command *cmd){

    // Case for if no command is inputed
    if (cmd->argv[0] == NULL){
        return;
    }

    // FOR CD
    if (strcmp(cmd->argv[0], "cd") == 0){
        char *path = cmd->argv[1];

        if (path == NULL){
            path = getenv("HOME");
            chdir(path);
        }

        if (chdir(path) < 0){
            fprintf(stderr, "Error %d (%s)\n", errno, strerror(errno));
        }
        return;
    }

    // FOR EXIT
    if (strcmp(cmd->argv[0], "exit") == 0){
        exit(0);
    }


    pid_t pid = fork();
    if (pid < 0){
        perror("Fork failed");
        return;
    }

    // command executed in child so execvp can be used and not close the whole thing
    if (pid == 0){
        
        // for <
        if (cmd-> inputFile != NULL){
            int file = open(cmd-> inputFile, O_RDONLY);
            if (file < 0){
                fprintf(stderr, "Error %d (%s)\n", errno, strerror(errno));
                exit(1);
            }

            // copies input
            dup2(file, STDIN_FILENO);


            close(file);
        }

        // for >
        if (cmd->outputFile != NULL){

            int file = open(cmd->outputFile, O_WRONLY | O_CREAT | O_TRUNC, 0644);
            if (file < 0){
                fprintf(stderr, "Error %d (%s)\n", errno, strerror(errno));
                exit(1);
            }

            dup2(file, STDOUT_FILENO);

            close(file);
        }

        // runs the command
        execvp(cmd->argv[0], cmd->argv);
        //runs if execvp fails to run
        fprintf(stderr, "Error %d (%s)\n", errno, strerror(errno));
        exit(1);



    } else{
        int status;
        waitpid(pid, &status, 0); // parent just waits on child to do its thing
    }

}


int main(){
for (;;){
    char* input;
    command cmd;

    // runs display prompt and gets the input from the user, p much recreates the look of bash
    displayPrompt();
    input = getInput();
    
    // parses through input to get the command and then executes it
    parseInput(input, &cmd);
    executeCommand(&cmd);




    // Frees up memory at the end of the loop
    free(input);
    }
}