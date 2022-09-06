/***************************************************************
 * Project 3 - Simple Shell Project
 * By Gavin Cutchin
 * 4/30/2022
 * CMSC 257 - 001
 * *************************************************************/

/* $begin shellmain */
#include <signal.h>
#include "csapp.h"
#define MAXARGS   128

/* Function prototypes */
void eval(char *cmdline);
int parseline(char *buf, char **argv);
int builtin_command(char **argv);
int cd(char **argv); 
int help();
pid_t pid();
pid_t ppid();
void tryExit();
void sigHandler(int sig);

int main(int argc, char *args[]) 
{   
    char cmdline[MAXLINE]; /* Command line */
    char *currentShellName = "sh257"; //shell prompt name

    //catch signals
    signal(SIGINT, sigHandler);
    signal(SIGTERM, sigHandler);
    
    //rename the shell prompt if given the -p flag
    if(args[1] != NULL && !strcmp(args[1], "-p") && args[2] != NULL){
        currentShellName = args[2];
    }

    while (1) {
	/* Read */
	printf("%s>", currentShellName);                   
	Fgets(cmdline, MAXLINE, stdin); 
	if (feof(stdin))
	    exit(0);

	/* Evaluate */
	eval(cmdline);
    }
}
/* $end shellmain */
  
/* $begin eval */
/* eval - Evaluate a command line */
void eval(char *cmdline) 
{
    char *argv[MAXARGS]; /* Argument list execve() */
    char buf[MAXLINE];   /* Holds modified command line */
    int bg;              /* Should the job run in bg or fg? */
    pid_t pid;           /* Process id */
    
    strcpy(buf, cmdline);
    bg = parseline(buf, argv); //bg doesn't matter to much but helps store bg or fg info if we wanted
    if (argv[0] == NULL){  
	return;   /* Ignore empty lines */
    }

    //runs builtin_commmand() first and will execute the appropriate function
    if (!builtin_command(argv)) { 
        if ((pid = Fork()) == 0) {   /* Child runs user job */
            //this only runs if the command was not found in builtin_command()
            if (execvp(argv[0], argv) < 0) {
                //command is unsupported
                printf("%s: Command not found or is an unsupported function.\n", argv[0]);
                exit(1);
            }
            else{
                exit(0);
            }
        }

    int status;
    //wait for child processes
    if (waitpid(pid, &status, 0) == -1) {
        perror("An error with waitpid has occurred");
    }
    
    //grab and print exit status of child processes
    if (WIFEXITED(status)) {
        int temp = WEXITSTATUS(status);
        printf("Process exited with status %d\n", temp);
    }
    return;
}
}

/* If first arg is a builtin command, run it and return true */
int builtin_command(char **argv) 
{
    if (!strcmp(argv[0], "&"))    /* Ignore singleton & */
	return 1;
    if (!strcmp(argv[0], "exit")){ /* exit command */
    tryExit();
    return 1;
    }
    if (!strcmp(argv[0], "cd")){   /* change directory */
        return cd(argv);
    }
    if (!strcmp(argv[0], "pid")){  /* get process id */
        printf("Process ID: %d\n", pid());
        return 1;
    }
    if (!strcmp(argv[0], "ppid")){ /* get parent process id */
        printf("Parent Process ID: %d\n", ppid());
        return 1;
    }
    if (!strcmp(argv[0], "help")){ /* list help */
        return help();
    }
    if (!strcmp(argv[0], "ls") || !strcmp(argv[0], "date") || !strcmp(argv[0], "echo") || !strcmp(argv[0], "ps")){ 
        /* 4 additional methods ls, date, echo and ps all included through execvp() */
        pid_t pid = fork(); //make a child process
        if(pid == -1){
            printf("Failed forking a child.\n");
        }
        else if(pid == 0){
            //if there is an issue with execvp
            if(execvp(argv[0], argv) < 0){
                printf("Command not executed.");
                exit(1); //exit with status 1 if error occurs
            }
        }

        int status;
        //wait for child processes
        if (waitpid(pid, &status, 0) == -1) {
            perror("An error with waitpid has occurred");
            return 1;
        }
        
        //grab and print exit status of child processes
        if (WIFEXITED(status)) {
            int temp = WEXITSTATUS(status);
            printf("Process exited with status %d\n", temp);
        }

        return 1;
    }

    //returns 0 if a function is not supported
    return 0;     
}
/* $end eval */

//THIS WAS SUPPLIED WITH SHELLEX.C
/* $begin parseline */
/* parseline - Parse the command line and build the argv array */
int parseline(char *buf, char **argv) 
{
    char *delim;         /* Points to first space delimiter */
    int argc;            /* Number of args */
    int bg;              /* Background job? */

    buf[strlen(buf)-1] = ' ';  /* Replace trailing '\n' with space */
    while (*buf && (*buf == ' ')) /* Ignore leading spaces */
	buf++;

    /* Build the argv list */
    argc = 0;
    while ((delim = strchr(buf, ' '))) {
	argv[argc++] = buf;
	*delim = '\0';
	buf = delim + 1;
	while (*buf && (*buf == ' ')) /* Ignore spaces */
            buf++;
    }
    argv[argc] = NULL;
    
    if (argc == 0)  /* Ignore blank line */
	return 1;

    /* Should the job run in the background? */
    if ((bg = (*argv[argc-1] == '&')) != 0)
	argv[--argc] = NULL;

    return bg;
}
/* $end parseline */

//cd function operates similar to current terminal cd
int cd(char** argv){
    //current working directory
    char cwd[MAXLINE];
    if(argv[1] == NULL){
        char cwd[MAXLINE];
        //if cd has no arguments, print the current directory
        if(getcwd(cwd, sizeof(cwd)) != NULL){
            printf("Current Directory: %s\n", cwd);
        }
        else{
            //error with getcwd somehow
            perror("getcwd() error");
        }
    } //otherwise change directory to the argument provided with cd
    else if(argv[1] != NULL && argv[2] == NULL){
        chdir(argv[1]); 
    }
    else{ //otherwise there are too many arguments
        printf("Too many arguments, try: cd <directory>\n");
    }

    return 1;
}

//Uses SIGTERM to attempt to terminate the program by sending a signal
void tryExit(){
    raise(SIGTERM);
}

//Returns the process ID
pid_t pid(){
    return getpid();
}

//returns the Parent Process ID
pid_t ppid(){
    return getppid();
}

//help - prints out a list of functions and their definitions and usage
int help(){
    printf("\n**********************************************************************\n");
    printf("A Custom Shell for CMSC 257\n");
    printf("    - Gavin Cutchin\n");
    printf("Usage:\n");
    printf("cd <dir> - to change directory\n");
    printf("man - for help with non built in functions\n");
    printf("date - to list current date and time\n");
    printf("echo <name> - to print you name or other string to console\n");
    printf("pid - to show current process ID\n");
    printf("ppid - to show parent process ID\n");
    printf("exit - to terminate and leave the shell\n");
    printf("ls <flag> - to list files of current directory\n");
    printf("      -l  - to use long listing format\n");
    printf("ps <flag> - shows the processes for current shell\n");
    printf("       w  - shows wide output of processes \n");
    printf("\n**********************************************************************\n");
    //returns 1 since it is a valid function
    return 1;
}

//signal handler that can deal with SIGINT from ctrl-c and the exit code SIGTERM
void sigHandler(int sig){
    //called when tryExit() is called when it uses raise(SIGTERM)
    if(sig == SIGTERM){
        printf("Terminated\n");
        exit(0);
    }

    if(sig == SIGINT){
        //do nothing :)
    }
}


