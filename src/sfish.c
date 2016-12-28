#include "sfish.h"

int main(int argc, char** argv)
{
    start = time(NULL);
    //DO NOT MODIFY THIS. If you do you will get a ZERO.
    rl_catch_signals = 0;
    //This is disable readline's default signal handlers, since you are going
    //to install your own.
    signal(SIGCHLD,sigchild_handler);
    signal(SIGTTOU, SIG_IGN);
    signal(SIGINT, ctrlhandler);

    rl_bind_key(2,savepid);
    rl_bind_key(7,getspid);
    rl_bind_key(8,help_binded);
    rl_bind_key(16,info);

    shell_pid = getpid();
    int use = 1;
    int mach = 1;
    char* usec = (char*)malloc(sizeof(char)*8);
    usec[0] = '\0';
    strcat(usec,"\033[00m");
    char*machc = (char*)malloc(sizeof(char)*8);
    machc[0] = '\0';
    strcat(machc,"\033[00m");

    if(update_prompt(use,mach,usec,machc) != 0)
    {
      fprintf(stderr,"%s\n", "Error setting up prompt");
      free(usec);
      free(machc);
      exit(EXIT_FAILURE);
    }

    char *cmd;
    char *prevdir=NULL;
    struct calls *current;
    estat = 0;

    jobs_head = NULL;

    numcommands = 0;

    while((cmd = readline(prompt)) != NULL)
    {
      jobname = strdup(cmd);
      int estatp;
      if((head = parse_background(cmd)) == NULL)
      {
        perror("sfish");
      }
      current = head;
      processnum = 0;
      createpipes();
      struct list *inputs;
      pgid = -1;

      pids = (int *)malloc(sizeof(int)*(numprocess+1));
      memset(pids,-1,sizeof(int)*(numprocess+1));

      if(current != NULL && current->action->args[0] != NULL)
        numcommands++;

      sigemptyset(&mask);
      sigaddset(&mask,SIGCHLD);
      // sigaddset(&mask,SIGTSTP);
      sigprocmask(SIG_BLOCK,&mask,&pmask);
      while(current != NULL)
      {
        estatp = 0;
        inputs = current->action;

        if(inputs->args[0] != NULL)
        {
          if(inputs->infile < 0 &&  processnum > 0)
            inputs->infile = pipes[(2*processnum)-2];

          if(processnum < numprocess-1)
          {
            if(inputs->outfile < 0)
              inputs->outfile = pipes[(2*processnum)+1];
            else
              estatp = 1;
          }

          if(strcmp(inputs->args[0],"exit")==0)
          {
            free(cmd);
            clear_programs(head);
            free(prompt);
            free(usec);
            free(machc);
            free_pipes();
            if(prevdir != NULL)
              free(prevdir);
            _exit(EXIT_SUCCESS);
          }
          else if(strcmp(inputs->args[0],"help")==0)
          {
            help(inputs);
          }
          else if(strcmp(inputs->args[0],"cd")==0)
          {
            int res;
            char* curprev = prevdir;
            if(inputs->args[1] == NULL)
            {
              if((prevdir = getcwd(NULL,PATH_MAX))==NULL)
              {
                prevdir = curprev;
                res = 1;
              }
              else
              {
                if(curprev != NULL)
                  free(curprev);
                res = chdir(getenv("HOME"));
              }
            }
            else if(strcmp(inputs->args[1],"-")==0)
            {
              estat = 0;
              if(prevdir == NULL)
              {
                res = 0;
                estat = 1;
                fprintf(stderr, "%s\n","sfish: cd: No previous directory" );
              }
              else if((prevdir = getcwd(NULL,PATH_MAX))==NULL)
              {
                res = 1;
              }
              else
              {
                res = chdir(curprev);
                if(curprev != NULL)
                  free(curprev);
              }
            }
            else
            {
              if((prevdir = getcwd(NULL,PATH_MAX))==NULL)
              {
                prevdir = curprev;
                res = 1;
              }
              else
              {
                res = chdir(inputs->args[1]);
                if(curprev != NULL)
                  free(curprev);
              }
            }
            if(res != 0)
            {
              perror("sfish: cd");
              estat = 1;
            }
          }
          else if(strcmp(inputs->args[0],"pwd")==0)
          {
            estat = pwd(inputs);
          }
          else if(strcmp(inputs->args[0],"prt")==0)
          {
            prt(inputs);
            estat = 0;
          }
          else if(strcmp(inputs->args[0],"chpmt")==0)
          {
            estat = 0;
            if(inputs->args[1] != NULL && inputs->args[2] != NULL && (strcmp(inputs->args[2],"0")==0 || strcmp(inputs->args[2],"1")==0))
            {
              if(strcmp(inputs->args[1],"user")==0)
              {
                use = *(inputs->args[2])-'0';
              }
              else if(strcmp(inputs->args[1],"machine")==0)
              {
                mach = *(inputs->args[2])-'0';
              }
              else
              {
                estat = 1;
                fprintf(stderr,"%s\n", "Illegal SETTING");
              }
            }
            else
            {
              fprintf(stderr, "%s\n", "Usage: chpmt SETTING TOGGLE");
              estat = 1;
            }
          }
          else if(strcmp(inputs->args[0],"chclr")==0)
          {
            estat = 0;
            if(inputs->args[1] != NULL && inputs->args[2] != NULL && inputs->args[3] != NULL && (strcmp(inputs->args[3],"0")==0 || strcmp(inputs->args[3],"1")==0))
            {
              char c1,c2;
              if(strcmp(inputs->args[2],"black")==0)
              {
                c1 = '3';
                c2 = '0';
              }
              else if(strcmp(inputs->args[2],"red")==0)
              {
                c1 = '3';
                c2 = '1';
              }
              else if(strcmp(inputs->args[2],"green")==0)
              {
                c1 = '3';
                c2 = '2';
              }
              else if(strcmp(inputs->args[2],"yellow")==0)
              {
                c1 = '3';
                c2 = '3';
              }
              else if(strcmp(inputs->args[2],"blue")==0)
              {
                c1 = '3';
                c2 = '4';
              }
              else if(strcmp(inputs->args[2],"magenta")==0)
              {
                c1 = '3';
                c2 = '5';
              }
              else if(strcmp(inputs->args[2],"cyan")==0)
              {
                c1 = '3';
                c2 = '6';
              }
              else if(strcmp(inputs->args[2],"white")==0)
              {
                c1 = '3';
                c2 = '7';
              }
              else
              {
                estat = 1;
                fprintf(stderr,"%s\n","Invalid Color." );
              }
              if(estat == 0 && strcmp(inputs->args[1],"user")==0)
              {
                usec[2] = c1;
                usec[3] = c2;
                if(*(inputs->args[3])=='0')
                {
                  usec[4] = 'm';
                  usec[5] = '\0';
                }
                else
                {
                  usec[4] = ';';
                  usec[5] = '1';
                  usec[6] = 'm';
                  usec[7] = '\0';
                }
              }
              else if(estat == 0 && strcmp(inputs->args[1],"machine")==0)
              {
                machc[2] = c1;
                machc[3] = c2;
                if(*(inputs->args[3])=='0')
                {
                  machc[4] = 'm';
                  machc[5] = '\0';
                }
                else
                {
                  machc[4] = ';';
                  machc[5] = '1';
                  machc[6] = 'm';
                  machc[7] = '\0';
                }
              }
            }
            else
            {
              fprintf(stderr, "%s\n", "Usage: chclr SETTING COLOR BOLD");
              estat = 1;
            }
          }
          else if(strcmp(inputs->args[0],"jobs")==0)
          {
            print_jobs(inputs);
            estat = 1;
          }
          else if(strcmp(inputs->args[0],"fg")==0)
          {
            bringtofront(inputs);
          }
          else if(strcmp(inputs->args[0],"bg")==0)
          {
            resumebackground(inputs);
          }
          else if(strcmp(inputs->args[0],"disown")==0)
          {
            disown(inputs);
          }
          else if(strcmp(inputs->args[0],"kill")==0)
          {
            killsig(inputs);
          }
          else
          {
            execute_program(inputs);
          }
        }

        //All your debug print statments should be surrounded by this #ifdef
        //block. Use the debug target in the makefile to run with these enabled.
        #ifdef DEBUG
        fprintf(stderr, "Length of command entered: %ld\n", strlen(cmd));
        #endif

        current = (estatp == 0 && estat == 0) ? current->next: NULL;

        processnum++;
        free(prompt);
        if(update_prompt(use,mach,usec,machc) != 0)
        {
          fprintf(stderr,"%s\n", "Error setting up prompt");
          free(cmd);
          clear_programs(head);
          free_pipes();
          free(usec);
          free(machc);
          exit(EXIT_FAILURE);
        }
      }

      if(bg==1)
      {
        // fprintf(stderr, "%s\n","ADDING JOB" );
        add_job(jobname);
      }
      else
      {
        free(jobname);
      }
      free(pids);
      sigprocmask(SIG_SETMASK,&pmask,NULL);

      free(cmd);
      clear_programs(head);
      free_pipes();
    }

    //Don't forget to free allocated memory, and close file descriptors.

    //WE WILL CHECK VALGRIND!

    return EXIT_SUCCESS;
}


