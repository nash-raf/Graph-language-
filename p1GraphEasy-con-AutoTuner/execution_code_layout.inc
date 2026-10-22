#ifndef SGPL_EXECUTION_CODE_LAYOUT_INC
#define SGPL_EXECUTION_CODE_LAYOUT_INC
/* A scoped execution-code placement experiment. No shared data is padded. */
#ifndef SGPL_SOURCE_GATE_ALIGNMENT
#define SGPL_SOURCE_GATE_ALIGNMENT 32
#endif
#if defined(__GNUC__) && defined(__x86_64__)
void autograph_exec_ctx_set_source_gate(sgpl_exec_ctx *,int32_t,int32_t)
  __attribute__((aligned(SGPL_SOURCE_GATE_ALIGNMENT)));
#endif
#endif
