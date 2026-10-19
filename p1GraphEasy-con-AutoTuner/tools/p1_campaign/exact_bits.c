/* Validation-only link interposition. Acceptance binaries never link this. */
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
static FILE *bits;
static void marker(const char *text) {
  if(!text)return;
  if(!strcmp(text,"VALUES_BEGIN")) {
    const char *path=getenv("SGPL_BITS_OUTPUT");if(!path)return;
    if(bits)abort();bits=fopen(path,"wb");if(!bits)abort();
  } else if(!strcmp(text,"VALUES_END")&&bits) {
    if(fclose(bits))abort();bits=NULL;
  }
}
int __real_puts(const char *);
int __wrap_puts(const char *text){marker(text);return __real_puts(text);}
static void capture(const char *format,va_list args) {
  va_list copy;va_copy(copy,args);
  if(!strcmp(format,"%s\n")||!strcmp(format,"%s"))marker(va_arg(copy,const char *));
  else if(bits&&!strcmp(format,"%.13g\n")) {
    double value=va_arg(copy,double);uint64_t raw;memcpy(&raw,&value,8);
    if(fwrite(&raw,8,1,bits)!=1)abort();
  }
  va_end(copy);
}
int __wrap_printf(const char *format,...) {
  va_list args;va_start(args,format);capture(format,args);
  int result=vprintf(format,args);va_end(args);return result;
}
extern int __vfprintf_chk(FILE *,int,const char *,va_list);
int __wrap___printf_chk(int flags,const char *format,...) {
  va_list args;va_start(args,format);capture(format,args);
  int result=__vfprintf_chk(stdout,flags,format,args);va_end(args);return result;
}
__attribute__((destructor)) static void finish(void){if(bits)_exit(73);}
