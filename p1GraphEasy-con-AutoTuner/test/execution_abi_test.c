#include "autotuner_runtime.h"
#include <assert.h>
#include <signal.h>
#include <sys/wait.h>
#include <unistd.h>
int main(void) {
  autograph_execution_abi_check_v1(SGPL_EXECUTION_ABI_VERSION,sizeof(sgpl_exec_ctx),sizeof(sgpl_runtime_op));
  for(int mismatch=0;mismatch<3;++mismatch) {
    pid_t pid=fork();assert(pid>=0);
    if(!pid) {
      autograph_execution_abi_check_v1(SGPL_EXECUTION_ABI_VERSION+(mismatch==0),sizeof(sgpl_exec_ctx)+(mismatch==1),sizeof(sgpl_runtime_op)+(mismatch==2));
      _exit(99);
    }
    int status;assert(waitpid(pid,&status,0)==pid);
    assert(WIFSIGNALED(status)&&WTERMSIG(status)==SIGABRT);
  }
  return 0;
}
