#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <pwd.h>
#include <syslog.h>
     
     




#define MAX_LEN 512


int main(int argc,char *argv[])
{
    char **newargv;
    int i;
    int ret;
    char uid[30];
    char scr[1024];
    char home[1024];
    struct passwd *pas;
    int tot=0,len;
    char **login = (char *[]){"docker", "login",NULL};
    char **pull = (char *[]){"docker", "pull","ubuntu",NULL};
    char command[MAX_LEN];

    command[0] = 0;
    for (i=1;i<argc;i++){
       int len=strlen(argv[i]);
       if ((tot+len+2)>MAX_LEN){
         break;
       }
       strcat(command,argv[i]);
       strcat(command," ");
       tot+=len;
    }
    setlogmask (LOG_UPTO (LOG_NOTICE));
     
    openlog ("docker", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_DAEMON);
    if (strcmp(argv[1],"login")==0){
      ret=execv("/usr/bin/docker",login);
      perror("Failed exec");
      return -1;
    }
    else if (strcmp(argv[1],"pull")==0){
      pull[2]=argv[2];
      ret=execv("/usr/bin/docker",pull);
      perror("Failed exec");
      return -1;
    }
     

    /* Before we get started, let's make sure the user isn't trying anything sneaky. */
    for (i=1;i<argc;i++){
      if (argv[i][0]=='-'){
        if ((strlen(argv[i])!=2) || (argv[i][1]!='i' && argv[i][1]!='t')) {
          syslog (LOG_WARNING, "Bad argument (uid=%d):%s", getuid(),command);
          fprintf(stderr,"Unallowed options.  Only -i or -t allowed for mydock.\n");
          return -1;
        }
      }
      else {
        /* So the argument doesn't start with a - so it must be the image name */
        break;
      }
    }
   

   syslog (LOG_NOTICE, "Start: (uid=%d) %s", getuid(),command );

    newargv=(char **)malloc(sizeof(char *)*(argc+16));
    if (newargv==NULL){
      fprintf(stderr,"Malloc failed\n");
      return -1;
    }
   
    /* Start building up the command string */ 
    newargv[0]="docker";
    newargv[1]="run";
    newargv[2]="-u";
    sprintf(uid, "%d", getuid());
    newargv[3]=uid;
    pas=getpwuid(getuid());

    /* Mount up scratch, /etc/passwd, and /etc/group */
    sprintf(scr, "/global/scratch2/sd/%s:/scratch", pas->pw_name);
    newargv[4]="-v";
    newargv[5]=scr;
    sprintf(home, "%s:%s", pas->pw_dir,pas->pw_dir);
    newargv[6]="-v";
    newargv[7]=home;
    newargv[8]="-v";
    newargv[9]="/usr/syscom/tig/docker/passwd:/etc/passwd";
    newargv[10]="-v";
    newargv[11]="/usr/syscom/tig/docker/group:/etc/group";
    newargv[12]="-w";
    newargv[13]=pas->pw_dir;
    newargv[14]="--rm";
    memcpy(&newargv[15], &argv[1], sizeof(char *) * (argc));
    //printf("%d %d\n",getuid(),geteuid());
    /*for (i=0;i<argc+4;i++){
     * printf("%d %s\n",i,newargv[i]);
     } */
    closelog ();
    ret=execv("/usr/bin/docker",newargv);
    perror("Failed exec");
}
