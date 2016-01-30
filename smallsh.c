#include "smallsh.h" /* include file for example */
#include <stdlib.h>
#include <signal.h>
#include <setjmp.h>

pid_t fgPid;
struct sigaction myAction;
sigjmp_buf env;
char *prompt = "Command>"; /* prompt */


void myHandler (int sig)
{
  int status;
  int c;
  if (fgPid) {
      kill(fgPid, SIGSTOP);
      printf(" Interrupt detected. Do you wish to terminate the forgeground process %d?\n[Y/n] ", fgPid);
      fflush(stdout);
      read(0, &c, 1);
      if (c == 'n' || c == 'N') {
        printf("Resuming proces %d...\n", fgPid);
        kill(fgPid, SIGCONT);
      }
      else {
        printf("Killing process %d...\n", fgPid);
        kill(fgPid, SIGINT);
      }
  }
  else {
    printf(" Interrupt detected, but there is no foreground process to terminate.\n%s ", prompt);
    fflush(stdout);
  }
  fflush(stdin);
}

/* program buffers and work pointers */
static char inpbuf[MAXBUF], tokbuf[2*MAXBUF], *ptr = inpbuf, *tok = tokbuf;

userin(p) /* print prompt and read a line */
char *p;
{
  int c, count;
  /* initialization for later routines */
  ptr = inpbuf;
  tok = tokbuf;

  /* display prompt */
  printf("%s ", p);
  for(count = 0;;){
    if((c = getchar()) == EOF)
        return(EOF);

    if(count < MAXBUF)
      inpbuf[count++] = c;

    if(c == '\n' && count < MAXBUF){
      inpbuf[count] = '\0';
      return(count);
    }

    /* if line too long restart */
    if(c == '\n'){
      printf("smallsh: input line too long\n");
      count = 0;
      printf("%s ", p);
    }
  }
}

static char special[] = {' ', '\t', '&', ';', '\n', '\0'};

inarg(c) /* are we in an ordinary argument */
char c;
{
  char *wrk;
  for(wrk = special; *wrk != '\0'; wrk++)
    if(c == *wrk)
      return(0);

  return(1);
}

gettok(outptr) /* get token and place into tokbuf */
char **outptr;
{
  int type;

  *outptr = tok;
  /* strip white space */
  for(;*ptr == ' ' || *ptr == '\t'; ptr++)
    ;

  *tok++ = *ptr;

  switch(*ptr++){
    case '\n':
      type = EOL; break;
    case '&':
      type = AMPERSAND; break;
    case ';':
      type = SEMICOLON; break;
    default:
      type = ARG;
      while(inarg(*ptr))
        *tok++ = *ptr++;
  }

  *tok++ = '\0';
  return(type);
}

/* execute a command with optional wait */
runcommand(cline, where)
char **cline;
int where;
{
  int pid, exitstat, ret;

  if((pid = fork()) < 0){
    perror("smallsh");
    return(-1);
  }

  if(pid == 0){
    setsid();
    execvp(*cline, cline);
    perror(*cline);
    exit(127);
  }

  /* code for parent */
  /* if background process print pid and exit */
  if(where == BACKGROUND){
    printf("[Process id %d]\n", pid);
    return(0);
  }

  fgPid = pid;
  /* wait until process pid exits */
  while( waitpid(pid, &exitstat, WUNTRACED) != -1 && !WIFSTOPPED(exitstat) )
    ;
  fgPid = 0;

  return(ret == -1 ? -1 : exitstat);
}

procline() /* process input line */
{
  char *arg[MAXARG+1]; /* pointer array for runcommand */
  int toktype; /* type of token in command */
  int narg; /* numer of arguments so far */
  int type; /* FOREGROUND or BACKGROUND? */

  for(narg = 0;;){ /* loop forever */
    /* take action according to token type */
    switch(toktype = gettok(&arg[narg])){
      case ARG:
        if(narg < MAXARG)
          narg++;
        break;

      case EOL:
      case SEMICOLON:
      case AMPERSAND:
        type = (toktype == AMPERSAND) ? BACKGROUND : FOREGROUND;

        if(narg != 0){
          arg[narg] = NULL;
          runcommand(arg, type);
        }
        
        if(toktype == EOL)
          return;

        narg = 0;
        break;
    }
  }
}

main()
{
  myAction.sa_handler = myHandler;
  myAction.sa_flags = SA_RESTART; // so that getchar in userin restarts after signal handler completes
  sigaction(SIGINT, &myAction, NULL);
  sigaction(SIGQUIT, &myAction, NULL);
  while(userin(prompt) != EOF)
      procline();
}