int parse_cmd(char *input,struct list *inputs,int isbg)
{
  char* tmp;
  while((tmp = strsep(&input," \r\t")) != NULL)
  {
    if(strcmp(tmp,"")!=0)
    {

      if(tmp[0]  == '<')
      {
        if(inputs->outfile > 2)
        {
          close(inputs->infile);
        }

        if(tmp[1] != '\0')
        {
          if((inputs->infile = open(tmp+1,O_RDWR ,0777)) == -1)
          {
            return 1;
          }
        }
        else
        {
          while((tmp = strsep(&input," \r\t")) != NULL && strcmp(tmp,"") == 0)
            ;

          if(tmp == NULL)
            return 1;

          if((inputs->infile = open(tmp,O_RDWR ,0777)) == -1)
          {
            return 1;
          }
        }
      }
      else if(tmp[0] == '>')
      {
        if(inputs->outfile > 2)
        {
          close(inputs->outfile);
        }

        if(tmp[1] != '\0')
        {
          if((inputs->outfile = open(strdup(tmp+1),O_RDWR | O_CREAT,0777)) == -1)
          {
            perror("sfish");
            return 1;
          }
        }
        else
        {
          while((tmp = strsep(&input," \r\t")) != NULL && strcmp(tmp,"") == 0)
            ;
          if(tmp == NULL)
            return 1;
          if((inputs->outfile = open(strdup(tmp),O_RDWR | O_CREAT,0777)) == -1)
          {
            return 1;
          }
        }
      }
      else if(tmp[0] == '1' && tmp[1] == '>')
      {
        if(inputs->outfile > 2)
        {
          close(inputs->outfile);
        }

        if(tmp[2] != '\0')
        {
          if((inputs->outfile = open(strdup(tmp+2),O_RDWR | O_CREAT,0777)) == -1)
          {
            perror("sfish");
            return 1;
          }
        }
        else
        {
          while((tmp = strsep(&input," \r\t")) != NULL && strcmp(tmp,"") == 0)
            ;
          if(tmp == NULL)
            return 1;

          if((inputs->outfile = open(strdup(tmp),O_RDWR | O_CREAT,0777)) == -1)
          {
            return 1;
          }
        }
      }
      else if(tmp[0] == '2' && tmp[1] == '>')
      {
        if(inputs->errfile > 2)
        {
          close(inputs->errfile);
        }

        if(tmp[2] != '\0')
        {
          if((inputs->errfile = open(strdup(tmp+2),O_RDWR | O_CREAT,0777)) == -1)
          {
            perror("sfish");
            return 1;
          }
        }
        else
        {
          while((tmp = strsep(&input," \r\t")) != NULL && strcmp(tmp,"") == 0)
            ;
          if(tmp == NULL)
            return 1;

          if((inputs->errfile = open(strdup(tmp),O_RDWR | O_CREAT,0777)) == -1)
          {
            return 1;
          }
        }
      }
      else
      {
        inputs->size++;
        if(inputs->size > inputs->cap)
        {
          inputs->cap += 10;
          if((inputs->args = (char**)realloc(inputs->args,sizeof(char*)*inputs->cap)) == NULL)
          {
            return 1;
          }
        }
        inputs->args[inputs->size-1] = strdup(tmp);

        //find pipes diectly connected to args
        for(int i = 0;i < inputs->size;i++)
        {
          tmp = inputs->args[i];

          char *tempend = strchr(tmp,'>');
          tempend = (tempend == NULL || (strchr(tmp,'<') != NULL && strchr(tmp,'<')  < tempend)) ? strchr(tmp,'<'): tempend;
          char* iotemp = tmp;
          char* temp2;
          char* temp3;

          while((iotemp = strchr(iotemp,'>')) != NULL)
          {
            if(inputs->outfile>1)
              close(inputs->outfile);

            iotemp++;
            if(iotemp[0] == '\0')
            {
              while((temp2 = strsep(&input," \r\t")) != NULL && strcmp(temp2,"") == 0)
                ;
              if(temp2 == NULL)
                return 1;
            }
            else
            {
              temp2 = strdup(iotemp);
              temp3 = strchr(temp2,'>');
              // printf("%s\n",temp3 );
              temp3 = (temp3 == NULL || (strchr(temp2,'<') != NULL && strchr(temp2,'<')  < temp3)) ? strchr(temp2,'<'): temp3;

              if(temp3 != NULL)
                temp3[0] = '\0';
            }

            if((inputs->outfile = open(strdup(temp2),O_RDWR | O_CREAT,0777)) == -1)
            {
              return 1;
            }
          }

          iotemp = tmp;
          while((iotemp = strchr(iotemp,'<')) != NULL)
          {
            if(inputs->infile>0)
              close(inputs->infile);

            iotemp++;
            if(iotemp[0] == '\0')
            {
              while((temp2 = strsep(&input," \r\t")) != NULL && strcmp(temp2,"") == 0)
                ;
              if(temp2 == NULL)
                return 1;
            }
            else
            {
              temp2 = strdup(iotemp);
              temp3 = strchr(temp2,'>');
              // printf("%s\n",temp3 );
              temp3 = (temp3 == NULL || (strchr(temp2,'<') != NULL && strchr(temp2,'<')  < temp3)) ? strchr(temp2,'<'): temp3;

              if(temp3 != NULL)
                temp3[0] = '\0';
            }

            if((inputs->infile = open(strdup(temp2),O_RDWR,0777)) == -1)
            {
              return 1;
            }
          }

          if(tempend != NULL)
            tempend[0] = '\0';
        }
      }
    }
  }
  return 0;
}

