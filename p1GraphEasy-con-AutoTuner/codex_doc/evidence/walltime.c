#define _GNU_SOURCE
#include <sys/wait.h>
#include <sys/resource.h>
#include <time.h>
#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
static volatile sig_atomic_t child;
static void stop(int sig){if(child>0)kill(-child,SIGKILL);}
static double sec(struct timespec t){return t.tv_sec+t.tv_nsec*1e-9;}
int main(int argc,char**argv){
 if(argc<3)return 2;
 struct timespec a,b;struct rusage u;int status;
 signal(SIGALRM,stop);signal(SIGTERM,stop);signal(SIGINT,stop);
 clock_gettime(CLOCK_MONOTONIC,&a);
 child=fork();if(child==0){setsid();execvp(argv[2],argv+2);_exit(127);}
 if(child<0)return 3;
 alarm(180);while(wait4(child,&status,0,&u)<0){}alarm(0);
 clock_gettime(CLOCK_MONOTONIC,&b);
 FILE*f=fopen(argv[1],"w");if(!f)return 4;
 int code=WIFEXITED(status)?WEXITSTATUS(status):128+WTERMSIG(status);
 fprintf(f,"{\"seconds\":%.12f,\"exit_code\":%d,\"rss_kib\":%ld,\"user_seconds\":%.9f,\"system_seconds\":%.9f,\"nvcsw\":%ld,\"nivcsw\":%ld,\"minflt\":%ld,\"majflt\":%ld}\n",
 sec(b)-sec(a),code,u.ru_maxrss,u.ru_utime.tv_sec+u.ru_utime.tv_usec*1e-6,u.ru_stime.tv_sec+u.ru_stime.tv_usec*1e-6,u.ru_nvcsw,u.ru_nivcsw,u.ru_minflt,u.ru_majflt);
 fclose(f);return code;
}
