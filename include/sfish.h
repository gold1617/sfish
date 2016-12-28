#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <unistd.h>
#include <limits.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include <time.h>

struct list
{
  char** args;
  int size;
  int cap;
  int outfile;
  int infile;
  int errfile;
  int isbg;
};

struct calls
{
  struct list *action;
  struct calls *next;
  struct calls *prev;
};

struct jobs
{
  char* name;
  int pgid;
  int stopped;
  int* pids;
  time_t start;
  struct jobs* next;
};


time_t start;
int numcommands;
int spid;
int shell_pid;
int bg;
int estat;
char* host;
char* prompt;
int numprocess;
int processnum;
int *pipes;
struct calls *head;
struct jobs *jobs_head;
int pgid;
sigset_t mask,pmask;
int* pids;
char *jobname;

int parse_cmd(char* input,struct list *func,int isbg);

int update_prompt(int usr,int mach,char* uc,char* mc);

void clear_inputs(struct list *inputs);

int execute_program(struct list *inputs);

struct calls *getprograms(char *input,int isbg);

void clear_programs(struct calls *head);

void createpipes(void);

void free_pipes(void);

int help(struct list *com);

int pwd(struct list *com);

void prt(struct list *com);

struct calls* parse_background(char *input);

void sigchild_handler(int sig);

void add_job(char *name);

void print_jobs(struct list* com);

char* remove_job(int gid);

struct jobs* getjob(int pid);

void bringtofront(struct list* inputs);

void resumebackground(struct list* inputs);

void disown(struct list* inputs);

void killsig(struct list* inputs);

void ctrlhandler(int sig);

int savepid(int type,int key);

int getspid(int type,int key);

int info(int type,int key);

int help_binded(int type,int key);

void printhelp(void);