int update_prompt(int usr,int mach,char* uc,char* mc)
{
  prompt = (char*)malloc(sizeof(char*)*(14+LOGIN_NAME_MAX+HOST_NAME_MAX+PATH_MAX));
  prompt[0]='\0';
  if(strcat(prompt,"sfish")==NULL)
    return 1;

  host = (char *)malloc(sizeof(char)*(HOST_NAME_MAX+1));

  if(usr == 1)
  {
    if(strcat(prompt,"-") == NULL || strcat(prompt,uc) == NULL || strcat(prompt,getenv("USER")) == NULL || strcat(prompt,"\033[00m") == NULL)
    {
      free(host);
      return 1;
    }
    if(mach == 1)
    {
      if(gethostname(host,HOST_NAME_MAX) != 0 || strcat(prompt,"@") == NULL || strcat(prompt,mc) == NULL ||  strcat(prompt,host) == NULL || strcat(prompt,"\033[00m") == NULL)
      {
        free(host);
        return 1;
      }
    }
  }
  else if(mach == 1)
  {
    if(strcat(prompt,"-") == NULL || gethostname(host,HOST_NAME_MAX) != 0 ||  strcat(prompt,mc) == NULL ||  strcat(prompt,host) == NULL || strcat(prompt,"\033[00m") == NULL)
    {
      free(host);
      return 1;
    }
  }


  if(strcat(prompt,":[") == NULL)
  {
    free(host);
    return 1;
  }

  char *tmp = getcwd(NULL,PATH_MAX);
  if(strstr(tmp,getenv("HOME"))==tmp)
  {
    if(strcat(prompt,"~") == NULL || strcat(prompt,tmp+strlen(getenv("HOME"))) == NULL)
    {
      free(host);
      free(tmp);
      return 1;
    }
  }
  else
  {
    if(strcat(prompt,tmp) == NULL)
    {
      free(tmp);
      free(host);
      return 1;
    }
  }
  if(strcat(prompt,"]> ") == NULL)
  {
    free(tmp);
    free(host);
    return 1;
  }
  free(tmp);
  free(host);
  return 0;
}

