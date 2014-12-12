#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <pwd.h>
#include <syslog.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <wait.h>

#define DOCK "/usr/bin/docker"


#define MAX_LEN 512
char name[30];
int quit=0;

void sig_handler(int signum);

int dkill(){
 struct sockaddr_un address;
 int  socket_fd, nbytes;
 char buffer[1024];

 socket_fd = socket(PF_UNIX, SOCK_STREAM, 0);
 if(socket_fd < 0)
 {
  printf("socket() failed\n");
  return 1;
 }

 /* start with a clean address structure */
 memset(&address, 0, sizeof(struct sockaddr_un));

 address.sun_family = AF_UNIX;
 snprintf(address.sun_path, 1024, "/var/run/docker.sock");

 if(connect(socket_fd, (struct sockaddr *) &address, sizeof(struct sockaddr_un)) != 0) {
  printf("connect() failed\n");
  return 1;
 }

 nbytes = snprintf(buffer, 1024, "POST /v1.12/containers/%s/kill?signal=KILL HTTP/1.1\r\nHost: /var/run/docker.sock\r\nUser-Agent: mydock/0.11.1-dev\r\nContent-Length: 0\r\nContent-Type: plain/text\r\nAccept-Encoding: gzip\r\n\r\n",name);
 write(socket_fd, buffer, nbytes);

 nbytes = read(socket_fd, buffer, 1024);
 buffer[nbytes] = 0;

 //printf("MESSAGE FROM SERVER: %s\n", buffer);

 close(socket_fd);
}

// Define the function to be called when ctrl-c (SIGINT) signal is sent to process
void sig_handler(int signum)
{
   printf("Caught signal %d. Killing %s\n",signum,name);
   quit=1;
   //exit(signum);
}
 
int main(int argc,char *argv[],char *envp[])
{
    char **newargv;
    int i,start;
    int ret;
    char uid[30];
    char scr[1024];
    char home[1024];
    struct passwd *pas;
    int tot=0,len;
    char **login = (char *[]){"docker", "login",NULL};
    char **pull = (char *[]){"docker", "pull","ubuntu",NULL};
    char command[MAX_LEN];
    pid_t pid;
    pid_t wpid;
    int pidstat;
    char **env, **sant;

    for (env=envp,sant=envp;*env!=NULL;env++){
      if (strstr(*env,"PATH=") || strstr(*env,"HOME=")){
        *sant=*env;
        sant++;
      }
    }
    *sant=NULL;

    if (argc<2){
       fprintf(stderr,"Usage: %s <run|login|pull>\n");
       return 1;
    }
    command[0] = 0;
    for (i=1;i<argc;i++){
       int len=strlen(argv[i]);
       if ((tot+len+2)>MAX_LEN){
         break;
       }
       strcat(command,argv[i]);
       strcat(command," ");
       tot+=(len+1);
    }
    start=1;
    setlogmask (LOG_UPTO (LOG_NOTICE));
    openlog ("docker", LOG_CONS | LOG_PID | LOG_NDELAY, LOG_DAEMON);
    if (strcmp(argv[1],"login")==0){
      ret=execve(DOCK,login,envp);
      perror("Failed exec");
      return -1;
    }
    else if (strcmp(argv[1],"pull")==0){
      pull[2]=argv[2];
      ret=execve(DOCK,pull,envp);
      perror("Failed exec");
      return -1;
    }
    else if (strcmp(argv[1],"run")==0){
      start=2;
    }
     

    /* Before we get started, let's make sure the user isn't trying anything sneaky. */
    for (i=start;i<argc;i++){
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

    newargv=(char **)malloc(sizeof(char *)*(argc+22));
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
    newargv[15]="--net=host";
    newargv[16]="--name";
    sprintf(name, "%d-%d", getuid(),getpid());
    newargv[17]=name;
    newargv[18]="-e";
    newargv[19]="HOME";
    memcpy(&newargv[20], &argv[start], sizeof(char *) * (argc));
    //printf("%d %d\n",getuid(),geteuid());
    /*for (i=0;i<argc+4;i++){
     * printf("%d %s\n",i,newargv[i]);
     } */
    closelog ();
    pid=fork();
    if (pid==0){
      ret=execve(DOCK,newargv,envp);
      perror("Failed exec");
    }
    else if (pid<0){
      perror("Fork failed");
    }
    else {
      signal(SIGINT, sig_handler);
      signal(SIGTERM, sig_handler);
      signal(SIGKILL, sig_handler);
      while (! quit){
        wpid=waitpid(pid,&pidstat,WNOHANG);
        if (wpid && WIFEXITED(pidstat)) {
            quit=1;
        }
        sleep(1);
      }
     // Do cleanup
      dkill();
    }
}
