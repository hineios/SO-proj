#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(int argc, const char * argv[])
{
 int result, status;
 int type=0;
 
 if (argc>1)
   type=atoi(argv[1]);
 else
  type=1;	
printf("Type is %d \n",type);
 result = fork();
 if (result == 0) {
  if (type==0 || type==1) 
    execlp("./cache-lock-w.sh",0,NULL);
  else
    execlp("./cache-lock-r.sh",0,NULL);
 } else {
  printf("Pai..:%d\n", getpid());
  if (type==1 || type==2) 
    execlp("./cache-lock-w.sh",0,NULL);
  else
    execlp("./cache-lock-r.sh",0,NULL);
  wait(&status);
 }
  printf("Fim.\n");
return 0;
}