void clear_inputs(struct list *inputs)
{
  char** args = inputs->args;
  for(int i = 0;i<inputs->size;i++)
  {
    free(args[i]);
  }

  free(inputs->args);
  inputs->size = 0;
  inputs->cap = 0;
}

int execute_program(struct list *inputs)
{
  char* path = NULL;
  char* fullpath = (char *)malloc(sizeof(char)*PATH_MAX);
  fullpath[0] = '\0';

  if(inputs->args[0][0] == '.' || inputs->args[0][0] == '/')
  {
    strcat(fullpath,inputs->args[0]);
    if(access(fullpath,F_OK) != 0)
    {
      free(fullpath);
      return 1;
    }
  }
  else
  {
    path = strdup(getenv("PATH"));
    char* modp = path;
    char* cur;
    while ((cur = strsep(&modp,":")) != NULL)
    {
      strcat(fullpath,cur);
      strcat(fullpath,"/");
      strcat(fullpath,inputs->args[0]);
      if(access(fullpath,F_OK) == 0)
      {
        break;
      }
      else
      {
        fullpath[0] = '\0';
      }
    }
  }
  if(fullpath[0] == '\0')
  {
    free(fullpath);
    free(path);
    printf("%s: %s\n",inputs->args[0], "command not found" );
    return -1;
  }
  else
  {
    pid_t pid = fork();

    if(pid == 0)
    {
      sigprocmask(SIG_SETMASK,&pmask,NULL);

      if(pgid == -1)
        pgid = pid;
      if(setpgid(pid,pgid)==-1)
      {
        ;
      }

      char** arglist = (char **)malloc(sizeof(char *)*(inputs->size+1));
      for(int i = 0;i < inputs->size;i++)
      {
        arglist[i] = strdup(inputs->args[i]);
      }
      arglist[inputs->size] = '\0';

      if(inputs->infile >= 0)
      {
        dup2(inputs->infile,0);
      }

      if(inputs->outfile >= 0)
      {
        dup2(inputs->outfile,1);
      }

      if(inputs->errfile >= 0)
      {
        dup2(inputs->errfile,2);
      }
      execvp(fullpath,arglist);
    }
    else
    {
      if(pgid == -1)
        pgid = pid;
      if(setpgid(pid,pgid)==-1)
      {
        ;
      }
      if(inputs->isbg == 0)
      {
        tcsetpgrp(0,pgid);
        waitpid(-pgid,&estat,WUNTRACED);
        if(WIFSTOPPED(estat)!=0)
        {
          pids[processnum] = pid;
          kill(pid,SIGTSTP);
          add_job(jobname);
          getjob(pid)->stopped=1;
          printf("[%d] Stopped %s\n",pgid,jobname);
        }
        else
        {
          estat = WEXITSTATUS(estat);
        }
        tcsetpgrp(0,getpgid(0));
      }
    }
    pids[processnum] = pid;
    if(inputs->infile > 0)
      close(inputs->infile);

    if(inputs->outfile > 0)
      close(inputs->outfile);

    if(inputs->errfile > 0)
      close(inputs->errfile);
    free(fullpath);
    if(path != NULL)
      free(path);
    return estat;
  }
}

