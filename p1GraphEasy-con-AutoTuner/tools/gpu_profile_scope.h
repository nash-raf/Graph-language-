#ifndef SGPL_DIAGNOSTIC_PROFILE_H
#define SGPL_DIAGNOSTIC_PROFILE_H
#include <stdint.h>
enum {GP_HOST,GP_PREP,GP_FINISH,GP_PLAN,GP_CACHE,GP_UPLOAD,GP_DOWNLOAD,
 GP_COHERENCE,GP_CLEANUP,GP_EXECUTE,GP_POLL,GP_INIT,GP_MODULE,GP_ALLOC,
 GP_FREE,GP_H2D,GP_D2H,GP_STATUS,GP_STREAM,GP_CONTEXT,GP_QUERY,GP_SYNC,
 GP_LAUNCH,GP_INSTRUMENT,GP_CUT,GP_RECORDS,GP_SNAPSHOT,GP_CONTEXT_PREP,
 GP_FRONTIER,GP_BEGIN,GP_PUBLISH,GP_COUNT};
typedef struct {int previous,active;} gp_scope;
gp_scope gp_enter(int bucket);
void gp_leave(gp_scope *scope);
void gp_bytes_add(int bucket,uint64_t bytes);
#define GP_SCOPE(B) gp_scope gp_scope_local __attribute__((cleanup(gp_leave)))=gp_enter(B)
#define GP_CALL(B,EXPR) ({gp_scope gp_call_scope __attribute__((cleanup(gp_leave)))=gp_enter(B);__auto_type gp_result=(EXPR);gp_result;})
#endif
