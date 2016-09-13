/*
 *  This is a simple shell program from
 *  rik0.altervista.org/snippetss/csimpleshell.html
 *  It's been modified a bit and comments were added.
 *
 *  It doesn't allow misdirection, e.g., <, >, >>, or |
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#define BUFFER_SIZE 80
#define ARR_SIZE 80
#define NUMPIPES 10

// #define DEBUG 1  /* In case you want debug messages */

void parse_args(char *buffer, char** args, 
                size_t args_size, size_t *nargs,
		int pipes[NUMPIPES])
{
/* 
 * size_t data type is defined in the 1999 ISO C standard (C99).
 * It is used to represent the sizes of objects. size_t is the
 * preferred way to declare arguments or variables that hold the
 * size of an object.
 */
    char *buf_args[args_size]; /* You need C99.  Note that args_size
                                  is normally a constant. */
    char **cp;  /* This is used as a pointer into the string array */
    char *wbuf;  /* String variable that has the command line */
    size_t i, j; 

    int p = 0;
    
    wbuf=buffer;
    buf_args[0]=buffer; 
    args[0] =buffer;
/*
 * Now 'wbuf' is parsed into the string array 'buf_args'
 *
 * The for-loop uses a string.h function
 *   char *strsep(char **stringp, const char *delim);
 *
 *   Description:  
 *   If *stringp = NULL then it returns NULL and does
 *   nothing else.  Otherwise the function finds the first token in
 *   the string *stringp, where tokens are delimited by symbols
 *   in the string 'delim'.  
 *
 *   In the example below, **stringp is &wbu, and 
 *   the delim = ' ', '\n', and '\t'.  So there are three possible 
 *   delimiters. 
 *
 *   So in the string " Aloha World\n", the spaces and "\n" are
 *   delimiters.  Thus, there are three delimiters.  The tokens
 *   are what's between the delimiters.  So the first token is
 *   "", which is nothing because a space is the first delimiter.
 *   The second token is "Aloha", and the third token is "World".
 *   
 *   The function will scan a character string starting from
 *   *stringp, search for the first delimiter.  It replaces
 *   the delimiter with '\0', and *stringp is updated to point
 *   past the token.  In case no delimiter was found, the
 *   token is taken to be the entire string *stringp, and *stringp
 *   is made NULL.   Strsep returns a pointer to the token. 
 *
 *   Example:  Suppose *stringp -> " Aloha World\n"
 *
 *   The first time strsep is called, the string is "\0Aloha World\n",
 *   and the pointer value returned = 0.  Note the token is nothing.
 *
 *   The second time it is called, the string is "\0Aloha\0World\n",
 *   and the pointer value returned = 1  Note that 'Aloha' is a token.
 *
 *   The third time it is called, the string is '\0Aloha\0World\0', 
 *   and the pointer value returned is 7.  Note that 'World' is a token.
 *
 *   The fourth time it is called, it returns NULL.
 *
 *   The for-loop, goes through buffer starting at the beginning.
 *   wbuf is updated to point to the next token, and cp is
 *   updated to point to the current token, which terminated by '\0'.
 *   Note that pointers to tokens are stored in array buf_args through cp.
 *   The loop stops if there are no more tokens or exceeded the
 *   array buf_args.
 */   
    /* cp is a pointer to buff_args */ 
    for(cp=buf_args; (*cp=strsep(&wbuf, " \n\t")) != NULL ;){
        if ((*cp != '\0') && (++cp >= &buf_args[args_size]))
            break; 
    }

/* 
 * Copy 'buf_args' into 'args'
 */    
    for (j=i=0; buf_args[i]!=NULL; i++){ 
        if(strlen(buf_args[i])>0)  /* Store only non-empty tokens */
            args[j++]=buf_args[i];
	if(!strcmp(args[j-1], "|")){
		pipes[p] = j - 1;
		p++;
	}
    }
    
    *nargs=j;
    args[j]=NULL;
}


int main(int argc, char *argv[], char *envp[]){
    char buffer[BUFFER_SIZE];
    char *args[ARR_SIZE];

    int *ret_status;
    size_t nargs;
    pid_t pid;
    
    while(1){
    int pipePos[NUMPIPES];
	for(int i = 0; i < NUMPIPES; i++)
		pipePos[i] = -1;


        printf("ee468>> "); /* Prompt */
        fgets(buffer, BUFFER_SIZE, stdin); /* Read in command line */
              /* Parse the command line into args */
        parse_args(buffer, args, ARR_SIZE, &nargs, pipePos); 
 
        if (nargs==0) continue; /* Nothing entered so prompt again */
        if (!strcmp(args[0], "exit" )) exit(0);       
  
	printf("PIPEPOS[0]: %d\n", pipePos[0]);
	if(pipePos[0] == -1){	// if there are no pipes
		pid = fork();
		if(pid){
			pid = wait(ret_status);
		}
		else{
			if(execvp(args[0], args)){
				puts(strerror(errno));
				exit(127);
			}
		}
	}
 		
    char * execArgs[ARR_SIZE];
    char * execArgs2[ARR_SIZE];

    for(int i = 0; i < nargs; i++){
	int j = 0;
	while(i < nargs && strcmp(args[i], "|")){
		execArgs[j] = args[i];
		i++;
		j++;
	}
	execArgs[j] = NULL;
	i++;
	j = 0;
	while(i < nargs){
		execArgs2[j] = args[i];
		printf("%s\n", execArgs2[j]);
		j++;
		i++;
	} 
	execArgs2[j] = NULL;
    }

    int fd[2];
    pipe(fd);
    int pid = fork();
    if (pid == 0) {
        close(fd[1]);
        int ret = dup2(fd[0],0);
        if (ret < 0) perror("dup2 - 0");
        execvp(execArgs2[0], execArgs2);
    }

    
    int pid2 = fork();
    if (pid2 == 0) {

        int ret = dup2(fd[1],1);
        if (ret < 0) perror("dup2 - 1");
        execvp(execArgs[0], execArgs);
    }

    close(fd[0]);
    close(fd[1]);
    int status;
    //pid = wait(ret_status);
    waitpid(pid, &status, 0);
    //printf("Done!\n");
//    return 0;
    
}
}