struct calls *getprograms(char *input, int isbg)
{
  struct calls *head = NULL;
  struct calls *prev = NULL;
  struct calls *cur = NULL;
  char* tmp = NULL;

  numprocess = 0;

  while((tmp = strsep(&input,"|")) != NULL)
  {
    if(tmp != NULL)
    {
      cur = (struct calls*)malloc(sizeof(struct calls));
      cur->action = (struct list*)malloc(sizeof(struct list));
      cur->action->size = 0;
      cur->action->cap = 10;
      cur->action->infile = -1;
      cur->action->outfile = -1;
      cur->action->errfile = -1;
      cur->action->isbg = isbg;
      cur->action->args = (char**)malloc(sizeof(char*)*10);
      memset(cur->action->args,'\0',sizeof(char*)*10);
      char *line = strdup(tmp);
      if(parse_cmd(line,cur->action,isbg) != 0)
      {
        clear_programs(head);
        return NULL;
      }
      free(line);
      cur->prev = prev;
      if(prev != NULL)
        cur->prev->next = cur;
      cur->next = NULL;
      prev = cur;
      if(head == NULL)
        head = cur;
      numprocess++;
    }
  }

  return head;
}

void clear_programs(struct calls* head)
{
  struct calls* call;
  while(head != NULL)
  {
    clear_inputs(head->action);
    free(head->action);
    call = head;
    head = head->next;
    free(call);
  }
}

void createpipes()
{
  pipes = (int *)malloc(sizeof(int)*2*numprocess);

  for(int i = 0;i < numprocess;i++)
  {
    pipe(pipes+(2*i));
  }
}

void free_pipes()
{
  for(int i = 0;i < 2*numprocess;i++)
  {
    close(pipes[i]);
  }
  free(pipes);
}

int help(struct list *com)
{
  int pid = fork();

  if(pid == 0)
  {
    if(com->infile >= 0)
    {
      dup2(com->infile,0);
    }

    if(com->outfile >= 0)
    {
      dup2(com->outfile,1);
    }

    if(com->errfile >= 0)
    {
      dup2(com->errfile,2);
    }
    printhelp();
    _exit(0);
  }
  else if(com->isbg != 0)
  {
    waitpid(pid,&estat,WUNTRACED);
    if(WIFSTOPPED(estat)!=0)
    {
      add_job(jobname);
      getjob(pid)->stopped=1;
      printf("[%d] Stopped %s\n",pgid,jobname);
    }
  }
  if(com->infile > 2)
    close(com->infile);
  if(com->outfile > 2)
    close(com->outfile);
  if(com->errfile > 2)
    close(com->errfile);
  return 0;
}

int pwd(struct list *com)
{
  int pid = fork();

  if(pid == 0)
  {
    if(com->infile >= 0)
    {
      dup2(com->infile,0);
    }

    if(com->outfile >= 0)
    {
      dup2(com->outfile,1);
    }

    if(com->errfile >= 0)
    {
      dup2(com->errfile,2);
    }
    char* dir;
    if((dir = getcwd(NULL,PATH_MAX))==NULL)
    {
      perror("sfish: pwd");
      estat = 1;
    }
    else
    {
      printf("%s\n",dir );
      estat = 0;
    }
    free(dir);
    _exit(estat);
  }
  else if(com->isbg != 0)
  {
    waitpid(pid,&estat,WUNTRACED);
    if(WIFSTOPPED(estat)!=0)
    {
      kill(pid,SIGTSTP);
      add_job(jobname);
      getjob(pid)->stopped=1;
      printf("[%d] Stopped %s\n",pgid,jobname);
    }
  }
  if(com->infile > 2)
    close(com->infile);
  if(com->outfile > 2)
    close(com->outfile);
  if(com->errfile > 2)
    close(com->errfile);
  return estat;
}

void prt(struct list *com)
{
  int pid = fork();

  if(pid == 0)
  {
    if(com->infile >= 0)
    {
      dup2(com->infile,0);
    }

    if(com->outfile >= 0)
    {
      dup2(com->outfile,1);
    }

    if(com->errfile >= 0)
    {
      dup2(com->errfile,2);
    }

    printf("%d\n", estat);
    _exit(0);
  }
  else if(com->isbg == 0)
  {
    waitpid(pid,&estat,WUNTRACED);
    if(WIFSTOPPED(estat)!=0)
    {
      kill(pid,SIGTSTP);
      add_job(jobname);
      getjob(pid)->stopped=1;
      printf("[%d] Stopped %s\n",pgid,jobname);
    }
    else
    {
      estat = WEXITSTATUS(estat);
    }
  }
  if(com->infile > 2)
    close(com->infile);
  if(com->outfile > 2)
    close(com->outfile);
  if(com->errfile > 2)
    close(com->errfile);
}

struct calls* parse_background(char* input)
{
  char* ptr;

  if((ptr = strchr(input,'&')) != NULL)
  {
    bg = 1;
    ptr[0] = '\0';
    return getprograms(input,1);
  }
  else
  {
    bg = 0;
    return getprograms(input,0);
  }
}


void sigchild_handler(int sig)
{
  int pid;
  int stat;
  char* name;
  struct jobs* job;
  int i,runningproc;

  sigfillset(&mask);
  sigprocmask(SIG_BLOCK,&mask,&pmask);
  int olderr = errno;

  while((pid = waitpid(-1,&stat,WNOHANG)) > 0)
  {
    if((job = getjob(pid))!=NULL)
    {
      i = 0;
      runningproc = 0;
      while(job->pids[i] != -1)
      {
        if(kill(job->pids[i],0)==0 || errno != ESRCH)
        {
          runningproc = 1;
        }
        i++;
      }
      if(runningproc == 0)
      {
        name = remove_job(job->pgid);
        printf("[%d]  Done.\t%s\n",pid,name);
      }
    }
  }

  errno = olderr;
  sigprocmask(SIG_SETMASK,&pmask,NULL);
}

void add_job(char *name)
{
  sigfillset(&mask);
  sigprocmask(SIG_BLOCK,&mask,&pmask);

  char *end = strchr(name,'&');
  if(end != NULL)
    *end = '\0';
  struct jobs *job = (struct jobs *)malloc(sizeof(struct jobs));

  job->start = time(NULL) - start;
  job->name = strdup(name);
  job->pgid = pgid;
  job->stopped = 0;
  job->pids = (int *)malloc((sizeof(int *)*numprocess)+1);
  memcpy(job->pids,pids,(sizeof(int *)*numprocess)+1);

  job->next = NULL;
  if(jobs_head == NULL)
  {
    jobs_head = job;
  }
  else
  {
    struct jobs *cur = jobs_head;
    while(cur->next != NULL)
    {
      cur = cur->next;
    }
    cur->next = job;
  }

  sigprocmask(SIG_SETMASK,&pmask,NULL);
}

void print_jobs(struct list * com)
{
  sigfillset(&mask);
  sigprocmask(SIG_BLOCK,&mask,&pmask);

  int pid = fork();

  if(pid == 0)
  {
      if(com->infile >= 0)
      {
        dup2(com->infile,0);
      }

      if(com->outfile >= 0)
      {
        dup2(com->outfile,1);
      }

      if(com->errfile >= 0)
      {
        dup2(com->errfile,2);
      }

    struct jobs* cur = jobs_head;
    int i = 1;
    if(cur == NULL)
    {
      printf("%s\n", "No current jobs.");
    }
    else
    {
      while(cur != NULL)
      {
        if(cur->stopped == 0)
        {
          printf("[%d]  %s\t%d\t%s\n",i,"Running",cur->pgid,cur->name);

        }
        else
        {
          printf("[%d]  %s\t%d\t%s\n",i,"Stopped",cur->pgid,cur->name);
        }
        i++;
        cur = cur->next;
      }
    }
    _exit(0);
  }
  else if(com->isbg == 0)
  {
    waitpid(pid,&estat,WUNTRACED);
    if(WIFSTOPPED(estat)!=0)
    {
      kill(pid,SIGTSTP);
      add_job(jobname);
      getjob(pid)->stopped=1;
      printf("[%d] Stopped %s\n",pgid,jobname);
    }
    else
    {
      estat = WEXITSTATUS(estat);
    }
  }
  if(com->infile > 2)
    close(com->infile);
  if(com->outfile > 2)
    close(com->outfile);
  if(com->errfile > 2)
    close(com->errfile);

  sigprocmask(SIG_SETMASK,&pmask,NULL);
}

char* remove_job(int gid)
{
  struct jobs *cur = jobs_head;

  struct jobs *prev = NULL;
  char* name = NULL;
  while(cur != NULL)
  {
    if(cur->pgid == gid)
    {
      name = strdup(cur->name);
      if(cur == jobs_head)
      {
        jobs_head = cur->next;
      }
      else
      {
        prev->next = cur->next;
      }
      free(cur->name);
      free(cur);
      return name;
    }
    prev = cur;
    cur = cur->next;
  }
  return name;
}

struct jobs* getjob(int pid)
{
  int i;
  struct jobs* cur = jobs_head;
  while(cur != NULL)
  {
    i = 0;
    while(cur->pids[i] != -1)
    {
      if(cur->pids[i]==pid)
        return cur;
      i++;
    }
    cur = cur->next;
  }
  return NULL;
}

void bringtofront(struct list * inputs)
{
  if(inputs->size < 2)
  {
    fprintf(stderr, "%s\n","Missing PID|JID" );
    estat = 1;
  }
  else
  {
    char* meh;
    int id,i;
    struct jobs* job;

    if(inputs->args[1][0] == '%')
    {
      id = strtol(&inputs->args[1][1],&meh,10);
      job = jobs_head;
      i = 1;

      while(job != NULL && i < id)
      {
        i++;
        job = job->next;
      }
      if(job != NULL)
      {
        job->stopped = 0;
        id = job->pgid;
        kill(-id,SIGCONT);
        tcsetpgrp(0,id);
        waitpid(-id,&estat,WUNTRACED);
        if(WIFSTOPPED(estat)!=0)
        {
          job->stopped = 1;
          printf("[%d] Stopped %s\n",id,job->name);
        }
        else
        {
          estat = WEXITSTATUS(estat);
        }
        tcsetpgrp(0,getpgid(0));
      }
    }
    else
    {
      id = strtol(inputs->args[1],&meh,10);
      if((job = getjob(id)) != NULL)
      {
        job->stopped = 0;
        id = job->pgid;
        kill(-id,SIGCONT);
        tcsetpgrp(0,id);
        waitpid(-id,&estat,WUNTRACED);
        if(WIFSTOPPED(estat)!=0)
        {
          job->stopped = 1;
          printf("[%d] Stopped %s\n",id,job->name);
        }
        else
        {
          estat = WEXITSTATUS(estat);
        }
        tcsetpgrp(0,getpgid(0));
      }
    }
    if(job != NULL && job->stopped == 0)
    {
      sigfillset(&mask);
      sigprocmask(SIG_BLOCK,&mask,&pmask);

      remove_job(id);

      sigprocmask(SIG_SETMASK,&pmask,NULL);
    }
    else
      estat = 1;
  }
}

void resumebackground(struct list* inputs)
{
  if(inputs->size < 2)
  {
    fprintf(stderr, "%s\n","Missing PID|JID" );
    estat = 1;
  }
  else
  {
    char* meh;
    int id,i;
    struct jobs* job;

    if(inputs->args[1][0] == '%')
    {
      id = strtol(&inputs->args[1][1],&meh,10);
      job = jobs_head;
      i = 1;

      while(job != NULL && i < id)
      {
        i++;
        job = job->next;
      }
      if(job != NULL)
      {
        id = job->pgid;
        job->stopped = 0;
        kill(-id,SIGCONT);
        estat = 0;
      }
      else
        estat = 1;
    }
    else
    {
      id = strtol(inputs->args[1],&meh,10);
      if((job = getjob(id)) != NULL)
      {
        id = job->pgid;
        job->stopped = 0;
        kill(-id,SIGCONT);
        estat = 0;
      }
      else
        estat = 1;
    }
  }
}

void disown(struct list* inputs)
{
  if(inputs->size < 2)
  {
    while(jobs_head != NULL)
    {
      sigfillset(&mask);
      sigprocmask(SIG_BLOCK,&mask,&pmask);

      remove_job(jobs_head->pgid);

      sigprocmask(SIG_SETMASK,&pmask,NULL);
    }
    estat = 0;
  }
  else
  {
    char* meh;
    int id,i;
    struct jobs* job;

    if(inputs->args[1][0] == '%')
    {
      id = strtol(&inputs->args[1][1],&meh,10);
      job = jobs_head;
      i = 1;

      while(job != NULL && i < id)
      {
        i++;
        job = job->next;
      }
    }
    else
    {
      id = strtol(inputs->args[1],&meh,10);
      job = getjob(id);
    }
    if(job != NULL)
    {
      sigfillset(&mask);
      sigprocmask(SIG_BLOCK,&mask,&pmask);

      remove_job(job->pgid);

      sigprocmask(SIG_SETMASK,&pmask,NULL);
      estat = 0;
    }
    else
      estat = 1;
  }
}

void killsig(struct list* inputs)
{
  int sig = SIGTERM;
  int id,i;
  char* meh;
  struct jobs* job;

  if(inputs->size < 2)
  {
    fprintf(stderr, "%s\n","Missing PID|JID" );
    estat = 1;
  }
  else if(inputs->size == 2)
  {
    if(inputs->args[1][0] == '%')
    {
      id = strtol(&inputs->args[1][1],&meh,10);
      job = jobs_head;
      i = 1;

      while(job != NULL && i < id)
      {
        i++;
        job = job->next;
      }
    }
    else
    {
      id = strtol(inputs->args[1],&meh,10);
      job = getjob(id);
    }
  }
  else
  {
    if(inputs->args[2][0] == '%')
    {
      id = strtol(&inputs->args[2][1],&meh,10);
      job = jobs_head;
      i = 1;

      while(job != NULL && i < id)
      {
        i++;
        job = job->next;
      }
    }
    else
    {
      id = strtol(inputs->args[2],&meh,10);
      job = getjob(id);
    }
    sig = strtol(inputs->args[1],&meh,10);
  }
  if(job != NULL && sig > 0 && sig <= 31)
  {
    sigfillset(&mask);
    sigprocmask(SIG_BLOCK,&mask,&pmask);
    kill(-1*job->pgid,sig);
    if(sig == SIGCONT)
      job->stopped = 0;
    else if(sig >= SIGSTOP && sig <= SIGTTOU)
      job->stopped = 1;
    printf("%d stopped by signal %d\n",job->pgid,sig );
    sigprocmask(SIG_SETMASK,&pmask,NULL);
    estat = 0;
  }
  else
  {
    fprintf(stderr, "%s\n", "sfish: unable to kill");
    estat = 1;
  }
}

void ctrlhandler(int sig)
{
  if(getpid() != shell_pid)
  {
    kill(-getpgid(0),SIGINT);
    tcsetpgrp(0,getpgid(shell_pid));
  }
}

int savepid(int type,int key)
{
  if(jobs_head == NULL)
  {
    spid = -1;
  }
  else
  {
    spid = jobs_head->pgid;
  }
  rl_on_new_line();
  return 0;
}

int getspid(int type,int key)
{
  struct jobs* job;
  if(spid == -1 || (job = getjob(spid)) == NULL)
  {
    printf("%s\n", "SPID not set.");
  }
  else
  {
    printf("SPID: %d terminated.\n", spid);
    kill(-job->pgid,SIGTERM);
  }
  rl_on_new_line();
  return 0;
}

int help_binded(int k,int c)
{
  printhelp();
  rl_on_new_line();
  return 0;
}

int info(int k,int a)
{
  printf("\n----Info----\nhelp\nprt\n----CTRL----\ncd\nchclr\nchpmt\npwd\nexit\n----Job Control----bg\nfg\ndisown\njobs\n----Number of Jobs Run----\n%d\n----Process Table----\n",numcommands);
  printf("PGID%4sPID%5sTIME%4sCMD\n","","","");

  struct jobs* job = jobs_head;

  while(job != NULL)
  {
    printf("%-8d%-8d%02ld:%02ld  %s\n",job->pgid,job->pgid,job->start/60,job->start%60,job->name);

    job = job->next;
  }

  rl_on_new_line();
  return 0;
}

void printhelp()
{
  printf("help\nexit\ncd [dir]\npwd\nchpmt user|machine 0|1\nchclr user|machine red|blue|yellow... 0|1\njobs\nfg PID|JID\nbg PID|JID\n");
  printf("kill [signal] PID|JID\ndisown [PID|JID]\n");

}
